#ifndef PROFILEWIDGET_HPP
#define PROFILEWIDGET_HPP

#include <QWidget>
#include <QTimer>
#include <QJSEngine>
#include <QNetworkReply>

#include "consolewidget.hpp"
#include "userinfoframe.hpp"
#include "betinfoframe.hpp"
#include "faucetdialog.hpp"

#include "socketio.hpp"
#include "primedice.hpp"

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
    RestAPI& GetRestAPI() { return restAPI; }

    ConsoleWidget* GetConsole() { return console; }
    ConsoleWidget* GetChat() { return chat; }
    QString GetUsername() { return profile_username; }
    Ui::ProfileWidget* GetUi() { return ui; }
    UserInfoFrame* GetUserInfoFrame() { return userInfo; }
    BetInfoFrame* GetBetInfoFrame() { return betInfo; }
    QJSEngine* GetScriptEngine() { return scriptEngine; }
    QString PrettyPrintMessage(QString message);
    void ResetFaucetTimer();
    void UpdateProfit();

Q_SIGNALS:
    void RequestAddLine(const ConsoleWidget* console, const QString &text);

private Q_SLOTS:
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
    void on_startPushButton_clicked();
    void updateFaucetTimer();
    void updateFaucetRemainingTimer();

    //void onFaucet(QNetworkReply* reply);
    void onLogin(QNetworkReply* reply);
    void onBet(QNetworkReply* reply);
    void onBetInfo(QNetworkReply* reply);
    void onUserInfo(QNetworkReply* reply);
    void onOwnUserInfo(QNetworkReply* reply);

private:
    Ui::ProfileWidget *ui;
    QString profile_username;
    SocketIO socketIO;
    RestAPI restAPI;
    ConsoleWidget* console{nullptr};
    ConsoleWidget* chat{nullptr};
    UserInfoFrame* userInfo{nullptr};
    BetInfoFrame* betInfo{nullptr};
    FaucetDialog* faucet{nullptr};
    QTimer* faucetTimer{nullptr};
    QJSEngine* scriptEngine{nullptr};
    int faucetClaimTime{180};
    bool authenticated{false};
};

#endif // PROFILEWIDGET_HPP
