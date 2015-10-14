#ifndef PRIMEDICE_HPP
#define PRIMEDICE_HPP

#include <QNetworkAccessManager>

#include "profile.hpp"

double roundtoprecision(double value, int precision);
double getpayout(double chance, double houseEdge);

namespace PrimeDiceAPI {

class RestAPI {
public:
    RestAPI();

    QString RequestHandler(QNetworkRequest& request, QNetworkReply& reply);
    QString Post(QString url, QByteArray querydata);
    QString Get(QString url);

    QString Login(Profile& profile, QString username, QString password, QString twofactorauthcode);
    QString Faucet(Profile& profile, QString response);
    QString SendMessage(Profile& profile, QString channel, QString message, QString username);
    QString TipUser(Profile& profile, QString username, uint64_t amount);
    QString Bet(Profile& profile, bool conditionHigh, double target, uint64_t amount);
    QString GetUserInfo(Profile& profile, QString username);
    QString GetOwnUserInfo(Profile& profile);
    QString GetBetInfo(Profile& profile, QString betId);

private:
    QNetworkAccessManager mgr;
    QString restAPIurl{"https://api.primedice.com/api"};
};

} // namespace PrimeDiceAPI

#endif // PRIMEDICE_HPP
