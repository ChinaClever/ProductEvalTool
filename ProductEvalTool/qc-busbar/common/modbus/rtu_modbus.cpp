/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "rtu_modbus.h"

Rtu_Modbus::Rtu_Modbus(QObject *parent) : QObject(parent)
{
    for(int i=0; i<RTU_RW_SIZE; ++i) {
        mRtuRw[i] = new RtuRw(this);
    }
}

Rtu_Modbus *Rtu_Modbus::bulid(QObject *parent)
{
    static Rtu_Modbus* sington = nullptr;
    if(sington == nullptr) {
        sington = new Rtu_Modbus(parent);
    }
    return sington;
}

RtuRw *Rtu_Modbus::get(int id)
{
    SerialPort *ser = nullptr;
    RtuRw *res = mRtuRw[id];
    sSerial *com = &(Cfg::bulid()->item->coms);

    switch (id) {
    case 0:  ser = com->ser1; break;
    case 1:  ser = com->ser2; break;
    case 2:  ser = com->ser3; break;
    }
    res->init(ser);
    ser->reflush();

    return res;
}
