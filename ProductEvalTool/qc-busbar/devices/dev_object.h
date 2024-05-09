#ifndef DEV_OBJECT_H
#define DEV_OBJECT_H

#include "rtu_modbus.h"

struct Rtu_Sent {
    Rtu_Sent():fn(3),reg(0),len(RTU_SENT_LEN_V30){} // 下位机有问题
    uchar addr; // 表示从机地址码
    uchar fn;  // 表示功能码
    ushort reg; // 表示寄存器首地址
    uint len; // 表示数据长度
    ushort crc; // 表示CRC校验
};



struct RtuUshortUnit {
    ushort svalue;
    ushort smin;
    ushort smax;//剩余电流时，存储报警界限值

    ushort scrMin; // 临界最小值
    ushort scrMax; // 临界最大值
    uchar supalarm;
    uchar salarm;
    uchar scrAlarm; // 临界告警值
};
struct RtuCurUintUnit {
    ushort svalue;
    ushort smin;
    ushort smax;//剩余电流时，存储报警界限值

    ushort scrMin; // 临界最小值
    ushort scrMax; // 临界最大值
    uchar supalarm;
    uchar salarm;
    uchar scrAlarm; // 临界告警值
};
struct RtuUintUnit {
    uint ivalue;
    uint imin;
    uint imax;

    uint icrMin; // 临界最小值
    uint icrMax; // 临界最大值

    uchar iupalarm;
    uchar ialarm;
    uchar icrAlarm; // 临界告警值
};

struct RtuRecvLine {
    RtuUshortUnit vol; // 电压
    RtuCurUintUnit cur; // 电流
    RtuUshortUnit lineVol; // 线电压
    RtuUintUnit pow; //有功功率
    uint apPow; // 视在功率
    uint ele; // 电能值

    ushort wave; // 谐波值
    uchar pf; // 功率因素
    uchar sw; // 开关
    uint reactivePower;// 无功功率
};

struct RtuEnvUnit {
    ushort value;
    ushort min;
    ushort max;
    uchar upalarm;
    uchar alarm;
    ushort crMin; // 临界最小值
    ushort crMax; // 临界最大值
    uchar crAlarm; // 临界告警值
};

struct RtuRecvEnv {
    RtuEnvUnit tem; // 温度
};

struct Rtu_recv {
    uchar addr; // 表示从机地址码
    uchar fn; // 表示功能码
    ushort len; // 表示数据字节数
    ushort version;// 表示版本
    uchar curSpecification;// 表示电流规格
    uchar workMode;// 0表示始端箱和插接箱主从模式 1表示始端箱Modbus模式
    uchar baudRate;// 表示波特率
    uchar iOF;// 表示iOF触点
    uchar alarmTime;// 表示告警滤波2-5
    uchar shuntRelease;// 表示分励脱扣
    uchar lightning; // 防雷开关
    uchar lpsState; // 防雷开关
    uchar breaker;// 始端箱开关状态
    uchar isd;// ISD状态
    uchar reState;// 剩余电流检测模块选配情况
//    uchar boxType;//插接箱 ： 0  温控 ：1
    uchar phaseFlag;//  0 ：单相  1 ：三相
    RtuRecvLine data[RTU_LOOP_NUM];
    RtuRecvEnv  env[RTU_TH_NUM];
    RtuUintUnit totalPow; //总有功功率
    RtuUshortUnit reCur;//剩余电流
    RtuCurUintUnit zeroLineCur;//零线电流
    RtuUshortUnit rate;//频率
    ushort volThd[RTU_THD_NUM][40];
    ushort curThd[RTU_THD_NUM][40];
    uchar lineNum;
    uchar proNum;//项目编号 0：标准 1：定制

    uchar volUnbalance;
    uchar curUnbalance;
    uchar buzzerStatus;//蜂鸣器
    uchar dc; // 交直流标志位
    uchar type;//箱子类型 0：插接箱 1： 始端箱

    uchar pl[3]; // 负载百分比
    uchar hc; //谐波通道(00表示A路电压通道，01表示B路电压通道，02表示C路电压通道 03表示A路电流通道，04表示B路电流通道，05表示C路电流通道)
    ushort thd[40]; //   谐波含量
    unsigned long long totalApPow;//   总视在功率
    ushort crc; // 检验码
};

class Dev_Object : public QThread
{
    Q_OBJECT
public:
    explicit Dev_Object(QObject *parent = nullptr);

    virtual bool readPduData()=0;
    void setModbus(int id);
    void changeBaudRate() {mModbus->changeBaudRate();}
    ushort rtu_crc(uchar *buf, int len);

protected:
    uchar *toInt(uchar *ptr, int line, uint *value);
    uchar *toChar(uchar *ptr, int line, uchar *value);
    uchar *toShort(uchar *ptr, int line, ushort *value);

    ushort getShort(uchar *ptr);
    uchar *getShort(uchar *ptr, int line, uchar *value);
    uchar *toOutputEle(uchar *ptr, int line, uint *value);
    uchar *toThreshold(uchar *ptr, int line, sDataUnit &unit);
    uchar *toEnvTh(uchar *ptr, sDataUnit &unit);
    uchar *toShort10(uchar *ptr, int line, ushort *value, int v);
    uchar *toThreshold10(uchar *ptr, int line, sDataUnit &unit);

    ushort calccrc(ushort crc, uchar crcbuf);
protected slots:
    virtual void initFunSlot();

protected:
    sDevType *mDt;
    sDevData *mDev;
    RtuRw *mModbus;
    sCfgItem *mItem;
    sProgress *mPro;
    sDataPacket *mPacket;
    sBusData *mBusData;
};

#endif // DEV_OBJECT_H
