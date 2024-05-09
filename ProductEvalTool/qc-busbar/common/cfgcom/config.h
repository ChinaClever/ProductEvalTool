#ifndef CONFIGBASH
#define CONFIGBASH
#include "cfgcom.h"
#include "serialstatuswid.h"
#include "datapacket.h"

enum eDevTypes {
    PDU=0,
    ZPDU,
    MPDU,
    RPDU,     // RPDU 执行板
    RPDU_Mc, // RPDU 主控
    SI_PDU,
    IP_PDU,
    BM_PDU,

    IDC_BUSBAR,

    AC = 1, // 交流
    DC,     // 直流

    Sum=0,
    Transformer, // 互感器
    Mn    // 锰铜
};

enum eIpTypes {
    IP_PDUV1=1,         // IPV1标准
    IP_PDUV3=3,         // IPV3标准
    IP_PDUV3_C3,        // IP-PDU 俄罗斯C3
    IP_PDUV3_EATON,     // IP-PDU伊顿
    IP_PDUV6,           // IPV6标准
    IP_PDUV6_HUAWEI,     // IPV6华为
    IP_PDUV1_YIXIN,      // IPV1易信
    IP_PDUV1_HUADA,     // IPV1华大    9
    IP_PDUV3_BYTE,    // IPV3字节    10
    IP_PDUV3_SHATE,     // IPV3沙特    11
    IP_PDUV3_ECC,    // IPV3前海    12

    START_BUSBAR=0,
    INSERT_BUSBAR,
    TEMPER_BUSBAR,
    SNMP=0,
    MODBUS
};

/**
 * 数据单元：包括当前值，阈值，临界值，告警状态等
 */
struct sUnitCfg
{
    uchar en;
    uchar id;
    uint rate;
    float min; // 最小值
    float max; // 最大值

    float crMin; // 最小值
    float crMax; // 最大值
};

/**
 * 数据对象：包括电流，电压，功率，电能，开关状态，插接位名称
 */
struct sObjCfg
{
    sUnitCfg vol; // 电压
    sUnitCfg cur; // 电流
    sUnitCfg pow;

    sUnitCfg tem; // 温度
    sUnitCfg hum; // 湿度
    sUnitCfg linevol; // 线电压
    sUnitCfg hz; // 频率
    sUnitCfg totalpow;
    sUnitCfg zerocur; // 零线电流
    sUnitCfg recur; // 剩余电流
};


struct sIpCfg {
    QString addr;
    ushort version;
    uchar language; //
    uchar lines;
    uchar modbus; //  1 表示 Modbus版本
    uchar log;
    uchar standard;
    uchar security;
    uchar lcd;
    uchar protocol;//  0 表示静态协议 1表示DHCP
    QString inFirst;
    QString inSecond;
    QString outFirst;
    QString outSecond;

    uchar ip_curtype;
    uchar ip_buzzer;
    uchar ip_filter;
    uchar ip_mode;
    uchar ip_num;
    uchar ip_ISD;
    uchar ip_iOF;
    uchar ip_shunt;
    uchar ip_residual;
    uchar ip_lightning;
};

struct sSiCfg {
    uchar si_mod;
    bool led;
    uchar ac;
    uchar lines;

    uchar si_baud;
    uchar si_buzzer;
    uchar si_filter;
    uchar si_iOF;
    ushort si_version;
    ushort si_phaseflag;

    //四个温度
    double temMax[4];
    double temMin[4];
    ushort tem_version;
    uchar tem_buzzer;
    uchar tem_filter;
};

struct sCfgDev {
    uchar type; // 0 不设置， 1 32安  2 16安， 3 63安  4 自定义
    uchar enModify;
    ushort vol_min;
    ushort vol_max;
    ushort cur_min;
    ushort cur_max;
    ushort loopcur_min;
    ushort loopcur_max;
};


/**
 * RTU传输统计结构体
 */
struct sCount
{
    sCount() {all=ok=err =0;}

    int num;
    int all;
    int ok;
    int err;
};

struct sErrRange
{
    ushort volErr;
    uint curErr;
    ushort powErr;
};

struct sSerial
{
    sSerial() {ser3=ser1=ser2=nullptr;}
    SerialPort *ser3; // 标准源
    SerialPort *ser1; // 串口对象
    SerialPort *ser2; // 串口对象
};

struct sCfgItem
{
    sErrRange err; // 电流误差
    sCfgDev cTh;
    sObjCfg si_cfg;
    sObjCfg ip_cfg;
    sIpCfg ip;
    sSiCfg si;
    sCount cnt;
    int logCount;

    uchar pcNum;
    QString user; // 客户名称
    QString sw_ver;
    QString hw_ver;
    QString pn;
    sSerial coms;
    ushort currentNum;
    bool eleCheck;
    bool temCheck;
    bool vref;
    bool printer;
    bool macprinter;
    uchar macCheck;
    uchar modeId;
    uchar addr;
    QString sn;//成品序列号
};


class Cfg
{
    Cfg();
public:
    static Cfg *bulid();

    sCfgItem *item;

    QString getSerialBr(const QString &com);
    QString getSerialName(const QString &key);
    void setSerialBr(const QString &com, const QString &br);
    void setSerialName(const QString &key, const QString &v);

    QString getLoginName();
    void setLoginName(const QString &name);

    void initCnt();
    int initAddr();
    void initPrint();
    void setPrinter();
    void writeCnt();
    void writeErrData();
    void writeCfgDev();
    void setAddr(int addr);

    void setCurrentNum();
    void write(const QString &key, const QVariant& v, const QString &g="Cfg");
    QVariant read(const QString &key, const QVariant &v = QVariant(), const QString &g="Cfg");

protected:

    void initErrData();
    void initCfgDev();
    bool getDate();
    void setDate();
    void initCurrentNum();

    void initSiData(const QString &g);
    void initIpData(const QString &g);
    void writeSiData(const QString &g);
    void writeIpData(const QString &g);
    void writeUnit(const QString& prefix, sUnitCfg &unit, const QString &g, int f=1);
    void initUnit(const QString& prefix, sUnitCfg &unit, const QString &g, int f=1);

private:
    CfgCom *mCfg;
};

#endif // CONFIGBASH
