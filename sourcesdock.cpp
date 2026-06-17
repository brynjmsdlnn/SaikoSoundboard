#include "sourcesdock.h"
#include <QDialog>
#include <QListWidget>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QFileIconProvider>
#include <QFileInfo>

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#endif

SourcesDock::SourcesDock(QWidget *parent) : QDockWidget("Audio Sources", parent)
{
    QWidget *content = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(content);

    m_listWidget = new QListWidget(content);
    m_addBtn = new QPushButton("Add Source", content);
    m_removeBtn = new QPushButton("Remove Source", content);

    layout->addWidget(m_listWidget);
    layout->addWidget(m_addBtn);
    layout->addWidget(m_removeBtn);

    setWidget(content);

    connect(m_addBtn, &QPushButton::clicked, this, &SourcesDock::onAddSourceClicked);
    connect(m_removeBtn, &QPushButton::clicked, this, &SourcesDock::onRemoveSourceClicked);
}

void SourcesDock::updateSourceList(const QList<AudioSource>& sources)
{
    m_listWidget->clear();
    QFileIconProvider iconProvider;
    for (const auto& src : sources) {
        QListWidgetItem *item = new QListWidgetItem(QString("%1 (%2)").arg(src.name, src.executableName));
        item->setData(Qt::UserRole, src.id);
        
        if (!src.executablePath.isEmpty()) {
            item->setIcon(iconProvider.icon(QFileInfo(src.executablePath)));
        }
        
        m_listWidget->addItem(item);
    }
}

void SourcesDock::onAddSourceClicked()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Select Process");
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    QListWidget *processList = new QListWidget(&dialog);
    
#ifdef Q_OS_WIN
    DWORD processes[1024], cbNeeded, cProcesses;
    QFileIconProvider iconProvider;
    if (EnumProcesses(processes, sizeof(processes), &cbNeeded)) {
        cProcesses = cbNeeded / sizeof(DWORD);
        for (unsigned int i = 0; i < cProcesses; i++) {
            if (processes[i] != 0) {
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);
                if (hProcess) {
                    WCHAR szPath[MAX_PATH];
                    if (GetModuleFileNameExW(hProcess, NULL, szPath, MAX_PATH)) {
                        QString fullPath = QString::fromWCharArray(szPath);
                        QFileInfo fileInfo(fullPath);
                        QString name = fileInfo.fileName();
                        
                        if (!name.isEmpty()) {
                            QListWidgetItem *item = new QListWidgetItem(name, processList);
                            item->setData(Qt::UserRole, name);
                            item->setData(Qt::UserRole + 1, fullPath); // Store full path
                            item->setIcon(iconProvider.icon(fileInfo));
                        }
                    }
                    CloseHandle(hProcess);
                }
            }
        }
    }
#endif
    processList->sortItems();
    
    // De-duplicate names
    for (int i = 0; i < processList->count(); ++i) {
        QString current = processList->item(i)->text();
        for (int j = i + 1; j < processList->count(); ++j) {
            if (processList->item(j)->text() == current) {
                delete processList->takeItem(j);
                --j;
            }
        }
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addWidget(processList);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted && processList->currentItem()) {
        QString exeName = processList->currentItem()->data(Qt::UserRole).toString();
        QString fullPath = processList->currentItem()->data(Qt::UserRole + 1).toString();
        AudioSource src;
        src.executableName = exeName;
        src.executablePath = fullPath;
        src.name = exeName.section('.', 0, 0); // Remove .exe
        emit sourceAdded(src);
    }
}

void SourcesDock::onRemoveSourceClicked()
{
    QListWidgetItem *item = m_listWidget->currentItem();
    if (item) {
        emit sourceRemoved(item->data(Qt::UserRole).toString());
    }
}
