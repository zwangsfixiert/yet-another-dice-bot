#include <QWidget>
#include <QScrollBar>

#include "consolewidget.hpp"
#include "ui_consolewidget.h"

#include "mainwindow.hpp"
#include "profilewidget.hpp"
#include "ui_profilewidget.h"

ConsoleWidget::ConsoleWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConsoleWidget)
{
    ui->setupUi(this);
}

ConsoleWidget::~ConsoleWidget() {
    delete ui;
}

void ConsoleWidget::on_consoleOutput_textChanged() {
    auto scrollbar = ui->consoleOutput->verticalScrollBar();
    scrollbar->setValue(scrollbar->maximum());
}

void ConsoleWidget::on_consoleInput_returnPressed() {
    auto win = (MainWindow*)topLevelWidget();
    ProfileWidget* profilewidget = (ProfileWidget*)parent()->parent()->parent();
    auto profile = win->GetProfileManager().GetProfile(profilewidget->GetUsername());

    //if() { } else
    if(profile->chatenabled) {
        //FIXME
        if(ui->consoleInput->text().startsWith("/msg", Qt::CaseInsensitive)) {
            win->GetRestAPI().SendMessage(*profile, "English", ui->consoleInput->text(), "");
        } else {
            win->GetRestAPI().SendMessage(*profile, "English", ui->consoleInput->text(), "");
        }
    }
    ui->consoleInput->setText("");
}
