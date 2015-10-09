#ifndef CONSOLEWIDGET_HPP
#define CONSOLEWIDGET_HPP

#include <QWidget>

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

private slots:
    void on_consoleOutput_textChanged();

    void on_consoleInput_returnPressed();

private:
    Ui::ConsoleWidget *ui;
};

#endif // CONSOLEWIDGET_HPP
