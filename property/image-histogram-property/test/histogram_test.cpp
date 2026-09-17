#include <QTest>
#include <QImage>
#include <QColor>
#include "histogram.h"

class HistogramTest : public QObject {
    Q_OBJECT

private slots:
    void testBlackPixel();
    void testWhitePixel();
    void testRedPixel();
    void testGreenPixel();
    void testBluePixel();
    void testGrayscalePixel();
    void testTransparentPixel();
    void testMixedImage();
    void testNullImage();
};

void HistogramTest::testBlackPixel()
{
    QImage img(1, 1, QImage::Format_RGB888);
    img.setPixelColor(0, 0, QColor(0, 0, 0));
    const auto stats = computeHistogram(img);
    QCOMPARE(stats.pixels, 1ULL);
    QCOMPARE(stats.alphaPixels, 0ULL);
    QCOMPARE(stats.red[0], 1ULL);
    QCOMPARE(stats.green[0], 1ULL);
    QCOMPARE(stats.blue[0], 1ULL);
    QCOMPARE(stats.shadowR, 1ULL);
    QCOMPARE(stats.shadowG, 1ULL);
    QCOMPARE(stats.shadowB, 1ULL);
    QCOMPARE(stats.clippedR, 0ULL);
    QCOMPARE(stats.clippedG, 0ULL);
    QCOMPARE(stats.clippedB, 0ULL);
    QCOMPARE(stats.meanR, 0.0);
    QCOMPARE(stats.meanG, 0.0);
    QCOMPARE(stats.meanB, 0.0);
}

void HistogramTest::testWhitePixel()
{
    QImage img(1, 1, QImage::Format_RGB888);
    img.setPixelColor(0, 0, QColor(255, 255, 255));
    const auto stats = computeHistogram(img);
    QCOMPARE(stats.pixels, 1ULL);
    QCOMPARE(stats.alphaPixels, 0ULL);
    QCOMPARE(stats.red[255], 1ULL);
    QCOMPARE(stats.green[255], 1ULL);
    QCOMPARE(stats.blue[255], 1ULL);
    QCOMPARE(stats.shadowR, 0ULL);
    QCOMPARE(stats.shadowG, 0ULL);
    QCOMPARE(stats.shadowB, 0ULL);
    QCOMPARE(stats.clippedR, 1ULL);
    QCOMPARE(stats.clippedG, 1ULL);
    QCOMPARE(stats.clippedB, 1ULL);
    QCOMPARE(stats.meanR, 255.0);
    QCOMPARE(stats.meanG, 255.0);
    QCOMPARE(stats.meanB, 255.0);
}

void HistogramTest::testRedPixel()
{
    QImage img(1, 1, QImage::Format_RGB888);
    img.setPixelColor(0, 0, QColor(255, 0, 0));
    const auto stats = computeHistogram(img);
    QCOMPARE(stats.pixels, 1ULL);
    QCOMPARE(stats.alphaPixels, 0ULL);
    QCOMPARE(stats.red[255], 1ULL);
    QCOMPARE(stats.green[0], 1ULL);
    QCOMPARE(stats.blue[0], 1ULL);
    QCOMPARE(stats.shadowR, 0ULL);
    QCOMPARE(stats.shadowG, 1ULL);
    QCOMPARE(stats.shadowB, 1ULL);
    QCOMPARE(stats.clippedR, 1ULL);
    QCOMPARE(stats.clippedG, 0ULL);
    QCOMPARE(stats.clippedB, 0ULL);
    QCOMPARE(stats.meanR, 255.0);
    QCOMPARE(stats.meanG, 0.0);
    QCOMPARE(stats.meanB, 0.0);
}

void HistogramTest::testGreenPixel()
{
    QImage img(1, 1, QImage::Format_RGB888);
    img.setPixelColor(0, 0, QColor(0, 255, 0));
    const auto stats = computeHistogram(img);
    QCOMPARE(stats.pixels, 1ULL);
    QCOMPARE(stats.alphaPixels, 0ULL);
    QCOMPARE(stats.red[0], 1ULL);
    QCOMPARE(stats.green[255], 1ULL);
    QCOMPARE(stats.blue[0], 1ULL);
    QCOMPARE(stats.shadowR, 1ULL);
    QCOMPARE(stats.shadowG, 0ULL);
    QCOMPARE(stats.shadowB, 1ULL);
    QCOMPARE(stats.clippedR, 0ULL);
    QCOMPARE(stats.clippedG, 1ULL);
    QCOMPARE(stats.clippedB, 0ULL);
    QCOMPARE(stats.meanR, 0.0);
    QCOMPARE(stats.meanG, 255.0);
    QCOMPARE(stats.meanB, 0.0);
}

void HistogramTest::testBluePixel()
{
    QImage img(1, 1, QImage::Format_RGB888);
    img.setPixelColor(0, 0, QColor(0, 0, 255));
    const auto stats = computeHistogram(img);
    QCOMPARE(stats.pixels, 1ULL);
    QCOMPARE(stats.alphaPixels, 0ULL);
    QCOMPARE(stats.red[0], 1ULL);
    QCOMPARE(stats.green[0], 1ULL);
    QCOMPARE(stats.blue[255], 1ULL);
    QCOMPARE(stats.shadowR, 1ULL);
    QCOMPARE(stats.shadowG, 1ULL);
    QCOMPARE(stats.shadowB, 0ULL);
    QCOMPARE(stats.clippedR, 0ULL);
    QCOMPARE(stats.clippedG, 0ULL);
    QCOMPARE(stats.clippedB, 1ULL);
    QCOMPARE(stats.meanR, 0.0);
    QCOMPARE(stats.meanG, 0.0);
    QCOMPARE(stats.meanB, 255.0);
}

void HistogramTest::testGrayscalePixel()
{
    QImage img(1, 1, QImage::Format_RGB888);
    img.setPixelColor(0, 0, QColor(128, 128, 128));
    const auto stats = computeHistogram(img);
    QCOMPARE(stats.pixels, 1ULL);
    QCOMPARE(stats.alphaPixels, 0ULL);
    QCOMPARE(stats.red[128], 1ULL);
    QCOMPARE(stats.green[128], 1ULL);
    QCOMPARE(stats.blue[128], 1ULL);
    QCOMPARE(stats.shadowR, 0ULL);
    QCOMPARE(stats.shadowG, 0ULL);
    QCOMPARE(stats.shadowB, 0ULL);
    QCOMPARE(stats.clippedR, 0ULL);
    QCOMPARE(stats.clippedG, 0ULL);
    QCOMPARE(stats.clippedB, 0ULL);
    QCOMPARE(stats.meanR, 128.0);
    QCOMPARE(stats.meanG, 128.0);
    QCOMPARE(stats.meanB, 128.0);
}

void HistogramTest::testTransparentPixel()
{
    QImage img(1, 1, QImage::Format_ARGB32);
    img.setPixelColor(0, 0, QColor(255, 128, 64, 100));
    const auto stats = computeHistogram(img);
    QCOMPARE(stats.pixels, 1ULL);
    QCOMPARE(stats.alphaPixels, 1ULL);
    QCOMPARE(stats.red[255], 1ULL);
    QCOMPARE(stats.green[128], 1ULL);
    QCOMPARE(stats.blue[64], 1ULL);
    QCOMPARE(stats.shadowR, 0ULL);
    QCOMPARE(stats.shadowG, 0ULL);
    QCOMPARE(stats.shadowB, 0ULL);
    QCOMPARE(stats.clippedR, 1ULL);
    QCOMPARE(stats.clippedG, 0ULL);
    QCOMPARE(stats.clippedB, 0ULL);
    QCOMPARE(stats.meanR, 255.0);
    QCOMPARE(stats.meanG, 128.0);
    QCOMPARE(stats.meanB, 64.0);
}

void HistogramTest::testMixedImage()
{
    QImage img(2, 2, QImage::Format_ARGB32);
    img.setPixelColor(0, 0, QColor(0, 0, 0, 255));
    img.setPixelColor(1, 0, QColor(255, 255, 255, 255));
    img.setPixelColor(0, 1, QColor(200, 100, 50, 120));
    img.setPixelColor(1, 1, QColor(0, 50, 255, 255));

    const auto stats = computeHistogram(img);
    QCOMPARE(stats.pixels, 4ULL);
    QCOMPARE(stats.alphaPixels, 1ULL);

    QCOMPARE(stats.red[0], 2ULL);
    QCOMPARE(stats.red[200], 1ULL);
    QCOMPARE(stats.red[255], 1ULL);

    QCOMPARE(stats.green[0], 1ULL);
    QCOMPARE(stats.green[50], 1ULL);
    QCOMPARE(stats.green[100], 1ULL);
    QCOMPARE(stats.green[255], 1ULL);

    QCOMPARE(stats.blue[0], 1ULL);
    QCOMPARE(stats.blue[50], 1ULL);
    QCOMPARE(stats.blue[255], 2ULL);

    QCOMPARE(stats.shadowR, 2ULL);
    QCOMPARE(stats.shadowG, 1ULL);
    QCOMPARE(stats.shadowB, 1ULL);

    QCOMPARE(stats.clippedR, 1ULL);
    QCOMPARE(stats.clippedG, 1ULL);
    QCOMPARE(stats.clippedB, 2ULL);

    QCOMPARE(stats.meanR, 113.75);
    QCOMPARE(stats.meanG, 101.25);
    QCOMPARE(stats.meanB, 140.0);
}

void HistogramTest::testNullImage()
{
    QImage img;
    const auto stats = computeHistogram(img);
    QCOMPARE(stats.pixels, 0ULL);
    QCOMPARE(stats.alphaPixels, 0ULL);
    QCOMPARE(stats.meanR, 0.0);
    QCOMPARE(stats.meanG, 0.0);
    QCOMPARE(stats.meanB, 0.0);
}

QTEST_MAIN(HistogramTest)
#include "histogram_test.moc"
