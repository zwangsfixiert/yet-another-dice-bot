#ifndef WEBVIEW_HPP
#define WEBVIEW_HPP

#include <QWebView>

namespace Ui {
class WebView;
}

class WebView : public QWebView
{
    Q_OBJECT

public:
    explicit WebView(QWidget *parent = 0);
    ~WebView();

private slots:
    void onLoadFinished(bool finished);

private:
    Ui::WebView *ui;
};

#endif // WEBVIEW_HPP
