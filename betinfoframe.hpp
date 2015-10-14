#ifndef BETINFOFRAME_HPP
#define BETINFOFRAME_HPP

#include <QFrame>

namespace Ui {
class BetInfoFrame;
}

class BetInfoFrame : public QFrame
{
    Q_OBJECT

public:
    explicit BetInfoFrame(QWidget *parent = 0);
    ~BetInfoFrame();

    Ui::BetInfoFrame* GetUi() { return ui; }
    void Display(QString betId);

private slots:
    void on_closeButton_clicked();

private:
    Ui::BetInfoFrame *ui;
};

#endif // BETINFOFRAME_HPP
