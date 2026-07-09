#ifndef FILEICONPROVIDER_H
#define FILEICONPROVIDER_H

#include <QQuickImageProvider>

class FileIconProvider : public QQuickImageProvider
{
public:
    FileIconProvider();
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;
};

#endif // FILEICONPROVIDER_H
