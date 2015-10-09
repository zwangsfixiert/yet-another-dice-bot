#ifndef PROFILEWIDGET_HPP
#define PROFILEWIDGET_HPP

#include <QWidget>

#include "socketio.hpp"

namespace Ui {
class ProfileWidget;
}

class ProfileWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProfileWidget(QString username, QWidget *parent = 0);
    ~ProfileWidget();

    SocketIO& GetSio() { return socketIO; }
    QString GetUsername() { return profile_username; }
    Ui::ProfileWidget* GetUi() { return ui; }

private slots:
    void on_faucetPushButton_clicked();

    void on_updateButton_clicked();

    void on_wagerLine_editingFinished();

    void on_winChanceDoubleSpinBox_valueChanged(double arg1);

    void on_highorlowComboBox_currentIndexChanged(int index);

    void on_rollPushButton_clicked();

    void on_minPushButton_clicked();

    void on_maxPushButton_clicked();

    void on_halfBetPushButton_clicked();

    void on_doubleBetPushButton_clicked();

private:
    Ui::ProfileWidget *ui;
    QString profile_username;
    SocketIO socketIO;


};

#endif // PROFILEWIDGET_HPP
