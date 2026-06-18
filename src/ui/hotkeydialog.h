#ifndef HOTKEYDIALOG_H
#define HOTKEYDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QKeySequence>
#include <QKeyEvent>

class HotkeyDialog : public QDialog
{
    Q_OBJECT
public:
    explicit HotkeyDialog(const QString &playKey, const QString &assignKey, QWidget *parent = nullptr);

    QString playHotkey() const { return m_playEdit->text(); }
    QString assignHotkey() const { return m_assignEdit->text(); }

private:
    QLineEdit *m_playEdit;
    QLineEdit *m_assignEdit;
};

// A small helper to capture key sequences in a QLineEdit
class KeySequenceEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit KeySequenceEdit(QWidget *parent = nullptr) : QLineEdit(parent) {
        setPlaceholderText("Press keys to bind...");
    }

protected:
    void keyPressEvent(QKeyEvent *event) override {
        int key = event->key();
        if (key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt || key == Qt::Key_Meta) {
            return;
        }

        Qt::KeyboardModifiers modifiers = event->modifiers();
        QKeySequence ks(modifiers | key);
        setText(ks.toString());
    }
};

#endif // HOTKEYDIALOG_H
