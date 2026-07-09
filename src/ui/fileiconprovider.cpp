#include "ui/fileiconprovider.h"
#include <QUrl>
#include <QPixmap>
#include <QImage>
#ifdef Q_OS_WIN
#include <windows.h>
#include <shellapi.h>
#endif

FileIconProvider::FileIconProvider()
    : QQuickImageProvider(QQuickImageProvider::Pixmap)
{
}

QPixmap FileIconProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    QString filePath = QUrl::fromPercentEncoding(id.toUtf8());
    QSize actualSize = requestedSize.isValid() ? requestedSize : QSize(32, 32);
    if (size) *size = actualSize;

#ifdef Q_OS_WIN
    SHFILEINFOW sfi = {};
    SHGetFileInfoW(reinterpret_cast<const wchar_t *>(filePath.utf16()),
                   0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_LARGEICON);
    if (sfi.hIcon) {
        QPixmap px = QPixmap::fromImage(QImage::fromHICON(sfi.hIcon));
        DestroyIcon(sfi.hIcon);
        return px.scaled(actualSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
#endif
    return QPixmap(actualSize);
}
