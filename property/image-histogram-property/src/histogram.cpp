#include "histogram.h"
#include <QColor>

HistogramStats computeHistogram(const QImage &image)
{
    HistogramStats stats{};
    if (image.isNull()) {
        return stats;
    }

    const int width = image.width();
    const int height = image.height();
    if (width <= 0 || height <= 0) {
        return stats;
    }

    stats.pixels = static_cast<quint64>(width) * static_cast<quint64>(height);
    const bool hasAlpha = image.hasAlphaChannel();

    quint64 sumR = 0;
    quint64 sumG = 0;
    quint64 sumB = 0;

    const auto format = image.format();

    if (format == QImage::Format_RGB32) {
        for (int y = 0; y < height; ++y) {
            const QRgb *scanLine = reinterpret_cast<const QRgb *>(image.constScanLine(y));
            for (int x = 0; x < width; ++x) {
                const QRgb pixel = scanLine[x];
                const int r = qRed(pixel);
                const int g = qGreen(pixel);
                const int b = qBlue(pixel);

                stats.red[r]++;
                stats.green[g]++;
                stats.blue[b]++;

                if (r == 0) ++stats.shadowR;
                if (r == 255) ++stats.clippedR;
                if (g == 0) ++stats.shadowG;
                if (g == 255) ++stats.clippedG;
                if (b == 0) ++stats.shadowB;
                if (b == 255) ++stats.clippedB;

                sumR += r;
                sumG += g;
                sumB += b;
            }
        }
    } else if (format == QImage::Format_ARGB32) {
        for (int y = 0; y < height; ++y) {
            const QRgb *scanLine = reinterpret_cast<const QRgb *>(image.constScanLine(y));
            for (int x = 0; x < width; ++x) {
                const QRgb pixel = scanLine[x];
                const int r = qRed(pixel);
                const int g = qGreen(pixel);
                const int b = qBlue(pixel);
                const int a = qAlpha(pixel);

                if (a < 255) {
                    ++stats.alphaPixels;
                }

                stats.red[r]++;
                stats.green[g]++;
                stats.blue[b]++;

                if (r == 0) ++stats.shadowR;
                if (r == 255) ++stats.clippedR;
                if (g == 0) ++stats.shadowG;
                if (g == 255) ++stats.clippedG;
                if (b == 0) ++stats.shadowB;
                if (b == 255) ++stats.clippedB;

                sumR += r;
                sumG += g;
                sumB += b;
            }
        }
    } else if (format == QImage::Format_RGB888) {
        for (int y = 0; y < height; ++y) {
            const uchar *scanLine = image.constScanLine(y);
            for (int x = 0; x < width; ++x) {
                const int r = scanLine[x * 3];
                const int g = scanLine[x * 3 + 1];
                const int b = scanLine[x * 3 + 2];

                stats.red[r]++;
                stats.green[g]++;
                stats.blue[b]++;

                if (r == 0) ++stats.shadowR;
                if (r == 255) ++stats.clippedR;
                if (g == 0) ++stats.shadowG;
                if (g == 255) ++stats.clippedG;
                if (b == 0) ++stats.shadowB;
                if (b == 255) ++stats.clippedB;

                sumR += r;
                sumG += g;
                sumB += b;
            }
        }
    } else {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                const QColor color = image.pixelColor(x, y);
                const int r = color.red();
                const int g = color.green();
                const int b = color.blue();
                const int a = color.alpha();

                if (hasAlpha && a < 255) {
                    ++stats.alphaPixels;
                }

                stats.red[r]++;
                stats.green[g]++;
                stats.blue[b]++;

                if (r == 0) ++stats.shadowR;
                if (r == 255) ++stats.clippedR;
                if (g == 0) ++stats.shadowG;
                if (g == 255) ++stats.clippedG;
                if (b == 0) ++stats.shadowB;
                if (b == 255) ++stats.clippedB;

                sumR += r;
                sumG += g;
                sumB += b;
            }
        }
    }

    if (stats.pixels > 0) {
        stats.meanR = static_cast<double>(sumR) / static_cast<double>(stats.pixels);
        stats.meanG = static_cast<double>(sumG) / static_cast<double>(stats.pixels);
        stats.meanB = static_cast<double>(sumB) / static_cast<double>(stats.pixels);
    }

    return stats;
}
