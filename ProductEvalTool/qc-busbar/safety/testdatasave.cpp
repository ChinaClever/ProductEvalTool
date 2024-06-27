/*
 *
 *
 *  Created on: 2018年10月1日
 *      Author: Lzy
 */
#include "testdatasave.h"
#include "sqlcom/dblogs.h"

TestDataSave::TestDataSave(QObject *parent) : QObject(parent)
{
    mItem = TestConfig::bulid()->item;
    mSaveThread = new Excel_SaveThread(this);
    mPacketEng = datapacket_English::bulid();
    mPacket = sDataPacket::bulid();
    mPro = mPacket->getPro();
    mDev = mPacket->getDev();
    mCfg = Cfg::bulid()->item;
    ePro = mPacketEng->getPro();
}

bool TestDataSave::saveDb()
{
    sStateItem item;

    item.sn = mDev->devType.sn;
    item.dev = mDev->devType.dev_type;
    item.QRcode = mItem->sn.sn; // 产品条码
    item.content = mPro->test_item;

    QString str = tr("成功");
    if(mItem->progress.errNum) str = tr("失败");
    item.passed = str;  // 结果

    // if(mCfg->cnt.num > 0) {
    //     if(mCfg->modeId == 2)
    //         mCfg->cnt.num -= 1;
    //     mPro->test_num = mCfg->cnt.all - mCfg->cnt.num;
    //     ePro->test_num = mCfg->cnt.all - mCfg->cnt.num;
    // }
    item.memo = mPro->itemData.join("; ");
    if(item.QRcode.size()) mLogItems << item;
    if(item.QRcode.isEmpty()) return false;

    return DbLogs::bulid()->insertItem(item);
}

void TestDataSave::saveTestData()
{
    bool ret = saveDb();
    if(ret) {
        mPacket->updatePro(tr("测试日志保存"));
        for(int i=0; i<mLogItems.size(); ++i) {
            DbStates::bulid()->insertItem(mLogItems.at(i));
        }
        mLogItems.clear();
    } else {
        mLogItems.clear();
    }
}

QString TestDataSave::bulidFileName(sSafetyItem *item)
{
    QString file = item->sn  + '_';
    file += QDateTime::currentDateTime().toString("yyyyMMdd_hhmm");

    return file;
}

QString TestDataSave::bulidPath(sSafetyItem *item)
{
    QDir dir;
    QString path = "C:/" + tr("安规测试日志") + '/';

    path += "日期_"+item->date.toString("yyyy_MM_dd")  + '/';
    dir.mkpath(path); //创建多级目录，如果已存在则会返回去true

    return path;
}
