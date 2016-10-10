#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <iostream>
#include <QDir>

const QString g_str_type = "#TYPE_Pleaceholder#";
const QString g_str_code = "#CODE_Pleaceholder#";
const QString g_str_css = "#CSS_Pleaceholder#";

void Convert(const QString & css_name, const QString & path_in, const QString & path_out )
{
    auto path_css = qApp->applicationDirPath()+"/styles/"+css_name;
    if ( !QFile::exists(path_css) ) {
        path_css = "://highlight/styles/default.css";
    }
    const auto dir = QFileInfo(path_out).absoluteDir();
    QFile::copy(path_css,dir.absoluteFilePath(css_name)) ;
    QFile::copy("://highlight/highlight.pack.js",dir.absoluteFilePath("highlight.pack.js")) ;


    QFile f(path_in);
    if ( !f.open(QFile::ReadOnly|QFile::Text) ) {
        return;
    }
    auto code = QString(f.readAll()).toHtmlEscaped();
    f.close();

    f.setFileName("://highlight/constant.html");
    if ( !f.open(QFile::ReadOnly|QFile::Text) ) {
        return;
    }
    auto html = QString(f.readAll());
    f.close();
    html.replace(g_str_code,code);
    html.replace(g_str_type,QFileInfo(path_in).suffix());
    html.replace(g_str_css,css_name);

    f.setFileName(path_out);
    if ( !f.open(QFile::WriteOnly|QFile::Text) ){
        return;
    }
    f.write(html.toUtf8());
    f.close();
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (a.arguments().size() != 4) {
        std::cout<<std::endl;
        std::cout<<"code_highlight -- part of the Seer app: http://1218.io"<<std::endl;
        std::cout<<"----------------------------------------------------------"<<std::endl;
        std::cout<<"Usage: code_highlight.exe css_filename input_path output_path"<<std::endl;
        std::cout<<std::endl;
        return -1;
    }

    Convert(a.arguments().at(1),a.arguments().at(2),a.arguments().at(3));
    return 0;
}
