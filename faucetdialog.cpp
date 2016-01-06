#include <QWebView>
#include <QWebFrame>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QWebElement>

#include "profile.hpp"
#include "primedice.hpp"

#include "faucetdialog.hpp"
#include "ui_faucetdialog.h"

#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include "profilewidget.hpp"
#include "ui_profilewidget.h"

QString recaptcha = "<html><head><title>Collect Faucet</title></head><body><div class=\"g-recaptcha\" data-sitekey=\"6LeX6AcTAAAAAMwAON0oEyRDoTbusREfJa2vxDMh\"></div><script src=\"https://www.google.com/recaptcha/api.js\" asyncr></script></body></html>";

FaucetDialog::FaucetDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FaucetDialog)
{
    ui->setupUi(this);

    ProfileWidget* profileWidget = (ProfileWidget*)parentWidget();

    connect(ui->webView->page()->networkAccessManager(),
            SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyReceived(QNetworkReply*)));
    connect(&profileWidget->GetRestAPI(),
            SIGNAL(FaucetHandler(QNetworkReply*)),
            this, SLOT(onFaucet(QNetworkReply*)));

    QWebSettings *settings = ui->webView->page()->settings();
    //settings->setAttribute(QWebSettings::JavascriptEnabled, true);
    //settings->setAttribute(QWebSettings::PrivateBrowsingEnabled, true);
    //settings->setAttribute(QWebSettings::LocalStorageEnabled, true);

    ui->webView->page()->action(QWebPage::Reload)->setVisible(false);
}

FaucetDialog::~FaucetDialog() {
    delete ui;
}

void FaucetDialog::Refresh() {
    ui->webView->setHtml(recaptcha, QUrl("https://primedice.com/play"));
}

void FaucetDialog::replyReceived(QNetworkReply* reply) {
    if(reply->request().url().path().endsWith("userverify")) {
        reply->write(reply->readAll());
    }
}

void FaucetDialog::on_buttonBox_accepted() {
    QString captchaResponse = ui->webView->page()->mainFrame()->evaluateJavaScript("grecaptcha.getResponse()").toString();
    //qDebug() << captchaResponse;
    if(captchaResponse == "") {
        QMessageBox msgBox;
        msgBox.setText("No response from the ReCaptcha service.");
        msgBox.exec();
        return;
    }

    MainWindow* win = (MainWindow*)topLevelWidget()->parentWidget()->topLevelWidget();
    ProfileWidget* profileWidget = (ProfileWidget*)parentWidget();

    Profile* profile = win->GetProfileManager().GetProfile(profileWidget->GetUsername());

    QNetworkReply* reply = profileWidget->GetRestAPI().Faucet(*profile, captchaResponse);
    hide();
}

void FaucetDialog::on_buttonBox_rejected() {
    //qDebug() << ui->webView->page()->mainFrame()->toHtml();
    hide();
}

void FaucetDialog::onFaucet(QNetworkReply* reply) {
    ProfileWidget* profileWidget = (ProfileWidget*)parentWidget();
    QString res = reply->readAll();
    qDebug() << res;
    QJsonDocument jsonRes = QJsonDocument::fromJson(res.toLocal8Bit());
    QJsonObject jsonObj = jsonRes.object();
    double balance = floor(jsonObj["balance"].toString().toDouble());
    if(balance > 1.0) {
        profileWidget->GetUi()->balanceLine->setText(QString::number(balance/1e8, 'f', 8));
        profileWidget->ResetFaucetTimer();
    } else {
        QMessageBox msgBox;
        msgBox.setText(res);
        msgBox.exec();
    }
}
