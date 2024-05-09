#ifndef RTU_WRITE_H
#define RTU_WRITE_H

#include "rtu_read.h"

struct sRtuSetItem
{
    uchar addr;
    uchar fn;
    ushort reg;
    ushort data;
    ushort crc;
};

struct sRtuSetItems
{
    uchar addr;
    uchar fn;
    ushort reg;
    ushort num;
    uchar len;
    uchar data[MODBUS_RTU_SIZE];
    ushort crc;
};

struct sRtuSetReplyItem
{
    uchar addr;
    uchar fn;
    uchar err;
    ushort crc;
};

struct Rtu_Sent_Single_Ushort_V3 {
    Rtu_Sent_Single_Ushort_V3():fn(3),reg(0),num(2){} // 下位机有问题
    uchar addr; // 表示从机地址码
    uchar fn;  // 表示功能码
    ushort reg; // 表示寄存器首地址
    ushort num; // 表示寄存器个数
    ushort val; // 表示
    ushort crc; // 表示CRC校验
};

struct Rtu_Sent_Ushort_V3 {
    Rtu_Sent_Ushort_V3():fn(3),reg(0),num(2){} // 下位机有问题
    uchar addr; // 表示从机地址码
    uchar fn;  // 表示功能码
    ushort reg; // 表示寄存器首地址
    ushort num; // 表示寄存器个数
    ushort val1; // 表示最小值
    ushort val2; // 表示最大值
    ushort crc; // 表示CRC校验
};

struct Rtu_Sent_Uint_V3 {
    Rtu_Sent_Uint_V3():fn(3),reg(0),num(4){} // 下位机有问题
    uchar addr; // 表示从机地址码
    uchar fn;  // 表示功能码
    ushort reg; // 表示寄存器首地址
    ushort num; // 表示寄存器个数
    ushort val1; // 表示最小值低位
    ushort val2; // 表示最小值高位
    ushort val3; // 表示最大值低位
    ushort val4; // 表示最大值高位
    ushort crc; // 表示CRC校验
};

class Rtu_Write : public Rtu_Read
{
    Q_OBJECT
public:
    explicit Rtu_Write(QObject *parent = nullptr);

    bool write(sRtuSetItem &pkt);
    bool writes(sRtuSetItems &pkt);
    bool write(Rtu_Sent_Single_Ushort_V3 &pkt);
    bool write(Rtu_Sent_Ushort_V3 &pkt);
    bool write(Rtu_Sent_Uint_V3 &pkt);

protected:
    int rtu_write_packet(Rtu_Sent_Single_Ushort_V3 *pkt, uchar *ptr);
    int rtu_write_packets(sRtuSetItems *pkt, uchar *ptr);

    int rtu_sent_packet_uint_V3(Rtu_Sent_Uint_V3 *pkt, uchar *ptr);
    int rtu_sent_packet_ushort_V3(Rtu_Sent_Ushort_V3 *pkt, uchar *ptr);
};

typedef Rtu_Write RtuRw;

#endif // RTU_WRITE_H
