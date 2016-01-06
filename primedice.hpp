#ifndef PRIMEDICE_HPP
#define PRIMEDICE_HPP

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

//#include "request.hpp"
#include "profile.hpp"

double roundtoprecision(double value, int precision);
double getpayout(double chance, double houseEdge);

class RestAPI : public QObject {
    Q_OBJECT

public:
    RestAPI();

    QNetworkReply* RequestHandler(QNetworkRequest& request, QNetworkReply& reply);
    QNetworkReply* Post(QString url, QByteArray querydata);
    QNetworkReply* Get(QString url);

    QNetworkReply* Login(Profile& profile, QString username, QString password, QString twofactorauthcode);
    QNetworkReply* Faucet(Profile& profile, QString response);
    QNetworkReply* SendMessage(Profile& profile, QString channel, QString message, QString username);
    QNetworkReply* TipUser(Profile& profile, QString username, uint64_t amount);
    QNetworkReply* Bet(Profile& profile, bool conditionHigh, double target, uint64_t amount);
    QNetworkReply* GetUserInfo(Profile& profile, QString username);
    QNetworkReply* GetOwnUserInfo(Profile& profile);
    QNetworkReply* GetBetInfo(Profile& profile, QString betId);

public slots:
    void replyFinished(QNetworkReply* reply);

signals:
    void LoginHandler(QNetworkReply* reply);
    void UserInfoHandler(QNetworkReply* reply);
    void OwnUserInfoHandler(QNetworkReply* reply);
    void FaucetHandler(QNetworkReply* reply);
    void SendMessageHandler(QNetworkReply* reply);
    void TipHandler(QNetworkReply* reply);
    void BetHandler(QNetworkReply* reply);
    void BetInfoHandler(QNetworkReply* reply);

private:
    QNetworkAccessManager mgr;
    QString restAPIurl{"https://api.primedice.com/api"};
};

#endif // PRIMEDICE_HPP
