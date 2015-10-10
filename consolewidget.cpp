#include <QWidget>
#include <QScrollBar>

#include "consolewidget.hpp"
#include "ui_consolewidget.h"

#include "mainwindow.hpp"
#include "ui_mainwindow.h"

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
        // TODO enable russian chat
        if(ui->consoleInput->text().startsWith("/msg", Qt::CaseInsensitive) ||
                ui->consoleInput->text().startsWith("/pm", Qt::CaseInsensitive)) {
            QString cmd = ui->consoleInput->text().section(" ", 0, 0);
            QString target = ui->consoleInput->text().section(" ", 1, 1);
            QString msg = ui->consoleInput->text().mid(cmd.length() + 1 + target.length() + 1);
            qDebug() << cmd << target << msg;
            win->GetRestAPI().SendMessage(*profile, "English", msg, target);
        } else {
            win->GetRestAPI().SendMessage(*profile, "English", ui->consoleInput->text(), "");
        }
        ui->consoleInput->setText("");
        return;
    }

    if (ui->consoleInput->text().startsWith("/tip", Qt::CaseInsensitive)) {
        QString target = ui->consoleInput->text().section(" ", 1, 1);
        QString stramount = ui->consoleInput->text().section(" ", 2, 2);
        double amount = stramount.toDouble();
        if(amount > 0.00050001 && amount <= profilewidget->GetUi()->balanceLine->text().toDouble()) {
            QString tipres = win->GetRestAPI().TipUser(*profile, target, (int64_t)(amount*1e8));
            qDebug() << "tip res" << tipres;
        }
        ui->consoleInput->setText("");
        return;
    }

    ui->consoleInput->setText("");
}
