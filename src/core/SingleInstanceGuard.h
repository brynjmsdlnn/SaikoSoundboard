#pragma once

#include <QObject>
#include <QString>
#include <QLocalServer>

/**
 * @brief Ensures only one instance of the application runs.
 *
 * The first instance creates a QLocalServer with a well-known name.
 * Subsequent instances attempt to connect to that server —
 * if the connection succeeds, they send an ACTIVATE message and exit.
 *
 * The running instance emits activateRequested() when a secondary
 * instance tries to launch, allowing the UI to restore/raise the window.
 *
 * Thread-safety: not required — all operations happen on the main thread.
 */
class SingleInstanceGuard : public QObject
{
    Q_OBJECT

public:
    /**
     * @param serverName  Unique name for the local socket server.
     *                    Use a GUID-prefixed string to avoid collisions.
     * @param parent      QObject parent.
     */
    explicit SingleInstanceGuard(const QString &serverName, QObject *parent = nullptr);
    ~SingleInstanceGuard() override;

    /**
     * Attempt to start the guard.
     *
     * @returns true if this is the first instance (server started).
     *          false if another instance is already running (ACTIVATE sent).
     */
    bool tryStart();

signals:
    /** Emitted when a secondary instance sends an ACTIVATE message. */
    void activateRequested();

private slots:
    void onNewConnection();

private:
    QString m_serverName;
    QLocalServer *m_server = nullptr;

    static const QByteArray kActivateMessage;
};
