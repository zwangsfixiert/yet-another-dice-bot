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

namespace PrimeDiceAPI {

RestAPI::RestAPI() {

}

QString RestAPI::RequestHandler(QNetworkRequest& request, QNetworkReply& reply) {
    QEventLoop eventLoop;
    QObject::connect(&mgr, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));

    eventLoop.exec(); // block stack FIXME be async
    if (reply.error() == QNetworkReply::NoError) {
        /*
        qDebug() << "Response headers";
        const QList<QByteArray>& rawHeaderListResp(reply.rawHeaderList());
         foreach (QByteArray rawHeader, rawHeaderListResp) {
           qDebug() << reply.rawHeader(rawHeader);
         }
         */
    } else {
        const QList<QByteArray>& rawHeaderList(reply.request().rawHeaderList());
         foreach (QByteArray rawHeader, rawHeaderList) {
           qDebug() << reply.request().rawHeader(rawHeader);
         }

         qDebug() << "Failure" << reply.errorString() << reply.request().sslConfiguration().protocol();
    }

    return reply.readAll();
}

QString RestAPI::Post(QString url, QByteArray querydata) {
    QNetworkRequest *req = new QNetworkRequest(QUrl(url));
    req->setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *reply = mgr.post(*req, querydata);

    return RequestHandler(*req, *reply);
}

QString RestAPI::Get(QString url) {
     QNetworkRequest *req = new QNetworkRequest(QUrl(url));
     QNetworkReply *reply = mgr.get(*req);

     return RequestHandler(*req, *reply);
}

QString RestAPI::Login(Profile& profile, QString username, QString password, QString twofactorauthcode)
{
    QByteArray data;
    QUrlQuery params;
    params.addQueryItem("username", username);
    params.addQueryItem("password", password);
    data.append(params.toString());

    return Post(restAPIurl + QString("/login"), data);
}

QString RestAPI::SendMessage(Profile& profile, QString channel, QString message, QString username)
{
    QByteArray data;
    QUrlQuery params;
    params.addQueryItem("username", profile.username);
    params.addQueryItem("room", channel);
    params.addQueryItem("message", message);
    params.addQueryItem("token", profile.accesstoken);
    if(username != "")
        params.addQueryItem("toUsername", username);
    data.append(params.toString());

    return Post(restAPIurl + QString("/send?access_token=" + profile.accesstoken), data);
}

QString RestAPI::Faucet(Profile& profile, QString response) {
    QByteArray data;
    QUrlQuery params;
    if(response != "") {
        params.addQueryItem("response", response);
        data.append(params.toString());

        return Post(restAPIurl + QString("/faucet/?access_token=" + profile.accesstoken), data);
    }

    return "";
}

QString RestAPI::TipUser(Profile& profile, QString username, uint64_t amount) {
    QByteArray data;
    QUrlQuery params;
    params.addQueryItem("username", username);
    params.addQueryItem("amount", QString::number(amount));
    data.append(params.toString());

    return Post(restAPIurl + QString("/tip?access_token=" + profile.accesstoken), data);
}

QString RestAPI::Bet(Profile& profile, bool conditionHigh, double target, uint64_t amount) {
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

QString RestAPI::GetOwnUserInfo(Profile& profile) {
    return Get(restAPIurl + QString("/users/1?access_token=" + profile.accesstoken));
}

} // namespace PrimeDiceAPI
