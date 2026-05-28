#include "webview.h"
#include "historymanager.h"
#include "mainwindow.h"
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QFile>
#include <QDir>

WebView::WebView(QWidget *parent) : QWebEngineView(parent)
{
    settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    settings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    settings()->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    settings()->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, true);

    // Transparent background for liquid glass effect
    setAttribute(Qt::WA_TranslucentBackground);
    page()->setBackgroundColor(Qt::transparent);

    // Set a real user agent
    page()->profile()->setHttpUserAgent(
        "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) "
        "VibiBrowser/1.1.0 Chrome/136.0.0.0 Safari/537.36"
    );

    connect(this, &QWebEngineView::urlChanged, this, [this](const QUrl &url){
        if (m_history && url.isValid() && url.scheme() != "vibi" && url.scheme() != "qrc")
            m_history->addEntry(url, title());

        // Catch vibi://install-extension before OS handles it
        if (url.toString().startsWith("vibi://install-extension/")) {
            QString extId = url.path().section('/', 1, 1);
            QString store = url.query().remove("store=");
            if (!extId.isEmpty()) {
                auto *mw = qobject_cast<MainWindow*>(window());
                if (mw) mw->downloadAndInstallExtension(extId, store);
            }
            return;
        }

        const QString host = url.host();

        // Spoof user agent for extension stores
        if (host.contains("chrome.google.com") ||
            host.contains("chromewebstore.google.com") ||
            host.contains("microsoftedge.microsoft.com") ||
            host.contains("addons.opera.com"))
        {
            page()->profile()->setHttpUserAgent(
                "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 "
                "(KHTML, like Gecko) Chrome/136.0.0.0 Safari/537.36"
            );
        } else {
            page()->profile()->setHttpUserAgent(
                "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 "
                "(KHTML, like Gecko) VibiBrowser/1.1.0 Chrome/136.0.0.0 Safari/537.36"
            );
        }
    });

    connect(this, &QWebEngineView::loadFinished, this, [this](bool ok){
        if(!ok) return;
        const QString host = url().host();
        if (host.contains("chromewebstore.google.com") ||
            host.contains("chrome.google.com") ||
            host.contains("microsoftedge.microsoft.com") ||
            host.contains("addons.opera.com"))
        {
            injectStoreButton();
        }
    });
}

void WebView::injectStoreButton()
{
    const QString js = R"JS(
(function(){
    if(document.getElementById('vibi-add-btn')) return;

    const selectors = [
        'div[class*="UywwFc"]',
        'button[class*="UywwFc"]',
        '.f1atmd',
        'div[class*="dd-Va"]',
        '.webstore-test-button-label',
        'div[jsname="XOBBBb"]',
        '.g-c-Pb button',
        'div[class*="nJBykc"]'
    ];

    let target = null;
    for(const sel of selectors){
        target = document.querySelector(sel);
        if(target) break;
    }

    if(!target){
        const btns = document.querySelectorAll('button,div[role="button"]');
        for(const b of btns){
            const t = b.textContent.trim();
            if(t === 'Add to Chrome' || t === 'Get' || t === 'Add to Edge'){
                target = b.parentElement;
                break;
            }
        }
    }

    const btn = document.createElement('button');
    btn.id = 'vibi-add-btn';
    btn.textContent = '➕ Add to VibiBrowser';
    btn.style.cssText = `
        background: linear-gradient(135deg, #7B5CF0, #5A3FA0);
        color: white;
        border: none;
        border-radius: 24px;
        padding: 10px 20px;
        font-size: 14px;
        font-weight: 600;
        cursor: pointer;
        margin: 8px 0;
        box-shadow: 0 4px 15px rgba(123,92,240,0.4);
        font-family: 'Google Sans', sans-serif;
        transition: all 0.2s;
        display: block;
        width: 100%;
    `;
    btn.onmouseover = () => btn.style.transform = 'scale(1.03)';
    btn.onmouseout  = () => btn.style.transform = 'scale(1)';

    btn.onclick = () => {
        const url = window.location.href;
        let extId = null;

        const cwsMatch = url.match(/\/detail\/[^\/]+\/([a-z]{32})/);
        if(cwsMatch) extId = cwsMatch[1];

        const edgeMatch = url.match(/\/detail\/([a-z]{32})/);
        if(!extId && edgeMatch) extId = edgeMatch[1];

        if(!extId){
            alert('Could not detect extension ID. Please make sure you are on an extension page.');
            return;
        }

        btn.textContent = '⏳ Installing...';
        btn.disabled = true;

        window.vibi_install_extension(extId, window.location.hostname);
    };

    if(target){
        target.parentElement.insertBefore(btn, target);
    } else {
        btn.style.cssText += 'position:fixed;top:80px;right:20px;width:auto;z-index:99999;';
        document.body.appendChild(btn);
    }
})();
    )JS";

    page()->runJavaScript(js);
}

void WebView::setupExtensionBridge()
{
    QWebEngineScript script;
    script.setName("vibi_bridge");
    script.setInjectionPoint(QWebEngineScript::DocumentCreation);
    script.setWorldId(QWebEngineScript::MainWorld);
    script.setSourceCode(R"JS(
        window.vibi_install_extension = function(extId, storeHost) {
            window.location.href = 'vibi://install-extension/' + extId + '?store=' + storeHost;
        };
    )JS");
    page()->scripts().insert(script);
}

void WebView::load(const QUrl &url)
{
    setupExtensionBridge();

    if (url.toString().startsWith("vibi://install-extension/"))
    {
        QString extId = url.path().section('/', 1, 1);
        QString store = url.query().remove("store=");
        if (!extId.isEmpty()) {
            auto *mw = qobject_cast<MainWindow*>(window());
            if (mw) mw->downloadAndInstallExtension(extId, store);
        }
        return;
    }

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