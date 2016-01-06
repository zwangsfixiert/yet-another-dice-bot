#ifndef FAUCETDIALOG_HPP
#define FAUCETDIALOG_HPP

#include <QWebView>
#include <QDialog>

#include "profile.hpp"

namespace Ui {
class FaucetDialog;
}

class FaucetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FaucetDialog(QWidget *parent = 0);
    ~FaucetDialog();

    void Refresh();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void replyReceived(QNetworkReply* reply);
    void onFaucet(QNetworkReply* reply);

private:
    Ui::FaucetDialog *ui;
};

#endif // FAUCETDIALOG_HPP
