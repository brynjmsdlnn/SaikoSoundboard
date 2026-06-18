#include "wavwriter.h"
#include <QDataStream>
#include <QDebug>

WavWriter::WavWriter(QObject *parent)
    : QObject(parent)
    , m_headerWritten(false)
{
    memset(&m_format, 0, sizeof(m_format));
}

WavWriter::~WavWriter()
{
    if (m_file.isOpen()) {
        close();
    }
}

bool WavWriter::open(const QString &fileName, const WAVEFORMATEXTENSIBLE &format)
{
    if (m_file.isOpen()) {
        close();
    }

    m_file.setFileName(fileName);
    if (!m_file.open(QIODevice::WriteOnly)) {
        return false;
    }

    m_fileName = fileName;
    m_format = format;
    m_headerWritten = false;

    if (m_format.Format.nSamplesPerSec > 0) {
        writeHeader();
    }

    return true;
}

void WavWriter::writePcm(const QByteArray &data)
{
    if (!m_file.isOpen()) return;

    if (!m_headerWritten && m_format.Format.nSamplesPerSec > 0) {
        writeHeader();
    }

    m_file.write(data);
}

void WavWriter::close()
{
    if (!m_file.isOpen()) return;

    if (m_headerWritten) {
        updateHeader();
    }
    m_file.close();
    m_headerWritten = false;
}

bool WavWriter::isOpen() const
{
    return m_file.isOpen();
}

QString WavWriter::fileName() const
{
    return m_fileName;
}

qint64 WavWriter::size() const
{
    return m_file.size();
}

void WavWriter::writeHeader()
{
    if (!m_file.isOpen()) return;

    m_file.seek(0);
    QDataStream out(&m_file);
    out.setByteOrder(QDataStream::LittleEndian);

    // RIFF chunk
    out.writeRawData("RIFF", 4);
    out << (quint32)0; // Placeholder for file size
    out.writeRawData("WAVE", 4);

    // fmt chunk
    out.writeRawData("fmt ", 4);
    out << (quint32)sizeof(WAVEFORMATEXTENSIBLE);
    out.writeRawData((const char*)&m_format, sizeof(WAVEFORMATEXTENSIBLE));

    // data chunk
    out.writeRawData("data", 4);
    out << (quint32)0; // Placeholder for data size

    m_headerWritten = true;
}

void WavWriter::updateHeader()
{
    if (!m_file.isOpen() || !m_headerWritten) return;

    quint32 fileSize = m_file.size();
    quint32 dataSize = fileSize - 44 - sizeof(WAVEFORMATEXTENSIBLE); // 44 is standard but we use EXTENSIBLE

    // Correct dataSize calculation for WAVEFORMATEXTENSIBLE:
    // RIFF (4) + Size (4) + WAVE (4) = 12
    // fmt (4) + Size (4) + FormatData (sizeof(WAVEFORMATEXTENSIBLE)) = 8 + 40 = 48
    // data (4) + Size (4) = 8
    // Total header = 12 + 48 + 8 = 68 bytes
    
    dataSize = fileSize - 68;

    m_file.seek(4);
    QDataStream out(&m_file);
    out.setByteOrder(QDataStream::LittleEndian);
    out << (quint32)(fileSize - 8);

    m_file.seek(64); // Seek to data size field (12 + 48 + 4)
    out << (quint32)dataSize;
    
    m_file.seek(fileSize); // Return to end
}
