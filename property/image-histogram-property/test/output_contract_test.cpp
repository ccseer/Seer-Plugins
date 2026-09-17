#include <QBuffer>
#include <QFile>
#include <QImage>
#include <QImageReader>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTest>

#include "histogramhelper.h"

class OutputContractTest : public QObject {
    Q_OBJECT

private slots:
    void testOutputDirectoryCaseInsensitive();
    void testMissingInput();
    void testUnsupportedFormat();
    void testCorruptImage();
    void testOutputDirectoryFailure();
    void testValidImageAndExactJsonFields();
    void testRelativeAttachmentValue();
    void testResourceLimitRejection();
    void testRejectionOfWriteOutsideOutputDirectory();
    void testCommandLineParsing();
    void testCommandLineRejectsUnknownAndDuplicateOptions();

private:
    QString createValidImage(const QString &dirPath);
    QString createLargeDimensionBmp(const QString &dirPath);
};

QString OutputContractTest::createValidImage(const QString &dirPath)
{
    const QString filePath
        = QDir(dirPath).filePath(QStringLiteral("valid.png"));
    QImage img(2, 2, QImage::Format_RGB888);
    img.setPixelColor(0, 0, QColor(0, 0, 0));
    img.setPixelColor(1, 0, QColor(255, 255, 255));
    img.setPixelColor(0, 1, QColor(100, 150, 200));
    img.setPixelColor(1, 1, QColor(50, 50, 50));
    img.save(filePath, "PNG");
    return filePath;
}

QString OutputContractTest::createLargeDimensionBmp(const QString &dirPath)
{
    const QString filePath
        = QDir(dirPath).filePath(QStringLiteral("large.bmp"));
    QByteArray bmpData;
    // BMP Header (14 bytes)
    bmpData.append("BM");
    quint32 fileSize = 54;
    bmpData.append(reinterpret_cast<const char *>(&fileSize), 4);
    quint32 reserved = 0;
    bmpData.append(reinterpret_cast<const char *>(&reserved), 4);
    quint32 offset = 54;
    bmpData.append(reinterpret_cast<const char *>(&offset), 4);

    // DIB Header (BITMAPINFOHEADER: 40 bytes)
    quint32 headerSize = 40;
    bmpData.append(reinterpret_cast<const char *>(&headerSize), 4);
    qint32 width
        = 20000;  // 20,000 * 10,000 = 200,000,000 pixels > 100,000,000 limit
    qint32 height = 10000;
    bmpData.append(reinterpret_cast<const char *>(&width), 4);
    bmpData.append(reinterpret_cast<const char *>(&height), 4);
    quint16 planes = 1;
    bmpData.append(reinterpret_cast<const char *>(&planes), 2);
    quint16 bitCount = 24;
    bmpData.append(reinterpret_cast<const char *>(&bitCount), 2);
    quint32 compression = 0;
    bmpData.append(reinterpret_cast<const char *>(&compression), 4);
    quint32 imageSize = 0;
    bmpData.append(reinterpret_cast<const char *>(&imageSize), 4);
    qint32 xPels = 0;
    qint32 yPels = 0;
    bmpData.append(reinterpret_cast<const char *>(&xPels), 4);
    bmpData.append(reinterpret_cast<const char *>(&yPels), 4);
    quint32 clrUsed      = 0;
    quint32 clrImportant = 0;
    bmpData.append(reinterpret_cast<const char *>(&clrUsed), 4);
    bmpData.append(reinterpret_cast<const char *>(&clrImportant), 4);

    QFile file(filePath);
    file.open(QIODevice::WriteOnly);
    file.write(bmpData);
    file.close();
    return filePath;
}

void OutputContractTest::testOutputDirectoryCaseInsensitive()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const auto input = createValidImage(tempDir.path());
    const auto output = tempDir.filePath(QStringLiteral("result"));
    QCOMPARE(executeImageHistogram(input, output, tempDir.path().toUpper()), 0);
    QVERIFY(QFile::exists(output + QStringLiteral(".json")));
}

void OutputContractTest::testMissingInput()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString input = tempDir.filePath(QStringLiteral("non_existent.png"));
    const QString outputBase = tempDir.filePath(QStringLiteral("output"));
    const int code = executeImageHistogram(input, outputBase, tempDir.path());
    QCOMPARE(code, 1);
}

void OutputContractTest::testUnsupportedFormat()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString input = tempDir.filePath(QStringLiteral("test.xyz"));
    QFile file(input);
    file.open(QIODevice::WriteOnly);
    file.write("sample text content");
    file.close();

    const QString outputBase = tempDir.filePath(QStringLiteral("output"));
    const int code = executeImageHistogram(input, outputBase, tempDir.path());
    QCOMPARE(code, 1);
}

void OutputContractTest::testCorruptImage()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString input = tempDir.filePath(QStringLiteral("corrupt.png"));
    QFile file(input);
    file.open(QIODevice::WriteOnly);
    file.write("NOT_A_VALID_PNG_IMAGE_DATA_12345");
    file.close();

    const QString outputBase = tempDir.filePath(QStringLiteral("output"));
    const int code = executeImageHistogram(input, outputBase, tempDir.path());
    QCOMPARE(code, 1);
}

void OutputContractTest::testOutputDirectoryFailure()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString input = createValidImage(tempDir.path());
    const QString nonExistentDir
        = tempDir.filePath(QStringLiteral("does_not_exist"));
    const QString outputBase
        = QDir(nonExistentDir).filePath(QStringLiteral("output"));

    const int code = executeImageHistogram(input, outputBase, nonExistentDir);
    QCOMPARE(code, 1);
}

void OutputContractTest::testValidImageAndExactJsonFields()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString input      = createValidImage(tempDir.path());
    const QString outputBase = tempDir.filePath(QStringLiteral("result"));

    const int code = executeImageHistogram(input, outputBase, tempDir.path());
    QCOMPARE(code, 0);

    const QString jsonPath = tempDir.filePath(QStringLiteral("result.json"));
    QVERIFY(QFile::exists(jsonPath));

    QFile jsonFile(jsonPath);
    QVERIFY(jsonFile.open(QIODevice::ReadOnly));
    const auto doc = QJsonDocument::fromJson(jsonFile.readAll());
    QVERIFY(doc.isObject());

    const auto root = doc.object();
    QCOMPARE(root.value(QStringLiteral("result_schema")).toInt(), 1);
    QVERIFY(root.value(QStringLiteral("data")).isObject());

    const auto data = root.value(QStringLiteral("data")).toObject();
    QCOMPARE(data.value(QStringLiteral("Width")).toInteger(), 2);
    QCOMPARE(data.value(QStringLiteral("Height")).toInteger(), 2);
    QCOMPARE(data.value(QStringLiteral("Pixels")).toInteger(), 4);
    QCOMPARE(data.value(QStringLiteral("Alpha Pixels")).toInteger(), 0);

    QVERIFY(data.contains(QStringLiteral("Mean R")));
    QVERIFY(data.contains(QStringLiteral("Mean G")));
    QVERIFY(data.contains(QStringLiteral("Mean B")));

    QCOMPARE(data.value(QStringLiteral("Clipped R")).toInteger(), 1);
    QCOMPARE(data.value(QStringLiteral("Clipped G")).toInteger(), 1);
    QCOMPARE(data.value(QStringLiteral("Clipped B")).toInteger(), 1);

    QCOMPARE(data.value(QStringLiteral("Shadow R")).toInteger(), 1);
    QCOMPARE(data.value(QStringLiteral("Shadow G")).toInteger(), 1);
    QCOMPARE(data.value(QStringLiteral("Shadow B")).toInteger(), 1);

    const auto yScale
        = data.value(QStringLiteral("Histogram Y Scale")).toObject();
    QCOMPARE(yScale.value(QStringLiteral("type")).toString(),
             QStringLiteral("text"));
    QCOMPARE(yScale.value(QStringLiteral("value")).toString(),
             QStringLiteral("log1p"));

    const auto histImg = data.value(QStringLiteral("RGB Histogram")).toObject();
    QCOMPARE(histImg.value(QStringLiteral("type")).toString(),
             QStringLiteral("image"));
    QCOMPARE(histImg.value(QStringLiteral("value")).toString(),
             QStringLiteral("rgb-histogram.png"));

    const QString attachmentPath
        = tempDir.filePath(QStringLiteral("rgb-histogram.png"));
    QVERIFY(QFile::exists(attachmentPath));
    QImageReader reader(attachmentPath);
    QVERIFY(reader.canRead());
    QCOMPARE(reader.size(), QSize(768, 256));
}

void OutputContractTest::testRelativeAttachmentValue()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString input      = createValidImage(tempDir.path());
    const QString outputBase = tempDir.filePath(QStringLiteral("test_att"));

    QCOMPARE(executeImageHistogram(input, outputBase, tempDir.path()), 0);

    QFile jsonFile(tempDir.filePath(QStringLiteral("test_att.json")));
    QVERIFY(jsonFile.open(QIODevice::ReadOnly));
    const auto doc     = QJsonDocument::fromJson(jsonFile.readAll());
    const auto data    = doc.object().value(QStringLiteral("data")).toObject();
    const auto histImg = data.value(QStringLiteral("RGB Histogram")).toObject();

    const QString attachmentVal
        = histImg.value(QStringLiteral("value")).toString();
    QCOMPARE(attachmentVal, QStringLiteral("rgb-histogram.png"));
    QVERIFY(!attachmentVal.contains(QLatin1Char('/')));
    QVERIFY(!attachmentVal.contains(QLatin1Char('\\')));
}

void OutputContractTest::testResourceLimitRejection()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString largeBmp   = createLargeDimensionBmp(tempDir.path());
    const QString outputBase = tempDir.filePath(QStringLiteral("large_out"));

    const int code
        = executeImageHistogram(largeBmp, outputBase, tempDir.path());
    QCOMPARE(code, 1);

    QVERIFY(!QFile::exists(tempDir.filePath(QStringLiteral("large_out.json"))));
    QVERIFY(
        !QFile::exists(tempDir.filePath(QStringLiteral("rgb-histogram.png"))));
}

void OutputContractTest::testRejectionOfWriteOutsideOutputDirectory()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString validImg = createValidImage(tempDir.path());

    const QString dirA = tempDir.filePath(QStringLiteral("dirA"));
    const QString dirB = tempDir.filePath(QStringLiteral("dirB"));
    QDir().mkpath(dirA);
    QDir().mkpath(dirB);

    // Target output inside dirB while output-dir is dirA
    const QString escapeOutput = QDir(dirB).filePath(QStringLiteral("escape"));
    const int codeEscape = executeImageHistogram(validImg, escapeOutput, dirA);
    QCOMPARE(codeEscape, 1);

    // Path traversal outside dirA
    const QString traversalOutput
        = QDir(dirA).filePath(QStringLiteral("../dirB/escape2"));
    const int codeTraversal
        = executeImageHistogram(validImg, traversalOutput, dirA);
    QCOMPARE(codeTraversal, 1);

    QVERIFY(!QFile::exists(QDir(dirB).filePath(QStringLiteral("escape.json"))));
    QVERIFY(
        !QFile::exists(QDir(dirB).filePath(QStringLiteral("escape2.json"))));
}

void OutputContractTest::testCommandLineParsing()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString validImg   = createValidImage(tempDir.path());
    const QString outputBase = tempDir.filePath(QStringLiteral("cli_result"));

    const QStringList args = {QStringLiteral("image_histogram.exe"),
                              QStringLiteral("--input"),
                              validImg,
                              QStringLiteral("--output"),
                              outputBase,
                              QStringLiteral("--output-dir"),
                              tempDir.path()};

    const int code = runImageHistogram(args);
    QCOMPARE(code, 0);
    QVERIFY(QFile::exists(tempDir.filePath(QStringLiteral("cli_result.json"))));
    QVERIFY(
        QFile::exists(tempDir.filePath(QStringLiteral("rgb-histogram.png"))));
}

void OutputContractTest::testCommandLineRejectsUnknownAndDuplicateOptions()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    const QString validImg = createValidImage(tempDir.path());
    const QString outputBase
        = tempDir.filePath(QStringLiteral("strict_result"));
    const QStringList prefix = {QStringLiteral("image_histogram.exe"),
                                QStringLiteral("--input"),
                                validImg,
                                QStringLiteral("--output"),
                                outputBase,
                                QStringLiteral("--output-dir"),
                                tempDir.path()};

    auto unknown = prefix;
    unknown.append({QStringLiteral("--unexpected"), QStringLiteral("value")});
    QCOMPARE(runImageHistogram(unknown), 1);

    auto duplicate = prefix;
    duplicate.insert(1, QStringLiteral("--input"));
    duplicate.insert(2, validImg);
    QCOMPARE(runImageHistogram(duplicate), 1);
}

QTEST_MAIN(OutputContractTest)
#include "output_contract_test.moc"
