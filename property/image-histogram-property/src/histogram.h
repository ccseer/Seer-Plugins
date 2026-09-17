#pragma once

#include <array>
#include <QtGlobal>
#include <QImage>

struct HistogramStats {
    std::array<quint64, 256> red{};
    std::array<quint64, 256> green{};
    std::array<quint64, 256> blue{};
    quint64 pixels = 0;
    quint64 alphaPixels = 0;
    quint64 clippedR = 0;
    quint64 clippedG = 0;
    quint64 clippedB = 0;
    quint64 shadowR = 0;
    quint64 shadowG = 0;
    quint64 shadowB = 0;
    double meanR = 0.0;
    double meanG = 0.0;
    double meanB = 0.0;
};

HistogramStats computeHistogram(const QImage &image);
