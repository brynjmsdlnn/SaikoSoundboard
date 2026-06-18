#include "hotkeydialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QKeyEvent>

HotkeyDialog::HotkeyDialog(const QString &playKey, const QString &assignKey, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Hotkey Bindings");
    setMinimumWidth(300);

    auto *layout = new QVBoxLayout(this);

    // Play Hotkey
    layout->addWidget(new QLabel("Play Player Hotkey:", this));
    m_playEdit = new KeySequenceEdit(this);
    m_playEdit->setText(playKey);
    auto *playHLayout = new QHBoxLayout();
    playHLayout->addWidget(m_playEdit);
    auto *clearPlayBtn = new QPushButton("Clear", this);
    playHLayout->addWidget(clearPlayBtn);
    layout->addLayout(playHLayout);

    // Assign Hotkey
    layout->addWidget(new QLabel("Assign Replay to Player Hotkey:", this));
    m_assignEdit = new KeySequenceEdit(this);
    m_assignEdit->setText(assignKey);
    auto *assignHLayout = new QHBoxLayout();
    assignHLayout->addWidget(m_assignEdit);
    auto *clearAssignBtn = new QPushButton("Clear", this);
    assignHLayout->addWidget(clearAssignBtn);
    layout->addLayout(assignHLayout);

    // Buttons
    auto *btnLayout = new QHBoxLayout();
    auto *okBtn = new QPushButton("Save", this);
    auto *cancelBtn = new QPushButton("Cancel", this);
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    connect(clearPlayBtn, &QPushButton::clicked, m_playEdit, &QLineEdit::clear);
    connect(clearAssignBtn, &QPushButton::clicked, m_assignEdit, &QLineEdit::clear);
}
