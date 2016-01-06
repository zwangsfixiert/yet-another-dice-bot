#ifndef CONSOLEWIDGET_HPP
#define CONSOLEWIDGET_HPP

#include <QWidget>
#include <QNetworkReply>

#include "usermenuframe.hpp"

namespace Ui {
class ConsoleWidget;
}

class ConsoleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConsoleWidget(QWidget *parent = 0);
    ~ConsoleWidget();

    Ui::ConsoleWidget* GetUi() { return ui; }

private Q_SLOTS:
    void on_consoleOutput_textChanged();
    void on_consoleInput_returnPressed();
    void on_consoleOutput_anchorClicked(const QUrl &arg1);

    void onTip(QNetworkReply* reply);

    void AddLine(const ConsoleWidget* console, const QString& test);

private:
    Ui::ConsoleWidget *ui;
    UserMenuFrame* userMenu{nullptr};
    bool userMenuShown{false};
};

#endif // CONSOLEWIDGET_HPP
