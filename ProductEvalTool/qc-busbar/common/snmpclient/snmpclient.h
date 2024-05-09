#ifndef SNMPCLIENT_H
#define SNMPCLIENT_H

#include <QThread>
#include <QScopedPointer>
#include <QMap>
#include <QtSnmpClient.h>
#include <QtCore>
#include "datapacket.h"
#include "config.h"

#define MIB_OID_CLEVER  ".1.3.6.1.4.1.30966."
#define IP_MIB_OID  MIB_OID_CLEVER"10.3.2"
#define BUSBAR_MIB_OID  "1.3.6.1.4.1.30966."

enum{
    SNMP_INTEGER_TYPE = 0x02,
    SNMP_STRING_TYPE = 0x04,
};

struct sSnmpSetCmd
{
    sSnmpSetCmd() {type=SNMP_STRING_TYPE;}
    QString oid;
    ushort type;
    QByteArray value;
};


class SnmpClient : public QThread
{
    Q_OBJECT
public:    
    explicit SnmpClient(QObject *parent = nullptr);
    //static SnmpClient *bulid(QObject *parent = nullptr);
    // ~SnmpClient();

    void setAddress(const QString& ip);
    void setValue(const sSnmpSetCmd &cmd);
    QtSnmpDataList requestValue(const QString& oid);
    QtSnmpDataList requestValue(const QStringList& oids);

protected:
    bool setValue();
    bool makeSubRequest();
    QtSnmpDataList waitForRespond(int size, int sec=1);

protected slots:
    void timeoutDone();
    void onResponseReceived(const qint32 request_id, const QtSnmpDataList& );
    void onRequestFailed(const qint32 request_id);

private:
    bool isErr;
    QTimer *timer;

    QMutex *mMutex;
    QStringList mOids;
    QtSnmpDataList mValues;
    QtSnmpClient *m_snmp_client;
    QList<sSnmpSetCmd> mSetCmdList;
};

#endif // SNMPCLIENT_H
