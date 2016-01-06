#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>

#include "profile.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    Ui::MainWindow* GetUi() { return ui; }

    void ReadSettings();
    void WriteSettings();
    QString GetScriptDir();
    ProfileManager& GetProfileManager() { return profileManager; }

public slots:
    void AddTab();

private slots:
    void on_tabWidget_tabCloseRequested(int index);

private:
    Ui::MainWindow *ui;
    QString appDir;
    QString scriptDir;
    ProfileManager profileManager;
};

#endif // MAINWINDOW_HPP
