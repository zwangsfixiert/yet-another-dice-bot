#include <QWebView>
#include <QWebFrame>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QWebElement>

#include "faucetdialog.hpp"
#include "ui_faucetdialog.h"

#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include "profilewidget.hpp"
#include "ui_profilewidget.h"
#include "profile.hpp"
#include "primedice.hpp"

QString recaptcha = "<html><head><title>Collect Faucet</title><script src=\"https://www.google.com/recaptcha/api.js\" async defer></script></head><body><div class=\"g-recaptcha\" data-sitekey=\"6LeX6AcTAAAAAMwAON0oEyRDoTbusREfJa2vxDMh\"></div></body></html>";

FaucetDialog::FaucetDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FaucetDialog)
{
    ui->setupUi(this);

    connect(ui->webView->page()->networkAccessManager(),
            SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyReceived(QNetworkReply*)));

    QWebSettings *settings = ui->webView->page()->settings();
    settings->setAttribute(QWebSettings::PrivateBrowsingEnabled , true);
    settings->setAttribute(QWebSettings::LocalStorageEnabled, true);

    ui->webView->setHtml(recaptcha, QUrl("https://primedice.com/"));

    ui->webView->show();
    ui->webView->page()->action(QWebPage::Reload)->setVisible(false);
}

FaucetDialog::~FaucetDialog() {
    delete ui;
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

    QString res = win->GetRestAPI().Faucet(*profile, captchaResponse);

    QJsonDocument jsonRes = QJsonDocument::fromJson(res.toLocal8Bit());
    QJsonObject jsonObj = jsonRes.object();
    double balance = floor(jsonObj["balance"].toString().toDouble());
    if(balance > 1.0) {
        profileWidget->GetUi()->balanceLine->setText(QString::number(balance/1e8, 'f', 8));
        profileWidget->ResetFaucetTimer();
        destroy();
    } else {
        QMessageBox msgBox;
        msgBox.setText(res);
        msgBox.exec();
    }
}

void FaucetDialog::on_buttonBox_rejected() {
    //qDebug() << ui->webView->page()->mainFrame()->toHtml();
    destroy();
}
