#ifndef HOME_LOADWID_H
#define HOME_LOADWID_H
#include "comtablewid.h"
#include "datapacket.h"
#include "config.h"

class Home_Loadwid : public ComTableWid
{
    Q_OBJECT
public:
    explicit Home_Loadwid(QWidget *parent = nullptr);
    Home_Loadwid();

protected:
    void initWid();
    void appendItem(sDevData *box);

protected slots:
    void timeoutDone();
private:
    sObjData *mData;
    sDevData *mDev;
    sCfgItem *mItem;
};

#endif // HOME_LOADWID_H
