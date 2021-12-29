#ifndef APKMETAINFO_H
#define APKMETAINFO_H

#include <QJsonObject>
#include <QObject>
#include <QProcess>

class ApkMetaInfo : public QObject {
    Q_OBJECT
public:
    explicit ApkMetaInfo(QObject *parent = nullptr);

    void run(const QString &p);
    Q_SIGNAL void sigFinished(const QJsonObject &s);

private:
    void parse(const QString &data);
    void parseLineShort(const QString &, const QString &str, QJsonObject &obj);
    void parseLine(const QString &pre, const QString &str, QJsonArray &array);
    void parseLine(const QString &pre, const QString &str, QJsonObject &obj);

    QByteArray m_output;
    QProcess m_exe;
};

#endif  // APKMETAINFO_H
