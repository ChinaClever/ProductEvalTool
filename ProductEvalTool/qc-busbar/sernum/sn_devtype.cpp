/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "sn_devtype.h"
#include "common/json/json_build.h"
#include "common/json/json_recv.h"

Sn_DevType::Sn_DevType()
{
    sDevData *dev = sDataPacket::bulid()->getDev();
    mDt = &(dev->devType);

    QJsonObject json;
    Json_Build::bulid()->getJson(json);
    //Json_Recv::bulid()->recv("pdu_id_eaton");
    Json_Recv::bulid()->recv("pdu_id");
}

Sn_DevType * Sn_DevType::bulid()
{
    static  Sn_DevType* sington = nullptr;
    if(sington == nullptr)
        sington = new  Sn_DevType();
    return sington;
}

QString Sn_DevType::getDevStr(uint id)
{
    mDt->dev_type = Json_Recv::bulid()->getDevTypeByID(id);
    return mDt->dev_type;
}

int Sn_DevType::getDevType(const QString &str)
{
    int ret = 0;
    if(str.contains("ZPDU")) ret = ZPDU;
    if(str.contains("MPDU")) ret = MPDU;
    if(str.contains("RPDU")) ret = RPDU;

    if(str.contains("BM-PDU")) ret = BM_PDU;
    if(str.contains("SI-PDU")) ret = SI_PDU;
    if(str.contains("IP-PDU")) {
        ret = IP_PDU;
        if(str.contains("SNMPV3")){
            if(str.contains("C3")) mDt->version = IP_PDUV3_C3;
            else if(str.contains("EATON")) mDt->version = IP_PDUV3_EATON;
            else if(str.contains("BYTE")) mDt->version = IP_PDUV3_BYTE;
            else if(str.contains("前海")) mDt->version = IP_PDUV3_ECC;
            else
            mDt->version = IP_PDUV3;
        }else if(str.contains("V6")){
            if(str.contains("HUAWEI")) mDt->version = IP_PDUV6_HUAWEI;
            else mDt->version = IP_PDUV6;
        }
        else{
          if(str.contains("YIXIN")) mDt->version = IP_PDUV1_YIXIN;
          else mDt->version = IP_PDUV1;
        }
    }
    if(str.contains("IDC-BUSBAR")) {
        if(str.contains("插接箱")) ret = INSERT_BUSBAR;
        else if(str.contains("温度传感器"))ret = TEMPER_BUSBAR;
        else if(str.contains("始端箱"))ret = START_BUSBAR;
    }

    return ret;
}

int Sn_DevType::getAcType(const QString &str)
{
    int ret = AC;
    if(str.contains("直流")) ret = DC;

    return ret;
}

int Sn_DevType::getColMode(const QString &str)
{
    int ret = Transformer;
    if(str.contains("锰铜")) ret = Mn;
    if(mDt->devType > RPDU) ret = Transformer;

    return ret;
}

int Sn_DevType::getLineNum(const QString &str)
{
    int ret = 1;
    if(str.contains("三相")) ret = 3;
    if(str.contains("两路")) ret = 2;

    return ret;
}

int Sn_DevType::getSerie(const QString &str)
{
    int ret = 0;
    if(str.contains("A系列")) ret = 1;
    if(str.contains("B系列")) ret = 2;
    if(str.contains("C系列")) ret = 3;
    if(str.contains("B/D系列")) ret = 4;

    return ret;
}

int Sn_DevType::getSceenType(const QString &str)
{
    int ret = 0;
    if(str.contains("断码屏")) ret = 1;
    if(str.contains("数码管")) ret = 2;
    if(str.contains("断码屏") && str.contains("V1")) ret = 3;
    return ret;
}

bool Sn_DevType::analysDevType(uint id)
{
    bool ret = true;
    QString str = getDevStr(id);
    if(str.size()) {
        mDt->devId = id;
        mDt->dev_type = str;
        mDt->devType = getDevType(str);
        mDt->ac = getAcType(str);
        // mDt->specs = getColMode(str);
        // mDt->series = getSerie(str);
        mDt->lines = getLineNum(str);
        mDt->screen = getSceenType(str);
    } else {
        ret = false;
    }

    return ret;
}

