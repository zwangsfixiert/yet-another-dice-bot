#ifndef PROFILEMANAGERFORM_HPP
#define PROFILEMANAGERFORM_HPP

#include <QtWidgets>

namespace Ui {
class ProfileManagerForm;
}

class ProfileManagerForm : public QWidget
{
    Q_OBJECT

public:
    explicit ProfileManagerForm(QWidget *parent = 0);
    ~ProfileManagerForm();

    void SetDefaults();

private slots:
    void on_addProfile_clicked();

    void on_loadProfile_clicked();

    void on_listWidget_profiles_itemDoubleClicked(QListWidgetItem *item);

private:
    Ui::ProfileManagerForm *ui;
};

#endif // PROFILEMANAGERFORM_HPP
