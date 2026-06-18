#ifndef WAVWRITER_H
#define WAVWRITER_H

#include <QObject>
#include <QFile>
#include <QString>

#ifdef Q_OS_WIN
#include <windows.h>
#include <mmreg.h>
#endif

class WavWriter : public QObject
{
    Q_OBJECT
public:
    explicit WavWriter(QObject *parent = nullptr);
    ~WavWriter();

    bool open(const QString &fileName, const WAVEFORMATEXTENSIBLE &format);
    void writePcm(const QByteArray &data);
    void close();

    bool isOpen() const;
    QString fileName() const;
    qint64 size() const;

private:
    void writeHeader();
    void updateHeader();

    QFile m_file;
    QString m_fileName;
    WAVEFORMATEXTENSIBLE m_format;
    bool m_headerWritten;
};

#endif // WAVWRITER_H
