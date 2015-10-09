#include "webview.hpp"

WebView::WebView(QWidget *parent) :
    QWebView(parent)
{
    qDebug() << "webview";
}

WebView::~WebView() {
}

void WebView::onLoadFinished(bool finished) {
    if(finished) {

    } else {

    }
}
