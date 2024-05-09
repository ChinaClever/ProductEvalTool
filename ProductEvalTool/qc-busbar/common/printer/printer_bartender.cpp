/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "printer_bartender.h"
#include "common.h"
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrinterInfo>

Printer_BarTender::Printer_BarTender(QObject *parent) : QObject(parent)
{
//    mSocket = new QUdpSocket(this);
//    mSocket->bind(QHostAddress::AnyIPv4, 47755);
//    connect(mSocket,SIGNAL(readyRead()),this,SLOT(recvSlot()));
}

Printer_BarTender *Printer_BarTender::bulid(QObject *parent)
{
    static Printer_BarTender* sington = nullptr;
    if(sington == nullptr)
        sington = new Printer_BarTender(parent);
    return sington;
}

QString Printer_BarTender::createOrder(sBarTend &it)
{
    QString str = "PN,HW,FW,Date\n";
    str += it.pn + ","; str += it.hw + ","; str += it.fw + ",";
    QString date = QDate::currentDate().toString("yyyy-MM-dd");
    str += date;
    return str;
}

void Printer_BarTender::delay(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

bool Printer_BarTender::recvResponse(int sec)
{
    mRes = false;
    for(int i=0; i<10*sec; ++i) {
        if (mRes) break; else delay(100);
    }

    return mRes;
}

bool Printer_BarTender::printer(sBarTend &it)
{
    int port = 30044;
    QHostAddress host = QHostAddress::Broadcast;
    sendMsg("start", port, host);

    QString order = createOrder(it);
    sendMsg(order.toLocal8Bit(), port+1, host);
    return recvResponse(10);
}

int Printer_BarTender::sendMsg(const QByteArray &msg, quint16 port, const QHostAddress &host)
{
    int ret = mSocket->writeDatagram(msg, host, port);
    if(ret > 0) mSocket->flush(); delay(100);
    return ret;
}

void Printer_BarTender::recvSlot()
{
    while (mSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = mSocket->receiveDatagram();
        if(datagram.data().size()) mRes = true;
    }
}

/**
 * @brief Print::RawDataToPrinter
 * 发送ZPL指令直接与打印机通信
 * @param szPrinterName 打印机名称
 * @param lpData  ZPL指令
 * @param dwCount
 * @return ZPL指令发送成功，返回true，ZPL指令发送失败，返回false
 */
bool Printer_BarTender::RawDataToPrinter(LPSTR szPrinterName, LPBYTE lpData, DWORD dwCount)
{
    bool        bStatus = FALSE;
    HANDLE      hPrinter = NULL;
    DOC_INFO_1A DocInfo;
    DWORD       dwJob = 0L;
    DWORD       dwBytesWritten = 0L;

    // 打开打印机的手柄，这里使用OpenPrinterA()而不是OpenPrinter()是因为当前Qt编码是ANSI
    bStatus = OpenPrinterA(szPrinterName, &hPrinter, NULL);
    if (bStatus)
    {
        // 填写打印文档信息
        DocInfo.pDocName = (LPSTR)"Raw Document";
        DocInfo.pOutputFile = NULL;
        DocInfo.pDatatype = (LPSTR)"RAW";


        // 通知后台处理程序文档正在开始
        dwJob = StartDocPrinterA(hPrinter, 1, (LPBYTE)&DocInfo);
        if (dwJob > 0)
        {
            // 开始一页的打印
            bStatus = StartPagePrinter(hPrinter);
            if (bStatus)
            {
                // 发送数据到打印机
                bStatus = WritePrinter(hPrinter, lpData, dwCount, &dwBytesWritten);
                EndPagePrinter (hPrinter);
            }
            // 通知后台处理程序文档正在结束
            EndDocPrinter(hPrinter);
        }
        // 关闭打印机手柄
        ClosePrinter(hPrinter);
    }
    // 检查是否写入了正确的字节数
    if (!bStatus || (dwBytesWritten != dwCount))
    {
        bStatus = false;
    }
    else
    {
        bStatus = true;
    }
    return bStatus;
}

bool Printer_BarTender::printMAC(QString cmd)
{
    QString cmd1 = tr("^XA^PW1000^LL800^LH0,0^MD13^PR2^FO250,30^A0,55,55^FD%1^FS^XZ").arg(cmd);
    int length = cmd1.length();
    QString printerName = QPrinterInfo::defaultPrinterName();
    bool ret = RawDataToPrinter((LPSTR)printerName.toLocal8Bit().data(),
                            (LPBYTE)cmd1.toLocal8Bit().data(), (DWORD)length);
    return ret;
}

bool Printer_BarTender::printerInfo(sBarTend &it)
{
    QString cmd = "^XA^PW1000^LL800^LH0,0^MD13^PR2";
    cmd += QString("^FO10,50^A0,45,45^FDPN:%1           Date:%2^FS").arg(it.pn).arg(QDate::currentDate().toString("yyyy-MM-dd"));
    cmd += QString("^FO10,120^A0,45,45^FDFW:%1           HW:%2^FS").arg(it.fw).arg(it.hw);
    cmd += "^XZ";
    int length = cmd.length();
    QString printerName = QPrinterInfo::defaultPrinterName();
    bool ret = RawDataToPrinter((LPSTR)printerName.toLocal8Bit().data(),
                            (LPBYTE)cmd.toLocal8Bit().data(), (DWORD)length);
    return ret;
}
