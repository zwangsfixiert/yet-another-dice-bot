#include <QWidget>

#include "usermenuframe.hpp"
#include "ui_usermenuframe.h"

#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "profilewidget.hpp"
#include "ui_profilewidget.h"

UserMenuFrame::UserMenuFrame(QWidget *parent) :
    QFrame(parent, Qt::Tool|Qt::Window|Qt::FramelessWindowHint),
    ui(new Ui::UserMenuFrame)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);
}

UserMenuFrame::~UserMenuFrame()
{
    delete ui;
}

void UserMenuFrame::Display(QString username) {
    QSize size = this->size();
    QRect topLevelGeom = parentWidget()->topLevelWidget()->geometry();
    if(ui->userLabel->text() == "") {
        qDebug() << "user not shown" << topLevelGeom;

        ui->userLabel->setText(username);
        setGeometry(topLevelGeom.center().x()-size.width()/2,
                    topLevelGeom.center().y()-size.height()/2,
                    size.width(), size.height());
        show();
    } else if(username == ui->userLabel->text()) {
        qDebug() << "user same" << topLevelGeom;

        ui->userLabel->setText("");
        hide();
    } else if(username != ui->userLabel->text()) {
        qDebug() << "user changed" << topLevelGeom;

        ui->userLabel->setText(username);
        setGeometry(topLevelGeom.center().x()-size.width()/2,
                    topLevelGeom.center().y()-size.height()/2,
                    size.width(), size.height());
        show();
    }
}

void UserMenuFrame::on_pushButton_clicked()
{
    ui->userLabel->setText("");
    hide();
}

void UserMenuFrame::on_statsButton_clicked()
{
    MainWindow* win = (MainWindow*)parentWidget()->topLevelWidget();
    ProfileWidget* profileWidget = (ProfileWidget*)win->GetUi()->tabWidget->currentWidget();

    hide();
    profileWidget->GetUserInfoFrame()->Display(ui->userLabel->text());
}
