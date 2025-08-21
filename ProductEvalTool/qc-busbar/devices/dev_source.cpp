/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "dev_source.h"

Dev_Source::Dev_Source(QObject *parent) : Dev_SiRtu(parent)
{
   mRk = new Rk_Serial(this);
//    mDev = sDataPacket::bulid()->getDev(0);
//    init();
}

Dev_Source *Dev_Source::bulid(QObject *parent)
{
    static Dev_Source* sington = nullptr;
    if(sington == nullptr)
        sington = new Dev_Source(parent);
    return sington;
}

void Dev_Source::initFunSlot()
{
//    setModbus(3);
   mRk->init(mItem->coms.ser4);
}

void Dev_Source::init()
{
    mDev->id = 1;
    sDevType *dev = &(mDev->devType);
    dev->version = 1; //
}

bool Dev_Source::readRk9901()
{
    sRkItem rkIt;
#if TEST1_TEST2
    mItem->coms.ser3->setBaudRate(4800);
    bool ret = mRk->readPacket(rkIt);

    if(ret) {
        int curUnit = 100;
        if(rkIt.curUnit) curUnit *= 100;
        sObjData *obj = &(mDev->line);
        for(int i=0; i<9; ++i) {
            obj->vol.value[i] = rkIt.vol;
            obj->cur.value[i] = rkIt.cur;
            obj->pow[i] = rkIt.pow/1000;//有功功率
            obj->hz[i] = rkIt.hz;
            obj->pf[i] = rkIt.pf;
//            obj->cur.value[i] = rkIt.cur / curUnit;
//            obj->pow[i] = rkIt.pow / 1000;
//            obj->hz[i] = rkIt.hz / 100;
//            obj->pf[i] = rkIt.pf / 10;
        }
        obj->size = 3;
    } else {
        QString str = tr("比对源PK9901数据读取失败，质检结束");
        mPacket->updatePro(str, ret);
    }
#else
    mItem->coms.ser4->setBaudRate(9600);

    bool ret = mRk->readPacket1(rkIt);
    qDebug()<<"ret: "<<ret;
    if(ret) {
        sObjData *obj = &(mDev->line);
        for(int i=0; i<3; ++i) {
            obj->source_vol[i] = rkIt.vola[i];
            obj->source_cur[i] = rkIt.cura[i];
            obj->pow[i] = rkIt.powa[i];//有功功率

//            obj->source_cur[i] = 16000;
//            obj->source_vol[i] = 220000;

            // obj->hz[i] = rkIt.hz;
            //            obj->cur.value[i] = rkIt.cur / curUnit;
            //            obj->pow[i] = rkIt.pow / 1000;
            //            obj->hz[i] = rkIt.hz / 100;
            //            obj->pf[i] = rkIt.pf / 10;
        }

        obj->size = 3;
    } else {
        QString str = tr("比对源PK9901数据读取失败，质检结束");
        mPacket->updatePro(str, ret);
    }
#endif

    return ret;
}

bool Dev_Source::readSiPdu()
{
    bool ret = true;
    for(int i=0; i<3; ++i) {
        ret = readPduData();
        if(ret) break; else mModbus->changeBaudRate();
    }

    if(!ret) {
        QString str = tr("比对源SI-PDU数据读取失败，质检结束");
        mPacket->updatePro(str, ret);
    }

    return ret;
}

bool Dev_Source::read()
{
    bool ret = false;
    qDebug()<<"read start!";
    ret = readRk9901();
    qDebug()<<"readRk9901 "<<ret;
//    else {
//        ret = readSiPdu();
//    }
    return ret;
}
