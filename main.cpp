#include <QApplication>
#include <QTextCodec>
#include <QTranslator>
#include <QLibraryInfo>
#include <QCommandLineParser>

#include "config_sudoku_solver.h"
#include "mainwindow.h"

int main(int argc, char *argv[]){
    QApplication app(argc, argv);


    QCoreApplication::setOrganizationName("DanSoft");
    QCoreApplication::setOrganizationDomain("dansoft.ru");
    QCoreApplication::setApplicationVersion(APP_VERSION);
    QCoreApplication::setApplicationName(QApplication::tr("Sudoku solver"));

    QTranslator translator;
    translator.load(QString(GLOBAL_PATH_USERDATA)+QString("/langs/sudoku-solver_") + QLocale::system().name());
    app.installTranslator(&translator);

    QCommandLineParser parser;
    parser.setApplicationDescription(QApplication::tr("Sudoku solver"));
    parser.addVersionOption();
    parser.process(app);


    MainWindow *window = new MainWindow();
    window->show();

    return app.exec();
}
