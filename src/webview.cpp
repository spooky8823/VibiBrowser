#include "webview.h"
#include "historymanager.h"
#include "mainwindow.h"
#include <QWebEngineProfile>
#include <QWebEngineSettings>

WebView::WebView(QWidget *parent) : QWebEngineView(parent)
{
    // Enable useful features
    settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    settings()->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, true);

    connect(this, &QWebEngineView::urlChanged, this, [this](const QUrl &url){
        if (m_history && url.isValid() && url.scheme() != "vibi")
            m_history->addEntry(url, title());
    });
}

QWebEngineView* WebView::createWindow(QWebEnginePage::WebWindowType type)
{
    // Open new tabs for target=_blank links
    auto *mw = qobject_cast<MainWindow*>(window());
    if (mw) {
        mw->newTab();
        return mw->currentWebView(); // returns the newly created one
    }
    return QWebEngineView::createWindow(type);
}
