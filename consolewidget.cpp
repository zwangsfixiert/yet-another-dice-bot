#include <QWidget>
#include <QScrollBar>

#include "consolewidget.hpp"
#include "ui_consolewidget.h"
#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "profilewidget.hpp"
#include "ui_profilewidget.h"
#include "usermenuframe.hpp"
#include "ui_usermenuframe.h"
#include "userinfoframe.hpp"
#include "ui_userinfoframe.h"

ConsoleWidget::ConsoleWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConsoleWidget)
{
    ui->setupUi(this);
    ui->consoleOutput->setOpenLinks(false);
    userMenu = new UserMenuFrame(this);
}

ConsoleWidget::~ConsoleWidget() {
    delete ui;
    delete userMenu;
}

void ConsoleWidget::AddLine(const ConsoleWidget* console, const QString &text) {
    qDebug() << "addline" << console << text;

    if(console == this) {
        ui->consoleOutput->moveCursor(QTextCursor::End);
        ui->consoleOutput->append(text);
    }
}

void ConsoleWidget::onTip(QNetworkReply* reply) {
    QString tipres = reply->readAll();
    QJsonDocument jsonRes = QJsonDocument::fromJson(tipres.toLocal8Bit());
    QJsonObject jsonObj = jsonRes.object();
    QJsonObject userObj = jsonObj["user"].toObject();
    double balance = floor(userObj["balance"].toDouble())/1e8;
    ProfileWidget* profileWidget = (ProfileWidget*)parent()->parent()->parent();
    profileWidget->GetUi()->balanceLine->setText(QString::number(balance, 'f', 8));
}

void ConsoleWidget::on_consoleOutput_textChanged() {
    auto scrollbar = ui->consoleOutput->verticalScrollBar();
    scrollbar->setValue(scrollbar->maximum());
}

void ConsoleWidget::on_consoleInput_returnPressed() {
    auto win = (MainWindow*)topLevelWidget();
    ProfileWidget* profileWidget = (ProfileWidget*)parent()->parent()->parent();
    auto profile = win->GetProfileManager().GetProfile(profileWidget->GetUsername());

    if (ui->consoleInput->text().startsWith("/bet", Qt::CaseInsensitive)) {

        QString target = ui->consoleInput->text().section(" ", 1, 1);
        BetInfoFrame* betInfo = profileWidget->GetBetInfoFrame();
        betInfo->Display(target);
    } else if (ui->consoleInput->text().startsWith("/user", Qt::CaseInsensitive)) {
        /*
         * "{"user":{
         *      "id":"566176","userid":"566176","username":"UnixPunk",
         *      "registered":"2015-04-08T18:40:23.714Z","wagered":955164234,
         *      "profit":65983552.5247,"bets":59700,"wins":11592,"losses":48108,
         *      "win_risk":75466580,"lose_risk":75813083,"messages":18948}}"
         */
        QString target = ui->consoleInput->text().section(" ", 1, 1);
        UserInfoFrame* userInfo = profileWidget->GetUserInfoFrame();
        userInfo->Display(target);
    } else if (ui->consoleInput->text().startsWith("/tip", Qt::CaseInsensitive)) {
        QString target = ui->consoleInput->text().section(" ", 1, 1);
        QString stramount = ui->consoleInput->text().section(" ", 2, 2);
        double amount = stramount.toDouble();
        qDebug() << target << amount << (int64_t)(amount*1e8);
        if(amount >= 0.00050001 && amount <= profileWidget->GetUi()->balanceLine->text().toDouble()) {
            profileWidget->GetRestAPI().TipUser(*profile, target, (int64_t)(round(amount*1e8)));
        }
    } else if(profile->chatenabled) {
        // TODO enable russian chat
        if(ui->consoleInput->text().startsWith("/msg", Qt::CaseInsensitive) ||
                ui->consoleInput->text().startsWith("/pm", Qt::CaseInsensitive)) {
            QString cmd = ui->consoleInput->text().section(" ", 0, 0);
            QString target = ui->consoleInput->text().section(" ", 1, 1);
            QString msg = ui->consoleInput->text().mid(cmd.length() + 1 + target.length() + 1);

            profileWidget->GetRestAPI().SendMessage(*profile, "English", msg, target);
        } else {
            profileWidget->GetRestAPI().SendMessage(*profile, "English", ui->consoleInput->text(), "");
        }
    }

    ui->consoleInput->setText("");
}

void ConsoleWidget::on_consoleOutput_anchorClicked(const QUrl &arg1)
{
    //qDebug() << arg1;

    QString uri = arg1.toString().section(":", 0, 0);
    QString cmd = arg1.toString().section("/", 3, 3);
    QString arg = arg1.toString().section("/", 4, 4);
    //qDebug() << uri << cmd << arg;

    ProfileWidget* profilewidget = (ProfileWidget*)parent()->parent()->parent();

    if(uri == "yadb") {
        if(cmd == "userMenu") {
            userMenu->Display(arg);
        } else if(cmd == "betInfo") {
            profilewidget->GetBetInfoFrame()->Display(arg);
        } else if(cmd == "userInfo") {
            profilewidget->GetUserInfoFrame()->Display(arg);
        }
    }
    else if(uri == "http" || uri == "https") {
        QDesktopServices::openUrl(arg1);
    }
}
