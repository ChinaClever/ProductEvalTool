#ifndef RK_SERIAL_H
#define RK_SERIAL_H

#include "serialport.h"
#include "ad_modbus.h"

#define TEST_ARRAY_SIZE 3
struct sRkItem
{
    ushort header;
    uchar flag;
    uchar rkId;   // 测试仪型号
    QString rkStr;
    uchar enAlarm; //是否启用报警及是否存在报警
    ushort vol;// 电压
    uint vola[TEST_ARRAY_SIZE]; // 电压
    uchar curUnit; //电流单位
    uint cur; //电流
    uint cura[TEST_ARRAY_SIZE];
    uint pow; //功率
    uint powa[TEST_ARRAY_SIZE];
    uint ele; //功
    ushort hz; //频率
    ushort pf; //功率因素
    uchar crc;
};


class Rk_Serial : public QThread
{
    Q_OBJECT
public:
    explicit Rk_Serial(QObject *parent = nullptr);
    void init(SerialPort *s){mSerial=s;}
    bool readPacket(sRkItem &it);
    bool readPacket1(sRkItem &it);
    bool clearEle();
protected:
    int transmit(uchar *recv);
    QString rkType(int id);

private:
    SerialPort *mSerial;
    Ad_Modbus *mModbus;
};

#endif // RK_SERIAL_H
