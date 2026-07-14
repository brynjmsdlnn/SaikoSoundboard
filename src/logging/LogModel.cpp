#include "LogModel.h"
#include "Logger.h"

#include <QMutexLocker>

namespace Saiko {
namespace Logging {

LogModel::LogModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_records.reserve(kMaxEntries);

    connect(&Logger::instance(), &Logger::logRecordCreated,
            this, &LogModel::onLogRecord, Qt::AutoConnection);
}

int LogModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    QMutexLocker lock(&m_mutex);
    return m_filtered.size();
}

QVariant LogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_filtered.size())
        return {};

    QMutexLocker lock(&m_mutex);
    const LogRecord &rec = m_records[m_filtered[index.row()]];

    switch (role) {
    case TimestampRole:
        return rec.timestamp.toString(QStringLiteral("HH:mm:ss.zzz"));
    case LevelRole:
        return static_cast<int>(rec.level);
    case LevelNameRole:
        return QString::fromLatin1(logLevelToString(rec.level));
    case CategoryRole:
        return QString::fromLatin1(rec.category);
    case MessageRole:
        return rec.message;
    case SourceLocationRole:
        return QStringLiteral("%1:%2")
                   .arg(QString::fromLatin1(logBasename(rec.file)))
                   .arg(rec.line);
    default:
        return {};
    }
}

QHash<int, QByteArray> LogModel::roleNames() const
{
    return {
        { TimestampRole,      "timestampDisplay" },
        { LevelRole,          "level" },
        { LevelNameRole,      "levelName" },
        { CategoryRole,       "category" },
        { MessageRole,        "message" },
        { SourceLocationRole, "sourceLocation" },
    };
}

void LogModel::setFilterLevel(int level)
{
    if (level == m_filterLevel)
        return;
    m_filterLevel = level;
    rebuildFilter();
    emit filterLevelChanged();
    emit countChanged();
}

void LogModel::clear()
{
    {
        QMutexLocker lock(&m_mutex);
        m_records.clear();
        m_filtered.clear();
        m_nextIndex = 0;
        m_wrapped = false;
    }

    beginResetModel();
    endResetModel();
    emit countChanged();
}

void LogModel::onLogRecord(const LogRecord &record)
{
    // ── Phase 1: store record under mutex ──
    bool passes = false;
    bool isWrapped = false;
    int insertRow = -1;

    {
        QMutexLocker lock(&m_mutex);

        passes = (static_cast<int>(record.level) >= m_filterLevel);
        isWrapped = m_wrapped;

        if (m_wrapped) {
            m_records[m_nextIndex] = record;
        } else if (m_records.size() < kMaxEntries) {
            m_records.append(record);
            insertRow = m_records.size() - 1;
        } else {
            // First time wrapping — nextIndex stays at 0
            m_wrapped = true;
            m_records[0] = record;
            isWrapped = true;
        }
    }

    if (!passes) {
        // Still advance ring-buffer index for wrapped state to maintain FIFO order
        if (isWrapped) {
            QMutexLocker lock(&m_mutex);
            m_nextIndex = (m_nextIndex + 1) % kMaxEntries;
        }
        return;
    }

    // ── Phase 2: notify UI without mutex ──
    if (isWrapped) {
        rebuildFilter();
    } else {
        beginInsertRows({}, m_filtered.size(), m_filtered.size());
        m_filtered.append(insertRow);
        endInsertRows();
    }

    emit countChanged();

    // ── Phase 3: advance ring-buffer index after UI notification ──
    if (isWrapped) {
        QMutexLocker lock(&m_mutex);
        m_nextIndex = (m_nextIndex + 1) % kMaxEntries;
    }
}

void LogModel::rebuildFilter()
{
    {
        QMutexLocker lock(&m_mutex);

        m_filtered.clear();
        const int total = m_wrapped ? kMaxEntries : m_records.size();
        m_filtered.reserve(total);
        for (int i = 0; i < total; ++i) {
            const int idx = m_wrapped ? (m_nextIndex + i) % kMaxEntries : i;
            if (static_cast<int>(m_records[idx].level) >= m_filterLevel) {
                m_filtered.append(idx);
            }
        }
    }

    beginResetModel();
    endResetModel();
}

} // namespace Logging
} // namespace Saiko
