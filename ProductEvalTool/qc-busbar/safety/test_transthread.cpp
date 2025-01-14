#include "test_transthread.h"
#include <cstring> // 为了使用 std::memcpy
Test_TransThread::Test_TransThread(QObject *parent) : QObject(parent)
{
     QTimer::singleShot(300,this,SLOT(initFunSLot()));
}


void Test_TransThread::initFunSLot()
{
    mSerial = Cfg::bulid()->item->coms.ser1;
    mSerialGND  = Cfg::bulid()->item->coms.ser2;
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
            // if(step == 1)
            //     mSerialGND->transmit(command.toLatin1(),recv,10);
            // else{
            if(mSerial)
                mSerial->transmit(command.toLatin1(),recv,10);
            // }
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

// void rightHexPadded(int command, int totalLength = 2) {
//     std::stringstream ss;
//     ss << std::hex << std::setw(totalLength) << std::setfill('0') << command;
//     std::string hexStr = ss.str();
//     // 如果totalLength大于command的十六进制表示长度，则字符串会被填充前导零。
//     // 我们只需要返回这个字符串，因为它已经是我们需要的长度了。
//     // 但为了明确起见，我们仍然可以取最右边的totalLength个字符（虽然这通常是多余的）。
//     return hexStr.substr(hexStr.size() - totalLength);
// }

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

void Test_TransThread::sendCtrlGnd(int command)
{
    uchar initialCmd[] = {0x01, 0x0F, 0x00, 0x00, 0x00, 0x08, 0x01};
    int cmdLength = sizeof(initialCmd) / sizeof(initialCmd[0]);

    QString hexCommand = QString::number(command, 16).toUpper().rightJustified(2, '0');
    QByteArray cmdArray(reinterpret_cast<const char*>(initialCmd), cmdLength);

    bool ok;
    uchar commandByte = hexCommand.right(2).toInt(&ok, 16); // 只取前两个字符进行转换
    if (ok && commandByte <= 0xFF) { // 检查转换是否成功且值在 uchar 范围内
        cmdArray.append(commandByte);
    } else {
        qDebug() << "Failed to convert command to hex byte.";
    }

    // 计算 CRC 并添加到数组中（注意：这里 CRC 的计算应该基于修改后的数组大小）
    ushort crc = rtu_crc(reinterpret_cast<const uchar*>(cmdArray.constData()), cmdArray.size());

    cmdArray.append(static_cast<uchar>(crc & 0xFF)); // 低字节
    cmdArray.append(static_cast<uchar>(crc >> 8));   // 高字节

    // int size = cmdArray.size();
    // std::vector<uchar> ucharArray(size);

    // // 将 QByteArray 中的数据复制到 uchar 数组中
    // for (int i = 0; i < size; ++i) {
    //     ucharArray[i] = static_cast<uchar>(cmdArray.at(i));
    // }

    QByteArray recv;

    mSerial->transmit(cmdArray,recv,10);

    qDebug() << cmdArray;
}

void Test_TransThread::recvPolarity(int command)
{

}
