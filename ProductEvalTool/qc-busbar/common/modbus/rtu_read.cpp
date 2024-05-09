/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "rtu_read.h"
#include "common.h"
Rtu_Read::Rtu_Read(QObject *parent) : Rtu_Serial(parent)
{

}



/**
  * 功　能：发送数据打包
  * 入口参数：pkt -> 发送结构体
  * 出口参数：ptr -> 缓冲区
  * 返回值：打包后的长度
  */
int Rtu_Read::rtuPacket(sRtuItem *pkt, uchar *ptr)
{
    uchar *buf = ptr;
    *(ptr++) = pkt->addr;  /*地址码*/
    *(ptr++) = pkt->fn; /*功能码*/

    /*填入寄存器首地址*/
    *(ptr++) = ((pkt->reg) >> 8); /*高8位*/
    *(ptr++) = (0xff)&(pkt->reg); /*低8位*/

    /*填入数据长度*/
    *(ptr++) = ((pkt->num) >> 8); /*长度高8位*/
    *(ptr++) = (0xff)&(pkt->num); /*低8位*/

    /*填入CRC*/
    pkt->crc = rtu_crc(buf, 6);
    *(ptr++) = (0xff)&(pkt->crc); /*低8位*/
    *(ptr++) = ((pkt->crc) >> 8); /*高8位*/

    return 8;
}

bool Rtu_Read::rtuRecvCrc(uchar *buf, int len)
{
    bool ret = true;
    int rtn = len-2; uchar *ptr = buf+rtn;
    if(rtn < 0) return false;

    ushort crc = (ptr[1]*256) + ptr[0]; // 获取校验码
    ushort res = rtu_crc(buf, rtn);
    if(crc != res) {
        ret = false;
        qDebug() << "Rtu Recv rtu recv crc Err!";
    }

    return ret;
}

bool Rtu_Read::recvCrc(uchar *buf, int len, sRtuReplyItem *msg)
{
    bool ret = rtuRecvCrc(buf, len);
    if(ret) {
        int rtn = len-2; uchar *ptr = buf+rtn;
        msg->crc = (ptr[1]*256) + ptr[0]; // 获取校验码
    }

    return ret;
}

int Rtu_Read::rtuRecvSn(uchar *ptr, sRtuReplyItem *pkt)
{
    pkt->addr = *(ptr++);// 从机地址码
    pkt->fn = *(ptr++);  /*功能码*/
    pkt->len = *(ptr++); /*数据长度*/
    if(0 == pkt->len) pkt->len = *(ptr++); /*SI-PDU数据长度是二个字节*/
    if(pkt->len < MODBUS_RTU_SIZE) {
        for(int i=0; i<pkt->len; ++i) {
            pkt->data[i] = *(ptr++);   
        }
    } else {
        pkt->len = 0;
    }

    return pkt->len;
}
int Rtu_Read::rtuRecvData(uchar *ptr, sRtuReplyItem *pkt)
{
    //    pkt->addr = *(ptr++);// 从机地址码
    //    pkt->fn = *(ptr++);  /*功能码*/
        pkt->len = (ptr[2]) * 256 + (ptr[3]) + 6; /*数据长度*/
    //    ptr+=2; /*IP-PDU数据长度*/
        if(pkt->len < MODBUS_RTU_SIZE) {
            for(int i=0; i<pkt->len; ++i) {
                pkt->data[i] = *(ptr++);
            }
        } else {
            pkt->len = 0;
        }

        return pkt->len;
}

int Rtu_Read::rtuRead(sRtuItem *pkt, sRtuReplyItem *recv)
{
    uchar sendBuf[64]={0}, recvBuf[MODBUS_RTU_SIZE]={0};
    int rtn = rtuPacket(pkt, sendBuf);
    rtn = transmit(sendBuf, rtn, recvBuf, 3);
    if(rtn > 0) {
        bool ret = recvCrc(recvBuf, rtn, recv);
        if(ret) {
            rtn = rtuRecvData(recvBuf, recv);
        } else {
            rtn = 0;
        }
    }

    return rtn;
}
int Rtu_Read::rtuReadSn(sRtuItem *pkt, sRtuReplyItem *recv)
{
    uchar sendBuf[64]={0}, recvBuf[MODBUS_RTU_SIZE]={0};
    int rtn = rtuPacket(pkt, sendBuf);
    rtn = transmit(sendBuf, rtn, recvBuf, 3);

    if(rtn > 0) {
        bool ret = recvCrc(recvBuf, rtn, recv);
        if(ret) {
            rtn = rtuRecvSn(recvBuf, recv);
        } else {
            rtn = 0;
        }
    }

    return rtn;
}

int Rtu_Read::read(sRtuItem &pkt, uchar *recv)
{
    sRtuReplyItem item;
    int ret = rtuRead(&pkt, &item);
    if(ret > 0) {
        for(int i=0; i<ret; ++i) {
            recv[i] = item.data[i];
        }
    }

    return ret;
}
int Rtu_Read::readSn(sRtuItem &pkt, uchar *recv)
{
    sRtuReplyItem item;
    int ret = rtuReadSn(&pkt, &item);
    if(ret > 0) {
        for(int i=0; i<ret; ++i) {
            recv[i] = item.data[i];
        }
    }

    return ret;
}
int Rtu_Read::read(uchar *recv)
{
    int ret = readSerial(recv , 3);

    return ret;
}

