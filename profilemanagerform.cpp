#include "profilemanagerform.hpp"
#include "ui_profilemanagerform.h"

#include "profilewidget.hpp"
#include "ui_profilewidget.h"

#include "mainwindow.hpp"
#include "ui_mainwindow.h"

ProfileManagerForm::ProfileManagerForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProfileManagerForm)
{
    ui->setupUi(this);

    MainWindow* win = (MainWindow*)parent;
    for(auto& profile : win->GetProfileManager().GetProfiles())
        ui->listWidget_profiles->addItem(profile.username);
}

ProfileManagerForm::~ProfileManagerForm() {
    delete ui;
}

void ProfileManagerForm::SetDefaults() {
    ui->lineEdit_username->setText("");
    ui->lineEdit_password->setText("");
    ui->lineEdit_token->setText("");
    ui->chatEnabled->setChecked(false);
    ui->autoloadEnabled->setChecked(false);
}

void ProfileManagerForm::on_addProfile_clicked() {
    //qDebug() << "addProfile_clicked " << objectName();

    MainWindow* win = (MainWindow*)topLevelWidget();
    qDebug() << win;

    QString username = ui->lineEdit_username->text();
    QString password = ui->lineEdit_password->text();
    QString token = ui->lineEdit_token->text();
    bool chatEnabled = ui->chatEnabled->isChecked();
    bool autoloadEnabled = ui->autoloadEnabled->isChecked();

    if(username == "") {
        SetDefaults();
        return;
    }

    QListWidgetItem *item;
    for(int i = 0; i < ui->listWidget_profiles->count(); ++i) {
        item = ui->listWidget_profiles->item(i);
        if(item->text() == username) {
            SetDefaults();
            return;
        }
    }

    auto& profileManager = win->GetProfileManager();
    auto profile = profileManager.GetProfile(username);
    if(profile == nullptr) {
        assert(profileManager.AddProfile(username, password, token, chatEnabled, autoloadEnabled) && "Could not add profile");
        ui->listWidget_profiles->addItem(username);
        win->WriteSettings();
    } else {
        QListWidgetItem *item;
        for(int i = 0; i < ui->listWidget_profiles->count(); ++i) {
            item = ui->listWidget_profiles->item(i);
            if(item->text() == profile->username) {
                item->setText(username);
                break;
            }
        }
        profile->username = username;
        profile->password = password;
        profile->accesstoken = token;
        profile->chatenabled = ui->chatEnabled->isChecked();
        profile->autoload = ui->autoloadEnabled->isChecked();
        // MsgBox
    }
    SetDefaults();
}

void ProfileManagerForm::on_loadProfile_clicked()
{
    //qDebug() << "loadProfile_clicked " << objectName();

    MainWindow* win = (MainWindow*)parent()->parent()->parent()->parent();

    for(int i = 0; i < ui->listWidget_profiles->count(); ++i) {
        QListWidgetItem *item = ui->listWidget_profiles->item(i);
        if(!item->isSelected())
            continue;
        if(item->text() == "")
            continue;

        auto profile = win->GetProfileManager().GetProfile(item->text());
        if(profile == nullptr)
            continue;

        if(profile->isLoaded == true)
            continue;

        auto profileWidget = new ProfileWidget(profile->username, (QWidget*)parent());

        profile->isLoaded = true;

        win->GetUi()->tabWidget->insertTab(win->GetUi()->tabWidget->count()-1, profileWidget, profile->username);
        win->GetUi()->tabWidget->setCurrentIndex(win->GetUi()->tabWidget->count()-1);
    }
}

void ProfileManagerForm::on_listWidget_profiles_itemDoubleClicked(QListWidgetItem *item)
{
    //qDebug() << "itemDoubleClicked " << objectName();

    MainWindow* win = (MainWindow*)topLevelWidget();
    Profile* profile = win->GetProfileManager().GetProfile(item->text());

    ui->lineEdit_username->setText(profile->username);
    ui->lineEdit_password->setText(profile->password);
    ui->lineEdit_token->setText(profile->accesstoken);
    ui->chatEnabled->setChecked(profile->chatenabled);
    ui->autoloadEnabled->setChecked(profile->autoload);
}

void ProfileManagerForm::on_removeProfile_clicked()
{
    MainWindow* win = (MainWindow*)topLevelWidget();

    for(int i = 0; i < ui->listWidget_profiles->count(); i++) {
        QListWidgetItem* item = ui->listWidget_profiles->item(i);
        if(item->isSelected()) {
            if(win->GetProfileManager().RemoveProfile(item->text())) {
                ui->listWidget_profiles->takeItem(i);
            }
        }
    }
    win->WriteSettings();
}
