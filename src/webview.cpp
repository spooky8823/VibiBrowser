#include "webview.h"
#include "historymanager.h"
#include "mainwindow.h"
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineScript>
#include <QFile>

WebView::WebView(QWidget *parent) : QWebEngineView(parent)
{
    settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    settings()->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, true);

    // Set a real user agent so Google doesn't captcha us
    page()->profile()->setHttpUserAgent(
        "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) "
        "VibiBrowser/1.1.0 Chrome/120.0.0.0 Safari/537.36"
    );

    connect(this, &QWebEngineView::urlChanged, this, [this](const QUrl &url){
        if (m_history && url.isValid() && url.scheme() != "vibi" && url.scheme() != "qrc")
            m_history->addEntry(url, title());
    });
}

void WebView::load(const QUrl &url)
{
    if (url.toString() == "vibi://newtab" || url.toString() == "vibi://newtab/") {
        QFile f(":/newtab.html");
        if (f.open(QIODevice::ReadOnly)) {
            setHtml(f.readAll(), QUrl("qrc:///newtab.html"));
            f.close();
        }
        return;
    }
    QWebEngineView::load(url);
}

QWebEngineView* WebView::createWindow(QWebEnginePage::WebWindowType type)
{
    auto *mw = qobject_cast<MainWindow*>(window());
    if (mw) {
        mw->newTab();
        return mw->currentWebView();
    }
    return QWebEngineView::createWindow(type);
}
