#include "test_transthread.h"
#include <cstring> // 为了使用 std::memcpy

#define CMD_SIZE 8

Test_TransThread::Test_TransThread(QObject *parent) : QObject(parent)
{
     QTimer::singleShot(300,this,SLOT(initFunSLot()));
}


void Test_TransThread::initFunSLot()
{
    mSerial = Cfg::bulid()->item->coms.ser1;//串口5
    mSerialPolar  = Cfg::bulid()->item->coms.ser2;//串口4
    mSerialCtrl = Cfg::bulid()->item->coms.ser3;//串口3

    mItem = Cfg::bulid()->item;
    mPacket = sDataPacket::bulid();
}

void Delay_MSec(unsigned int msec)
{
    QTime _Timer = QTime::currentTime().addMSecs(msec);

    while( QTime::currentTime() < _Timer )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

QString Test_TransThread::sentStep(int step , int i , QString & command ,int extra)
{
    QByteArray recv;
    if(step == 1)
        command = sentResisCommand(i , extra);//GND
    else
        command = sentResisCommand(i , extra);//IR//ACW

    for(int count = 0 ; count < 3 ; count ++)
    {
        if(recv.isEmpty()){

        if(mSerial)
            mSerial->transmit(command.toLatin1(),recv,10);
        }
        if(!recv.isEmpty())
            break;
        Delay_MSec(1000);
    }

    return recv;
}


QString Test_TransThread::sentResisCommand(int command , int extra)
{
    QString str;
    switch(command)
    {
        case ConnectReady://1
            str = "*IDN?\r\n";
            break;
        case Reset://2
            str = "RESET\r\n";
            break;
        case ChoseeFile://3
            str = QString("FL %1\r\n").arg(extra);
            break;
        case StepTotal://4
            str = "ST?\r\n";
            break;
        case Test://5
            str = "TEST\r\n";
            break;
        case ReadData://6
            str = QString("RD %1?\r\n").arg(extra);
            break;
        case TestParm://7
            str = QString("LS2 %1?\r\n").arg(extra);
            break;
        case GndParm://8
            str = QString("LS %1?\r\n").arg(extra);
            break;

    }
    return str ;
}

ushort Test_TransThread::calccrc (ushort crc, uchar crcbuf)
{
    uchar x, kkk=0;
    crc = crc^crcbuf;
    for(x=0;x<8;x++)
    {
        kkk = crc&1;
        crc >>= 1;
        crc &= 0x7FFF;
        if(kkk == 1)
            crc = crc^0xa001;
        crc=crc&0xffff;
    }
    return crc;
}

/**
  * 功　能：CRC校验
  * 入口参数：buf -> 缓冲区  len -> 长度
  * 返回值：CRC
  */
ushort Test_TransThread::rtu_crc(const uchar *buf, int len)
{
    ushort crc = 0xffff;
    for(int i=0; i<len; i++)
        crc = calccrc(crc, buf[i]);
    return crc;
}

QByteArray Test_TransThread::sendCmd(int command)
{
    uchar initialCmd[CMD_SIZE] = {0x01, 0x0F, 0x00, 0x00, 0x00, 0x08, 0x01};
    QString hexCommand = QString::number(command, 16).rightJustified(2, '0');

    bool ok;
    int decimalNumber = hexCommand.toInt(&ok,16);
    uchar value = static_cast<uchar>(decimalNumber);
    initialCmd[sizeof(initialCmd)-1] = static_cast<int>(value);

    int cmdLength = sizeof(initialCmd) / sizeof(initialCmd[0]);
    QByteArray cmdArray(reinterpret_cast<const char*>(initialCmd), cmdLength);

    ushort crc = rtu_crc(reinterpret_cast<const uchar*>(cmdArray.constData()), cmdArray.size());
    cmdArray.append(crc & 0xFF); // 低字节
    cmdArray.append(crc >> 8);   // 高字节

    qDebug() << "cmdArray"<<cmdArray;

    return cmdArray;
}

//串口3控制器
void Test_TransThread::sendCtrlGnd(int command)
{
    QByteArray arry = sendCmd(command);
    QByteArray recv;
    int ret = mSerialCtrl->transmit(arry,recv,20);
    if(!ret) mSerialCtrl->transmit(arry,recv,20);

    QString str = tr("控制器指令%1，ret = %2").arg(command).arg(ret);
    if(ret>0) {str += tr("成功"); mPacket->updatePro(str, true);}
    else {str += tr("失败"); mPacket->updatePro(str, false);}
}

//串口4---极性测试  010300180019
bool Test_TransThread::recvPolarity()
{
    sendCtrlGnd(0); Delay_MSec(2000); sendCtrlGnd(128);
    uchar initialCmd[] = {0x01, 0x03, 0x00, 0x18, 0x00, 0x19};
    int cmdLength = sizeof(initialCmd) / sizeof(initialCmd[0]);
    QByteArray cmdArray(reinterpret_cast<const char*>(initialCmd), cmdLength);
    ushort crc = rtu_crc(reinterpret_cast<const uchar*>(cmdArray.constData()), cmdArray.size());
    cmdArray.append(crc & 0xFF); // 低字节
    cmdArray.append(crc >> 8);   // 高字节

    QByteArray recv;
    int ret = mSerialPolar->transmit(cmdArray,recv,20);
    if(!ret){
        ret = mSerialPolar->transmit(cmdArray,recv,20);
        if(!ret) return 0;
    }

    QString str = recv.toHex();
    int start = 6; // 从第七个字符开始（注意：QString的索引从0开始）
    int length = 4; // 长度为4
    int temp = 0;

    QStringList result;
    QList<int> Intresult;
    bool res = true;

    if(str.size()>42) {
        for (int i = start; i <= (start + length*9); i += length) {
            QString subString = str.mid(i, length);
            result.append(subString);
            bool ok;
            int value = result.at(temp).toInt(&ok, 16);
            Intresult.append(value);
            temp ++;
        }
        //单相设备-----------------------------
        int volValue = 0;  int loop = mItem->si.loopNum/3;
        if(mItem->si.si_phaseflag == 0){
            for(int i=0;i<9;i +=4){
                volValue = Intresult.at(i)*3 /100;// result.at(1);
                if(i==0){
                    if(volValue <15) { res = false; mPacket->updatePro("第一组单相，A相线序故障", res); }
                }else if(i==4){
                    if(volValue <8) {res = false; mPacket->updatePro("第二组单相，B相线序故障", res); }
                }else if(i==8){
                    if(volValue <3) {res = false; mPacket->updatePro("第三组单相，C相线序故障", res); }
                }
            }
        }else {
            QString error;
            for(int i=0;i<3*loop;i++){
                volValue = Intresult.at(i)*3 /100;// result.at(1);
                if((i==0) || (i==3) ||(i==6)){
                    if(volValue <15) {
                        res = false; error = tr("第%1组单相，A相线序故障").arg(transStr(i));
                        mPacket->updatePro(error, res); }
                }else if((i==1) || (i==4) ||(i==7)){
                    if(volValue <8) {
                        res = false; error = tr("第%1组单相，B相线序故障").arg(transStr(i));
                        mPacket->updatePro(error, res); }
                }else if((i==2) || (i==5) ||(i==8)){
                    if(volValue <3) {
                        res = false; error = tr("第%1组单相，C相线序故障").arg(transStr(i));
                        mPacket->updatePro(error, res); }
                }
            }
        }
    }else {
        res = false; mPacket->updatePro("极性采集数据失败", false);
    }

    sendCtrlGnd(0);

    return res;
}

QString Test_TransThread::transStr(int command)
{
    QString str;
    switch(command){
    case 0: str = "一"; break;
    case 1: str = "一"; break;
    case 2: str = "一"; break;
    case 3: str = "二"; break;
    case 4: str = "二"; break;
    case 5: str = "二"; break;
    case 6: str = "三"; break;
    case 7: str = "三"; break;
    case 8: str = "三"; break;
    default: break;
    }

    return str;
}
