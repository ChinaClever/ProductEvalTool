#ifndef SET_MAINWID_H
#define SET_MAINWID_H

#include <QWidget>
#include "set_workwid.h"
#include "set_temper.h"

namespace Ui {
class Set_MainWid;
}

class Set_MainWid : public QWidget
{
    Q_OBJECT

public:
    explicit Set_MainWid(QWidget *parent = nullptr);
    ~Set_MainWid();

protected:
    void initWid();

private:
    Ui::Set_MainWid *ui;
    Set_IpWid *mIpWid;
    Set_SiWid *mSiWid;
    Set_WorkWid *mWorkWid;
    Set_temper *mtemper;
};

#endif // SET_MAINWID_H
