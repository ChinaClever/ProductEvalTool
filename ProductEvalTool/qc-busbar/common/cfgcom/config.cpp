/*
 * 配置文件公共基类
 *
 *  Created on: 2018年1月1日
 *      Author: Lzy
 */
#include "config.h"

Cfg::Cfg()
{
    mCfg = CfgCom::bulid();
    item = new sCfgItem();

    initCnt();
    initErrData();
    initCfgDev();
    initCurrentNum();
    initJudgItem();
}

Cfg *Cfg::bulid()
{
    static Cfg* sington = nullptr;
    if(sington == nullptr)
        sington = new Cfg();
    return sington;
}


int Cfg::initAddr()
{
    item->vref = read("vref", false,"BusbarSys").toBool();
//    item->temCheck = read("temcheck", true,"Sys").toBool();
//    item->macCheck = read("maccheck" , 1).toInt();
    return read("addr", 1,"BusbarSys").toInt();
}

void Cfg::initPrint()
{
    item->printer = read("printer", false ).toBool();
    item->macprinter = read("macprinter", false ).toBool();
}

void Cfg::setPrinter()
{
    write("macprinter", item->macprinter);
    write("printer", item->printer);
}

void Cfg::setAddr(int addr)
{
    write("addr", addr, "BusbarSys");
    write("vref", item->vref,"BusbarSys");
//    write("temcheck", item->temCheck,"Sys");
//    write("maccheck", item->macCheck);
}

void Cfg::initErrData()
{
    item->err.volErr = read("vol", 5,"BusbarErr").toInt();
    item->err.curErr = read("cur", 5,"BusbarErr").toInt();
    item->err.powErr = read("pow", 10,"BusbarErr").toInt();
}

void Cfg::writeErrData()
{
    write("vol", item->err.volErr, "BusbarErr");
    write("cur", item->err.curErr, "BusbarErr");
    write("pow", item->err.powErr, "BusbarErr");
}

void Cfg::initCnt()
{
    item->cnt.num = read("num", 0, "BusbarCnt").toInt();
    item->cnt.all = read("all", 0, "BusbarCnt").toInt();
    item->cnt.ok = read("ok", 0, "BusbarCnt").toInt();
    item->cnt.err = read("err", 0, "BusbarCnt").toInt();
}

void Cfg::writeCnt()
{
    write("num", item->cnt.num, "BusbarCnt");
    write("all", item->cnt.all, "BusbarCnt");
    write("ok", item->cnt.ok, "BusbarCnt");
    write("err", item->cnt.err, "BusbarCnt");
}



void Cfg::initCfgDev()
{
    item->addr = read("addr", 1, "BusbarSys").toInt();
    item->modeId = read("modeId", 0, "BusbarSys").toInt();
    item->user = read("user", "", "BusbarUser").toString();


    QString g = "BusbarInsertCfg";
    item->si.si_iOF = read("si_iOF", 0, g).toInt();
    item->si.si_buzzer = read("si_buzzer", 0, g).toInt();
    item->si.si_filter = read("si_filter", 5, g).toInt();   
    initSiData(g);

    g = "BusbarStartCfg";
    item->ip.ip_buzzer = read("ip_buzzer", 0 , g).toInt();
    item->ip.ip_filter = read("ip_filter", 5 , g).toInt();
    item->ip.ip_ISD = read("ip_ISD", 0 , g).toInt();
    item->ip.ip_iOF = read("ip_iOF", 0 , g).toInt();
    item->ip.ip_shunt = read("ip_shunt", 0 , g).toInt();  
    item->ip.ip_lightning = read("ip_lightning", 0 , g).toInt();
    initIpData(g);
}

void Cfg::writeCfgDev()
{
    write("addr", item->addr, "BusbarSys");
    write("modeId", item->modeId, "BusbarSys");
    write("user", item->user, "BusbarUser");

    QString g = "BusbarStartCfg";
    write("ip_buzzer", item->ip.ip_buzzer, g);
    write("ip_filter", item->ip.ip_filter, g);
    write("ip_ISD", item->ip.ip_ISD, g);
    write("ip_iOF", item->ip.ip_iOF, g);
    write("ip_shunt", item->ip.ip_shunt, g);
    write("ip_lightning", item->ip.ip_lightning, g);
    writeIpData(g);

    QString q = "BusbarInsertCfg";
    write("si_buzzer", item->si.si_buzzer, q);
    write("si_filter", item->si.si_filter, q);
    write("si_iOF", item->si.si_iOF, q);
    writeSiData(q);
}

void Cfg::writeIpData(const QString &g)
{
    sObjCfg *ptr = &(item->ip_cfg);
    writeUnit("vol", ptr->vol , g);
    writeUnit("cur", ptr->cur , g);
    writeUnit("tem", ptr->tem , g);
    writeUnit("pow", ptr->pow , g);

    writeUnit("totalpow", ptr->totalpow , g);
    writeUnit("linevol", ptr->linevol , g);
    writeUnit("hz", ptr->hz , g);
    writeUnit("zerocur", ptr->zerocur , g);
    writeUnit("recur", ptr->recur , g);
}

void Cfg::initSiData(const QString &g)
{
    sObjCfg *ptr = &(item->si_cfg);
    initUnit("vol", ptr->vol , g);
    initUnit("cur", ptr->cur, g);

    initUnit("tem", ptr->tem, g);
    initUnit("pow", ptr->pow, g);

#if ZHIJIANGINSERTBOXZERO==1
    initUnit("zerocur", ptr->zerocur, g);
#endif
}


void Cfg::initIpData(const QString &g)
{
    sObjCfg *ptr = &(item->ip_cfg);
    initUnit("vol", ptr->vol , g);
    initUnit("cur", ptr->cur , g);
    initUnit("tem", ptr->tem , g);
    initUnit("pow", ptr->pow , g);


    initUnit("totalpow", ptr->totalpow , g);
    initUnit("linevol", ptr->linevol , g);
    initUnit("hz", ptr->hz , g);
    initUnit("zerocur", ptr->zerocur , g);
    initUnit("recur", ptr->recur , g);
}

void Cfg::writeSiData(const QString &g)
{
    sObjCfg *ptr = &(item->si_cfg);
    writeUnit("vol", ptr->vol , g);
    writeUnit("cur", ptr->cur , g);

    writeUnit("tem", ptr->tem , g);
    writeUnit("pow", ptr->pow , g);

#if ZHIJIANGINSERTBOXZERO==1
    writeUnit("zerocur", ptr->zerocur , g);
#endif
}

void Cfg::writeUnit(const QString& prefix, sUnitCfg &unit,const QString &g, int f)
{
    write(prefix+"_min", QString::number(unit.min) , g);
    write(prefix+"_max", QString::number(unit.max) , g);
}

void Cfg::initUnit(const QString& prefix, sUnitCfg &unit,const QString &g, int f)
{
    unit.min = read(prefix+"_min", 0 , g).toFloat();
    unit.max = read(prefix+"_max", 10 , g).toFloat();
}


/**
 * @brief 获取串口名称
 * @return 串口名
 */
QString Cfg::getSerialName(const QString &key)
{
    return mCfg->read(key, "", "Serial").toString();
}

/**
 * @brief 设置串口名
 * @param name
 */
void Cfg::setSerialName(const QString &key, const QString &v)
{
    mCfg->write(key, v, "Serial");
}

QString Cfg::getSerialBr(const QString &com)
{
    QString key = QString("BR_%1").arg(com);
    return mCfg->read(key, "", "Serial").toString();
}

void Cfg::setSerialBr(const QString &com, const QString &br)
{
    QString key = QString("BR_%1").arg(com);
    mCfg->write(key, br, "Serial");
}


/**
 * @brief 根据名称获取配置文件数值
 * @return 对应的配置文件数值
 */
QVariant Cfg::read(const QString &key, const QVariant &v, const QString &g)
{
    return mCfg->read(key, v, g);
}

/**
 * @brief 设置对应名称的配置文件数值
 * @param value
 * @param name
 */
void Cfg::write(const QString &key, const QVariant& v, const QString &g)
{
    mCfg->write(key, v, g);
}

/**
 * @brief 获取当前用户名称
 * @return 用户名
 */
QString Cfg::getLoginName()
{
    return mCfg->read("name", "admin", "Login").toString();
}

/**
 * @brief 设置当前用户名
 * @param name
 */
void Cfg::setLoginName(const QString &name)
{
    mCfg->write("name", name, "Login");
}


void Cfg::setDate()
{
    QString value = QDate::currentDate().toString("yyyy-MM-dd");
    write("date", value, "Date");
}

bool Cfg::getDate()
{
    bool ret = false;
    QString str = read("date","","Date").toString();
    if(!str.isEmpty()) {
        QDate date = QDate::fromString(str, "yyyy-MM-dd");
        if(QDate::currentDate() > date) ret = true;
    }

    return ret;
}

void Cfg::setCurrentNum()
{
    setDate();
    write("num", item->currentNum, "Date");
}

void Cfg::initCurrentNum()
{
    bool ret = getDate();
    if(ret) {
        item->currentNum = 0;
        setCurrentNum();
    } else {
        int value = read("num", 0,"Date").toInt();
        item->currentNum = value;
    }
}

void Cfg::initJudgItem()
{
    QString g = "BusbarPeopleTest";
    item->supCheck1 = read("sup_check1", "", g).toString();
    item->supCheck2 = read("sup_check2", "", g).toString();
}

void Cfg::writeJudgItem()
{
    QString g = "BusbarPeopleTest";
    write("sup_check1", item->supCheck1, g);
    write("sup_check2", item->supCheck2, g);
}
