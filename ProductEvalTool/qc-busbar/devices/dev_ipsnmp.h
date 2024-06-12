#ifndef DEV_IPSNMP_H
#define DEV_IPSNMP_H

#include "dev_sirtu.h"
#include "snmpclient.h"
#include <libsnmp.h>
#include "snmp/snmp_pp/snmp_pp.h"

using namespace Snmp_pp;

class Dev_IpSnmp : public Dev_Object
{
    Q_OBJECT
    explicit Dev_IpSnmp(QObject *parent = nullptr);
public:
    static Dev_IpSnmp *bulid(QObject *parent = nullptr);
    bool readPduData();
    void resetSnmp();
    bool SetInfo(QString o , QString val);
    QString getConnectModeOid(){return "1.3.6.1.4.1.30966.12.1.1.2.5.0";}
    QString getFilterOid(){return "1.3.6.1.4.1.30966.12.1.1.2.10.0";}
    QString setShuntReleaseCtrlOid(){return "1.3.6.1.4.1.30966.12.1.1.2.17.0";}
protected:
    bool lineNumV3();
    bool devDataV3();
    bool devDataV1();
    int getItemByOid(const QString &oid, int id);

    void praseMasterVal(QString id , QString value);
    void startBoxBaseInfo(QString id, QString val);
    void startBoxEleParaInfo(QString id, QString val);
    void startBoxNetInfo(QString id, QString val);
    void baseMsInformation(QString id, QString val);
    void parseVolHar(QString val, int line);
    void parseCurHar(QString val, int line);
    void loopMsInformation(QString id, QString val);
    void temMsInformation(QString id, QString val);

    bool determine_vb(SmiUINT32 val, Vb &vb, QString str);

private:
    sDevData *mDev;
    sCfgItem *mItem;
    sProgress *mPro;    
    SnmpClient *mSnmp;
    sDataPacket *mPacket;
    sBusData *mBusData;
};

#endif // DEV_IPSNMP_H
