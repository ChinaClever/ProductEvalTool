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
    item->si.si_baud = read("si_baud", 3, g).toInt();
    item->si.si_iOF = read("si_iOF", 0, g).toInt();
    item->si.si_buzzer = read("si_buzzer", 0, g).toInt();
    item->si.si_filter = read("si_filter", 5, g).toInt();
    item->si.si_version = read("si_version", 100, g).toInt();
    item->si.si_phaseflag = read("si_phaseflag", 0, g).toInt();
    initSiData(g);

    g = "BusbarStartCfg";
    item->ip.ip_curtype = read("ip_curtype", 2 , g).toInt();
    item->ip.ip_buzzer = read("ip_buzzer", 0 , g).toInt();
    item->ip.ip_filter = read("ip_filter", 5 , g).toInt();

    item->ip.ip_mode = read("ip_mode", 1 , g).toInt();
    item->ip.ip_num = read("ip_num", 6 , g).toInt();
    item->ip.ip_ISD = read("ip_ISD", 0 , g).toInt();
    item->ip.ip_iOF = read("ip_iOF", 0 , g).toInt();
    item->ip.ip_shunt = read("ip_shunt", 0 , g).toInt();
    item->ip.ip_residual = read("ip_residual", 0 , g).toInt();
    item->ip.ip_lightning = read("ip_lightning", 0 , g).toInt();
    item->ip.version = read("ip_version", 200 , g).toInt();
    initIpData(g);

    g = "BusbarEnvCfg";
    item->si.tem_buzzer = read("tem_buzzer", 0 , g).toInt();
    item->si.tem_filter = read("tem_filter", 5 , g).toInt();
    item->si.tem_version = read("tem_version", 100 , g).toInt();
    item->si.temMin[0] = read("temMin_1", 0 , g).toDouble();
    item->si.temMin[1] = read("temMin_2", 0 , g).toDouble();
    item->si.temMin[2] = read("temMin_3", 0 , g).toDouble();
    item->si.temMin[3] = read("temMin_4", 0 , g).toDouble();
    item->si.temMax[0] = read("temMax_1", 0 , g).toDouble();
    item->si.temMax[1] = read("temMax_2", 0 , g).toDouble();
    item->si.temMax[2] = read("temMax_3", 0 , g).toDouble();
    item->si.temMax[3] = read("temMax_4", 0 , g).toDouble();
}

void Cfg::writeCfgDev()
{
    write("addr", item->addr, "BusbarSys");
    write("modeId", item->modeId, "BusbarSys");
    write("user", item->user, "BusbarUser");

    if(item->modeId == START_BUSBAR){
        QString g = "BusbarStartCfg";
        write("ip_curtype", item->ip.ip_curtype, g);
        write("ip_buzzer", item->ip.ip_buzzer, g);
        write("ip_filter", item->ip.ip_filter, g);
        write("ip_mode", item->ip.ip_mode, g);
        write("ip_num", item->ip.ip_num, g);
        write("ip_ISD", item->ip.ip_ISD, g);
        write("ip_iOF", item->ip.ip_iOF, g);
        write("ip_shunt", item->ip.ip_shunt, g);
        write("ip_residual", item->ip.ip_residual, g);
        write("ip_lightning", item->ip.ip_lightning, g);
        write("ip_version", item->ip.version, g);
        writeIpData(g);
    }else if(item->modeId == INSERT_BUSBAR){
        QString g = "BusbarInsertCfg";
        write("si_baud", item->si.si_baud, g);
        write("si_buzzer", item->si.si_buzzer, g);
        write("si_filter", item->si.si_filter, g);
        write("si_iOF", item->si.si_iOF, g);
        write("si_version", item->si.si_version, g);
        write("si_phaseflag", item->si.si_phaseflag, g);
        writeSiData(g);
    }else if(item->modeId == TEMPER_BUSBAR){
        QString g = "BusbarEnvCfg";
        //温度阈值
        write("tem_buzzer", item->si.tem_buzzer, g);
        write("tem_filter", item->si.tem_filter, g);
        write("tem_version", item->si.tem_version, g);
        write("temMin_1", item->si.temMin[0], g);
        write("temMin_2", item->si.temMin[1], g);
        write("temMin_3", item->si.temMin[2], g);
        write("temMin_4", item->si.temMin[3], g);
        write("temMax_1", item->si.temMax[0], g);
        write("temMax_2", item->si.temMax[1], g);
        write("temMax_3", item->si.temMax[2], g);
        write("temMax_4", item->si.temMax[3], g);
    }
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
