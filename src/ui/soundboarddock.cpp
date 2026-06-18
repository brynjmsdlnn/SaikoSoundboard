#include "soundboarddock.h"
#include "hotkeydialog.h"
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

SoundboardDock::SoundboardDock(SoundboardManager *manager, ActionManager *actionManager, QWidget *parent)
    : QDockWidget("Soundboard", parent)
    , m_manager(manager)
    , m_actionManager(actionManager)
{
    auto *mainWidget = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(mainWidget);

    // Top Bar containing Add button and Routing controls
    auto *topBarLayout = new QHBoxLayout();

    auto *addBtn = new QPushButton("Add New Player", this);
    addBtn->setFixedHeight(36);
    topBarLayout->addWidget(addBtn);

    // Global Routing Control Group
    auto *routingGroup = new QGroupBox("Soundboard Routing", this);
    auto *routingLayout = new QHBoxLayout(routingGroup);
    routingLayout->setContentsMargins(8, 4, 8, 4);
    routingLayout->setSpacing(10);

    auto *micCb = new QCheckBox("Mic Output", this);
    micCb->setChecked(m_manager->isMicOutputEnabled());
    
    auto *localCb = new QCheckBox("Local Monitoring", this);
    localCb->setChecked(m_manager->isLocalMonitoringEnabled());

    routingLayout->addWidget(micCb);
    routingLayout->addWidget(localCb);

    // Device selection
    auto *micCombo = new QComboBox(this);
    auto *localCombo = new QComboBox(this);
    micCombo->setToolTip("Device for Mic Output Path");
    localCombo->setToolTip("Device for Local Monitoring Path");

    const auto outputs = QMediaDevices::audioOutputs();
    micCombo->addItem("Default Mic Device", "");
    localCombo->addItem("Default Local Device", "");
    for (const auto &device : outputs) {
        micCombo->addItem(device.description(), device.description());
        localCombo->addItem(device.description(), device.description());
    }

    // Set selected devices
    QString savedMic = m_manager->settings()->micOutputDevice();
    QString savedLocal = m_manager->settings()->localMonitorDevice();
    int micIdx = micCombo->findData(savedMic);
    if (micIdx >= 0) micCombo->setCurrentIndex(micIdx);
    int localIdx = localCombo->findData(savedLocal);
    if (localIdx >= 0) localCombo->setCurrentIndex(localIdx);

    routingLayout->addWidget(new QLabel("Mic Dev:", this));
    routingLayout->addWidget(micCombo);
    routingLayout->addWidget(new QLabel("Local Dev:", this));
    routingLayout->addWidget(localCombo);

    topBarLayout->addWidget(routingGroup);
    topBarLayout->addStretch();
    mainLayout->addLayout(topBarLayout);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollContent = new QWidget(this);
    m_scrollLayout = new QHBoxLayout(m_scrollContent);
    m_scrollLayout->setAlignment(Qt::AlignLeft);
    m_scrollArea->setWidget(m_scrollContent);
    mainLayout->addWidget(m_scrollArea);

    setWidget(mainWidget);
    setMinimumHeight(390); // Adjusted height for embedded waveforms & controls
    setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);

    connect(addBtn, &QPushButton::clicked, this, &SoundboardDock::onAddPlayer);
    connect(m_manager, &SoundboardManager::slotsChanged, this, &SoundboardDock::refresh);
    connect(m_manager, &SoundboardManager::waveformGenerated, this, &SoundboardDock::onWaveformGenerated);
    connect(m_manager, &SoundboardManager::playerPositionChanged, this, &SoundboardDock::onPlayerPositionChanged);

    connect(micCb, &QCheckBox::toggled, this, [this](bool checked) {
        m_manager->setMicOutputEnabled(checked);
    });
    connect(localCb, &QCheckBox::toggled, this, [this](bool checked) {
        m_manager->setLocalMonitoringEnabled(checked);
    });
    connect(micCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, micCombo](int index) {
        m_manager->setMicOutputDevice(micCombo->itemData(index).toString());
    });
    connect(localCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, localCombo](int index) {
        m_manager->setLocalMonitorDevice(localCombo->itemData(index).toString());
    });

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
        auto *renameBtn = new QPushButton("...", card);
        renameBtn->setFixedWidth(25);
        topLayout->addWidget(nameLabel, 1);
        topLayout->addWidget(renameBtn);
        cardLayout->addLayout(topLayout);

        // Menu for options button
        auto *menu = new QMenu(renameBtn);
        auto *renameAction = menu->addAction("Rename");
        auto *hotkeyAction = menu->addAction("Hotkey Bindings");
        
        bool isTemp = !slot.filePath.isEmpty() && slot.filePath.startsWith(QDir::tempPath());
        QAction *makePermanentAction = nullptr;
        if (isTemp) {
            makePermanentAction = menu->addAction("Make Permanent");
            connect(makePermanentAction, &QAction::triggered, this, [this, id = slot.id]() { onMakePermanent(id); });
        }

        connect(renameAction, &QAction::triggered, this, [this, id = slot.id]() { onRenamePlayer(id); });
        connect(hotkeyAction, &QAction::triggered, this, [this, id = slot.id]() { onHotkeySetup(id); });
        renameBtn->setMenu(menu);
        renameBtn->setStyleSheet("QPushButton::menu-indicator { image: none; }");

        // File Path & Temp Badge
        QString fileName = slot.filePath.isEmpty() ? "No file" : QFileInfo(slot.filePath).fileName();
        if (isTemp) {
            fileName = "[TEMP] " + fileName;
        }
        auto *fileLabel = new QLabel(fileName, card);
        fileLabel->setStyleSheet(isTemp ? "color: #ff9800; font-size: 10px; font-weight: bold;" : "color: gray; font-size: 10px;");
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
        routingCombo->addItem("Both", static_cast<int>(OutputRouting::Both));
        routingCombo->addItem("Mic Only", static_cast<int>(OutputRouting::MicOnly));
        routingCombo->addItem("Local Only", static_cast<int>(OutputRouting::LocalOnly));

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
        auto *removeBtn = new QPushButton("X", card);
        removeBtn->setFixedWidth(25);
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
    if (m_actionManager) {
        m_actionManager->dispatch(Action::createMakePermanent(id));
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
