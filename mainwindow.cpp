#include <QtWidgets>

#include <functional>
#include <iostream>

#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include "profilemanagerform.hpp"

#include "profile.hpp"
#include "profilewidget.hpp"

#include "primedice.hpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    appDir = QCoreApplication::applicationDirPath();
    scriptDir = appDir + "/scripts";
    qDebug() << appDir << scriptDir;

    ReadSettings();

    QToolButton *tb = new QToolButton();
    tb->setAutoRaise(true);
    tb->setStyleSheet(QString("QToolButton { width: 0; height: 0;}"));

    connect(tb, SIGNAL(clicked()), this, SLOT(AddTab()));

    ProfileManagerForm* widget = new ProfileManagerForm(this);

    ui->tabWidget->addTab(widget, QString("Profiles"));
    ui->tabWidget->setTabEnabled(0, true);

    ui->tabWidget->tabBar()->setTabButton(0, QTabBar::RightSide, tb);
}

MainWindow::~MainWindow() {
    WriteSettings();

    delete ui;
}

void MainWindow::AddTab() {
    ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-1);
}

void MainWindow::ReadSettings() {
     QSettings settings;

     int size = settings.beginReadArray("Profiles");
     for (int i = 0; i < size; ++i) {
         settings.setArrayIndex(i);
         Profile profile(settings.value("username").toString(),
                         settings.value("password").toString(),
                         settings.value("accesstoken").toString(),
                         settings.value("chatenabled").toBool(),
                         settings.value("autoload").toBool());
         profileManager.AddProfile(profile);
     }
     settings.endArray();
}

void MainWindow::WriteSettings() {
    QSettings settings;

    auto profiles(profileManager.GetProfiles());
    settings.beginWriteArray("Profiles");
    for (int i = 0; i < profileManager.GetProfiles().size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("username", profiles.at(i).username);
        settings.setValue("password", profiles.at(i).password);
        settings.setValue("accesstoken", profiles.at(i).accesstoken);
        settings.setValue("chatenabled", profiles.at(i).chatenabled);
        settings.setValue("autoload", profiles.at(i).autoload);
    }
    settings.endArray();
}

QString MainWindow::GetScriptDir() {
    return appDir + "/scripts";
}

void MainWindow::on_tabWidget_tabCloseRequested(int index) {
    qDebug() << "closing tab " << ui->tabWidget->tabText(index);

    auto profile = profileManager.GetProfile(ui->tabWidget->tabText(index));
    profile->isLoaded = false;

    ui->tabWidget->removeTab(index);
}
