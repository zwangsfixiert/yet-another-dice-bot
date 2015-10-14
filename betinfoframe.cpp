#include "betinfoframe.hpp"
#include "ui_betinfoframe.h"

#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "profilewidget.hpp"
#include "ui_profilewidget.h"

BetInfoFrame::BetInfoFrame(QWidget *parent) :
        QFrame(parent, Qt::Tool|Qt::Window|Qt::FramelessWindowHint),
        ui(new Ui::BetInfoFrame)
{
    ui->setupUi(this);
}

BetInfoFrame::~BetInfoFrame() {
    delete ui;
}

void BetInfoFrame::Display(QString betId) {
    betId.replace(',', "");
    qDebug() << betId.toULongLong();

    MainWindow* win = (MainWindow*)parentWidget()->topLevelWidget();
    ProfileWidget* profileWidget = (ProfileWidget*)win->GetUi()->tabWidget->currentWidget();
    QString profilename = profileWidget->GetUi()->usernameEdit->text();
    Profile* profile = win->GetProfileManager().GetProfile(profilename);
    QString res = win->GetRestAPI().GetBetInfo(*profile, betId);

    /*
     "{"bet":{
        "target":99.98,"player_id":529361,
        "server":"23b826b9ce2b3....",
        "player":"xxxx","nonce":94623,"jackpot":0,"profit":-636,
        "id":7013787533,"condition":">","multiplier":9900,
        "client":"wC0b3DKgXKL2zZteTb2HG3xJz2TC8B","roll":72.7,
        "timestamp":"Tue Oct 13 2015 03:50:55 GMT+0000 (UTC)",
        "amount":636,"win":0}}"
       */

    QJsonDocument jsonRes = QJsonDocument::fromJson(res.toLocal8Bit());
    QJsonObject jsonObj = jsonRes.object();
    QJsonObject betObj = jsonObj["bet"].toObject();
    QString id = QString::number(qulonglong(betObj["id"].toDouble()));
    QString target = QString::number(betObj["target"].toDouble(), 'f', 2);
    QString roll = QString::number(betObj["roll"].toDouble(), 'f', 2);
    QString condition = betObj["condition"].toString();
    QString wager = QString::number(betObj["amount"].toDouble()/1e8, 'f', 8);
    QString profit = QString::number(betObj["profit"].toDouble()/1e8, 'f', 8);
    QString serverHash = betObj["server"].toString();
    QString multiplier = QString::number(betObj["multiplier"].toDouble(), 'f', 5);
    QString user = betObj["player"].toString();

    ui->betInfoLabel->setText("Bet #" + id);
    ui->username->setText(user);
    ui->target->setText(condition + target);
    ui->roll->setText(roll);
    ui->wager->setText(wager);
    ui->profit->setText(profit);
    ui->serverSeedHash->setText(serverHash);
    ui->multiplier->setText(multiplier + "x");

    QRect topLevelGeom = win->geometry();
    QSize size = this->size();
    setGeometry(topLevelGeom.center().x()-size.width()/2,
                          topLevelGeom.center().y()-size.height()/2,
                          size.width(), size.height());
    show();
}

void BetInfoFrame::on_closeButton_clicked() {
    hide();
}
