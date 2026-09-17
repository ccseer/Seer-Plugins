#include "histogramrenderer.h"

#include <QBuffer>
#include <QFont>
#include <QPainter>
#include <QPainterPath>
#include <cmath>

namespace {
void drawChannel(QPainter &painter, const std::array<quint64, 256> &bins, double maxLog,
                 double plotLeft, double plotTop, double plotWidth, double plotHeight,
                 const QColor &fillColor, const QColor &strokeColor)
{
    const double plotBottom = plotTop + plotHeight;
    QPainterPath path;
    path.moveTo(plotLeft, plotBottom);

    for (int i = 0; i < 256; ++i) {
        const double x = plotLeft + (static_cast<double>(i) / 255.0) * plotWidth;
        const double normalized = (maxLog > 0.0) ? (std::log1p(static_cast<double>(bins[i])) / maxLog) : 0.0;
        const double y = plotBottom - normalized * plotHeight;
        path.lineTo(x, y);
    }

    path.lineTo(plotLeft + plotWidth, plotBottom);
    path.closeSubpath();

    painter.fillPath(path, fillColor);

    // Draw curve line
    QPainterPath linePath;
    for (int i = 0; i < 256; ++i) {
        const double x = plotLeft + (static_cast<double>(i) / 255.0) * plotWidth;
        const double normalized = (maxLog > 0.0) ? (std::log1p(static_cast<double>(bins[i])) / maxLog) : 0.0;
        const double y = plotBottom - normalized * plotHeight;
        if (i == 0) {
            linePath.moveTo(x, y);
        } else {
            linePath.lineTo(x, y);
        }
    }

    QPen pen(strokeColor, 1.5);
    painter.strokePath(linePath, pen);
}
} // namespace

RenderedHistogram renderHistogram(const HistogramStats &stats)
{
    constexpr int kWidth = 768;
    constexpr int kHeight = 256;

    QImage image(kWidth, kHeight, QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(30, 30, 32));

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    const double plotLeft = 44.0;
    const double plotTop = 28.0;
    const double plotWidth = static_cast<double>(kWidth) - 16.0 - plotLeft;
    const double plotHeight = static_cast<double>(kHeight) - 26.0 - plotTop;
    const double plotBottom = plotTop + plotHeight;
    const double plotRight = plotLeft + plotWidth;

    // Grid lines
    QPen gridPen(QColor(55, 55, 60), 1.0, Qt::DashLine);
    painter.setPen(gridPen);

    for (int step = 1; step <= 3; ++step) {
        const double y = plotTop + (plotHeight / 4.0) * step;
        painter.drawLine(QPointF(plotLeft, y), QPointF(plotRight, y));
    }

    for (int bin : {64, 128, 192}) {
        const double x = plotLeft + (static_cast<double>(bin) / 255.0) * plotWidth;
        painter.drawLine(QPointF(x, plotTop), QPointF(x, plotBottom));
    }

    // Border around plot area
    painter.setPen(QPen(QColor(70, 70, 78), 1.0, Qt::SolidLine));
    painter.drawRect(QRectF(plotLeft, plotTop, plotWidth, plotHeight));

    // Independent log1p maximum per channel
    double maxLogR = 0.0;
    double maxLogG = 0.0;
    double maxLogB = 0.0;
    for (int i = 0; i < 256; ++i) {
        maxLogR = std::max(maxLogR, std::log1p(static_cast<double>(stats.red[i])));
        maxLogG = std::max(maxLogG, std::log1p(static_cast<double>(stats.green[i])));
        maxLogB = std::max(maxLogB, std::log1p(static_cast<double>(stats.blue[i])));
    }

    // Draw channels (Red, Green, Blue)
    drawChannel(painter, stats.red, maxLogR, plotLeft, plotTop, plotWidth, plotHeight,
                QColor(255, 70, 70, 45), QColor(255, 90, 90, 220));
    drawChannel(painter, stats.green, maxLogG, plotLeft, plotTop, plotWidth, plotHeight,
                QColor(70, 220, 70, 45), QColor(90, 230, 90, 220));
    drawChannel(painter, stats.blue, maxLogB, plotLeft, plotTop, plotWidth, plotHeight,
                QColor(70, 140, 255, 45), QColor(100, 170, 255, 220));

    // Labels & Text
    QFont labelFont = painter.font();
    labelFont.setPointSize(8);
    painter.setFont(labelFont);

    // Y scale indicator
    painter.setPen(QColor(160, 160, 170));
    painter.drawText(QRectF(plotLeft, 6, 120, 18), Qt::AlignLeft | Qt::AlignVCenter,
                     QStringLiteral("Y: log1p"));

    // Channel legend
    const double legendRight = plotRight;
    const double legendY = 6;
    painter.setPen(QColor(255, 100, 100));
    painter.drawText(QRectF(legendRight - 90, legendY, 24, 18), Qt::AlignCenter, QStringLiteral("R"));
    painter.setPen(QColor(100, 240, 100));
    painter.drawText(QRectF(legendRight - 60, legendY, 24, 18), Qt::AlignCenter, QStringLiteral("G"));
    painter.setPen(QColor(120, 180, 255));
    painter.drawText(QRectF(legendRight - 30, legendY, 24, 18), Qt::AlignCenter, QStringLiteral("B"));

    // X-axis ticks
    painter.setPen(QColor(140, 140, 150));
    const double tickY = plotBottom + 4;
    painter.drawText(QRectF(plotLeft - 15, tickY, 30, 16), Qt::AlignCenter, QStringLiteral("0"));
    painter.drawText(QRectF(plotLeft + (64.0 / 255.0) * plotWidth - 15, tickY, 30, 16), Qt::AlignCenter, QStringLiteral("64"));
    painter.drawText(QRectF(plotLeft + (128.0 / 255.0) * plotWidth - 15, tickY, 30, 16), Qt::AlignCenter, QStringLiteral("128"));
    painter.drawText(QRectF(plotLeft + (192.0 / 255.0) * plotWidth - 15, tickY, 30, 16), Qt::AlignCenter, QStringLiteral("192"));
    painter.drawText(QRectF(plotRight - 15, tickY, 30, 16), Qt::AlignCenter, QStringLiteral("255"));

    painter.end();

    QByteArray pngData;
    QBuffer buffer(&pngData);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");

    RenderedHistogram result;
    result.image = image;
    result.pngData = pngData;
    result.yAxisScale = QStringLiteral("log1p");
    return result;
}
