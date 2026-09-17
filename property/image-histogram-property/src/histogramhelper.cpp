#include "histogramhelper.h"

#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QSet>
#include <cmath>

#include "histogram.h"
#include "histogramrenderer.h"

namespace {
bool isContainedInDirectory(const QString &filePath, const QString &dirPath)
{
    const QDir dir(dirPath);
    if (!dir.exists()) {
        return false;
    }
    const QString cleanDir = QDir::cleanPath(dir.canonicalPath());
    if (cleanDir.isEmpty()) {
        return false;
    }

    const QFileInfo fileInfo(filePath);
    const QDir fileParentDir = fileInfo.dir();
    if (!fileParentDir.exists()) {
        return false;
    }
    const QString cleanParent = QDir::cleanPath(fileParentDir.canonicalPath());
    if (cleanParent.isEmpty()) {
        return false;
    }

    if (cleanParent.compare(cleanDir, Qt::CaseInsensitive) == 0) {
        return true;
    }
    if (cleanParent.startsWith(cleanDir + QLatin1Char('/'),
                               Qt::CaseInsensitive)) {
        return true;
    }
    return false;
}
}  // namespace

int executeImageHistogram(const QString &inputPath,
                          const QString &outputBasePath,
                          const QString &outputDirPath)
{
    const QDir outDir(outputDirPath);
    if (!outDir.exists()) {
        return 1;
    }

    QString outputJsonPath = outputBasePath;
    if (!outputJsonPath.endsWith(QStringLiteral(".json"),
                                 Qt::CaseInsensitive)) {
        outputJsonPath += QStringLiteral(".json");
    }

    if (!isContainedInDirectory(outputJsonPath, outputDirPath)) {
        return 1;
    }

    const QFileInfo inputInfo(inputPath);
    if (!inputInfo.exists() || !inputInfo.isFile()) {
        return 1;
    }

    const QString suffix                      = inputInfo.suffix().toLower();
    static const QSet<QString> kSupportedExts = {
        QStringLiteral("png"), QStringLiteral("jpg"),  QStringLiteral("jpeg"),
        QStringLiteral("bmp"), QStringLiteral("webp"), QStringLiteral("tif"),
        QStringLiteral("tiff")};
    if (!kSupportedExts.contains(suffix)) {
        return 1;
    }

    QImageReader reader(inputPath);
    if (!reader.canRead()) {
        return 1;
    }

    const QSize size = reader.size();
    if (!size.isValid() || size.width() <= 0 || size.height() <= 0) {
        return 1;
    }

    const quint64 w      = static_cast<quint64>(size.width());
    const quint64 h      = static_cast<quint64>(size.height());
    const quint64 pixels = w * h;
    if (pixels > 100000000ULL) {
        return 1;
    }
    const quint64 estimatedBytes = pixels * 4ULL;
    if (estimatedBytes > 536870912ULL) {
        return 1;
    }

    QImage image;
    if (!reader.read(&image) || image.isNull()) {
        return 1;
    }

    const auto stats    = computeHistogram(image);
    const auto rendered = renderHistogram(stats);

    const QString attachmentPath
        = outDir.filePath(QStringLiteral("rgb-histogram.png"));
    QSaveFile imageFile(attachmentPath);
    if (!imageFile.open(QIODevice::WriteOnly)) {
        return 1;
    }
    if (imageFile.write(rendered.pngData) != rendered.pngData.size()) {
        return 1;
    }
    if (!imageFile.commit()) {
        return 1;
    }

    QJsonObject dataObj;
    dataObj[QStringLiteral("Width")]  = static_cast<qint64>(image.width());
    dataObj[QStringLiteral("Height")] = static_cast<qint64>(image.height());
    dataObj[QStringLiteral("Pixels")] = static_cast<qint64>(stats.pixels);
    dataObj[QStringLiteral("Mean R")] = std::round(stats.meanR * 10.0) / 10.0;
    dataObj[QStringLiteral("Mean G")] = std::round(stats.meanG * 10.0) / 10.0;
    dataObj[QStringLiteral("Mean B")] = std::round(stats.meanB * 10.0) / 10.0;
    dataObj[QStringLiteral("Alpha Pixels")]
        = static_cast<qint64>(stats.alphaPixels);
    dataObj[QStringLiteral("Clipped R")] = static_cast<qint64>(stats.clippedR);
    dataObj[QStringLiteral("Clipped G")] = static_cast<qint64>(stats.clippedG);
    dataObj[QStringLiteral("Clipped B")] = static_cast<qint64>(stats.clippedB);
    dataObj[QStringLiteral("Shadow R")]  = static_cast<qint64>(stats.shadowR);
    dataObj[QStringLiteral("Shadow G")]  = static_cast<qint64>(stats.shadowG);
    dataObj[QStringLiteral("Shadow B")]  = static_cast<qint64>(stats.shadowB);

    QJsonObject yScaleObj;
    yScaleObj[QStringLiteral("type")]            = QStringLiteral("text");
    yScaleObj[QStringLiteral("value")]           = rendered.yAxisScale;
    dataObj[QStringLiteral("Histogram Y Scale")] = yScaleObj;

    QJsonObject histogramImageObj;
    histogramImageObj[QStringLiteral("type")] = QStringLiteral("image");
    histogramImageObj[QStringLiteral("value")]
        = QStringLiteral("rgb-histogram.png");
    dataObj[QStringLiteral("RGB Histogram")] = histogramImageObj;

    QJsonObject rootObj;
    rootObj[QStringLiteral("result_schema")] = 1;
    rootObj[QStringLiteral("data")]          = dataObj;

    QSaveFile jsonFile(outputJsonPath);
    if (!jsonFile.open(QIODevice::WriteOnly)) {
        return 1;
    }
    const auto jsonBytes
        = QJsonDocument(rootObj).toJson(QJsonDocument::Indented);
    if (jsonFile.write(jsonBytes) != jsonBytes.size()) {
        return 1;
    }
    if (!jsonFile.commit()) {
        return 1;
    }

    return 0;
}

int runImageHistogram(const QStringList &arguments)
{
    if (arguments.size() != 7) {
        return 1;
    }

    QString input;
    QString output;
    QString outputDir;
    bool inputSeen     = false;
    bool outputSeen    = false;
    bool outputDirSeen = false;
    for (int i = 1; i < arguments.size(); i += 2) {
        const auto &option = arguments[i];
        const auto &value  = arguments[i + 1];
        if (value.isEmpty()) {
            return 1;
        }
        if (option == QStringLiteral("--input") && !inputSeen) {
            input     = value;
            inputSeen = true;
        }
        else if (option == QStringLiteral("--output") && !outputSeen) {
            output     = value;
            outputSeen = true;
        }
        else if (option == QStringLiteral("--output-dir") && !outputDirSeen) {
            outputDir     = value;
            outputDirSeen = true;
        }
        else {
            return 1;
        }
    }

    if (!inputSeen || !outputSeen || !outputDirSeen) {
        return 1;
    }

    return executeImageHistogram(input, output, outputDir);
}
