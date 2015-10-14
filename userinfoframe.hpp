#ifndef USERINFOFRAME_HPP
#define USERINFOFRAME_HPP

#include <QFrame>

namespace Ui {
class UserInfoFrame;
}

class UserInfoFrame : public QFrame
{
    Q_OBJECT

public:
    explicit UserInfoFrame(QWidget *parent = 0);
    ~UserInfoFrame();

    Ui::UserInfoFrame* GetUi() { return ui; }
    void Display(QString username);

private slots:
    void on_closeButton_clicked();

private:
    Ui::UserInfoFrame *ui;
};

#endif // USERINFOFRAME_HPP
