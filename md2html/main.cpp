#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <iostream>
#include <QDir>

const QString g_str_code = "#Markdown_Pleaceholder#";

void PrintError(){
    std::cout<<"required: marked.js, constant.html, md_style.css and {input file}." <<std::endl;
}

void Convert(const QString & path_in, const QString & path_out)
{
    const auto app_dir = qApp->applicationDirPath()+"/";
    const QStringList required_file{
        app_dir+"marked.js",
                app_dir+"md_style.css",
                app_dir+"constant.html"
    };
    foreach(const auto & i, required_file) {
        if(!QFile::exists(i)) {
            return PrintError();
        }
    }

    QFile f(path_in);
    if ( !f.open(QFile::ReadOnly|QFile::Text) ) {
        return;
    }
    const auto md = QString(f.readAll()).toHtmlEscaped();
    f.close();

    f.setFileName(app_dir+"constant.html");
    if ( !f.open(QFile::ReadOnly|QFile::Text) ) {
        return;
    }
    auto constant = QString(f.readAll());
    f.close();

    constant.replace(g_str_code, md);

    //write
    f.setFileName(path_out);
    if (!f.open(QFile::WriteOnly|QFile::Text|QFile::Truncate) ){
        return;
    }
    f.write(constant.toUtf8());
    f.close();

    const auto dir = QFileInfo(path_out).absoluteDir();
    foreach(const auto & i, required_file) {
        if(!i.endsWith("html")) {
            const auto fn = QFileInfo(i).fileName();
//            QFile::remove(dir.absoluteFilePath(fn));
            QFile::copy(i, dir.absoluteFilePath(fn)) ;
        }
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (a.arguments().size() != 3) {
        std::cout<<std::endl;
        std::cout<<"Markdown2Html -- part of the Seer app: http://1218.io"<<std::endl;
        std::cout<<"----------------------------------------------------------"<<std::endl;
        std::cout<<"Usage: md2html.exe input_path output_path"<<std::endl;
        std::cout<<std::endl;
        return -1;
    }
    Convert(a.arguments().at(1),a.arguments().at(2));

//    Convert("E:/2.md","E:/2222.html");
    return 0;
}
