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
   mSocket = new QUdpSocket(this);
   mSocket->bind(QHostAddress::LocalHost, 47755);
   connect(mSocket,SIGNAL(readyRead()),this,SLOT(recvSlot()));
}

Printer_BarTender *Printer_BarTender::bulid(QObject *parent)
{
    static Printer_BarTender* sington = nullptr;
    if(sington == nullptr)
        sington = new Printer_BarTender(parent);
    return sington;
}

bool Printer_BarTender::restartProgram(const QString &exeName,        // 进程名，如 "notepad.exe"
                    const QString &programPath,    // 完整路径，如 "C:/Windows/notepad.exe"
                    const QStringList &arguments)  // 启动参数
{
    // 1. 检测是否正在运行（通过 tasklist 过滤进程名）
    QProcess tasklist;
    tasklist.start("tasklist", QStringList() << "/FI" << ("IMAGENAME eq " + exeName) << "/NH");
    if (!tasklist.waitForFinished(3000)) {
        qWarning() << "tasklist 超时";
        return false;
    }

    QString output = tasklist.readAllStandardOutput();
    // 如果输出包含进程名，则认为正在运行
    if (output.contains(exeName, Qt::CaseInsensitive)) {
        // 2. 强制终止该进程
        QProcess taskkill;
        taskkill.start("taskkill", QStringList() << "/F" << "/IM" << exeName);
        if (!taskkill.waitForFinished(3000)) {
            qWarning() << "taskkill 超时";
            return false;
        }
        // 可选：检查 taskkill 输出确认成功
        QString err = taskkill.readAllStandardError();
        if (!err.isEmpty()) {
            qWarning() << "taskkill 错误：" << err;
            return false;
        }
        qDebug() << "已终止进程：" << exeName;
        // 等待进程完全退出（可选）
        QThread::sleep(1);
    } else {
        qDebug() << "进程未运行：" << exeName;
    }

    // 3. 启动程序（传递参数）
    bool started = QProcess::startDetached(programPath, arguments);
    if (started) {
        qDebug() << "启动成功：" << programPath << arguments;
    } else {
        qCritical() << "启动失败：" << programPath;
    }
    return started;
}

void Printer_BarTender::http_post(sBarTend &it)
{
    QStringList str;
    QString web = "https://clbusbar.legrandchina.cn/report?";
    web += QString("productSN=%1&orderId=%2&moduleSN=%3").arg(it.pn).arg(it.on).arg(it.sn);

    QDateTime dateTime;
    QString dateTime_str = dateTime.currentDateTime().toString("yyyy/MM/dd hh:mm");
    str << dateTime_str;
    str << QString("SN=SN:%1").arg(it.sn);
    str << QString("PN=PN:%1").arg(it.pn);
    str << QString("ON=ON:%1").arg(it.on);
    str << QString("QR=%1").arg(web);
    str << QString("AutoPrint=true");
    restartProgram("标签条码打印系统.exe","D:\\Release\\标签条码打印系统.exe",str);

}

QString Printer_BarTender::http_post(const QString &method, const QString &ip, sBarTend &it, int port)
{
    QByteArray json; QString str;
    QString order = createOrder(it);
    json.append(order.toLocal8Bit());
    qDebug()<<"createOrder"<<order;
    qDebug()<<"json"<<json;
    AeaQt::HttpClient http;
    http.clearAccessCache();
    http.clearConnectionCache();
    QString url = "http://%1:%2/%3";
    http.post(url.arg(ip).arg(port).arg(method))
        .header("content-type", "plain")
        .onSuccess([&](QString result) {qDebug()<<"result"<<result; str = result;})
        .onFailed([&](QString error) {qDebug()<<"error"<<error; str = error;})
        .onTimeout([&](QNetworkReply *) {qDebug()<<"http_post timeout";}) // 超时处理
        .timeout(5) // 1s超时
        .block()
        .body(json)
        .exec();

    return str;
}

QString Printer_BarTender::createOrder(sBarTend &it)
{
    QString web = "https://clbusbar.legrandchina.cn/report?";
    QString str = "ON,PN,SN,Date,QR\n";
    str += it.on + ","; str += it.pn + ","; str += it.sn + ",";

    QDateTime dateTime;
    QString dateTime_str = dateTime.currentDateTime().toString("yyyy/MM/dd hh:mm");
    str += dateTime_str + ",";
    web += QString("productSN=%1&orderId=%2&moduleSN=%3").arg(it.pn).arg(it.on).arg(it.sn);
    str += web;
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
    int port = 4000;
    // QHostAddress host = QHostAddress::Broadcast;
    sendMsg("start", port, QHostAddress::LocalHost);

    QString order = createOrder(it);
    sendMsg(order.toLocal8Bit(), port+1, QHostAddress::LocalHost);
    sendMsg(order.toLocal8Bit(), port, QHostAddress::LocalHost);

    return recvResponse(10);
}

int Printer_BarTender::sendMsg(const QByteArray &msg, quint16 port, const QHostAddress &host)
{
    int ret = mSocket->writeDatagram(msg, host, port);
    if(ret > 0) mSocket->flush(); delay(1000);
    return ret;
}

void Printer_BarTender::recvSlot()
{
    while (mSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = mSocket->receiveDatagram();
        if(datagram.data().size()) {
            mRes = true; mRcvData = datagram.data();
        }
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
