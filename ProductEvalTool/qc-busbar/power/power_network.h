#ifndef POWER_NETWORK_H
#define POWER_NETWORK_H

#include "power_logs.h"
#include "udprecvsocket.h"

class Power_NetWork : public Power_Object
{
    Q_OBJECT
    explicit Power_NetWork(QObject *parent = nullptr);
public:
    static Power_NetWork *bulid(QObject *parent = nullptr);

protected:
    void run();
    void workDown();

signals:
    void sendMACSig(QString str);

protected slots:
    void initFunSlot();

private:
    Power_Logs *mLogs;
    UdpRecvSocket *mUdp;
};

#endif // TEST_NETWORK_H
