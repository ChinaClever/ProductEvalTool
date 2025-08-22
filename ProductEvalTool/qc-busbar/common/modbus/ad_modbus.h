#ifndef AD_MODBUS_H
#define AD_MODBUS_H

#include "dbstates.h"

#define MODBUS_RTU_SIZE 255

struct sRtuItemRK{
    sRtuItemRK():fn(3),reg(0),num(0){} // 下位机有问题
    uchar addr; // 表示从机地址码
    uchar fn;  // 表示功能码
    ushort reg; // 表示寄存器首地址
    ushort num; // 表示数据长度
    ushort crc; // 表示CRC校验
};

struct sRtuReplyItemRK {
    sRtuReplyItemRK():fn(3),len(0){} // 下位机有问题
    uchar addr; // 表示从机地址码
    uchar fn;  // 表示功能码
    uchar len; // 表示数据长度
    uchar data[MODBUS_RTU_SIZE]; // 表示寄存器首地址
    ushort crc; // 表示CRC校验
};

struct sRtuSetItemRK
{
    uchar addr;
    uchar fn;
    ushort reg;
    ushort data;
    ushort crc;
};

struct sRtuSetItemsRK
{
    uchar addr;
    uchar fn;
    ushort reg;
    ushort num;
    uchar len;
    uchar data[MODBUS_RTU_SIZE];
    ushort crc;
};

struct sRtuSetReplyItemRK
{
    uchar addr;
    uchar fn;
    uchar err;
    ushort crc;
};


class Ad_Modbus : public QThread
{
    Q_OBJECT
    explicit Ad_Modbus(QObject *parent = nullptr);
public:
    static Ad_Modbus *bulid(QObject *parent = nullptr);
    void init(SerialPort *s){mSerial=s;}
    int readSerial(quint8 *cmd, int secs);
    bool writeSerial(quint8 *cmd, int len);
    int transmit(uchar *sent, int len, uchar *recv, int secs);

    ushort rtu_crc(uchar *buf, int len);
    uchar getXorNumber(uchar *buf, int len);

    int read(sRtuItemRK &pkt, uchar *recv);
    int rtuRead(sRtuItemRK *pkt, sRtuReplyItemRK *recv);

    bool rtuWrite(sRtuSetItemRK *pkt);
    bool rtuWrites(sRtuSetItemsRK *pkt);
    // bool delay(int s);

    // bool appendLogItem(bool pass);
    bool changeBaudRate();
    void writeLogs();
    void autoSetAddress();


protected:
    void initSerial();
    ushort calccrc (ushort crc, uchar crcbuf);
    int rtu_sent_packet(sRtuItemRK *pkt, uchar *ptr);
    int rtuRecvData(uchar *ptr,  sRtuReplyItemRK *pkt);
    int rtuRecvIPData(uchar *ptr, sRtuReplyItemRK *pkt);

    bool rtuRecvCrc(uchar *buf, int len);
    bool rtuRecvCrc(uchar *buf, int len, sRtuReplyItemRK *msg);

    int rtu_write_packet(sRtuSetItemRK *pkt, uchar *ptr);
    int rtu_write_packets(sRtuSetItemsRK *pkt, uchar *ptr);

private:
    SerialPort *mSerial;
    QList<sStateItem> mLogItems;
};

#endif // AD_MODBUS_H
