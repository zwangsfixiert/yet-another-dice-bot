#ifndef USERMENUFRAME_HPP
#define USERMENUFRAME_HPP

#include <QFrame>

namespace Ui {
class UserMenuFrame;
}

class UserMenuFrame : public QFrame
{
    Q_OBJECT

public:
    explicit UserMenuFrame(QWidget *parent = nullptr);
    ~UserMenuFrame();

    Ui::UserMenuFrame * GetUi() { return ui; }
    void Display(QString username);

private slots:
    void on_pushButton_clicked();

    void on_statsButton_clicked();

private:
    Ui::UserMenuFrame *ui;
};

#endif // USERMENUFRAME_HPP
