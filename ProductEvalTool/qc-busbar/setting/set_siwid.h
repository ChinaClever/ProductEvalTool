#ifndef HOME_SIWID_H
#define HOME_SIWID_H

#include "set_lineunitwid.h"

namespace Ui {
class Set_SiWid;
}

class Set_SiWid : public QWidget
{
    Q_OBJECT

public:
    explicit Set_SiWid(QWidget *parent = nullptr);
    ~Set_SiWid();

signals:
    void errSig();

protected:
    void initType();

    void updateType();
    void updateData();

    bool inputCheck();
    bool dataSave();

public slots:
    void initFunSlot();
    void enabledSlot(bool en);

private:
    Ui::Set_SiWid *ui;
    Set_LineUnitWid *mUnitWid;

    sCfgItem *mItem;
//    Dev_Object *mObj;
};

#endif // HOME_SIWID_H
