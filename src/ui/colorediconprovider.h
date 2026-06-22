#ifndef COLOREDICONPROVIDER_H
#define COLOREDICONPROVIDER_H

#include <QQuickImageProvider>
#include <QImage>

class ColoredIconProvider : public QQuickImageProvider
{
public:
    ColoredIconProvider();
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
};

#endif
