#ifndef HOMEMAINWID_H
#define HOMEMAINWID_H

#include <QWidget>
#include "serialstatuswid.h"
#include "home_workwid.h"
#include "home_looptabwid.h"


namespace Ui {
class Home_MainWid;
}

class Home_MainWid : public QWidget
{
    Q_OBJECT

public:
    explicit Home_MainWid(QWidget *parent = nullptr);
    ~Home_MainWid();

signals:
    // void startSig();

protected:
    void initWid();

private:
    Ui::Home_MainWid *ui;

    Home_WorkWid *mWorkWid;
    Home_LoopTabWid *mLineTabWid;
};

#endif // HOMEMAINWID_H
