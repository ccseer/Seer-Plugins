#include "apkmetainfo.h"

#include <QDebug>
#include <QJsonArray>
#include <QTextCodec>

/////////////////////////////////////////////////////////
/// run exe
ApkMetaInfo::ApkMetaInfo(QObject *parent) : QObject(parent)
{
    connect(&m_exe, &QProcess::readyReadStandardOutput, this,
            [=]() { m_output.append(m_exe.readAllStandardOutput()); });
    connect(&m_exe, &QProcess::errorOccurred, this,
            [=]() { emit sigFinished({}); });
    connect(&m_exe,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [=]() {
                m_output.append(m_exe.readAllStandardOutput());

                if (QTextCodec *codec = QTextCodec::codecForName(m_output)) {
                    parse(codec->toUnicode(m_output));
                }
                else {
                    parse(m_output);
                }
            });
}

void ApkMetaInfo::run(const QString &p)
{
    m_exe.start("aapt.exe", QStringList{"dump", "badging", p});
    if (!m_exe.waitForStarted()) {
        qDebug() << m_exe.errorString();
        m_exe.kill();
        emit sigFinished({});
    }
}

//////////////////////////////////////////////////////////////////////
/// parse data
void ApkMetaInfo::parseLine(const QString &pre,
                            const QString &str,
                            QJsonObject &obj)
{
    // str
    // "package: name='com.github.kr328.clash' versionCode='203022'
    // versionName='2.3.22' compileSdkVersion='30'
    // compileSdkVersionCodename='11'"

    const auto parts
        = QString(str).remove(pre + ":").split(" ", Qt::SkipEmptyParts);
    for (const auto &j : parts) {
        auto pair = j.split("=");
        if (pair.size() == 2) {
            obj[pair[0]] = pair[1].remove("'");
        }
        else {
            qDebug() << "err" << pair << str;
        }
    }
}

void ApkMetaInfo::parseLine(const QString &pre,
                            const QString &str,
                            QJsonArray &array)
{
    const auto parts
        = QString(str).remove(pre + ":").split(" ", Qt::SkipEmptyParts);
    for (const auto &j : parts) {
        auto pair = j.split("=");
        if (pair.size() == 2) {
            array.append(pair[1].remove("'"));
        }
        else {
            qDebug() << "err" << pair << str;
        }
    }
}

void ApkMetaInfo::parseLineShort(const QString & /*pre*/,
                                 const QString &str,
                                 QJsonObject &obj)
{
    // str
    // ""application-icon-160:'r/n_.xml'"
    const auto parts = str.split(":", Qt::SkipEmptyParts);
    if (parts.size() == 2) {
        obj[parts.value(0)] = parts.value(1).remove("'").trimmed();
    }
    else {
        qDebug() << "err" << parts;
    }
}

void ApkMetaInfo::parse(const QString &data)
{
    const QString pre_package = "package";
    const QString pre_sdk_v   = "sdkVersion";
    const QString pre_sdk_t_v = "targetSdkVersion";
    const QString pre_up      = "uses-permission";
    const QString pre_il      = "install-location";
    const QString pre_al      = "application-label";
    const QString pre_nc      = "native-code";

    QJsonObject obj_package;
    QJsonArray obj_up;
    QJsonArray obj_other;
    QString line;

    const auto list = data.split('\n', Qt::SkipEmptyParts);
    for (const auto &i : list) {
        line = i.trimmed();

        if (line.startsWith(pre_package)) {
            parseLine(pre_package, line, obj_package);
        }
        else if (line.startsWith(pre_sdk_v)) {
            parseLineShort(pre_sdk_v, line, obj_package);
        }
        else if (line.startsWith(pre_sdk_t_v)) {
            parseLineShort(pre_sdk_t_v, line, obj_package);
        }
        else if (line.startsWith(pre_il)) {
            parseLineShort(pre_il, line, obj_package);
        }
        else if (line.startsWith(pre_al + "-") || line.startsWith(pre_al)) {
            if (line.startsWith(pre_al + "-")) {
                continue;
            }
            parseLineShort(pre_al, line, obj_package);
        }
        else if (line.startsWith(pre_nc)) {
            parseLineShort(pre_nc, line, obj_package);
        }
        else if (line.startsWith(pre_up)) {
            parseLine(pre_up, line, obj_up);
        }
        else {
            obj_other.append(line);
        }
    }

    QJsonObject obj_root;
    obj_root["Package"]         = obj_package;
    obj_root["Uses Permission"] = obj_up;
    // should label it any character after "U"
    obj_root["X"] = obj_other;
    emit sigFinished(obj_root);
}
