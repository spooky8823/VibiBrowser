#include "mainwindow.h"
#include "webview.h"
#include "downloadmanager.h"
#include "historymanager.h"
#include <QApplication>
#include <QWebEngineProfile>
#include <QWebEngineDownloadRequest>
#include <QUrl>
#include <QRegularExpression>
#include <QProcess>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_settings("Vibifiy", "VibiBrowser")
{
    setWindowTitle("VibiBrowser");
    setMinimumSize(900, 600);
    resize(1280, 800);

    // Frameless window
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground, true);

    m_dlManager   = new DownloadManager(this);
    m_histManager = new HistoryManager(this);

    // Handle downloads from profile
    connect(QWebEngineProfile::defaultProfile(),
            &QWebEngineProfile::downloadRequested,
            m_dlManager, &DownloadManager::handleDownload);

    setupUI();
    applyTheme();

    m_darkMode = m_settings.value("darkMode", false).toBool();
    if (m_darkMode) toggleDarkMode();

    newTab();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI()
{
    m_central = new QWidget(this);
    setCentralWidget(m_central);
    m_layout = new QVBoxLayout(m_central);
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0,0,0,0);

    setupTitleBar();
    setupTabBar();
    setupToolBar();

    m_stack = new QStackedWidget;
    m_layout->addWidget(m_stack);
}

void MainWindow::setupTitleBar()
{
    m_titleBar = new QWidget;
    m_titleBar->setFixedHeight(40);
    m_titleBar->setObjectName("titleBar");
    m_titleLayout = new QHBoxLayout(m_titleBar);
    m_titleLayout->setContentsMargins(8,0,8,0);
    m_titleLayout->setSpacing(4);

    m_logo = new QLabel("● VibiBrowser");
    m_logo->setObjectName("logoLabel");

    // Tab bar in title bar
    m_tabBar = new QTabBar;
    m_tabBar->setObjectName("tabBar");
    m_tabBar->setTabsClosable(true);
    m_tabBar->setMovable(true);
    m_tabBar->setExpanding(false);
    m_tabBar->setElideMode(Qt::ElideRight);
    connect(m_tabBar, &QTabBar::currentChanged, this, &MainWindow::switchTab);
    connect(m_tabBar, &QTabBar::tabCloseRequested, this, &MainWindow::closeTab);

    m_newTabBtn = new QPushButton("+");
    m_newTabBtn->setObjectName("newTabBtn");
    m_newTabBtn->setFixedSize(26,26);
    connect(m_newTabBtn, &QPushButton::clicked, this, [this]{ newTab(); });

    // Window controls
    auto *minBtn  = new QPushButton("─");
    auto *maxBtn  = new QPushButton("□");
    auto *closeBtn= new QPushButton("✕");
    minBtn->setObjectName("winBtn");
    maxBtn->setObjectName("winBtn");
    closeBtn->setObjectName("winBtnClose");
    for (auto *b : {minBtn, maxBtn, closeBtn}) b->setFixedSize(28,28);

    connect(minBtn,   &QPushButton::clicked, this, &QMainWindow::showMinimized);
    connect(maxBtn,   &QPushButton::clicked, this, [this]{
        isMaximized() ? showNormal() : showMaximized();
    });
    connect(closeBtn, &QPushButton::clicked, this, &QMainWindow::close);

    m_titleLayout->addWidget(m_logo);
    m_titleLayout->addWidget(m_tabBar, 1);
    m_titleLayout->addWidget(m_newTabBtn);
    m_titleLayout->addSpacing(8);
    m_titleLayout->addWidget(minBtn);
    m_titleLayout->addWidget(maxBtn);
    m_titleLayout->addWidget(closeBtn);

    m_layout->addWidget(m_titleBar);

    // Drag to move window
    m_titleBar->installEventFilter(this);
}

void MainWindow::setupToolBar()
{
    m_toolBar = new QWidget;
    m_toolBar->setObjectName("toolBar");
    m_toolBar->setFixedHeight(42);
    auto *tl = new QHBoxLayout(m_toolBar);
    tl->setContentsMargins(8,0,8,0);
    tl->setSpacing(4);

    m_backBtn   = new QPushButton("←");
    m_fwdBtn    = new QPushButton("→");
    m_reloadBtn = new QPushButton("↺");
    for (auto *b : {m_backBtn, m_fwdBtn, m_reloadBtn}) {
        b->setObjectName("toolBtn");
        b->setFixedSize(28,28);
    }

    m_urlBar = new QLineEdit;
    m_urlBar->setObjectName("urlBar");
    m_urlBar->setPlaceholderText("Search or enter address...");
    connect(m_urlBar, &QLineEdit::returnPressed, this, [this]{
        navigateTo(m_urlBar->text());
    });

    m_progress = new QProgressBar;
    m_progress->setObjectName("loadBar");
    m_progress->setFixedHeight(2);
    m_progress->setTextVisible(false);
    m_progress->setRange(0,100);
    m_progress->setValue(0);
    m_progress->hide();

    // Menu button
    auto *menuBtn = new QPushButton("⋮");
    menuBtn->setObjectName("toolBtn");
    menuBtn->setFixedSize(28,28);

    connect(m_backBtn,   &QPushButton::clicked, this, &MainWindow::goBack);
    connect(m_fwdBtn,    &QPushButton::clicked, this, &MainWindow::goForward);
    connect(m_reloadBtn, &QPushButton::clicked, this, &MainWindow::reload);

    tl->addWidget(m_backBtn);
    tl->addWidget(m_fwdBtn);
    tl->addWidget(m_reloadBtn);
    tl->addWidget(m_urlBar, 1);
    tl->addWidget(menuBtn);

    m_layout->addWidget(m_toolBar);
    m_layout->addWidget(m_progress);
}

void MainWindow::setupTabBar() {} // tab bar is inside title bar

void MainWindow::newTab(const QUrl &url)
{
    auto *wv = new WebView(this);
    wv->setHistoryManager(m_histManager);

    connect(wv, &WebView::urlChanged,    this, &MainWindow::onUrlChanged);
    connect(wv, &WebView::titleChanged,  this, &MainWindow::onTitleChanged);
    connect(wv, &WebView::loadProgress,  this, &MainWindow::onLoadProgress);
    connect(wv, &WebView::loadFinished,  this, &MainWindow::onLoadFinished);

    int idx = m_stack->addWidget(wv);
    int tabIdx = m_tabBar->addTab("New Tab");
    m_tabBar->setCurrentIndex(tabIdx);
    m_stack->setCurrentIndex(idx);

    wv->load(url.isEmpty() ? QUrl("vibi://newtab") : url);
}

void MainWindow::closeTab(int index)
{
    if (m_tabBar->count() <= 1) { close(); return; }
    QWidget *w = m_stack->widget(index);
    m_stack->removeWidget(w);
    w->deleteLater();
    m_tabBar->removeTab(index);
}

void MainWindow::switchTab(int index)
{
    if (index < 0 || index >= m_stack->count()) return;
    m_stack->setCurrentIndex(index);
    auto *wv = currentWebView();
    if (wv) m_urlBar->setText(wv->url().toString());
}

void MainWindow::navigateTo(const QString &input)
{
    auto *wv = currentWebView();
    if (!wv) return;
    wv->load(QUrl(resolveUrl(input)));
}

QString MainWindow::resolveUrl(const QString &input)
{
    if (input.startsWith("vibi://") ||
        input.startsWith("http://") ||
        input.startsWith("https://") ||
        input.startsWith("file://"))
        return input;

    // Looks like a domain?
    static QRegularExpression domainRe(R"(^[a-zA-Z0-9\-\.]+\.[a-zA-Z]{2,}(/.*)?$)");
    if (domainRe.match(input).hasMatch())
        return "https://" + input;

    // Search
    return "https://www.google.com/search?q=" + QUrl::toPercentEncoding(input);
}

void MainWindow::onUrlChanged(const QUrl &url)
{
    if (sender() == currentWebView())
        m_urlBar->setText(url.toString() == "vibi://newtab" ? "" : url.toString());
}

void MainWindow::onTitleChanged(const QString &title)
{
    int idx = m_stack->indexOf(qobject_cast<QWidget*>(sender()));
    if (idx >= 0) m_tabBar->setTabText(idx, title.isEmpty() ? "New Tab" : title);
}

void MainWindow::onLoadProgress(int progress)
{
    if (sender() != currentWebView()) return;
    if (progress < 100) { m_progress->show(); m_progress->setValue(progress); }
    else                { m_progress->hide(); m_progress->setValue(0); }
}

void MainWindow::onLoadFinished(bool) { m_progress->hide(); }

void MainWindow::goBack()    { if (auto *w = currentWebView()) w->back(); }
void MainWindow::goForward() { if (auto *w = currentWebView()) w->forward(); }
void MainWindow::reload()    { if (auto *w = currentWebView()) w->reload(); }

WebView* MainWindow::currentWebView()
{
    return qobject_cast<WebView*>(m_stack->currentWidget());
}

void MainWindow::toggleDarkMode()
{
    m_darkMode = !m_darkMode;
    m_settings.setValue("darkMode", m_darkMode);
    applyTheme();
}

void MainWindow::applyTheme()
{
    QString accent   = m_settings.value("accentColor", "#7B5CF0").toString();
    QString accentDk = m_settings.value("accentDark",  "#5A3FA0").toString();

    QString bg, sf, tx, br, tb;
    if (m_darkMode) {
        bg="#1A1035"; sf="#221545"; tx="#EDE9FE"; br="rgba(167,139,250,0.2)"; tb="#2C1C58";
    } else {
        bg="#FAFAFE"; sf="#F4F1FF"; tx="#1A1A2E"; br="rgba(123,92,240,0.15)"; tb="#EDE8FF";
    }

    setStyleSheet(QString(R"(
        QMainWindow, QWidget#centralWidget { background: %1; }
        QWidget#titleBar  { background: %6; }
        QWidget#toolBar   { background: %2; border-bottom: 1px solid %5; }
        QLabel#logoLabel  { font-family: 'Outfit', sans-serif; font-weight: 700;
                            font-size: 13px; color: #fff; padding-left: 4px; }
        QTabBar::tab      { background: %6; color: rgba(255,255,255,0.75);
                            border: 1px solid rgba(255,255,255,0.2);
                            border-bottom: none; border-radius: 6px 6px 0 0;
                            padding: 4px 10px; min-width: 80px; max-width: 200px; }
        QTabBar::tab:selected { background: %4; color: %3; }
        QPushButton#newTabBtn { border-radius: 13px; border: 1px solid rgba(255,255,255,0.35);
                                background: rgba(255,255,255,0.12); color: #fff; font-size: 16px; }
        QPushButton#newTabBtn:hover { background: rgba(255,255,255,0.25); }
        QPushButton#winBtn  { border-radius: 4px; border: none;
                              background: transparent; color: rgba(255,255,255,0.8); font-size: 11px; }
        QPushButton#winBtn:hover { background: rgba(255,255,255,0.15); }
        QPushButton#winBtnClose { border-radius: 4px; border: none;
                                  background: transparent; color: rgba(255,255,255,0.8); font-size: 11px; }
        QPushButton#winBtnClose:hover { background: #e81123; color: #fff; }
        QPushButton#toolBtn { border-radius: 14px; border: none;
                              background: transparent; color: %3; font-size: 13px; }
        QPushButton#toolBtn:hover { background: %4; color: %2; }
        QLineEdit#urlBar  { border-radius: 14px; border: 1px solid %5;
                            background: %4; padding: 0 10px; font-size: 12px; color: %3; }
        QLineEdit#urlBar:focus { border-color: %2; }
        QProgressBar#loadBar { background: transparent; border: none; }
        QProgressBar#loadBar::chunk { background: %2; }
    )")
    .arg(bg)        // %1 main bg
    .arg(accent)    // %2 accent
    .arg(tx)        // %3 text
    .arg(sf)        // %4 surface
    .arg(br)        // %5 border
    .arg(accentDk)  // %6 title bar
    );
}

void MainWindow::installExtension(const QString &crxPath)
{
    // Unpack CRX into extensions folder
    QString extId = QFileInfo(crxPath).baseName();
    QString destPath = QDir::homePath() + "/.config/VibiBrowser/extensions/" + extId;
    QDir().mkpath(destPath);

    // CRX3 format: skip header and unzip the rest
    QFile crx(crxPath);
    if (!crx.open(QIODevice::ReadOnly)) return;

    QByteArray data = crx.readAll();
    crx.close();

    // Find the ZIP magic bytes PK\x03\x04 in the CRX
    int zipStart = data.indexOf("\x50\x4B\x03\x04");
    if (zipStart < 0) return;

    QString zipPath = destPath + "/ext.zip";
    QFile zip(zipPath);
    if (!zip.open(QIODevice::WriteOnly)) return;
    zip.write(data.mid(zipStart));
    zip.close();

    // Unzip using system unzip
    QProcess::execute("unzip", {"-o", zipPath, "-d", destPath});
    QFile::remove(zipPath);

    // Tell user to restart
    QMessageBox::information(this, "Extension Installed",
        "Extension installed! Please restart VibiBrowser to activate it.");
}

void MainWindow::openSettings()  { /* TODO: open settings page */ }
void MainWindow::openHistory()   { /* TODO: open history page  */ }
void MainWindow::openDownloads() { m_dlManager->showPanel(); }
