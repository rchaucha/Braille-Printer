#include <QApplication>
#include <QTextEdit>
#include <QTranslator>
#include <QLibraryInfo>
#include <QTextCodec>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    //////INITIALISATION DE LA LANGUE ET DE L'ENCODAGE//////

    QString locale = QLocale::system().name().section('_', 0, 0);
    QTranslator translator;
    translator.load(QString("qt_") + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&translator);

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf8"));

    ////////////////////////////////////////////////////////

    MainWindow win;

    win.connection();
    if(!win.isProgramClosed())
    {
        win.setWindowState(Qt::WindowMaximized);

        win.show();

        return app.exec();
    }

    return 0;
}
