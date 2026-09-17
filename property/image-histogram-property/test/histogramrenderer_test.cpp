#include <QTest>
#include <QImageReader>
#include <QBuffer>
#include "histogramrenderer.h"

class HistogramRendererTest : public QObject {
    Q_OBJECT

private slots:
    void testRenderDimensionsAndScale();
    void testEmptyStatsRendering();
    void testNonEmptyDataRendering();
    void testDeterministicFixtureRendering();
};

void HistogramRendererTest::testRenderDimensionsAndScale()
{
    HistogramStats stats{};
    const auto rendered = renderHistogram(stats);

    QCOMPARE(rendered.yAxisScale, QStringLiteral("log1p"));
    QCOMPARE(rendered.image.width(), 768);
    QCOMPARE(rendered.image.height(), 256);
    QVERIFY(!rendered.pngData.isEmpty());

    QBuffer buffer(const_cast<QByteArray *>(&rendered.pngData));
    buffer.open(QIODevice::ReadOnly);
    QImageReader reader(&buffer);
    QVERIFY(reader.canRead());
    QCOMPARE(reader.size(), QSize(768, 256));

    QImage decoded = reader.read();
    QVERIFY(!decoded.isNull());
    QCOMPARE(decoded.size(), QSize(768, 256));
}

void HistogramRendererTest::testEmptyStatsRendering()
{
    HistogramStats stats{};
    const auto rendered = renderHistogram(stats);
    QVERIFY(!rendered.pngData.isEmpty());
}

void HistogramRendererTest::testNonEmptyDataRendering()
{
    HistogramStats stats{};
    stats.pixels = 100;
    stats.red[50] = 30;
    stats.red[100] = 70;
    stats.green[128] = 100;
    stats.blue[200] = 100;

    const auto rendered = renderHistogram(stats);
    QVERIFY(!rendered.pngData.isEmpty());

    const QImage img = rendered.image;
    bool hasForeground = false;
    const QRgb bg = img.pixel(0, 0);
    for (int y = 0; y < img.height() && !hasForeground; ++y) {
        for (int x = 0; x < img.width(); ++x) {
            if (img.pixel(x, y) != bg) {
                hasForeground = true;
                break;
            }
        }
    }
    QVERIFY(hasForeground);
}

void HistogramRendererTest::testDeterministicFixtureRendering()
{
    HistogramStats stats{};
    stats.pixels = 2;
    stats.red[0] = 1;
    stats.red[255] = 1;
    stats.green[128] = 2;
    stats.blue[64] = 2;

    const auto render1 = renderHistogram(stats);
    const auto render2 = renderHistogram(stats);

    QCOMPARE(render1.pngData, render2.pngData);
}

QTEST_MAIN(HistogramRendererTest)
#include "histogramrenderer_test.moc"
