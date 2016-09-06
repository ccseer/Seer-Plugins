#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <iostream>
#include <QDir>

const QString g_str_type = "#TYPE_Pleaceholder#";
const QString g_str_code = "#CODE_Pleaceholder#";

void Convert(const QString & path_in, const QString & path_out )
{
    QFile f(path_in);
    if (!f.open(QFile::ReadOnly|QFile::Text)) {
        return;
    }
    auto code = QString(f.readAll()).toHtmlEscaped();
    f.close();

    f.setFileName("://highlight/constant.html");
    if (!f.open(QFile::ReadOnly|QFile::Text)) {
        return;
    }
    auto html = QString(f.readAll());
    f.close();
    html.replace(g_str_code,code);
    html.replace(g_str_type,QFileInfo(path_in).suffix());

    f.setFileName(path_out);
    if ( !f.open(QFile::WriteOnly|QFile::Text) ){
        return;
    }
    f.write(html.toUtf8());
    f.close();

    auto path_css = qApp->applicationDirPath()+"/styles/default.css";
    if (!QFile::exists(path_css)) {
        path_css = "://highlight/styles/default.css";
    }
    const auto dir = QFileInfo(path_out).absoluteDir();
    QFile::copy(path_css,dir.absoluteFilePath("default.css")) ;
    QFile::copy("://highlight/highlight.pack.js",dir.absoluteFilePath("highlight.pack.js")) ;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (a.arguments().size() != 3) {
        std::cout<<std::endl;
        std::cout<<"code2html -- part of the Seer app: www.1218.io"<<std::endl;
        std::cout<<"----------------------------------------------------------"<<std::endl;
        std::cout<<"Usage: code2html.exe input_path output_path"<<std::endl;
        std::cout<<std::endl;
        return -1;
    }

    Convert(a.arguments().at(1),a.arguments().at(2));
    return 0;
}
