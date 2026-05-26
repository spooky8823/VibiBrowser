#include <QApplication>
#include <QWebEngineProfile>
#include <QDir>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setApplicationName("VibiBrowser");
    QCoreApplication::setApplicationVersion(APP_VERSION);
    QCoreApplication::setOrganizationName("Vibifiy");

    QApplication app(argc, argv);

    // Set up persistent profile
    QWebEngineProfile::defaultProfile()->setPersistentCookiesPolicy(
        QWebEngineProfile::AllowPersistentCookies);
    QWebEngineProfile::defaultProfile()->setCachePath(
        QDir::homePath() + "/.config/VibiBrowser/cache");
    QWebEngineProfile::defaultProfile()->setPersistentStoragePath(
        QDir::homePath() + "/.config/VibiBrowser/storage");

    MainWindow w;
    w.show();

    return app.exec();
}
