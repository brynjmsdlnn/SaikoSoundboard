#include "SingleInstanceGuard.h"
#include "logging/LogMacros.h"

#include <QLocalSocket>

const QByteArray SingleInstanceGuard::kActivateMessage = "ACTIVATE";

SingleInstanceGuard::SingleInstanceGuard(const QString &serverName, QObject *parent)
    : QObject(parent)
    , m_serverName(serverName)
{
}

SingleInstanceGuard::~SingleInstanceGuard()
{
    if (m_server) {
        m_server->close();
    }
}

bool SingleInstanceGuard::tryStart()
{
    // --- Attempt to connect to an existing server ---
    QLocalSocket socket;
    socket.connectToServer(m_serverName);

    if (socket.waitForConnected(1000)) {
        // Another instance is already running — send ACTIVATE and exit.
        socket.write(kActivateMessage);
        socket.waitForBytesWritten(1000);
        socket.disconnectFromServer();
        LOG_INFO(LogCategory::General,
                 QStringLiteral("[SingleInstanceGuard] Another instance detected, sent ACTIVATE (server: %1).").arg(m_serverName));
        return false;
    }

    // --- No existing server — we are the first instance ---
    m_server = new QLocalServer(this);
    m_server->setSocketOptions(QLocalServer::UserAccessOption);

    // Remove any stale pipe file that may linger from a previous crash.
    // On Windows QLocalServer will fail to listen() if the name is already
    // registered, even if the owning process is gone.
    QLocalServer::removeServer(m_serverName);

    if (!m_server->listen(m_serverName)) {
        LOG_WARN(LogCategory::General,
                 QStringLiteral("[SingleInstanceGuard] Failed to start server (server: %1, error: %2).").arg(m_serverName).arg(m_server->errorString()));
        // Fallback: allow the app to continue anyway. If two instances end
        // up running it's a minor UX issue, not a crash.
        return true;
    }

    connect(m_server, &QLocalServer::newConnection, this, &SingleInstanceGuard::onNewConnection);

    LOG_INFO(LogCategory::General,
             QStringLiteral("[SingleInstanceGuard] Single instance server listening (server: %1).").arg(m_serverName));
    return true;
}

void SingleInstanceGuard::onNewConnection()
{
    QLocalSocket *clientSocket = m_server->nextPendingConnection();
    if (!clientSocket)
        return;

    // Read the message (non-blocking, small payload expected).
    if (clientSocket->waitForReadyRead(2000)) {
        const QByteArray data = clientSocket->readAll().trimmed();
        if (data == kActivateMessage) {
            LOG_DEBUG(LogCategory::General,
                      QStringLiteral("[SingleInstanceGuard] Received activate request from second instance (server: %1).").arg(m_serverName));
            emit activateRequested();
        }
    }

    clientSocket->disconnectFromServer();
    clientSocket->deleteLater();
}
