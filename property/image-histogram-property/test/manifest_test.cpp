#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QTemporaryDir>
#include <filesystem>
#include <iostream>

namespace {
int failures = 0;

void check(bool condition, const char *message)
{
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

bool exactStringArray(const QJsonArray &array, const QStringList &expected)
{
    if (array.size() != expected.size()) {
        return false;
    }
    for (int i = 0; i < expected.size(); ++i) {
        if (array.at(i).toString() != expected[i]) {
            return false;
        }
    }
    return true;
}

bool isWithin(const std::filesystem::path &root,
              const std::filesystem::path &path)
{
    auto rootPart = root.begin();
    auto pathPart = path.begin();
    while (rootPart != root.end()) {
        if (pathPart == path.end() || *rootPart != *pathPart)
            return false;
        ++rootPart;
        ++pathPart;
    }
    return true;
}
}  // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QString packageRoot;
    if (argc >= 2) {
        packageRoot = QString::fromLocal8Bit(argv[1]);
    }
    else {
        if (QFile::exists(QStringLiteral("plugin.json"))) {
            packageRoot = QStringLiteral(".");
        }
        else if (QFile::exists(QStringLiteral("../plugin.json"))) {
            packageRoot = QStringLiteral("..");
        }
        else if (QFile::exists(QStringLiteral("../../plugin.json"))) {
            packageRoot = QStringLiteral("../..");
        }
    }

    const QDir rootDir(packageRoot);
    const QString manifestPath
        = rootDir.filePath(QStringLiteral("plugin.json"));
    check(QFile::exists(manifestPath), "plugin.json exists in package root");

    QFile manifestFile(manifestPath);
    if (!manifestFile.open(QIODevice::ReadOnly)) {
        std::cerr << "FAIL: unable to open plugin.json at "
                  << manifestPath.toStdString() << '\n';
        return 1;
    }

    const auto doc = QJsonDocument::fromJson(manifestFile.readAll());
    check(doc.isObject(), "plugin.json root is JSON object");
    const auto root = doc.object();

    check(root.value(QStringLiteral("schema_version")).toInt() == 1,
          "schema_version is 1");
    check(root.value(QStringLiteral("id")).toString()
              == QStringLiteral("io.1218.seer.image-histogram"),
          "manifest id is io.1218.seer.image-histogram");
    check(root.value(QStringLiteral("name")).toString()
              == QStringLiteral("RGB Histogram"),
          "manifest name is RGB Histogram");
    check(root.value(QStringLiteral("version")).toString()
              == QStringLiteral("1.0.0"),
          "manifest version is 1.0.0");
    check(root.value(QStringLiteral("appMinVersion")).toString()
              == QStringLiteral("4.5.10"),
          "appMinVersion is 4.5.10");
    check(root.value(QStringLiteral("backend")).toString()
              == QStringLiteral("process"),
          "backend is process");

    check(exactStringArray(root.value(QStringLiteral("capabilities")).toArray(),
                           {QStringLiteral("property")}),
          "capabilities contains only property");

    const QStringList expectedExtensions = {
        QStringLiteral("png"), QStringLiteral("jpg"),  QStringLiteral("jpeg"),
        QStringLiteral("bmp"), QStringLiteral("webp"), QStringLiteral("tif"),
        QStringLiteral("tiff")};
    check(exactStringArray(root.value(QStringLiteral("extensions")).toArray(),
                           expectedExtensions),
          "extensions match declared image formats");

    check(root.contains(QStringLiteral("invocations")),
          "manifest has invocations");
    const auto invocations
        = root.value(QStringLiteral("invocations")).toObject();
    check(invocations.contains(QStringLiteral("property")),
          "invocations contains property");

    const auto propInv
        = invocations.value(QStringLiteral("property")).toObject();
    const QString command = propInv.value(QStringLiteral("command")).toString();
    check(command == QStringLiteral("image_histogram.exe"),
          "command is image_histogram.exe");
    check(!command.contains(QLatin1Char('/'))
              && !command.contains(QLatin1Char('\\')),
          "command is package-relative");

    const QStringList expectedArgs
        = {QStringLiteral("--input"),      QStringLiteral("${input_file}"),
           QStringLiteral("--output"),     QStringLiteral("${output_file}"),
           QStringLiteral("--output-dir"), QStringLiteral("${output_dir}")};
    check(exactStringArray(propInv.value(QStringLiteral("arguments")).toArray(),
                           expectedArgs),
          "arguments use input_file, output_file base token, and output_dir");

    check(propInv.value(QStringLiteral("result_schema")).toInt() == 1,
          "result_schema is 1");
    check(propInv.value(QStringLiteral("timeout_ms")).toInt() == 30000,
          "timeout_ms is 30000");

    const auto exitCodes
        = propInv.value(QStringLiteral("success_exit_codes")).toArray();
    check(exitCodes.size() == 1 && exitCodes.first().toInt() == 0,
          "success_exit_codes contains 0");

    const QString exePath = rootDir.filePath(command);
    check(QFileInfo(exePath).isFile(),
          "staged helper exists in the package root");
    if (QFileInfo(exePath).isFile()) {
        const auto root
            = std::filesystem::weakly_canonical(packageRoot.toStdWString());
        const auto executable
            = std::filesystem::weakly_canonical(exePath.toStdWString());
        check(isWithin(root, executable),
              "staged helper remains inside the package root");
    }
    if (QFileInfo(exePath).isFile()) {
        QTemporaryDir tempDir;
        check(tempDir.isValid(), "unable to create helper test directory");
        if (!tempDir.isValid())
            return 1;
        const QString testImg = tempDir.filePath(QStringLiteral("sample.png"));
        QImage img(4, 4, QImage::Format_RGB888);
        img.fill(QColor(100, 150, 200));
        img.save(testImg, "PNG");

        const QString outputBase
            = tempDir.filePath(QStringLiteral("sample_out"));
        QProcess process;
        process.start(
            exePath,
            {QStringLiteral("--input"), testImg, QStringLiteral("--output"),
             outputBase, QStringLiteral("--output-dir"), tempDir.path()});
        check(process.waitForFinished(10000) && process.exitCode() == 0,
              "image_histogram.exe runs and exits 0 on valid image");

        const QString jsonPath
            = tempDir.filePath(QStringLiteral("sample_out.json"));
        const QString pngPath
            = tempDir.filePath(QStringLiteral("rgb-histogram.png"));
        check(QFile::exists(jsonPath), "sample_out.json is created");
        check(QFile::exists(pngPath),
              "rgb-histogram.png is created in output-dir");

        QFile jf(jsonPath);
        if (jf.open(QIODevice::ReadOnly)) {
            const auto outDoc = QJsonDocument::fromJson(jf.readAll());
            check(outDoc.object().value(QStringLiteral("result_schema")).toInt()
                      == 1,
                  "staged helper output has result_schema: 1");
            const auto data
                = outDoc.object().value(QStringLiteral("data")).toObject();
            check(data.value(QStringLiteral("Pixels")).toInteger() == 16,
                  "staged helper computes correct pixel count");
        }
    }

    return failures == 0 ? 0 : 1;
}
