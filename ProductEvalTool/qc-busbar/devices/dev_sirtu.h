#ifndef DEV_SITHREAD_H
#define DEV_SITHREAD_H

#include "dev_object.h"

class Dev_SiRtu : public Dev_Object
{
    Q_OBJECT
protected:
    explicit Dev_SiRtu(QObject *parent = nullptr);
public:
    static Dev_SiRtu *bulid(QObject *parent = nullptr);
    bool readPduData();
    Rtu_recv *mRtuPkt;
    sBusData *mBusData;
    bool readRtuData();//SNMP read RTU

protected:
    int recvLine(int len);
    void initRtuItem(Rtu_Sent &it);
    bool recvPacket(uchar *buf, int len);
    int recvAcData(uchar *ptr, int line);
    int recvDcData(uchar *ptr, int line);

    bool rtu_recv_packetV3(uchar *buf, int len, Rtu_recv *pkt);
    int rtu_recv_head(uchar *ptr,  Rtu_recv *pkt);
    int rtu_recv_len(uchar *buf, int len , int rtn);
    int rtu_start_recv_init(uchar *ptr, Rtu_recv *msg);
    int rtu_start_recv_line_data(uchar *ptr, Rtu_recv *msg , int index);
    int rtu_start_recv_other_data(uchar *ptr, Rtu_recv *msg);
    int rtu_start_recv_env_data(uchar *ptr, Rtu_recv *msg , int index);
    int rtu_start_recv_thd_data(uchar *ptr, Rtu_recv *msg , int index);
    int rtu_start_recv_some_alarm_data(uchar *ptr, Rtu_recv *msg);
    int rtu_start_recv_last_alarm_data(uchar *ptr, Rtu_recv *msg, int index);
    int rtu_plug_recv_init(uchar *ptr, Rtu_recv *msg);
    int rtu_plug_recv_loop_data(uchar *ptr, Rtu_recv *msg , int index);
    int rtu_plug_recv_thd_pl_data(uchar *ptr, Rtu_recv *msg);
    int rtu_plug_recv_env_data(uchar *ptr, Rtu_recv *msg , int index);
    int rtu_plug_recv_loop_alarm_data(uchar *ptr, Rtu_recv *msg , int index);
    int rtu_plug_recv_env_alarm_data(uchar *ptr, Rtu_recv *msg , int index);
    int rtu_plug_recv_zero_data(uchar *ptr, Rtu_recv *msg);
    bool rtu_recv_crc(uchar *buf, int len, Rtu_recv *msg);

    void loopData(sBoxData *box, Rtu_recv *pkt);
    void loopObjData(sObjectData *loop, int id, RtuRecvLine *data);
    void envData(sEnvData *env, Rtu_recv *pkt);
    void initData(sBoxData *box, Rtu_recv *pkt);
    void thdDataV3(Rtu_recv *pkt);
};

#endif // DEV_SITHREAD_H
