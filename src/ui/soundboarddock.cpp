#include "soundboarddock.h"
#include "hotkeydialog.h"
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

SoundboardDock::SoundboardDock(SoundboardManager *manager, QWidget *parent)
    : QDockWidget("Soundboard", parent)
    , m_manager(manager)
{
    auto *mainWidget = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(mainWidget);

    auto *addBtn = new QPushButton("Add New Player", this);
    mainLayout->addWidget(addBtn);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollContent = new QWidget(this);
    m_scrollLayout = new QHBoxLayout(m_scrollContent);
    m_scrollLayout->setAlignment(Qt::AlignLeft);
    m_scrollArea->setWidget(m_scrollContent);
    mainLayout->addWidget(m_scrollArea);

    setWidget(mainWidget);
    setMinimumHeight(300); // Ensure DJ decks have enough room
    setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);

    connect(addBtn, &QPushButton::clicked, this, &SoundboardDock::onAddPlayer);
    connect(m_manager, &SoundboardManager::slotsChanged, this, &SoundboardDock::refresh);

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
        connect(renameAction, &QAction::triggered, this, [this, id = slot.id]() { onRenamePlayer(id); });
        connect(hotkeyAction, &QAction::triggered, this, [this, id = slot.id]() { onHotkeySetup(id); });
        renameBtn->setMenu(menu);
        renameBtn->setStyleSheet("QPushButton::menu-indicator { image: none; }"); // Hide arrow

        // File Path
        QString fileName = slot.filePath.isEmpty() ? "No file" : QFileInfo(slot.filePath).fileName();
        auto *fileLabel = new QLabel(fileName, card);
        fileLabel->setStyleSheet("color: gray; font-size: 10px;");
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

        // Config buttons at bottom
        auto *cfgLayout = new QHBoxLayout();
        auto *assignBtn = new QPushButton("Set", card);
        auto *removeBtn = new QPushButton("X", card);
        removeBtn->setFixedWidth(25);
        cfgLayout->addWidget(assignBtn, 1);
        cfgLayout->addWidget(removeBtn);
        cardLayout->addLayout(cfgLayout);

        m_scrollLayout->addWidget(card);

        // Connections
        connect(playBtn, &QPushButton::clicked, this, [this, id = slot.id]() { onPlayPlayer(id); });
        connect(stopBtn, &QPushButton::clicked, this, [this, id = slot.id]() { onStopPlayer(id); });
        connect(assignBtn, &QPushButton::clicked, this, [this, id = slot.id]() { onAssignFile(id); });
        connect(removeBtn, &QPushButton::clicked, this, [this, id = slot.id]() { onRemovePlayer(id); });
        connect(volumeSlider, &QSlider::valueChanged, this, [this, id = slot.id](int val) { onVolumeChanged(id, val); });
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
