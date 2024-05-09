#ifndef HOME_IPWID_H
#define HOME_IPWID_H

#include "set_siwid.h"

namespace Ui {
class Set_IpWid;
}

class Set_IpWid : public QWidget
{
    Q_OBJECT

public:
    explicit Set_IpWid(QWidget *parent = nullptr);
    ~Set_IpWid();

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
    Ui::Set_IpWid *ui;
    Set_LineUnitWid *mUnitWid;

    sCfgItem *mItem;
};

#endif // HOME_IPWID_H
