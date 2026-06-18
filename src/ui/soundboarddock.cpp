#include "soundboarddock.h"
#include "hotkeydialog.h"
#include "managers/actionmanager.h"
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
    setMinimumHeight(355); // Adjusted height for added combobox
    setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);

    connect(addBtn, &QPushButton::clicked, this, &SoundboardDock::onAddPlayer);
    connect(m_manager, &SoundboardManager::slotsChanged, this, &SoundboardDock::refresh);

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
        auto *renameBtn = new QPushButton("...", card); // Compact rename
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
        renameBtn->setStyleSheet("QPushButton::menu-indicator { image: none; }"); // Hide arrow

        // File Path & Temp Badge
        QString fileName = slot.filePath.isEmpty() ? "No file" : QFileInfo(slot.filePath).fileName();
        if (isTemp) {
            fileName = "[TEMP] " + fileName;
        }
        auto *fileLabel = new QLabel(fileName, card);
        fileLabel->setStyleSheet(isTemp ? "color: #ff9800; font-size: 10px; font-weight: bold;" : "color: gray; font-size: 10px;");
        fileLabel->setWordWrap(true);
        cardLayout->addWidget(fileLabel);

        // Volume Slider (Vertical for DJ feel)
        auto *volLayout = new QHBoxLayout();
        auto *volumeSlider = new QSlider(Qt::Vertical, card);
        volumeSlider->setRange(0, 100);
        volumeSlider->setValue(static_cast<int>(slot.volume * 100));
        volumeSlider->setFixedHeight(60);
        
        // Play/Stop buttons stacked next to slider
        auto *btnStack = new QVBoxLayout();
        auto *playBtn = new QPushButton("PLAY", card);
        playBtn->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold; height: 30px;");
        auto *stopBtn = new QPushButton("STOP", card);
        stopBtn->setStyleSheet("background-color: #f44336; color: white; font-weight: bold; height: 20px;");
        btnStack->addWidget(playBtn);
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

        // Connections
        connect(playBtn, &QPushButton::clicked, this, [this, id = slot.id]() { onPlayPlayer(id); });
        connect(stopBtn, &QPushButton::clicked, this, [this, id = slot.id]() { onStopPlayer(id); });
        connect(removeBtn, &QPushButton::clicked, this, [this, id = slot.id]() { onRemovePlayer(id); });
        connect(volumeSlider, &QSlider::valueChanged, this, [this, id = slot.id](int val) { onVolumeChanged(id, val); });
        connect(routingCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, id = slot.id](int index) {
            m_manager->setPlayerRouting(id, static_cast<OutputRouting>(index));
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
