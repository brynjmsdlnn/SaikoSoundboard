#ifndef SAIKO_LOGGING_LOGMODEL_H
#define SAIKO_LOGGING_LOGMODEL_H

#include <QAbstractListModel>
#include <QVector>
#include <QMutex>

#include "LogRecord.h"

namespace Saiko {
namespace Logging {

// QAbstractListModel that buffers recent LogRecord objects and exposes them
// to QML. Connects to Logger::logRecordCreated to receive live entries.
//
// Roles:
//   timestampDisplay  — formatted HH:mm:ss.zzz
//   level             — int (LogLevel)
//   levelName         — string ("TRACE", "DEBUG", ...)
//   category          — string
//   message           — string
//   sourceLocation    — "filename:line"
//
class LogModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int filterLevel READ filterLevel WRITE setFilterLevel NOTIFY filterLevelChanged)

public:
    enum Roles {
        TimestampRole = Qt::UserRole + 1,
        LevelRole,
        LevelNameRole,
        CategoryRole,
        MessageRole,
        SourceLocationRole,
    };

    explicit LogModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Total number of buffered records (not filtered).
    int count() const { return m_records.size(); }

    int filterLevel() const { return m_filterLevel; }
    void setFilterLevel(int level);

    // Clear all buffered records.
    Q_INVOKABLE void clear();

signals:
    void countChanged();
    void filterLevelChanged();

private slots:
    void onLogRecord(const LogRecord &record);

private:
    void rebuildFilter();

    static constexpr int kMaxEntries = 2000;

    QVector<LogRecord> m_records;          // ring buffer
    QVector<int> m_filtered;               // indices into m_records (sorted)
    int m_filterLevel = 0;                 // 0 = Trace (show all)
    int m_nextIndex = 0;                   // for ring-buffer wrap
    bool m_wrapped = false;

    mutable QMutex m_mutex;
};

} // namespace Logging
} // namespace Saiko

#endif // SAIKO_LOGGING_LOGMODEL_H
