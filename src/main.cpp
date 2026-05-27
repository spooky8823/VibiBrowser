#include <QApplication>
#include <QWebEngineProfile>
#include <QDir>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

// Performance flags for low-end hardware
qputenv("QTWEBENGINE_CHROMIUM_FLAGS",
    "--enable-gpu-rasterization "
    "--enable-zero-copy "
    "--disable-gpu-driver-bug-workarounds "
    "--enable-accelerated-video-decode "
    "--num-raster-threads=2 "
    "--renderer-process-limit=3 "
    "--memory-pressure-thresholds-mb=256,512 "
    "--disable-features=UseChromeOSDirectVideoDecoder "
    "--enable-features=VaapiVideoDecoder,VaapiVideoEncoder"
);
    QCoreApplication::setApplicationName("VibiBrowser");
    QCoreApplication::setApplicationVersion(APP_VERSION);
    QCoreApplication::setOrganizationName("Vibifiy");

    QApplication app(argc, argv);

    // Set up persistent profile
    QWebEngineProfile::defaultProfile()->setPersistentCookiesPolicy(
        QWebEngineProfile::AllowPersistentCookies);
        QWebEngineProfile::defaultProfile()->setHttpCacheMaximumSize(50 * 1024 * 1024); // 50MB cache limit
    QWebEngineProfile::defaultProfile()->setCachePath(
        QDir::homePath() + "/.config/VibiBrowser/cache");
    QWebEngineProfile::defaultProfile()->setPersistentStoragePath(
        QDir::homePath() + "/.config/VibiBrowser/storage");

    MainWindow w;
    w.show();

    return app.exec();
}
