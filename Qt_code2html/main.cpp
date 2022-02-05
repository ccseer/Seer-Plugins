#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QFile>

#define PLACEHOLDER_CSS "PLACEHOLDER_CSS"
#define PLACEHOLDER_NAME "PLACEHOLDER_NAME"
#define PLACEHOLDER_CODE "PLACEHOLDER_CODE"
#define PLACEHOLDER_DIR "PLACEHOLDER_DIR"

bool parse(const QStringList &args,
           QString &input,
           QString &output,
           QString &css,
           QString &codename)
{
    // code2html.exe -i "1.sql" -o "2.html" -n "sql" -c "styles/dark.min.css"
    QCommandLineParser parser;

    parser.setApplicationDescription("You know what it is.");

    QCommandLineOption opt_c("c", "css file: highlight/styles/dark.min.css",
                             "file");
    parser.addOption(opt_c);
    QCommandLineOption opt_n("n", "code name: cpp", "language");
    parser.addOption(opt_n);
    QCommandLineOption opt_i("i", "input <file>", "file");
    parser.addOption(opt_i);
    QCommandLineOption opt_o("o", "output <file>.", "file");
    parser.addOption(opt_o);
    if (!parser.parse(args)) {
        return false;
    }
    if (!parser.isSet(opt_c) || !parser.isSet(opt_n) || !parser.isSet(opt_o)
        || !parser.isSet(opt_i)) {
        return false;
    }
    css      = parser.value(opt_c);
    codename = parser.value(opt_n);
    output   = parser.value(opt_o);
    input    = parser.value(opt_i);
    if (css.isEmpty() || codename.isEmpty() || output.isEmpty()
        || input.isEmpty()) {
        return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QString input, output, css, codename;
    const auto args = a.arguments();
    if (!parse(args, input, output, css, codename)) {
        return -1;
    }
    QFile f("highlight/code.html");
    if (!f.open(f.ReadOnly)) {
        return -2;
    }
    auto html = f.readAll();
    f.close();

    f.setFileName(input);
    if (!f.open(f.ReadOnly)) {
        return -3;
    }
    auto code = f.readAll();
    f.close();

    html.replace(PLACEHOLDER_CODE, code);
    html.replace(PLACEHOLDER_CSS, css.toUtf8());
    html.replace(PLACEHOLDER_NAME, codename.toUtf8());
    html.replace(PLACEHOLDER_DIR, a.applicationDirPath().toUtf8());

    if (!output.endsWith(".html", Qt::CaseInsensitive)) {
        output.append(".html");
    }
    f.setFileName(output);
    if (!f.open(f.WriteOnly)) {
        return -4;
    }
    f.write(html);
    f.close();

    return 0;
}
