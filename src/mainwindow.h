#pragma once
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabBar>
#include <QLineEdit>
#include <QToolBar>
#include <QStatusBar>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QWebEngineView>
#include <QProgressBar>
#include <QSettings>

class WebView;
class DownloadManager;
class HistoryManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void newTab(const QUrl &url = QUrl("vibi://newtab"));
    void closeTab(int index);
    void switchTab(int index);
    void navigateTo(const QString &input);
    void onUrlChanged(const QUrl &url);
    void onTitleChanged(const QString &title);
    void onLoadProgress(int progress);
    void onLoadFinished(bool ok);
    void goBack();
    void goForward();
    void reload();
    void toggleDarkMode();
    void openSettings();
    void openHistory();
    void openDownloads();

private:
    void setupUI();
    void setupTitleBar();
    void setupTabBar();
    void setupToolBar();
    void applyTheme();
    WebView* currentWebView();
    QString resolveUrl(const QString &input);

    // Layout
    QWidget     *m_central;
    QVBoxLayout *m_layout;

    // Title bar (frameless)
    QWidget     *m_titleBar;
    QHBoxLayout *m_titleLayout;
    QLabel      *m_logo;

    // Tab bar
    QTabBar     *m_tabBar;
    QPushButton *m_newTabBtn;

    // Toolbar
    QWidget     *m_toolBar;
    QLineEdit   *m_urlBar;
    QPushButton *m_backBtn, *m_fwdBtn, *m_reloadBtn;
    QProgressBar *m_progress;

    // Content area
    QStackedWidget *m_stack;

    // State
    bool m_darkMode = false;
    QSettings m_settings;

    // Managers
    DownloadManager *m_dlManager;
    HistoryManager  *m_histManager;
};
