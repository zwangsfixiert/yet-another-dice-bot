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

void ConsoleWidget::on_consoleOutput_textChanged() {
    auto scrollbar = ui->consoleOutput->verticalScrollBar();
    scrollbar->setValue(scrollbar->maximum());
}

void ConsoleWidget::on_consoleInput_returnPressed() {
    auto win = (MainWindow*)topLevelWidget();
    ProfileWidget* profilewidget = (ProfileWidget*)parent()->parent()->parent();
    auto profile = win->GetProfileManager().GetProfile(profilewidget->GetUsername());

    // FIXME implement proper command parsing

    if (ui->consoleInput->text().startsWith("/bet", Qt::CaseInsensitive)) {

        QString target = ui->consoleInput->text().section(" ", 1, 1);
        BetInfoFrame* betInfo = profilewidget->GetBetInfoFrame();
        betInfo->Display(target);

        ui->consoleInput->setText("");
        return;

    } else if (ui->consoleInput->text().startsWith("/user", Qt::CaseInsensitive)) {
        /*
         * "{"user":{
         *      "id":"566176","userid":"566176","username":"UnixPunk",
         *      "registered":"2015-04-08T18:40:23.714Z","wagered":955164234,
         *      "profit":65983552.5247,"bets":59700,"wins":11592,"losses":48108,
         *      "win_risk":75466580,"lose_risk":75813083,"messages":18948}}"
         */
        QString target = ui->consoleInput->text().section(" ", 1, 1);
        UserInfoFrame* userInfo = profilewidget->GetUserInfoFrame();
        userInfo->Display(target);

        ui->consoleInput->setText("");
        return;

    } else if (ui->consoleInput->text().startsWith("/tip", Qt::CaseInsensitive)) {
        QString target = ui->consoleInput->text().section(" ", 1, 1);
        QString stramount = ui->consoleInput->text().section(" ", 2, 2);
        double amount = stramount.toDouble();
        qDebug() << target << amount << (int64_t)(amount*1e8);
        if(amount >= 0.00050001 && amount <= profilewidget->GetUi()->balanceLine->text().toDouble()) {
            QString tipres = win->GetRestAPI().TipUser(*profile, target, (int64_t)(round(amount*1e8)));
            QJsonDocument jsonRes = QJsonDocument::fromJson(tipres.toLocal8Bit());
            QJsonObject jsonObj = jsonRes.object();
            QJsonObject userObj = jsonObj["user"].toObject();
            double balance = floor(userObj["balance"].toDouble())/1e8;
            profilewidget->GetUi()->balanceLine->setText(QString::number(balance, 'f', 8));
        }
        ui->consoleInput->setText("");
        return;
    }

    if(profile->chatenabled) {
        // TODO enable russian chat
        if(ui->consoleInput->text().startsWith("/msg", Qt::CaseInsensitive) ||
                ui->consoleInput->text().startsWith("/pm", Qt::CaseInsensitive)) {
            QString cmd = ui->consoleInput->text().section(" ", 0, 0);
            QString target = ui->consoleInput->text().section(" ", 1, 1);
            QString msg = ui->consoleInput->text().mid(cmd.length() + 1 + target.length() + 1);

            win->GetRestAPI().SendMessage(*profile, "English", msg, target);
            ui->consoleInput->setText("");
            return;
        } else {
            win->GetRestAPI().SendMessage(*profile, "English", ui->consoleInput->text(), "");
            ui->consoleInput->setText("");
            return;
        }
    }

    ui->consoleInput->setText("");
}

void ConsoleWidget::on_consoleOutput_anchorClicked(const QUrl &arg1)
{
    qDebug() << arg1;

    QString uri = arg1.toString().section(":", 0, 0);
    QString cmd = arg1.toString().section("/", 3, 3);
    QString arg = arg1.toString().section("/", 4, 4);
    qDebug() << uri << cmd << arg;

    if(uri == "yadb") {
        if(cmd == "userMenu") {
            userMenu->Display(arg);
        } else if(cmd == "betInfo") {
            ProfileWidget* profilewidget = (ProfileWidget*)parent()->parent()->parent();
            profilewidget->GetBetInfoFrame()->Display(arg);
        } else if(cmd == "userInfo") {
            ProfileWidget* profilewidget = (ProfileWidget*)parent()->parent()->parent();
            profilewidget->GetUserInfoFrame()->Display(arg);
        }
    }
    else if(uri == "http" || uri == "https") {
        QDesktopServices::openUrl(arg1);
    }
}
