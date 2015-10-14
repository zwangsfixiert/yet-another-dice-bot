#include <QWidget>
#include <QTabBar>
#include <QStringList>

#include "profilewidget.hpp"
#include "ui_profilewidget.h"

#include "primedice.hpp"
#include "mainwindow.hpp"
#include "faucetdialog.hpp"
#include "consolewidget.hpp"
#include "ui_consolewidget.h"

ProfileWidget::ProfileWidget(QString username, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProfileWidget), profile_username(username)
{
    ui->setupUi(this);

    MainWindow* win = (MainWindow*)parent->topLevelWidget();
    Profile* profile = win->GetProfileManager().GetProfile(username);
    assert(profile != nullptr && "profile invalid, not found");

    ui->usernameEdit->setText(profile->username);
    ui->passwordEdit->setText(profile->password);
    ui->tokenEdit->setText(profile->accesstoken);
    ui->chatEnabledCheckBox->setChecked(profile->chatenabled);
    ui->autoloadEnabledCheckBox->setChecked(profile->autoload);

    ui->winChanceDoubleSpinBox->setValue(49.5);
    double target = 49.5;
    if(ui->highorlowComboBox->currentText().toLower() == "high")
        target = 99.99-target;
    ui->payoutLine->setText(QString::number(getpayout(49.5, 1.0), 'f', 5));
    ui->targetDoubleSpinBox->setValue(target);

    ConsoleWidget* console = new ConsoleWidget;
    ui->consoleTab->addTab(console, "Console");

    QDir preBetScriptDir(win->GetScriptDir()+ "/prebet");
    QDir postBetScriptDir(win->GetScriptDir()+ "/postbet");
    QDir targetScriptDir(win->GetScriptDir()+ "/target");

    qDebug() << preBetScriptDir << postBetScriptDir << targetScriptDir;
    QStringList files = preBetScriptDir.entryList(QDir::Files);
    ui->preBetScriptList->addItems(files);

    files = postBetScriptDir.entryList(QDir::Files);
    ui->postBetScriptList->addItems(files);

    files = targetScriptDir.entryList(QDir::Files);
    ui->targetScriptList->addItems(files);

    bool authenticated = false;
    if(profile->accesstoken != "") {
        QString res = win->GetRestAPI().GetOwnUserInfo(*profile);
        QJsonDocument jsonRes = QJsonDocument::fromJson(res.toLocal8Bit());
        QJsonObject jsonObj = jsonRes.object();
        QJsonObject userObj = jsonObj["user"].toObject();
        qDebug() << res << "has token" << userObj["balance"].toString() << userObj["username"].toString();

        if(userObj["username"].toString() == profile->username) {
            qDebug() << "has token authed";
            authenticated = true;

            double balance = floor(userObj["balance"].toDouble())/1e8;
            ui->balanceLine->setText(QString::number(balance, 'f', 8));
        }
    }

    if(!authenticated) {
        QString res = win->GetRestAPI().Login(*profile, profile->username, profile->password, "");
        qDebug() << "not authed" << res;
        QJsonDocument jsonRes = QJsonDocument::fromJson(res.toLocal8Bit());
        QJsonObject jsonObj = jsonRes.object();
        QString token = jsonObj["access_token"].toString();
        if(token != "") {
            QString userinfores = win->GetRestAPI().GetOwnUserInfo(*profile);
            QJsonDocument jsonRes = QJsonDocument::fromJson(userinfores.toLocal8Bit());
            QJsonObject jsonObj = jsonRes.object();
            QJsonObject userObj = jsonObj["user"].toObject();
            double balance = floor(userObj["balance"].toDouble())/1e8;
            ui->balanceLine->setText(QString::number(balance, 'f', 8));

            profile->accesstoken = token;
            authenticated = true;
            win->WriteSettings();
        }
    }

    QString msg;
    if(authenticated)
        msg = "Successfully authenticated as user " + profile->username + ".";
    else
        msg = "Unable to authenticate as user " + profile->username + ". Ensure your profile configuration is correct.";

    console->GetUi()->consoleOutput->moveCursor(QTextCursor::End);
    console->GetUi()->consoleOutput->append(msg);

    show();

    socketIO.Initialize(profile->chatenabled);
    socketIO.Connect(profile);

    if(profile->chatenabled) {
        ConsoleWidget* chat = new ConsoleWidget;
        ui->consoleTab->addTab(chat, "Chat");

        socketIO.GetSio().socket()->on("pm", sio::socket::event_listener_aux(
            [chat, this] (std::string const& name, sio::message::ptr const& data, bool isAck, sio::message::list &ack_resp) {
                std::string user = data->get_map()["username"]->get_string();
                std::string touser = data->get_map()["toUsername"]->get_string();
                std::string message = data->get_map()["message"]->get_string();
                message.erase(std::remove(message.begin(), message.end(), '\n'), message.end());

                qDebug() << "user " << user.c_str() << " touser " << touser.c_str() << " msg " << message.c_str();

                QString parsemessage = PrettyPrintMessage(message.c_str());

                // TODO support multiple msg tabs or a single profile global message tabs
                QString chatentry;
                if(user == profile_username.toStdString()) {
                    chatentry = "You whispered to &lt;<a href=\"yadb://local/userMenu/" +
                            QString(touser.c_str()) + "\">" +
                            QString(touser.c_str()) + "</a>&gt; " +
                            parsemessage;
                } else {
                    chatentry = "&lt;<a href=\"yadb://local/userMenu/" +
                            QString(user.c_str()) + "\">" +
                            QString(user.c_str()) + "</a>&gt; whispered " +
                            parsemessage;
                }

                chat->GetUi()->consoleOutput->moveCursor(QTextCursor::End);
                chat->GetUi()->consoleOutput->append(chatentry);
        }));

        socketIO.GetSio().socket()->on("msg", sio::socket::event_listener_aux(
            [chat, this] (std::string const& name, sio::message::ptr const& data, bool isAck, sio::message::list &ack_resp) {
                std::string user = data->get_map()["username"]->get_string();
                std::string message = data->get_map()["message"]->get_string();
                std::string room = data->get_map()["room"]->get_string();
                message.erase(std::remove(message.begin(), message.end(), '\n'), message.end());

                QString parsemessage = PrettyPrintMessage(message.c_str());

                // TODO support multiple room tabs
                if(room == "English") {
                    QString chatentry = "&lt;<a href=\"yadb://local/userMenu/" +
                            QString(user.c_str()) + "\">" +
                            QString(user.c_str()) + "</a>&gt; " +
                            parsemessage;

                    chat->GetUi()->consoleOutput->moveCursor(QTextCursor::End);
                    chat->GetUi()->consoleOutput->append(chatentry);
                }
        }));
    }

    // tip {"userid":"566176","user":"UnixPunk","amount":61288,"sender":"singpays","senderid":"334"}
    socketIO.GetSio().socket()->on("tip", sio::socket::event_listener_aux(
        [this] (std::string const& name, sio::message::ptr const& data, bool isAck, sio::message::list &ack_resp) {
            //std::string sender = data->get_map()["sender"]->get_string();
            double amount = data->get_map()["amount"]->get_double()/1e8;
            double balance = ui->balanceLine->text().toDouble();

            ui->balanceLine->setText(QString::number(balance+amount, 'f', 8));
    }));

    userInfo = new UserInfoFrame(this);
    betInfo = new BetInfoFrame(this);

    QTimer* faucetRemainingTimer = new QTimer(this);
    connect(faucetRemainingTimer, SIGNAL(timeout()), this, SLOT(updateFaucetRemainingTimer()));
    faucetRemainingTimer->start(50);

    faucetTimer = new QTimer(this);
    connect(faucetTimer, SIGNAL(timeout()), this, SLOT(updateFaucetTimer()));
    faucetTimer->start(50);
}

ProfileWidget::~ProfileWidget() {
    delete ui;
    delete faucetTimer;
}

QString ProfileWidget::PrettyPrintMessage(QString message) {
    int betIdxStart = message.indexOf(QRegExp("(b:|bet:)", Qt::CaseInsensitive), 0);
    int betIdxEnd = message.indexOf(" ", betIdxStart);
    if(betIdxStart >= 0) {
        if(betIdxEnd < 0) {
            // End of message reached
            betIdxEnd = message.count();
        }
        QString bet = message.mid(betIdxStart, betIdxEnd-betIdxStart);
        QString replace = "<a href=\"yadb://local/betInfo/" +
                           bet.section(":", 1, 1) + "\">" + bet +
                           "</a>";
        message.replace(betIdxStart, betIdxEnd-betIdxStart, replace);
    }

    int userIdxStart = message.indexOf(QRegExp("(u:|user:)", Qt::CaseInsensitive), 0);
    int userIdxEnd = message.indexOf(" ", userIdxStart);
    if(userIdxStart >= 0) {
        if(userIdxEnd < 0) {
            // End of message reached
            userIdxEnd = message.count();
        }
        QString user = message.mid(userIdxStart, userIdxEnd-userIdxStart);
        QString replace = "<a href=\"yadb://local/userInfo/" +
                           user.section(":", 1, 1) + "\">" + user +
                           "</a>";
        message.replace(userIdxStart, userIdxEnd-userIdxStart, replace);
    }

    int urlIdxStart = message.indexOf(QRegExp("(http:|https:)"), 0);
    int urlIdxEnd = message.indexOf(" ", urlIdxStart);
    if(urlIdxStart >= 0) {
        if(urlIdxEnd < 0) {
            // End of message reached
            urlIdxEnd = message.count();
        }
        QString url = message.mid(urlIdxStart, urlIdxEnd-urlIdxStart);
        QString replace = "<a href=\"" +
                           url + "\">" + url +
                           "</a>";
        message.replace(urlIdxStart, urlIdxEnd-urlIdxStart, replace);
    }

    return message;
}

void ProfileWidget::ResetFaucetTimer() {
    faucetTimer->setInterval(180*1000);
    faucetTimer->start();
    ui->faucetPushButton->setDisabled(true);
}

void ProfileWidget::on_faucetPushButton_clicked() {
    MainWindow* win = (MainWindow*)topLevelWidget();
    Profile* profile = win->GetProfileManager().GetProfile(profile_username);

    QString userinfores = win->GetRestAPI().GetOwnUserInfo(*profile);
    QJsonDocument jsonRes = QJsonDocument::fromJson(userinfores.toLocal8Bit());
    QJsonObject jsonObj = jsonRes.object();
    QJsonObject userObj = jsonObj["user"].toObject();
    double balance = floor(userObj["balance"].toDouble());

    if(balance < 1) {
        FaucetDialog* faucet = new FaucetDialog(this);
        faucet->show();
    } else {
        QMessageBox msgBox;
        msgBox.setText("Your balance must be 0 to use the faucet.");
        msgBox.exec();
    }
}

void ProfileWidget::on_updateButton_clicked() {
    MainWindow* win = (MainWindow*)topLevelWidget();
    Profile* profile = win->GetProfileManager().GetProfile(profile_username);
    profile->password = ui->passwordEdit->text();
    profile->accesstoken = ui->tokenEdit->text();
    win->WriteSettings();
}

void ProfileWidget::UpdateProfit() {
    ui->profitLine->setText(QString::number(ui->wagerLine->text().toDouble() *
                                            ui->payoutLine->text().toDouble() -
                                            ui->wagerLine->text().toDouble(),
                                            'f', 8));
}

void ProfileWidget::on_wagerLine_editingFinished() {
    qDebug() << "wager line edited" << ui->wagerLine->text().toDouble() << " "
             << ui->payoutLine->text().toDouble();

    ui->wagerLine->setText(QString::number(ui->wagerLine->text().toDouble(),
                                           'f', 8));

    UpdateProfit();
}

void ProfileWidget::on_winChanceDoubleSpinBox_valueChanged(double arg1)
{
     double target = arg1;
     if(ui->highorlowComboBox->currentText().toLower() == "high")
         target = 99.99-arg1;
     else
         target = arg1;

     ui->payoutLine->setText(QString::number(getpayout(arg1, 1.0), 'f', 5));
     ui->targetDoubleSpinBox->setValue(target);

     UpdateProfit();
}

void ProfileWidget::on_highorlowComboBox_currentIndexChanged(int index)
{
    double chance = ui->winChanceDoubleSpinBox->value();
    double target = 0.0;

    if(index == 0)
        target = 99.99-chance;
    else
        target = chance;

    ui->targetDoubleSpinBox->setValue(target);
}

void ProfileWidget::on_rollPushButton_clicked()
{
    MainWindow* win = (MainWindow*)topLevelWidget();
    Profile* profile = win->GetProfileManager().GetProfile(profile_username);
    QString res = win->GetRestAPI().Bet(*profile,
                          ui->highorlowComboBox->currentText().toLower() == "high",
                          ui->targetDoubleSpinBox->value(),
                          (uint64_t)(ui->wagerLine->text().toDouble()*1e8));
 /*
    "{"bet":{"id":6942729536,"player":"PaidBack","player_id":"273890",
            "amount":0,"target":50.49,"profit":0,"win":false,"condition":">",
        "roll":20.36,"nonce":4003,"client":"`","multiplier":2,
        "timestamp":"2015-10-06T14:02:15.502Z","jackpot":false,
        "server":"6c05cb0b7d454130602b03bce6625738ea049c278009efa44a2b693621ccba15","revealed":false}
*/
    QJsonDocument jsonBetRes = QJsonDocument::fromJson(res.toLocal8Bit());
    QJsonObject jsonBetObj = jsonBetRes.object();
    jsonBetObj = jsonBetObj["bet"].toObject();
    bool won = jsonBetObj["win"].toBool();

    QString msg =  "Wagered " + QString::number(jsonBetObj["amount"].toDouble()/1e8, 'f', 8) + " on " +
            jsonBetObj["condition"].toString() +
            QString::number(jsonBetObj["target"].toDouble(), 'f', 2) + " " +
            QString::number(jsonBetObj["roll"].toDouble(), 'f', 2) + " and ";
    if(won)
        msg += "won " + QString::number(round(jsonBetObj["profit"].toDouble())/1e8, 'f', 8);
     else
        msg += "lost -" + QString::number(round(jsonBetObj["amount"].toDouble())/1e8, 'f', 8);

    for(int i = 0; i < ui->consoleTab->count(); i++) {
        if(ui->consoleTab->tabBar()->tabText(i) != "Console")
            continue;

        ConsoleWidget* console = (ConsoleWidget*)ui->consoleTab->widget(i);
        console->GetUi()->consoleOutput->moveCursor(QTextCursor::End);
        console->GetUi()->consoleOutput->append(msg);
    }

    res = win->GetRestAPI().GetOwnUserInfo(*profile);
    QJsonDocument jsonRes = QJsonDocument::fromJson(res.toLocal8Bit());
    QJsonObject jsonObj = jsonRes.object();

    QJsonObject userObj = jsonObj["user"].toObject();
    double balance = floor(userObj["balance"].toDouble())/1e8;
    ui->balanceLine->setText(QString::number(balance, 'f', 8));
}

void ProfileWidget::on_minPushButton_clicked()
{
    ui->wagerLine->setText(QString::number(0.00000001, 'f', 8));
    UpdateProfit();
}

void ProfileWidget::on_maxPushButton_clicked()
{
    ui->wagerLine->setText(ui->balanceLine->text());
    UpdateProfit();
}

void ProfileWidget::on_halfBetPushButton_clicked()
{
    double currWagerSats = ui->wagerLine->text().toDouble() * 1e8;
    if(currWagerSats >= 1.0) {
        currWagerSats = floor(currWagerSats/2.0) / 1e8;
        ui->wagerLine->setText(QString::number(currWagerSats, 'f', 8));
        UpdateProfit();
    }
}

void ProfileWidget::on_doubleBetPushButton_clicked()
{
    double currWagerSats = ui->wagerLine->text().toDouble() * 1e8;
    if(currWagerSats >= 1.0) {
        currWagerSats = round(currWagerSats*2.0) / 1e8;
        ui->wagerLine->setText(QString::number(currWagerSats, 'f', 8));
        UpdateProfit();
    }
    if(currWagerSats > floor(ui->balanceLine->text().toDouble()*1e8)) {
        ui->wagerLine->setToolTip("Insufficient balance");
    }
}

void ProfileWidget::on_startPushButton_clicked()
{
    QString preBetScript = ui->preBetScriptList->currentItem()->text();
    QString postBetScript = ui->postBetScriptList->currentItem()->text();
    QString targetSelectionScript = ui->targetScriptList->currentItem()->text();

    if(preBetScript == "" || postBetScript == "" ||
            targetSelectionScript == "") {
        return;
    }

    // TODO
}
void ProfileWidget::updateFaucetRemainingTimer() {
    if(faucetTimer->remainingTime() > 0) {
        ui->faucetPushButton->setText("Faucet (" +
                                      QString::number(faucetTimer->remainingTime()/1000) +
                                      " seconds)");
    } else {
        ui->faucetPushButton->setText("Faucet (Ready)");
    }
}

void ProfileWidget::updateFaucetTimer() {
    ui->faucetPushButton->setEnabled(true);
    faucetTimer->stop();
}
