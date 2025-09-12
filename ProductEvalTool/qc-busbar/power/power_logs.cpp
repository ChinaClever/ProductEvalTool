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

    if(mPro->result != Test_Fail) {
        it.passed = tr("通过");
        mItem->cnt.ok += 1;
        if(mItem->cnt.num > 0) {
            if(mPro->work_mode == 3)
                mItem->cnt.num -= 1;
        }
    } else {
        mItem->cnt.err += 1;
        it.passed = tr("失败");
    }

    int num = mPro->stepResult.size();
    QString str;
    for(int i=0; i<num; ++i)
    {
        str += mPro->test_function.at(i) + ":" + mPro->itemData.at(i) + ";";
    }

    it.memo = str;

    Cfg::bulid()->writeCnt();

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

//str1 测试要求 str2 结果判定 str3 测试项目
void Power_Logs::writeData(const QString &str1,const QString &str2, const QString &str3, bool pass)
{
    if(mPro->step < Test_End) {
        if(pass) {
            mPro->stepResult << "1";
        } else {
            mPro->stepResult << "0";
        }
        mPro->stepRequest << str1; mPro->itemData << str2;
        mPro->test_function << str3;
        if(mPro->online)
        {
            msleep(20);
            Json_Pack::bulid()->FuncData(mPro->stepNum);
            mPro->stepNum++;
        }
    }
}

//str1 测试要求 str2 结果判定 str3 测试项目
void Power_Logs::writeDataEng(const QString &str1,const QString &str2, const QString &str3, bool pass)
{
    if(mPro->step < Test_End) {        
        if(pass) {
            ePro->stepResult << "1";
        } else {
            ePro->stepResult << "0";
        }
        ePro->stepRequest << str1; ePro->itemData << str2;
        ePro->test_function << str3;
        if(mPro->online)
        {
            msleep(20);
            Json_Pack::bulid()->FuncData_Lan(mPro->stepNumEng);
            mPro->stepNumEng++;
        }
    }
}
