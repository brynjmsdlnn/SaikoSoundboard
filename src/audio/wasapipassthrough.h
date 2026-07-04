#ifndef WASAPIPASSTHROUGH_H
#define WASAPIPASSTHROUGH_H

#include <QObject>
#include <QFuture>
#include <QString>
#include <atomic>

class WasapiPassthrough : public QObject
{
    Q_OBJECT
public:
    explicit WasapiPassthrough(QObject *parent = nullptr);
    ~WasapiPassthrough();

    void start(const QString &inputDeviceDesc, const QString &outputDeviceDesc);
    void stop();

signals:
    void error(const QString &message);

private:
    void runPassthrough(const QString &inputDeviceDesc, const QString &outputDeviceDesc);

    std::atomic<bool> m_running;
    QFuture<void> m_future;
};

#endif // WASAPIPASSTHROUGH_H
