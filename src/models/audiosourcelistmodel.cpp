#include "audiosourcelistmodel.h"

AudioSourceListModel::AudioSourceListModel(SettingsManager *settings, QObject *parent)
    : QAbstractListModel(parent)
    , m_settings(settings)
{
    m_sources = m_settings->sources();
    connect(m_settings, &SettingsManager::sourcesChanged, this, &AudioSourceListModel::onSourcesChanged);
}

int AudioSourceListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_sources.size();
}

QVariant AudioSourceListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_sources.size())
        return {};

    const auto &src = m_sources[index.row()];

    switch (role) {
    case SourceIdRole:       return src.id;
    case NameRole:           return src.name;
    case ExecutableNameRole: return src.executableName;
    case ExecutablePathRole: return src.executablePath;
    case EnabledRole:        return src.enabled;
    case VolumeRole:         return src.volume;
    default: return {};
    }
}

QHash<int, QByteArray> AudioSourceListModel::roleNames() const
{
    return {
        {SourceIdRole,       "sourceId"},
        {NameRole,           "name"},
        {ExecutableNameRole, "executableName"},
        {ExecutablePathRole, "executablePath"},
        {EnabledRole,        "enabled"},
        {VolumeRole,         "volume"},
    };
}

bool AudioSourceListModel::addSource(const QString &name, const QString &executableName, const QString &executablePath)
{
    for (const auto &existing : m_sources) {
        if (existing.executableName.compare(executableName, Qt::CaseInsensitive) == 0)
            return false;
        if (!executablePath.isEmpty() && !existing.executablePath.isEmpty()
            && existing.executablePath.compare(executablePath, Qt::CaseInsensitive) == 0)
            return false;
    }
    AudioSource src;
    src.name = name;
    src.executableName = executableName;
    src.executablePath = executablePath;
    auto sources = m_settings->sources();
    sources.append(src);
    m_settings->setSources(sources);
    m_settings->save();
    return true;
}

void AudioSourceListModel::removeSource(const QString &sourceId)
{
    auto sources = m_settings->sources();
    for (int i = 0; i < sources.size(); ++i) {
        if (sources[i].id == sourceId) {
            sources.removeAt(i);
            break;
        }
    }
    m_settings->setSources(sources);
    m_settings->save();
}

bool AudioSourceListModel::hasExecutable(const QString &executableName) const
{
    for (const auto &src : m_sources) {
        if (src.executableName.compare(executableName, Qt::CaseInsensitive) == 0)
            return true;
    }
    return false;
}

QString AudioSourceListModel::getSourceId(int row) const
{
    if (row < 0 || row >= m_sources.size())
        return {};
    return m_sources[row].id;
}

void AudioSourceListModel::onSourcesChanged()
{
    beginResetModel();
    m_sources = m_settings->sources();
    endResetModel();
}
