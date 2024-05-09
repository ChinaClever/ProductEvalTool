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
    void writePcNum();
    void initErrData();
    void updateErrData();

protected slots:
    void initFunSlot();
    void checkPcNumSlot();
    void on_pcBtn_clicked();
    void on_verBtn_clicked();
    void on_saveBtn_clicked();

private:
    Ui::Setup_MainWid *ui;

    sCfgItem *mItem;
    UserMainWid *mUserWid;
    SerialStatusWid *mComWid;
    SerialStatusWid *mSourceWid;
    SerialStatusWid *mControlWid;
};

#endif // SETUP_MAINWID_H
