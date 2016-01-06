#include <mutex>

#include <QWidget>
#include <QTabBar>
#include <QStringList>
#include <QJSEngine>

#include "profilewidget.hpp"
#include "ui_profilewidget.h"

#include "primedice.hpp"
#include "mainwindow.hpp"
#include "faucetdialog.hpp"
#include "ui_faucetdialog.h"

#include "consolewidget.hpp"
#include "ui_consolewidget.h"

#include "userinfoframe.hpp"
#include "ui_userinfoframe.h"

#include "betinfoframe.hpp"
#include "ui_betinfoframe.h"

std::mutex g_chat_lock;

ProfileWidget::ProfileWidget(QString username, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProfileWidget), profile_username(username)
{
    scriptEngine = new QJSEngine();
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

    console = new ConsoleWidget(this);
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

    userInfo = new UserInfoFrame(this);
    betInfo = new BetInfoFrame(this);
    faucet = new FaucetDialog(this);

    connect(this, SIGNAL(RequestAddLine(const ConsoleWidget*, QString)), console, SLOT(AddLine(const ConsoleWidget*, QString)));

    connect(&restAPI, SIGNAL(LoginHandler(QNetworkReply*)), this, SLOT(onLogin(QNetworkReply*)));
    connect(&restAPI, SIGNAL(BetHandler(QNetworkReply*)), this, SLOT(onBet(QNetworkReply*)));
    connect(&restAPI, SIGNAL(OwnUserInfoHandler(QNetworkReply*)), this, SLOT(onOwnUserInfo(QNetworkReply*)));

    connect(&restAPI, SIGNAL(BetInfoHandler(QNetworkReply*)), this, SLOT(onBetInfo(QNetworkReply*)));
    connect(&restAPI, SIGNAL(UserInfoHandler(QNetworkReply*)), this, SLOT(onUserInfo(QNetworkReply*)));

    connect(&restAPI, SIGNAL(TipHandler(QNetworkReply*)), console, SLOT(onTip(QNetworkReply*)));

    if(profile->accesstoken != "") {
        QNetworkReply* reply = restAPI.GetOwnUserInfo(*profile);
    }

    using namespace sio;

    if(profile->chatenabled) {
        chat = new ConsoleWidget(this);
        ui->consoleTab->addTab(chat, "Chat");

        connect(this, SIGNAL(RequestAddLine(const ConsoleWidget*, QString)), chat, SLOT(AddLine(const ConsoleWidget*, QString)));

        socketIO.GetSio().socket()->on("pm", socket::event_listener_aux(
            [this] (std::string const& name, message::ptr const& data, bool isAck, message::list &ack_resp) {
                std::string user = data->get_map()["username"]->get_string();
                std::string touser = data->get_map()["toUsername"]->get_string();
                std::string msg = data->get_map()["message"]->get_string();

                bool isSpecialUser = data->get_map()["admin"]->get_flag() == message::flag_string;
                bool isMod = isSpecialUser && data->get_map()["admin"]->get_string() == "M";
                bool isAdmin = isSpecialUser && data->get_map()["admin"]->get_string() == "A";
                bool isVIP = isSpecialUser && data->get_map()["admin"]->get_string() == "VIP";

                msg.erase(std::remove(msg.begin(), msg.end(), '\n'), msg.end());

                QString parsemessage = PrettyPrintMessage(msg.c_str());

                QString colour = "#000000";
                if(isVIP)
                    colour = "#0000FF";
                if(isAdmin || isMod)
                    colour = "#CC0000";

                // TODO support multiple msg tabs or a single profile global message tabs
                QString chatentry;
                if(user == profile_username.toStdString()) {
                    chatentry = "<b>You whispered to &lt;<a href=\"yadb://local/userMenu/" +
                            QString(touser.c_str()) + "\" style=\"text-decoration: none;\">" +
                            QString(touser.c_str()) + "</a>&gt;</b> " +
                            parsemessage;
                } else {
                    chatentry = "<b>&lt;<a href=\"yadb://local/userMenu/" +
                            QString(user.c_str()) + "\" style=\"text-decoration: none; color:" + colour +"\"" + ">" +
                            QString(user.c_str()) + "</a>&gt; whispered:</b> " +
                            parsemessage;
                }

                Q_EMIT RequestAddLine(chat, chatentry);
        }));

        socketIO.GetSio().socket()->on("msg", socket::event_listener_aux(
            [this] (std::string const& name, message::ptr const& data, bool isAck, message::list &ack_resp) {
                std::string user = data->get_map()["username"]->get_string();
                std::string msg = data->get_map()["message"]->get_string();
                std::string room = data->get_map()["room"]->get_string();

                bool isSpecialUser = data->get_map()["admin"]->get_flag() == message::flag_string;
                bool isMod = isSpecialUser && data->get_map()["admin"]->get_string() == "M";
                bool isAdmin = isSpecialUser && data->get_map()["admin"]->get_string() == "A";
                bool isVIP = isSpecialUser && data->get_map()["admin"]->get_string() == "VIP";

                msg.erase(std::remove(msg.begin(), msg.end(), '\n'), msg.end());

                QString parsemessage = PrettyPrintMessage(msg.c_str());

                QString colour = "#000000";
                if(isVIP)
                    colour = "#0000FF";
                if(isAdmin || isMod)
                    colour = "#CC0000";

                // TODO support multiple room tabs
                if(room == "English") {
                    QString chatentry = "<b>&lt;<a href=\"yadb://local/userMenu/" +
                    QString(user.c_str()) + "\" style=\"text-decoration: none; color:" + colour + "\">" +
                    QString(user.c_str()) + "</a>&gt;</b> " + parsemessage;

                    Q_EMIT RequestAddLine(chat, chatentry);
                }
        }));
    }

    show();

    // tip {"userid":"566176","user":"UnixPunk","amount":61288,"sender":"singpays","senderid":"334"}
    socketIO.GetSio().socket()->on("tip", socket::event_listener_aux(
        [this] (std::string const& name, message::ptr const& data, bool isAck, message::list &ack_resp) {
            //std::string sender = data->get_map()["sender"]->get_string();
            double amount = data->get_map()["amount"]->get_double()/1e8;
            double balance = ui->balanceLine->text().toDouble();

            ui->balanceLine->setText(QString::number(balance+amount, 'f', 8));
    }));

    socketIO.Initialize(profile->chatenabled);
    socketIO.Connect(profile);

    QTimer* faucetRemainingTimer = new QTimer(this);
    connect(faucetRemainingTimer, SIGNAL(timeout()), this, SLOT(updateFaucetRemainingTimer()));
    faucetRemainingTimer->start(100);

    faucetTimer = new QTimer(this);
    connect(faucetTimer, SIGNAL(timeout()), this, SLOT(updateFaucetTimer()));
    faucetTimer->start(100);
}

ProfileWidget::~ProfileWidget() {
    delete ui;
    delete faucetTimer;

    scriptEngine->collectGarbage();
    delete scriptEngine;

    socketIO.Disconnect();
}

void ProfileWidget::onBetInfo(QNetworkReply* reply) {
    qDebug() << "onBetInfo";
    /*
     "{"bet":{
        "target":99.98,"player_id":529361,
        "server":"23b826b9ce2b3....",
        "player":"xxxx","nonce":94623,"jackpot":0,"profit":-636,
        "id":7013787533,"condition":">","multiplier":9900,
        "client":"wC0b3DKgXKL2zZteTb2HG3xJz2TC8B","roll":72.7,
        "timestamp":"Tue Oct 13 2015 03:50:55 GMT+0000 (UTC)",
        "amount":636,"win":0}}"
       */
    QString res = reply->readAll();
    QJsonDocument jsonRes = QJsonDocument::fromJson(res.toLocal8Bit());
    QJsonObject jsonObj = jsonRes.object();
    QJsonObject betObj = jsonObj["bet"].toObject();
    QString id = QString::number(qulonglong(betObj["id"].toDouble()));
    QString target = QString::number(betObj["target"].toDouble(), 'f', 2);
    QString roll = QString::number(betObj["roll"].toDouble(), 'f', 2);
    QString condition = betObj["condition"].toString();
    QString wager = QString::number(betObj["amount"].toDouble()/1e8, 'f', 8);
    QString profit = QString::number(betObj["profit"].toDouble()/1e8, 'f', 8);
    QString serverHash = betObj["server"].toString();
    QString multiplier = QString::number(betObj["multiplier"].toDouble(), 'f', 5);
    QString user = betObj["player"].toString();

    betInfo->GetUi()->betInfoLabel->setText("Bet #" + id);
    betInfo->GetUi()->username->setText(user);
    betInfo->GetUi()->target->setText(condition + target);
    betInfo->GetUi()->roll->setText(roll);
    betInfo->GetUi()->wager->setText(wager);
    betInfo->GetUi()->profit->setText(profit);
    betInfo->GetUi()->serverSeedHash->setText(serverHash);
    betInfo->GetUi()->multiplier->setText(multiplier + "X");

    betInfo->show();

    QSize size = betInfo->size();
    QRect geom = topLevelWidget()->geometry();
    int cx = geom.center().x();
    int cy = geom.center().y();

    betInfo->setGeometry(cx-size.width()/2, cy-size.height()/2,
                size.width(), size.height());
}

void ProfileWidget::onOwnUserInfo(QNetworkReply* reply) {
    MainWindow* win = (MainWindow*)topLevelWidget();
    Profile* profile = win->GetProfileManager().GetProfile(profile_username);

    QString res = reply->readAll();
    QJsonDocument jsonRes = QJsonDocument::fromJson(res.toLocal8Bit());
    QJsonObject jsonObj = jsonRes.object();
    QJsonObject userObj = jsonObj["user"].toObject();

    if(authenticated) {
        double balance = floor(userObj["balance"].toDouble())/1e8;
        ui->balanceLine->setText(QString::number(balance, 'f', 8));
    } else {
        // token check
        qDebug() << res << "has token" << userObj["balance"].toDouble()/1e8 << userObj["username"].toString();

        QString msg = "";
        if(userObj["username"].toString() == profile->username) {
            qDebug() << "has token authed";
            authenticated = true;

            double balance = floor(userObj["balance"].toDouble())/1e8;
            ui->balanceLine->setText(QString::number(balance, 'f', 8));

            msg = "Successfully authenticated as user " + profile->username + " using cached token.";
        } else {
            msg = "Unable to authenticate as user " + profile->username + " using cached token, attempting to login.";
            QNetworkReply* reply = restAPI.Login(*profile, profile->username, profile->password, "");
        }

        console->GetUi()->consoleOutput->moveCursor(QTextCursor::End);
        console->GetUi()->consoleOutput->append(msg);
    }
}

void ProfileWidget::onLogin(QNetworkReply* reply) {
    QString res = reply->readAll();

    MainWindow* win = (MainWindow*)topLevelWidget();
    Profile* profile = win->GetProfileManager().GetProfile(profile_username);

    qDebug() << "not authed" << res;
    QJsonDocument jsonRes = QJsonDocument::fromJson(res.toLocal8Bit());
    QJsonObject jsonObj = jsonRes.object();
    QString token = jsonObj["access_token"].toString();
    if(token != "") {
        QNetworkReply* reply = restAPI.GetOwnUserInfo(*profile);
    }
}

void ProfileWidget::onBet(QNetworkReply* reply) {
    /*
        "{"bet":{"id":6942729536,"player":"PaidBack","player_id":"273890",
                "amount":0,"target":50.49,"profit":0,"win":false,"condition":">",
            "roll":20.36,"nonce":4003,"client":"`","multiplier":2,
            "timestamp":"2015-10-06T14:02:15.502Z","jackpot":false,
            "server":"6c05cb0b7d454130602b03bce6625738ea049c278009efa44a2b693621ccba15","revealed":false}
    */
    qDebug() << reply;

    MainWindow* win = (MainWindow*)topLevelWidget();
    Profile* profile = win->GetProfileManager().GetProfile(profile_username);

    QString res = reply->readAll();
    QJsonDocument jsonBetRes = QJsonDocument::fromJson(res.toLocal8Bit());
    QJsonObject jsonObj = jsonBetRes.object();
    QJsonObject betObj = jsonObj["bet"].toObject();
    QJsonObject userObj = jsonObj["user"].toObject();

    double balance = floor(userObj["balance"].toDouble())/1e8;
    ui->balanceLine->setText(QString::number(balance, 'f', 8));

    QString msg =  "Wagered " + QString::number(betObj["amount"].toDouble()/1e8, 'f', 8) + " on " +
                betObj["condition"].toString() +
                QString::number(betObj["target"].toDouble(), 'f', 2) + " " +
                QString::number(betObj["roll"].toDouble(), 'f', 2) + " and ";

    bool won = betObj["win"].toBool();
    if(won)
        msg += "won " + QString::number(round(betObj["profit"].toDouble())/1e8, 'f', 8);
    else
        msg += "lost -" + QString::number(round(betObj["amount"].toDouble())/1e8, 'f', 8);

    msg += " B:" + QString::number((int64_t)betObj["id"].toDouble()) + " N:" + QString::number(betObj["nonce"].toInt());

    for(int i = 0; i < ui->consoleTab->count(); i++) {
        if(ui->consoleTab->tabBar()->tabText(i) != "Console")
             continue;

        ConsoleWidget* console = (ConsoleWidget*)ui->consoleTab->widget(i);
        console->GetUi()->consoleOutput->moveCursor(QTextCursor::End);
        console->GetUi()->consoleOutput->append(msg);
     }
}

void ProfileWidget::onUserInfo(QNetworkReply* reply) {
    QString res = reply->readAll();
    QJsonDocument jsonRes = QJsonDocument::fromJson(res.toLocal8Bit());
    QJsonObject jsonObj = jsonRes.object();
    QJsonObject userObj = jsonObj["user"].toObject();
    QString user = userObj["username"].toString();
    QString wagered = QString::number(userObj["wagered"].toDouble()/1e8, 'f', 8);
    QString profit = QString::number(userObj["profit"].toDouble()/1e8, 'f', 8);
    QString messages = QString::number(userObj["messages"].toInt());
    QString bets = QString::number(userObj["bets"].toInt());
    QString wins = QString::number(userObj["wins"].toInt());
    QString losses = QString::number(userObj["losses"].toInt());
    QString luck = QString::number(userObj["win_risk"].toDouble()/userObj["lose_risk"].toDouble()*100., 'f', 8);

    userInfo->GetUi()->userinfoLabel->setText(user);
    userInfo->GetUi()->totalWagered->setText(wagered);
    userInfo->GetUi()->totalProfit->setText(profit);
    userInfo->GetUi()->chatMessages->setText(messages);
    userInfo->GetUi()->totalBets->setText(bets);
    userInfo->GetUi()->wins->setText(wins);
    userInfo->GetUi()->losses->setText(losses);
    userInfo->GetUi()->luck->setText(luck + "%");

    userInfo->show();

    QSize size = userInfo->size();
    QRect geom = topLevelWidget()->geometry();
    int cx = geom.center().x();
    int cy = geom.center().y();

    userInfo->setGeometry(cx-size.width()/2, cy-size.height()/2,
                size.width(), size.height());
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
    faucetTimer->setInterval(faucetClaimTime*1000);
    faucetTimer->start();
    ui->faucetPushButton->setDisabled(true);
}

void ProfileWidget::on_faucetPushButton_clicked() {
    MainWindow* win = (MainWindow*)topLevelWidget();
    Profile* profile = win->GetProfileManager().GetProfile(profile_username);

    //QNetworkReply* reply = restAPI.GetOwnUserInfo(*profile);

    //FaucetDialog* faucet = new FaucetDialog(this);
    faucet->Refresh();
    faucet->show();
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
    QNetworkReply* reply = restAPI.Bet(*profile,
                          ui->highorlowComboBox->currentText().toLower() == "high",
                          ui->targetDoubleSpinBox->value(),
                          (uint64_t)(ui->wagerLine->text().toDouble()*1e8));
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
    QString preBetScript;
    QString postBetScript;
    QString targetSelectionScript;

    if(ui->preBetScriptList->currentItem() != nullptr)
        preBetScript = ui->preBetScriptList->currentItem()->text();

    if(ui->postBetScriptList->currentItem() != nullptr)
        postBetScript = ui->postBetScriptList->currentItem()->text();

    if(ui->targetScriptList->currentItem() != nullptr)
        targetSelectionScript = ui->targetScriptList->currentItem()->text();

    if((preBetScript == "" && postBetScript == "") ||
            targetSelectionScript == "") {
        //return;
    }

    QString preBetSource;
    QString postBetSource;
    QString targetSelectionSource;

    MainWindow* win = (MainWindow*)topLevelWidget();
    if(preBetScript != "") {
        QFile filePreBet(win->GetScriptDir()+ "/prebet/" + preBetScript);
        if(filePreBet.open(QFile::ReadOnly|QFile::Text)) {
            QTextStream in(&filePreBet);
            preBetSource = in.readAll();
        }
    }

    QJSValue res = scriptEngine->evaluate(preBetSource);

    if (res.isError()) {
        qWarning() << "eval error:" << res.toString();
        return;
    }

    if (!scriptEngine->globalObject().hasProperty("main")) {
        qWarning() << "Script has no \"main\" function";
        return;
    }

    if (!scriptEngine->globalObject().property("main").isCallable()) {
        qWarning() << "\"main\" property of script is not callable";
        return;
    }

    QJSValueList args;
    args << 1;
    QJSValue callres = scriptEngine->globalObject().property("main").call(args);
    if (callres.isError()) {
        qWarning() << "Error calling \"main\" function:" << callres.toString();
    }
    qWarning() << callres.toNumber();
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
