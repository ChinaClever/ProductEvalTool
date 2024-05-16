/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "rtu_write.h"

Rtu_Write::Rtu_Write(QObject *parent) : Rtu_Read(parent)
{

}

bool Rtu_Write::writes(sRtuSetItems &pkt)
{
    bool ret = false;
    uchar sendBuf[256]={0}, recvBuf[128]={0};
    int rtn = rtu_write_packets(&pkt, sendBuf);
    rtn = transmit(sendBuf, rtn, recvBuf, 4);
    if(rtn > 0) {
        ret = rtuRecvCrc(recvBuf, rtn);
        if(ret) {
            uchar fn = recvBuf[1];
            if(fn < 0x80) { // 设置正常
                ret = true;
            }
        }
    }
    return ret;
}

bool Rtu_Write::write(sRtuSetItem &pkt)
{
    bool ret = false;
    return ret;
}

bool Rtu_Write::write(Rtu_Sent_Single_Ushort_V3 &pkt)
{
    bool ret = false;
    uchar sendBuf[256]={0}, recvBuf[128]={0};
    int rtn = rtu_write_packet(&pkt, sendBuf);
    rtn = transmit(sendBuf, rtn, recvBuf, 2);
    if(rtn > 0) {
        ret = rtuRecvCrc(recvBuf, rtn);
        if(ret) {
            uchar fn = recvBuf[1];
            if(fn < 0x80) { // 设置正常
                ret = true;
            }
        }
    }
    return ret;
}

bool Rtu_Write::write(Rtu_Sent_Ushort_V3 &pkt)
{
    bool ret = false;
    uchar sendBuf[256]={0}, recvBuf[128]={0};
    int rtn = rtu_sent_packet_ushort_V3(&pkt, sendBuf);
    rtn = transmit(sendBuf, rtn, recvBuf, 2);
    if(rtn > 0) {
        ret = rtuRecvCrc(recvBuf, rtn);
        if(ret) {
            uchar fn = recvBuf[1];
            if(fn < 0x80) { // 设置正常
                ret = true;
            }
        }
    }
    return ret;
}

bool Rtu_Write::write(Rtu_Sent_Uint_V3 &pkt)
{
    bool ret = false;
    uchar sendBuf[256]={0}, recvBuf[128]={0};
    int rtn = rtu_sent_packet_uint_V3(&pkt, sendBuf);
    rtn = transmit(sendBuf, rtn, recvBuf, 2);
    if(rtn > 0) {
        ret = rtuRecvCrc(recvBuf, rtn);
        if(ret) {
            uchar fn = recvBuf[1];
            if(fn < 0x80) { // 设置正常
                ret = true;
            }
        }
    }
    return ret;
}


int Rtu_Write::rtu_write_packet(Rtu_Sent_Single_Ushort_V3 *pkt, uchar *ptr)
{
    uchar *buf = ptr;
    *(ptr++) = pkt->addr;  /*地址码*/
    *(ptr++) = pkt->fn; /*功能码*/

    /*填入寄存器首地址*/
    *(ptr++) = ((pkt->reg) >> 8); /*高8位*/
    *(ptr++) = (0xff)&(pkt->reg); /*低8位*/

    /*填入寄存器个数*/
    *(ptr++) = ((pkt->num) >> 8); /*高8位*/
    *(ptr++) = (0xff)&(pkt->num); /*低8位*/

    *(ptr++) = pkt->num * 2;

    /*填入数据2*/
    *(ptr++) = ((pkt->val) >> 8); /*高8位*/
    *(ptr++) = (0xff)&(pkt->val); /*低8位*/

    /*填入CRC*/
    pkt->crc = rtu_crc(buf, 9);
    *(ptr++) = (0xff)&(pkt->crc); /*低8位*/
    *(ptr++) = ((pkt->crc) >> 8); /*高8位*/

    return 11;
}

int Rtu_Write::rtu_write_packets(sRtuSetItems *pkt, uchar *ptr)
{
    uchar *buf = ptr;
    *(ptr++) = pkt->addr;  /*地址码*/
    *(ptr++) = pkt->fn; /*功能码*/

    /*填入寄存器首地址*/
    *(ptr++) = ((pkt->reg) >> 8); /*高8位*/
    *(ptr++) = (0xff)&(pkt->reg); /*低8位*/

    *(ptr++) = ((pkt->num) >> 8); /*长度高8位*/
    *(ptr++) = (0xff)&(pkt->num); /*低8位*/
    *(ptr++) = pkt->len;

    for(int i=0; i<pkt->len; ++i) {
        *(ptr++) = pkt->data[i];
    }

    /*填入CRC*/
    pkt->crc = rtu_crc(buf, 7+pkt->len);
    *(ptr++) = (0xff)&(pkt->crc); /*低8位*/
    *(ptr++) = ((pkt->crc) >> 8); /*高8位*/

    return 9 + pkt->len;
}

/**
  * 功　能：发送数据打包
  * 入口参数：pkt -> 发送结构体
  * 出口参数：ptr -> 缓冲区
  * 返回值：打包后的长度
  */
int Rtu_Write::rtu_sent_packet_ushort_V3(Rtu_Sent_Ushort_V3 *pkt, uchar *ptr)
{
    uchar *buf = ptr;
    *(ptr++) = pkt->addr;  /*地址码*/
    *(ptr++) = pkt->fn; /*功能码*/

    /*填入寄存器首地址*/
    *(ptr++) = ((pkt->reg) >> 8); /*高8位*/
    *(ptr++) = (0xff)&(pkt->reg); /*低8位*/

    /*填入寄存器个数*/
    *(ptr++) = ((pkt->num) >> 8); /*高8位*/
    *(ptr++) = (0xff)&(pkt->num); /*低8位*/

    *(ptr++) = pkt->num * 2;

    /*填入数据1*/
    *(ptr++) = ((pkt->val1) >> 8); /*高8位*/
    *(ptr++) = (0xff)&(pkt->val1); /*低8位*/

    /*填入数据2*/
    *(ptr++) = ((pkt->val2) >> 8); /*高8位*/
    *(ptr++) = (0xff)&(pkt->val2); /*低8位*/

    /*填入CRC*/
    pkt->crc = rtu_crc(buf, 11);
    *(ptr++) = (0xff)&(pkt->crc); /*低8位*/
    *(ptr++) = ((pkt->crc) >> 8); /*高8位*/

    return 13;
}

/**
  * 功　能：发送数据打包
  * 入口参数：pkt -> 发送结构体
  * 出口参数：ptr -> 缓冲区
  * 返回值：打包后的长度
  */
int Rtu_Write::rtu_sent_packet_uint_V3(Rtu_Sent_Uint_V3 *pkt, uchar *ptr)
{
    uchar *buf = ptr;
    *(ptr++) = pkt->addr;  /*地址码*/
    *(ptr++) = pkt->fn; /*功能码*/

    /*填入寄存器首地址*/
    *(ptr++) = ((pkt->reg) >> 8); /*高8位*/
    *(ptr++) = (0xff)&(pkt->reg); /*低8位*/

    /*填入寄存器个数*/
    *(ptr++) = ((pkt->num) >> 8); /*高8位*/
    *(ptr++) = (0xff)&(pkt->num); /*低8位*/

    *(ptr++) = pkt->num * 2;

    /*填入数据1*/
    *(ptr++) = ((pkt->val1) >> 8); /*高8位*/
    *(ptr++) = (0xff)&(pkt->val1); /*低8位*/

    /*填入数据2*/
    *(ptr++) = ((pkt->val2) >> 8); /*高8位*/
    *(ptr++) = (0xff)&(pkt->val2); /*低8位*/

    /*填入数据3*/
    *(ptr++) = ((pkt->val3) >> 8); /*高8位*/
    *(ptr++) = (0xff)&(pkt->val3); /*低8位*/

    /*填入数据4*/
    *(ptr++) = ((pkt->val4) >> 8); /*高8位*/
    *(ptr++) = (0xff)&(pkt->val4); /*低8位*/

    /*填入CRC*/
    pkt->crc = rtu_crc(buf, 15);
    *(ptr++) = (0xff)&(pkt->crc); /*低8位*/
    *(ptr++) = ((pkt->crc) >> 8); /*高8位*/

    return 17;
}

void Rtu_Write::autoSetAddress()
{
    uchar recvbuffer[1024];
    memset(recvbuffer,0,sizeof(recvbuffer));
    int count = 6;

    while(count--){
        memset(recvbuffer,0,sizeof(recvbuffer));
        int rtn = readSerial(recvbuffer,5);
        QByteArray array2;
        QString strArray2;
        array2.append((char *)recvbuffer, rtn);
        strArray2 = array2.toHex(); // 十六进制
        qDebug()<<" strArray2 "<<strArray2;
        if(rtn / 8 == 1 && strArray2.contains("ff7b"))emit sendNumAndIndexSig(recvbuffer[5]);
        if(rtn / 8 == 2 && recvbuffer[8]==0xff && recvbuffer[9]==0x7b)emit sendNumAndIndexSig(recvbuffer[8+5]);
        if(rtn / 8 >= 3 && recvbuffer[16]==0xff && recvbuffer[17]==0x7b){qDebug()<<"sendNumAndIndexSig";emit sendNumAndIndexSig(recvbuffer[16+5]);break;}
        //if(recvbuffer[0]==0x01 && recvbuffer[1]==0x6a){ emit sendDelaySig();break;}
        if(recvbuffer[5] == 0xCC){count=-1;break;}
    }
    if(count == -1){ emit sendDelaySig(); }
}
