#pragma once

#include "histogram.h"
#include <QByteArray>
#include <QImage>
#include <QString>

struct RenderedHistogram {
    QImage image;
    QByteArray pngData;
    QString yAxisScale = QStringLiteral("log1p");
};

RenderedHistogram renderHistogram(const HistogramStats &stats);
