#ifndef DEV_SOURCETHREAD_H
#define DEV_SOURCETHREAD_H

#include "dev_iprtu.h"
#include "rk_serial.h"
#include "ad_modbus.h"

class Dev_Source : public Dev_SiRtu
{
    Q_OBJECT
    explicit Dev_Source(QObject *parent = nullptr);
public:
    static Dev_Source *bulid(QObject *parent = nullptr);
    bool read();

protected:
    void init();

protected slots:
    void initFunSlot();
    bool readRk9901();
    bool readSiPdu();

private:
    Rk_Serial *mRk;

};

#endif // DEV_SOURCETHREAD_H
