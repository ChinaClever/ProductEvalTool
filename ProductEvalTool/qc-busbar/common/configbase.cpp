/*
 * 配置文件公共基类
 *
 *  Created on: 2018年1月1日
 *      Author: Lzy
 */
#include "configbase.h"

ConfigBase::ConfigBase()
{
    item = new sTestConfigItem();
}

void ConfigBase::initConfig(sSafetyItem *it)
{
    if(!it) it = &(item->sn);

    it->name = getName();
    it->snClear = getSnClear();    
}

void ConfigBase::saveConfig(sSafetyItem *it)
{
    if(!it) it = &(item->sn);

    setName(it->name);
    setSnClear(it->snClear);
}

/**
 * @brief 获取串口名称
 * @return 串口名
 */
QString ConfigBase::getSerialName(const QString &prefix)
{
    //QString prefix = getPrefix();
    QString str = QString("%1_COM").arg(prefix);
    return sys_configFile_readStr(str, prefix);
}

/**
 * @brief 设置串口名
 * @param name
 */
void ConfigBase::setSerialName(const QString &name , const QString &prefix)
{
    //QString prefix = getPrefix();
    QString str = QString("%1_COM").arg(prefix);
    sys_configFile_writeParam(str, name, prefix);
}


QString ConfigBase::getName()
{
    QString prefix = getPrefix();
    QString str = QString("%1_name").arg(prefix);
    return sys_configFile_readStr(str, prefix);
}

void ConfigBase::setName(const QString &arg)
{
    QString prefix = getPrefix();
    QString str = QString("%1_name").arg(prefix);
    sys_configFile_writeParam(str, arg, prefix);
}

QString ConfigBase::getOp()
{
    QString prefix = getPrefix();
    QString str = QString("%1_op").arg(prefix);
    return sys_configFile_readStr(str, prefix);
}

void ConfigBase::setOp(const QString &arg)
{
    QString prefix = getPrefix();
    QString str = QString("%1_op").arg(prefix);
    sys_configFile_writeParam(str, arg, prefix);
}


QString ConfigBase::getCn()
{
    QString prefix = getPrefix();
    QString str = QString("%1_cn").arg(prefix);
    return sys_configFile_readStr(str, prefix);
}

void ConfigBase::setCn(const QString &arg)
{
    QString prefix = getPrefix();
    QString str = QString("%1_cn").arg(prefix);
    sys_configFile_writeParam(str, arg, prefix);
}


QString ConfigBase::getBarCode()
{
    QString prefix = getPrefix();
    QString str = QString("%1_clientname").arg(prefix);
    return sys_configFile_readStr(str, prefix);
}

void ConfigBase::setBarCode(const QString &arg)
{
    QString prefix = getPrefix();
    QString str = QString("%1_clientname").arg(prefix);
    sys_configFile_writeParam(str, arg, prefix);
}


QString ConfigBase::getBatch()
{
    QString prefix = getPrefix();
    QString str = QString("%1_batch").arg(prefix);
    return sys_configFile_readStr(str, prefix);
}

void ConfigBase::setBatch(const QString &arg)
{
    QString prefix = getPrefix();
    QString str = QString("%1_batch").arg(prefix);
    sys_configFile_writeParam(str, arg, prefix);
}

QString ConfigBase::getPurpose()
{
    QString prefix = getPrefix();
    QString str = QString("%1_purpose").arg(prefix);
    return sys_configFile_readStr(str, prefix);
}

void ConfigBase::setPurpose(const QString &arg)
{
    QString prefix = getPrefix();
    QString str = QString("%1_purpose").arg(prefix);
    sys_configFile_writeParam(str, arg, prefix);
}


/**
 * @brief 获取相数
 * @return
 */
bool ConfigBase::getSnClear()
{
    bool ret = true;
    QString prefix = getPrefix();
    QString str = QString("%1_snclear").arg(prefix);
    int rtn = sys_configFile_readInt(str, prefix);
    if(rtn == 0)  ret = false;
    return ret;
}

/**
 * @brief 设置相数
 * @param num
 */
void ConfigBase::setSnClear(bool mode)
{
    int ret = 0;
    if(mode) ret = 1;
    QString prefix = getPrefix();
    QString str = QString("%1_snclear").arg(prefix);
    sys_configFile_writeParam(str, QString::number(ret), prefix);
}
