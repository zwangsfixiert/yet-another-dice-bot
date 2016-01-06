#include "userinfoframe.hpp"
#include "ui_userinfoframe.h"
#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "profilewidget.hpp"
#include "ui_profilewidget.h"

#include "primedice.hpp"

UserInfoFrame::UserInfoFrame(QWidget *parent) :
    QFrame(parent, Qt::Tool|Qt::Window|Qt::FramelessWindowHint),
    ui(new Ui::UserInfoFrame)
{
    ui->setupUi(this);

    //ProfileWidget* profileWidget = (ProfileWidget*)parent;
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
    QNetworkReply* reply = profileWidget->GetRestAPI().GetUserInfo(*profile, username);
}

void UserInfoFrame::on_closeButton_clicked()
{
    hide();
}
