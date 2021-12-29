#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "metainfo.h"

void addStr(QJsonObject& o, const QString& n, const QString& v)
{
    if (v.isEmpty() || n.isEmpty()) {
        return;
    }
    o[n] = v;
}

void addNum(QJsonObject& o,
            const QString& n,
            qint64 v,
            bool is_data_size = true)
{
    if (v == 0 || n.isEmpty()) {
        return;
    }
    if (is_data_size) {
        o[n] = QLocale::system().formattedDataSize(v);
    }
    else {
        o[n] = QLocale::system().toString(v);
    }
}

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);
    QStringList arguments = a.arguments();
    if (arguments.length() != 3) {
        return -1;
    }

    QFile f(arguments[1]);
    if (!f.open(f.ReadOnly)) {
        qDebug() << f.errorString();
        return -1;
    }
    auto data = f.readAll();
    f.close();

    MetaInfo info;
    if (!info.parse(data)) {
        qDebug() << info.errorString();
        return -1;
    }
    QJsonObject root_obj;
    root_obj["file"] = f.fileName();
    addStr(root_obj, "announceUrl", info.announceUrl());
    qDebug() << "announceList" << info.announceList();
    // map["announceList"] = info.announceList();
    addStr(root_obj, "creationDate", info.creationDate().toString());
    addStr(root_obj, "comment", info.comment());
    addStr(root_obj, "createdBy", info.createdBy());
    addNum(root_obj, "totalSize", info.totalSize());

    QJsonArray file_contents;
    if (info.fileForm() == info.SingleFileForm) {
        QJsonObject obj;
        auto misf = info.singleFile();
        addNum(obj, "totalSize", misf.length);
        addStr(obj, "md5sum", misf.md5sum);
        addNum(obj, "pieceLength", misf.pieceLength, false);
        addStr(obj, "name", misf.name);
        addNum(obj, "sha1SumsCount", misf.sha1Sums.size(), false);
        // for (int j = 0; j < misf.sha1Sums.size(); ++j) {
        //     root_obj["sha1Sums " + QString::number(j)]
        //         = QString(misf.sha1Sums[j]);
        // }
        file_contents.append(obj);
    }
    else {
        // TODO: add tree structure
        auto mimf = info.multiFiles();
        foreach (const auto& misf, mimf) {
            QJsonObject obj;
            addNum(obj, "length", misf.length);
            addStr(obj, "md5sum", misf.md5sum);
            addStr(obj, "path", misf.path);
            file_contents.append(obj);
        }
    }
    root_obj["fileContents"] = file_contents;

    QString output = arguments[2];
    if (QFileInfo(output).suffix().isEmpty()) {
        output.append(".json");
    }
    QJsonDocument doc(root_obj);
    QFile fo(output);
    fo.open(fo.WriteOnly | fo.Text | fo.Truncate);
    fo.write(doc.toJson());
    fo.close();

    return 0;
}
