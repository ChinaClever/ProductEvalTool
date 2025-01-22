#ifndef SETUP_MAINWID_H
#define SETUP_MAINWID_H

#include <QWidget>
#include "usermainwid.h"

namespace Ui {
class Setup_MainWid;
}

class Setup_MainWid : public QWidget
{
    Q_OBJECT

public:
    explicit Setup_MainWid(QWidget *parent = nullptr);
    ~Setup_MainWid();

protected:
    void initSerial();

    void initLogCount();
    void writeLogCount();
    void initPcNum();
    void initAddr();
    void writePcNum();
    void writeAddr();
    void checkPcNum();
    void checkAddr();
protected slots:
    void initFunSlot();
    void checkPcNumSlot();
    void on_pcBtn_clicked();
    void on_verBtn_clicked();

private:
    Ui::Setup_MainWid *ui;

    sCfgItem *mItem;
    UserMainWid *mUserWid;
    SerialStatusWid *mComWid1;
    SerialStatusWid *mComWid2;
    SerialStatusWid *mComWid3;
    SerialStatusWid *mComWid4;
    SerialStatusWid *mComWid5;
    SerialStatusWid *mComWid6;
};

#endif // SETUP_MAINWID_H
