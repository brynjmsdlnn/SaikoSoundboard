#include "colorediconprovider.h"
#include <QFile>
#include <QSvgRenderer>
#include <QPainter>
#include <QUrlQuery>

ColoredIconProvider::ColoredIconProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
}

QImage ColoredIconProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QStringList parts = id.split('?');
    QString iconName = parts[0];

    QColor color(Qt::white);
    if (parts.size() > 1) {
        QUrlQuery query(parts[1]);
        QString colorStr = query.queryItemValue("color");
        if (!colorStr.isEmpty())
            color = QColor(colorStr);
    }

    QFile file(QStringLiteral(":/icons/%1.svg").arg(iconName));
    if (!file.open(QIODevice::ReadOnly)) {
        if (size) *size = QSize();
        return QImage();
    }

    QString svgData = QString::fromUtf8(file.readAll());
    svgData.replace(QStringLiteral("currentColor"), color.name());

    QSvgRenderer renderer(svgData.toUtf8());
    QSize defaultSize = renderer.defaultSize();
    QSize actualSize = requestedSize.isValid() ? requestedSize : defaultSize;
    if (actualSize.isEmpty())
        actualSize = QSize(24, 24);

    QImage image(actualSize, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    renderer.render(&painter);
    painter.end();

    if (size) *size = actualSize;
    return image;
}
