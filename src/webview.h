#pragma once
#include <QWebEngineView>
#include <QWebEnginePage>
class HistoryManager;

class WebView : public QWebEngineView
{
    Q_OBJECT
public:
    explicit WebView(QWidget *parent = nullptr);
    void setHistoryManager(HistoryManager *hm) { m_history = hm; }
    void load(const QUrl &url);

protected:
    QWebEngineView *createWindow(QWebEnginePage::WebWindowType type) override;

private:
    HistoryManager *m_history = nullptr;
};
