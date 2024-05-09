#ifndef POWER_DEVCTRL_H
#define POWER_DEVCTRL_H

#include "power_network.h"

class Power_DevCtrl : public Power_Object
{
    Q_OBJECT
    explicit Power_DevCtrl(QObject *parent = nullptr);
public:
    static Power_DevCtrl *bulid(QObject *parent = nullptr);

    bool setCurTh(int i);
    bool setVolTh(int i);
    bool factorySet();
    bool eleClean();

protected:
    bool initDev();

protected slots:
    void initFunSlot();

private:
    Ctrl_Object *mRtu;
    Ctrl_SiRtu *mSiRtu;
    Ctrl_IpRtu *mIpRtu;
};

#endif // POWER_DEVCTRL_H
