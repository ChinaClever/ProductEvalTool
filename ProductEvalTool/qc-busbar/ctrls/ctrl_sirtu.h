#ifndef CTRL_SIRTU_H
#define CTRL_SIRTU_H

#include "ctrl_iprtu.h"

class Ctrl_SiRtu : public Ctrl_Object
{
    Q_OBJECT
    explicit Ctrl_SiRtu(QObject *parent = nullptr);
public:
    static Ctrl_SiRtu *bulid(QObject *parent = nullptr);

    bool setModel();
    void eleClean();
    bool factorySet();
    bool setCurTh(int i);
    bool setVolTh(int i);
    bool sentRtuCmd(ushort reg, ushort value, uchar fn=0x10);
    bool setHorizontaOrVertical();

    bool setBusbarStartEle(int index);//index [1,3]
    bool setBusbarStartPow(int val1 , int val2);
    bool setBusbarStartLineVol(int index , int val1 , int val2);

    bool setBusbarInsertEle(int index);//index [1,9]
    bool setBusbarInsertPow(int index , int val1 , int val2);
    bool setBusbarInsertVol(int index , int val1 , int val2);
    bool setBusbarInsertTem(int index , int val1 , int val2);
    bool setBusbarInsertCur(int index , int val1 , int val2);
    bool setBusbarInsertBaud(int val);
    bool setBusbarInsertiOF(int val);
    bool setBusbarInsertBuzzer(int val);
    bool setBusbarInsertFilter(int val);


    bool rtu_sent_ushortV3_buff(uchar addr, ushort reg, uint num,  uint val1, uint val2);
    bool rtu_sent_uintV3_buff(uchar addr, ushort reg, uint num,  uint val1, uint val2);

};

#endif // CTRL_SIRTU_H
