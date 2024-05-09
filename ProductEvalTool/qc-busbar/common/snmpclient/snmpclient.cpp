/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "snmpclient.h"

SnmpClient::SnmpClient(QObject *parent) : QThread(parent)
{
    mMutex = new QMutex();
    m_snmp_client =  new QtSnmpClient(this);
    m_snmp_client->setCommunity( "public" );
    m_snmp_client->setReponseTimeout(2*1000);
    connect( m_snmp_client,SIGNAL(responseReceived(qint32,QtSnmpDataList)), this, SLOT(onResponseReceived(qint32,QtSnmpDataList)) );
    connect( m_snmp_client, SIGNAL(requestFailed(qint32)), this, SLOT(onRequestFailed(qint32)));

    timer =  new QTimer(this);
    timer->start(55);
    connect(timer, SIGNAL(timeout()), SLOT(timeoutDone()));
    setAddress("192.168.1.163");
}

//SnmpClient::~SnmpClient()
//{
//    delete timer;
//    delete mMutex;
//    delete m_snmp_client;
//}

//SnmpClient *SnmpClient::bulid(QObject *parent)
//{
//    static SnmpClient* sington = nullptr;
//    if(sington == nullptr)
//        sington = new SnmpClient(parent);
//    return sington;
//}

void SnmpClient::setAddress(const QString& ip)
{
    m_snmp_client->setAgentAddress(QHostAddress(ip));
}

QtSnmpDataList SnmpClient::waitForRespond(int size, int sec)
{
    isErr = false;
    mValues.clear();
    for(int i=0; i<=10*sec; ++i) {
        if((mValues.size() >= size) || isErr) {
            break;
        } else {
            msleep(100);
        }
    }

    return mValues;
}

QtSnmpDataList SnmpClient::requestValue(const QString& oid)
{
    QStringList oids(oid);
    return requestValue(oids);
}

QtSnmpDataList SnmpClient::requestValue(const QStringList& oids)
{
    QMutexLocker locker(mMutex);
    mOids = oids;

    return waitForRespond(oids.size());
}

void SnmpClient::setValue(const sSnmpSetCmd &cmd)
{
    QMutexLocker locker(mMutex);
    mSetCmdList << cmd;
}

bool SnmpClient::setValue()
{
    bool ret = false;
    if(mSetCmdList.size()) {
        ret = !m_snmp_client->isBusy();
        if(ret) {
            sSnmpSetCmd cmd = mSetCmdList.first();
            m_snmp_client->setValue("private", cmd.oid, cmd.type, cmd.value);
            mSetCmdList.removeFirst();
        }
    }
    return ret;
}

void SnmpClient::onResponseReceived(const qint32, const QtSnmpDataList& values )
{
    mValues << values;
    //for(const auto& value : values)  qDebug() << value.address() << value.data();
}

void SnmpClient::onRequestFailed( const qint32 request_id )
{
    isErr = true;
    qDebug() << "SnmpClient onRequest Failed" << request_id;
}

bool SnmpClient::makeSubRequest()
{
    bool ret = false;
    if(mOids.size()) {
        ret = !m_snmp_client->isBusy();
        if(ret) {
            m_snmp_client->requestSubValues(mOids.first());
            mOids.removeFirst();
        }
    }

    return ret;
}

void SnmpClient::timeoutDone()
{
    bool ret = setValue();
    if(!ret) ret = makeSubRequest();
}
