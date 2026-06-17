#ifndef SOURCESDOCK_H
#define SOURCESDOCK_H

#include <QDockWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include "audiosource.h"

class SourcesDock : public QDockWidget
{
    Q_OBJECT
public:
    explicit SourcesDock(QWidget *parent = nullptr);

    void updateSourceList(const QList<AudioSource>& sources);

signals:
    void sourceAdded(const AudioSource& source);
    void sourceRemoved(const QString& sourceId);

private slots:
    void onAddSourceClicked();
    void onRemoveSourceClicked();

private:
    QListWidget *m_listWidget;
    QPushButton *m_addBtn;
    QPushButton *m_removeBtn;
};

#endif // SOURCESDOCK_H
