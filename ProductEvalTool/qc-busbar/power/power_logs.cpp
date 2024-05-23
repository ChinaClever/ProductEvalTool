/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "Power_Logs.h"
extern QString user_land_name();

Power_Logs::Power_Logs(QObject *parent) : Power_Object(parent)
{
}


Power_Logs *Power_Logs::bulid(QObject *parent)
{
    static Power_Logs* sington = nullptr;
    if(sington == nullptr)
        sington = new Power_Logs(parent);
    return sington;
}

bool Power_Logs::appendLogItem(const QString &str, bool pass)
{
    sStateItem it;

    it.dev = mDev->devType.dev_type;
    it.QRcode = mItem->user;
    it.sn = mDev->devType.sn;
    it.content = mPro->test_item;

    if(pass) {
        it.passed = tr("通过");
    } else {
        it.passed = tr("失败");
    }

    QString str1 = mPro->itemData.join("; ");
    it.memo = str;

    mLogItems << it;

    return pass;
}

void Power_Logs::saveLogs()
{
    bool ret = writeLog();
    if(ret) {
        mPacket->updatePro(tr("测试日志保存"));
        writeLogs();
    } else {
        mLogItems.clear();
    }
}

bool Power_Logs::writeLog()
{
    sStateItem it;

    it.dev = mDev->devType.dev_type;
    it.QRcode = mPro->product_sn;
    it.sn = mDev->devType.sn;
    it.content = mPro->test_item;

    mItem->cnt.all += 1;
    if(mPro->result != Test_Fail) {
        it.passed = tr("通过");
        mItem->cnt.ok += 1;
        if(mItem->cnt.num > 0) {
            mItem->cnt.num -= 1;
            if(!mItem->cnt.num)  {
                mItem->user.clear();
                Cfg::bulid()->write("user", mItem->user, "User");
            }
        }
    } else {
        mItem->cnt.err += 1;
        it.passed = tr("失败");
    }
    it.memo = mPro->itemData.join("; ");

    if(it.QRcode.size()) mLogItems << it;
    if(it.QRcode.isEmpty()) return false;

    return DbLogs::bulid()->insertItem(it);
}

void Power_Logs::writeLogs()
{
    for(int i=0; i<mLogItems.size(); ++i) {
        DbStates::bulid()->insertItem(mLogItems.at(i));
    }
    mLogItems.clear();
}

bool Power_Logs::updatePro(const QString &str, bool pass, int sec)
{
    if(mPro->step < Test_End) {
        mPacket->updatePro(str, pass, sec);
    }

    return pass;
}

void Power_Logs::writeData(const QString &str1,const QString &str2, bool pass)
{
    if(mPro->step < Test_End) {
        mPro->testStartTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        if(pass) {
            mPro->stepResult << "1";
        } else {
            mPro->stepResult << "0";
        }
        mPro->stepRequest << str1; mPro->itemData << str2;

    }
}
