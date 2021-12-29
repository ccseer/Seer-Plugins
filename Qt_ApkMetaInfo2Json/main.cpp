#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>

#include "apkmetainfo.h"

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);
    QStringList arguments = a.arguments();
    if (arguments.length() != 3) {
        return -1;
    }

    ApkMetaInfo apk;
    apk.connect(&apk, &ApkMetaInfo::sigFinished, [&](const QJsonObject& obj) {
        if (obj.isEmpty()) {
            a.quit();
            return;
        }
        QJsonDocument doc(obj);

        QString output = arguments[2];
        if (QFileInfo(output).suffix().isEmpty()) {
            output.append(".json");
        }
        QFile fo(output);
        fo.open(fo.WriteOnly | fo.Text | fo.Truncate);
        fo.write(doc.toJson());
        fo.close();
        a.quit();
    });
    apk.run(arguments[1]);

    return a.exec();
}
