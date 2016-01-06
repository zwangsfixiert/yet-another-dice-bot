#include <cassert>

#include <QtWidgets>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonValue>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "profile.hpp"
#include "primedice.hpp"

constexpr double minChance = 0.01;
constexpr double maxChance = 98.0;

double roundtoprecision(double value, int precision)
{
    double power(pow(10, precision));
    return floor(value*power)/power;
}

double getpayout(double chance, double houseEdge)
{
    assert(chance >= minChance && chance <= maxChance && "chance is out of range");
    return roundtoprecision(100./chance*(100.-houseEdge)/100., 5);
}

RestAPI::RestAPI() {
    connect(&mgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
}

void RestAPI::replyFinished(QNetworkReply* reply) {
    QString path = reply->url().path();
    qDebug() << path;

    QString api = path.section("/", 2, 2);

    if(api == "login") {
        emit LoginHandler(reply);
    } else if(api == "users" && path.section("/", 3, 3) == "1") {
        emit OwnUserInfoHandler(reply);
    } else if(api == "users" && path.section("/", 3, 3) != "") {
        emit UserInfoHandler(reply);
    } else if(api == "tip") {
        emit TipHandler(reply);
    }  else if(api == "send") {
        emit SendMessageHandler(reply);
    } else if(api == "faucet") {
        emit FaucetHandler(reply);
    } else if(api == "bet") {
        emit BetHandler(reply);
    } else if(api == "bets") {
        emit BetInfoHandler(reply);
    }
}

QNetworkReply* RestAPI::RequestHandler(QNetworkRequest& request, QNetworkReply& reply) {
    /*
    QEventLoop eventLoop;
    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));

    eventLoop.exec(); // block stack FIXME be async
    if (reply.error() == QNetworkReply::NoError) {

        //qDebug() << "Response headers";
        //const QList<QByteArray>& rawHeaderListResp(reply.rawHeaderList());
        // foreach (QByteArray rawHeader, rawHeaderListResp) {
        //   qDebug() << reply.rawHeader(rawHeader);
        // }

    } else {
        const QList<QByteArray>& rawHeaderList(reply.request().rawHeaderList());
         foreach (QByteArray rawHeader, rawHeaderList) {
           qDebug() << reply.request().rawHeader(rawHeader);
         }

         qDebug() << "Failure" << reply.errorString() << reply.request().sslConfiguration().protocol();
    }
    */
    return &reply;//.readAll();
}

QNetworkReply* RestAPI::Post(QString url, QByteArray querydata) {
    qDebug() << url;

    QNetworkRequest *req = new QNetworkRequest(QUrl(url));
    req->setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = mgr.post(*req, querydata);

    //return RequestHandler(*req, *reply);
    return reply;

}

QNetworkReply* RestAPI::Get(QString url) {
    qDebug() << url;

    QNetworkRequest *req = new QNetworkRequest(QUrl(url));
    QNetworkReply *reply = mgr.get(*req);

    return reply;
}

QNetworkReply* RestAPI::Login(Profile& profile, QString username, QString password, QString twofactorauthcode)
{
    QByteArray data;
    QUrlQuery params;
    params.addQueryItem("username", username);
    params.addQueryItem("password", password);
    data.append(params.toString());

    return Post(restAPIurl + QString("/login"), data);
}

QNetworkReply* RestAPI::SendMessage(Profile& profile, QString channel, QString message, QString username)
{
    QByteArray data;
    QUrlQuery params;
    params.addQueryItem("username", profile.username);
    params.addQueryItem("room", channel);
    params.addQueryItem("message", QString(QUrl::toPercentEncoding(message)));
    params.addQueryItem("token", profile.accesstoken);
    if(username != "")
        params.addQueryItem("toUsername", username);
    data.append(params.toString());

    return Post(restAPIurl + QString("/send?access_token=" + profile.accesstoken), data);
}

QNetworkReply* RestAPI::Faucet(Profile& profile, QString response) {
    QByteArray data;
    QUrlQuery params;
    if(response != "") {
        params.addQueryItem("response", response);
        data.append(params.toString());

        return Post(restAPIurl + QString("/faucet/?access_token=" + profile.accesstoken), data);
    }

    return nullptr;
}

QNetworkReply* RestAPI::TipUser(Profile& profile, QString username, uint64_t amount) {
    QByteArray data;
    QUrlQuery params;
    params.addQueryItem("username", username);
    params.addQueryItem("amount", QString::number(amount));
    data.append(params.toString());

    return Post(restAPIurl + QString("/tip?access_token=" + profile.accesstoken), data);
}

QNetworkReply* RestAPI::Bet(Profile& profile, bool conditionHigh, double target, uint64_t amount) {
    QByteArray data;
    QUrlQuery params;
    params.addQueryItem("amount", QString::number(amount));
    params.addQueryItem("target", QString::number(target));
    QString conditionVal = "<";
    if(conditionHigh)
        conditionVal = ">";
    params.addQueryItem("condition", conditionVal);
    data.append(params.toString());

    return Post(restAPIurl + QString("/bet?access_token=" + profile.accesstoken), data);
}

QNetworkReply* RestAPI::GetUserInfo(Profile& profile, QString username) {
    return Get(restAPIurl + QString("/users/" + username + "?access_token=" + profile.accesstoken));
}

QNetworkReply* RestAPI::GetOwnUserInfo(Profile& profile) {
    return Get(restAPIurl + QString("/users/1?access_token=" + profile.accesstoken));
}

QNetworkReply* RestAPI::GetBetInfo(Profile& profile, QString betId) {
    return Get(restAPIurl + QString("/bets/" + betId + "?access_token=" + profile.accesstoken));
    //connect(&mgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
}
