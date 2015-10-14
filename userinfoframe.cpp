#include "userinfoframe.hpp"
#include "ui_userinfoframe.h"
#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "profilewidget.hpp"
#include "ui_profilewidget.h"

UserInfoFrame::UserInfoFrame(QWidget *parent) :
    QFrame(parent, Qt::Tool|Qt::Window|Qt::FramelessWindowHint),
    ui(new Ui::UserInfoFrame)
{
    ui->setupUi(this);
}

UserInfoFrame::~UserInfoFrame()
{
    delete ui;
}

void UserInfoFrame::Display(QString username) {
    MainWindow* win = (MainWindow*)parentWidget()->topLevelWidget();
    ProfileWidget* profileWidget = (ProfileWidget*)win->GetUi()->tabWidget->currentWidget();
    QString profilename = profileWidget->GetUi()->usernameEdit->text();
    Profile* profile = win->GetProfileManager().GetProfile(profilename);
    QString res = win->GetRestAPI().GetUserInfo(*profile, username);

    QJsonDocument jsonRes = QJsonDocument::fromJson(res.toLocal8Bit());
    QJsonObject jsonObj = jsonRes.object();
    QJsonObject userObj = jsonObj["user"].toObject();
    QString user = userObj["username"].toString();
    QString wagered = QString::number(userObj["wagered"].toDouble()/1e8, 'f', 8);
    QString profit = QString::number(userObj["profit"].toDouble()/1e8, 'f', 8);
    QString messages = QString::number(userObj["messages"].toInt());
    QString bets = QString::number(userObj["bets"].toInt());
    QString wins = QString::number(userObj["wins"].toInt());
    QString losses = QString::number(userObj["losses"].toInt());
    QString luck = QString::number(userObj["win_risk"].toDouble()/userObj["lose_risk"].toDouble(), 'f', 8);

    ui->userinfoLabel->setText(user);
    ui->totalWagered->setText(wagered);
    ui->totalProfit->setText(profit);
    ui->chatMessages->setText(messages);
    ui->totalBets->setText(bets);
    ui->wins->setText(wins);
    ui->losses->setText(losses);
    ui->luck->setText(luck);

    QRect topLevelGeom = win->geometry();
    QSize size = this->size();
    setGeometry(topLevelGeom.center().x()-size.width()/2,
                          topLevelGeom.center().y()-size.height()/2,
                          size.width(), size.height());
    show();
}

void UserInfoFrame::on_closeButton_clicked()
{
    hide();
}
