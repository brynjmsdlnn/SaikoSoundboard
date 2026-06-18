#include "soundboarddock.h"
#include "hotkeydialog.h"
#include "routingdialog.h"
#include "managers/actionmanager.h"
#include "ui/waveformwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <QSlider>
#include <QFileDialog>
#include <QInputDialog>
#include <QFileInfo>
#include <QFrame>
#include <QMenu>
#include <QAction>
#include <QCheckBox>
#include <QGroupBox>
#include <QComboBox>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QMessageBox>

SoundboardDock::SoundboardDock(SoundboardManager *manager, ActionManager *actionManager, QWidget *parent)
    : QDockWidget("Soundboard", parent)
    , m_manager(manager)
    , m_actionManager(actionManager)
{
    auto *mainWidget = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(mainWidget);

    // Top Bar containing Add button and Settings button
    auto *topBarLayout = new QHBoxLayout();

    auto *addBtn = new QPushButton("➕", this);
    addBtn->setFixedSize(36, 36);
    addBtn->setToolTip("Add New Player");
    
    auto *routingBtn = new QPushButton("⚙️", this);
    routingBtn->setFixedSize(36, 36);
    routingBtn->setToolTip("Audio Routing & Settings");

    // Make icon fonts clean and clear
    QFont iconFont = addBtn->font();
    iconFont.setPointSize(12);
    addBtn->setFont(iconFont);
    routingBtn->setFont(iconFont);

    topBarLayout->addStretch();
    topBarLayout->addWidget(addBtn);
    topBarLayout->addWidget(routingBtn);

    mainLayout->addLayout(topBarLayout);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollContent = new QWidget(this);
    m_scrollLayout = new QHBoxLayout(m_scrollContent);
    m_scrollLayout->setAlignment(Qt::AlignLeft);
    m_scrollArea->setWidget(m_scrollContent);
    mainLayout->addWidget(m_scrollArea);

    setWidget(mainWidget);
    setMinimumHeight(500); // Slightly adjusted height
    setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);

    connect(addBtn, &QPushButton::clicked, this, &SoundboardDock::onAddPlayer);
    connect(routingBtn, &QPushButton::clicked, this, [this]() {
        RoutingDialog dlg(m_manager, this);
        dlg.exec();
    });
    connect(m_manager, &SoundboardManager::slotsChanged, this, &SoundboardDock::refresh);
    connect(m_manager, &SoundboardManager::waveformGenerated, this, &SoundboardDock::onWaveformGenerated);
    connect(m_manager, &SoundboardManager::playerPositionChanged, this, &SoundboardDock::onPlayerPositionChanged);

    refresh();
}

void SoundboardDock::refresh()
{
    clearLayout();
    m_waveformWidgets.clear();

    const auto playerSlots = m_manager->getSlots();
    for (const auto &slot : playerSlots) {
        auto *card = new QFrame(m_scrollContent);
        card->setFrameShape(QFrame::StyledPanel);
        card->setFixedWidth(180); // Compact deck width
        auto *cardLayout = new QVBoxLayout(card);

        // Name and Rename
        auto *topLayout = new QHBoxLayout();
        auto *nameLabel = new QLabel(QString("<b>%1</b>").arg(slot.name), card);
        nameLabel->setWordWrap(true);
        auto *renameBtn = new QPushButton("⚙️", card);
        renameBtn->setFixedSize(25, 25);
        renameBtn->setToolTip("Options");
        renameBtn->setStyleSheet("QPushButton { padding: 0px; text-align: center; }");
        QFont menuFont = renameBtn->font();
        menuFont.setPointSize(10);
        renameBtn->setFont(menuFont);
        topLayout->addWidget(nameLabel, 1);
        topLayout->addWidget(renameBtn);
        cardLayout->addLayout(topLayout);

        // Menu for options button
        auto *menu = new QMenu(renameBtn);
        auto *renameAction = menu->addAction("Rename");
        auto *hotkeyAction = menu->addAction("Hotkey Bindings");
        
        bool isTemp = !slot.filePath.isEmpty() && slot.filePath.startsWith(QDir::tempPath());

        connect(renameAction, &QAction::triggered, this, [this, id = slot.id]() { onRenamePlayer(id); });
        connect(hotkeyAction, &QAction::triggered, this, [this, id = slot.id]() { onHotkeySetup(id); });
        connect(renameBtn, &QPushButton::clicked, this, [renameBtn, menu]() {
            menu->exec(renameBtn->mapToGlobal(QPoint(0, renameBtn->height())));
        });

        // File Path & Temp Badge
        QString fileName = slot.filePath.isEmpty() ? "No file" : QFileInfo(slot.filePath).fileName();
        if (isTemp) {
            fileName = "[TEMP] " + fileName;
        }
        auto *fileLabel = new ClickableLabel(fileName, card);
        if (isTemp) {
            fileLabel->setStyleSheet("color: #ff9800; font-size: 10px; font-weight: bold; text-decoration: underline;");
            fileLabel->setCursor(Qt::PointingHandCursor);
            fileLabel->setToolTip("Temporary recording. Click to save permanently.");
            connect(fileLabel, &ClickableLabel::clicked, this, [this, id = slot.id]() { onMakePermanent(id); });
        } else {
            fileLabel->setStyleSheet("color: gray; font-size: 10px;");
            fileLabel->setCursor(Qt::ArrowCursor);
            fileLabel->setToolTip("");
        }
        fileLabel->setWordWrap(true);
        cardLayout->addWidget(fileLabel);

        // Waveform Visualizer
        auto *waveform = new WaveformWidget(card);
        waveform->setFixedHeight(45);
        cardLayout->addWidget(waveform);
        m_waveformWidgets.insert(slot.id, waveform);

        // Selected Clip Inputs
        auto *clipGroup = new QGroupBox("Selected Clip", card);
        auto *clipLayout = new QFormLayout(clipGroup);
        clipLayout->setContentsMargins(4, 4, 4, 4);
        clipLayout->setSpacing(4);
        clipGroup->setStyleSheet("QGroupBox { font-size: 9px; font-weight: bold; }");

        auto *startSpin = new QDoubleSpinBox(card);
        startSpin->setDecimals(1);
        startSpin->setSingleStep(0.1);
        startSpin->setSuffix("s");
        startSpin->setRange(0.0, 3600.0);
        startSpin->setValue(slot.startTimeMs / 1000.0);
        startSpin->setStyleSheet("font-size: 9px;");

        auto *endSpin = new QDoubleSpinBox(card);
        endSpin->setDecimals(1);
        endSpin->setSingleStep(0.1);
        endSpin->setSuffix("s");
        endSpin->setRange(0.0, 3600.0);
        endSpin->setStyleSheet("font-size: 9px;");

        // Set duration boundaries and default values
        double durSec = 0.0;
        WaveformData wdata = m_manager->getWaveformData(slot.id);
        if (wdata.isValid && wdata.durationMs > 0) {
            durSec = wdata.durationMs / 1000.0;
        }
        
        startSpin->setMaximum(durSec > 0.0 ? durSec : 3600.0);
        endSpin->setMaximum(durSec > 0.0 ? durSec : 3600.0);

        if (slot.endTimeMs == -1) {
            endSpin->setValue(durSec > 0.0 ? durSec : 0.0);
        } else {
            endSpin->setValue(slot.endTimeMs / 1000.0);
        }

        clipLayout->addRow("Start:", startSpin);
        clipLayout->addRow("End:", endSpin);
        cardLayout->addWidget(clipGroup);

        // Volume Slider (Vertical for DJ feel) and Playback controls
        auto *volLayout = new QHBoxLayout();
        auto *volumeSlider = new QSlider(Qt::Vertical, card);
        volumeSlider->setRange(0, 100);
        volumeSlider->setValue(static_cast<int>(slot.volume * 100));
        volumeSlider->setFixedHeight(65);
        
        auto *btnStack = new QVBoxLayout();
        auto *playRow = new QHBoxLayout();
        
        auto *playBtn = new QPushButton("PLAY", card);
        playBtn->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold; height: 26px; font-size: 10px;");
        
        auto *previewBtn = new QPushButton("PREV", card);
        previewBtn->setStyleSheet("background-color: #00bcd4; color: white; font-weight: bold; height: 26px; font-size: 10px;");
        previewBtn->setToolTip("Preview locally only");

        playRow->addWidget(playBtn, 1);
        playRow->addWidget(previewBtn, 1);
        btnStack->addLayout(playRow);

        auto *stopBtn = new QPushButton("STOP", card);
        stopBtn->setStyleSheet("background-color: #f44336; color: white; font-weight: bold; height: 20px; font-size: 10px;");
        btnStack->addWidget(stopBtn);

        volLayout->addWidget(volumeSlider);
        volLayout->addLayout(btnStack);
        cardLayout->addLayout(volLayout);

        // Routing Selector per card
        auto *routingLayout = new QHBoxLayout();
        auto *routingLabel = new QLabel("Route:", card);
        routingLabel->setStyleSheet("font-size: 10px;");
        auto *routingCombo = new QComboBox(card);
        routingCombo->setStyleSheet("font-size: 10px; height: 18px;");
        routingCombo->addItem("Broadcast & Monitor", static_cast<int>(OutputRouting::Both));
        routingCombo->addItem("Broadcast Only", static_cast<int>(OutputRouting::MicOnly));
        routingCombo->addItem("Monitor Only", static_cast<int>(OutputRouting::LocalOnly));

        int rIndex = routingCombo->findData(static_cast<int>(slot.outputRouting));
        if (rIndex >= 0) {
            routingCombo->setCurrentIndex(rIndex);
        }
        routingLayout->addWidget(routingLabel);
        routingLayout->addWidget(routingCombo, 1);
        cardLayout->addLayout(routingLayout);

        // Config buttons at bottom
        auto *cfgLayout = new QHBoxLayout();
        auto *assignBtn = new QPushButton("Set", card);
        auto *removeBtn = new QPushButton("🗑️", card);
        removeBtn->setFixedSize(25, 25);
        removeBtn->setToolTip("Delete Player");
        removeBtn->setStyleSheet("QPushButton { color: #FF4D4D; border: 1px solid #FF4D4D; border-radius: 4px; background-color: rgba(255, 77, 77, 0.1); } QPushButton:hover { background-color: rgba(255, 77, 77, 0.2); }");
        QFont rmFont = removeBtn->font();
        rmFont.setPointSize(10);
        removeBtn->setFont(rmFont);
        cfgLayout->addWidget(assignBtn, 1);
        cfgLayout->addWidget(removeBtn);
        cardLayout->addLayout(cfgLayout);

        // Preserve checkbox toggle under configuration buttons
        auto *preserveCb = new QCheckBox("Preserve Sound", card);
        preserveCb->setStyleSheet("font-size: 10px;");
        preserveCb->setToolTip("Prevent over-writing if a sound is already loaded into this deck");
        cardLayout->addWidget(preserveCb);

        // Menu for assigning file or replay
        auto *assignMenu = new QMenu(assignBtn);
        auto *fromFileAct = assignMenu->addAction("From File...");
        auto *fromReplayAct = assignMenu->addAction("From Replay Buffer");
        connect(fromFileAct, &QAction::triggered, this, [this, id = slot.id]() { onAssignFile(id); });
        connect(fromReplayAct, &QAction::triggered, this, [this, id = slot.id, preserveCb]() { onAssignReplay(id, preserveCb->isChecked()); });
        assignBtn->setMenu(assignMenu);

        m_scrollLayout->addWidget(card);

        // Populate cached waveform if it exists
        if (!slot.filePath.isEmpty()) {
            if (wdata.isValid) {
                waveform->setWaveformData(wdata);
                waveform->setClipRange(slot.startTimeMs, slot.endTimeMs);
            } else {
                m_manager->loadWaveformData(slot.id, slot.filePath);
            }
        }

        // Connections
        connect(playBtn, &QPushButton::clicked, this, [this, id = slot.id]() { onPlayPlayer(id); });
        connect(previewBtn, &QPushButton::clicked, this, [this, id = slot.id]() { onPlayPreview(id); });
        connect(stopBtn, &QPushButton::clicked, this, [this, id = slot.id]() { onStopPlayer(id); });
        connect(removeBtn, &QPushButton::clicked, this, [this, id = slot.id]() { onRemovePlayer(id); });
        connect(volumeSlider, &QSlider::valueChanged, this, [this, id = slot.id](int val) { onVolumeChanged(id, val); });
        connect(routingCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, id = slot.id](int index) {
            m_manager->setPlayerRouting(id, static_cast<OutputRouting>(index));
        });

        // Double SpinBox input connections
        auto updateRangeFunc = [this, id = slot.id, startSpin, endSpin, waveform]() {
            qint64 startMs = static_cast<qint64>(startSpin->value() * 1000.0);
            qint64 endMs = static_cast<qint64>(endSpin->value() * 1000.0);
            
            if (startMs > endMs - 50) {
                if (this->sender() == startSpin) {
                    startSpin->blockSignals(true);
                    startSpin->setValue(std::max(0.0, (endMs - 50) / 1000.0));
                    startSpin->blockSignals(false);
                    startMs = std::max(0LL, endMs - 50);
                } else {
                    endSpin->blockSignals(true);
                    endSpin->setValue((startMs + 50) / 1000.0);
                    endSpin->blockSignals(false);
                    endMs = startMs + 50;
                }
            }

            m_manager->setPlayerClipRange(id, startMs, endMs);
            waveform->setClipRange(startMs, endMs);
        };

        connect(startSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, updateRangeFunc);
        connect(endSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, updateRangeFunc);

        // Interactive dragging connections
        connect(waveform, &WaveformWidget::trimRangeChanged, this, [startSpin, endSpin, waveform](qint64 startMs, qint64 endMs) {
            startSpin->blockSignals(true);
            endSpin->blockSignals(true);
            startSpin->setValue(startMs / 1000.0);
            qint64 maxVal = static_cast<qint64>(endSpin->maximum() * 1000.0);
            qint64 currentEnd = endMs == -1 ? maxVal : endMs;
            endSpin->setValue(currentEnd / 1000.0);
            startSpin->blockSignals(false);
            endSpin->blockSignals(false);
            waveform->setClipRange(startMs, endMs);
        });

        connect(waveform, &WaveformWidget::trimRangeCommit, this, [this, id = slot.id](qint64 startMs, qint64 endMs) {
            m_manager->setPlayerClipRange(id, startMs, endMs);
        });
    }
}

void SoundboardDock::onAssignReplay(const QString &id, bool preserveExisting)
{
    if (m_actionManager) {
        m_actionManager->dispatch(Action::createAssignReplay(id, preserveExisting));
    }
}

void SoundboardDock::onHotkeySetup(const QString &id)
{
    SoundPlayerSlot *slot = m_manager->getSlot(id);
    if (!slot) return;

    HotkeyDialog dlg(slot->playHotkey, slot->assignHotkey, this);
    if (dlg.exec() == QDialog::Accepted) {
        m_manager->setHotkeys(id, dlg.playHotkey(), dlg.assignHotkey());
    }
}

void SoundboardDock::onAddPlayer()
{
    m_manager->addPlayer();
}

void SoundboardDock::onRemovePlayer(const QString &id)
{
    SoundPlayerSlot *slot = m_manager->getSlot(id);
    if (!slot) return;

    bool hasFile = !slot->filePath.isEmpty();
    bool hasHotkey = !slot->playHotkey.isEmpty() || !slot->assignHotkey.isEmpty();

    if (hasFile || hasHotkey) {
        QString message = "Are you sure you want to delete this player?";
        if (hasFile && hasHotkey) {
            message += "\n\nIt has an assigned audio file and hotkey bindings.";
        } else if (hasFile) {
            message += "\n\nIt has an assigned audio file.";
        } else if (hasHotkey) {
            message += "\n\nIt has hotkey bindings.";
        }

        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            "Confirm Delete",
            message,
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );
        if (reply != QMessageBox::Yes) {
            return;
        }
    }

    m_manager->removePlayer(id);
}

void SoundboardDock::onAssignFile(const QString &id)
{
    QString path = QFileDialog::getOpenFileName(this, "Select Audio File", "", "Audio Files (*.wav *.mp3 *.ogg)");
    if (!path.isEmpty()) {
        m_manager->assignAudioFile(id, path);
    }
}

void SoundboardDock::onPlayPlayer(const QString &id)
{
    m_manager->playPlayer(id);
}

void SoundboardDock::onPlayPreview(const QString &id)
{
    m_manager->playPlayerPreview(id);
}

void SoundboardDock::onStopPlayer(const QString &id)
{
    m_manager->stopPlayer(id);
}

void SoundboardDock::onVolumeChanged(const QString &id, int volume)
{
    m_manager->setVolume(id, static_cast<float>(volume) / 100.0f);
}

void SoundboardDock::onRenamePlayer(const QString &id)
{
    SoundPlayerSlot *slot = m_manager->getSlot(id);
    if (!slot) return;

    bool ok;
    QString newName = QInputDialog::getText(this, "Rename Player", "Name:", QLineEdit::Normal, slot->name, &ok);
    if (ok && !newName.isEmpty()) {
        m_manager->renamePlayer(id, newName);
    }
}

void SoundboardDock::onMakePermanent(const QString &id)
{
    SoundPlayerSlot *slot = m_manager->getSlot(id);
    if (!slot || slot->filePath.isEmpty()) return;

    QFileInfo fileInfo(slot->filePath);
    QString originalName = fileInfo.fileName();

    bool ok;
    QString newName = QInputDialog::getText(
        this,
        "Make File Permanent",
        "Enter permanent file name:",
        QLineEdit::Normal,
        originalName,
        &ok
    );
    if (!ok || newName.trimmed().isEmpty()) {
        return;
    }

    // Ensure it ends with the original extension
    QString suffix = fileInfo.suffix();
    if (!newName.endsWith("." + suffix, Qt::CaseInsensitive)) {
        newName += "." + suffix;
    }

    if (m_actionManager) {
        m_actionManager->dispatch(Action::createMakePermanent(id, newName));
    }
}

void SoundboardDock::onWaveformGenerated(const QString &playerId, const WaveformData &data)
{
    (void)data;
    // Refresh the UI to rebuild cards with correct SpinBox ranges and loaded waveforms
    SoundPlayerSlot *slot = m_manager->getSlot(playerId);
    if (slot) {
        refresh();
    }
}

void SoundboardDock::onPlayerPositionChanged(const QString &playerId, qint64 position)
{
    if (WaveformWidget *widget = m_waveformWidgets.value(playerId, nullptr)) {
        widget->setPlayPosition(position);
    }
}

void SoundboardDock::clearLayout()
{
    QLayoutItem *item;
    while ((item = m_scrollLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
}
