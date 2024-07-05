#ifndef PRINTER_BARTENDER_H
#define PRINTER_BARTENDER_H

#include <QObject>
#include <QtNetwork>
#include <windows.h>
#include "http/httpclient.h"

struct sBarTend {
    QString pn;//"成品代码"
    QString on;//"订单号"
    QString hw;//"硬件版本"
    QString fw;//"软件版本"
    QString sn;//"序列号"
    QString code;//
};

class Printer_BarTender : public QObject
{
    Q_OBJECT
    explicit Printer_BarTender(QObject *parent = nullptr);
public:
    static Printer_BarTender *bulid(QObject *parent = nullptr);
    bool printer(sBarTend &it);
    QString createOrder(sBarTend &it);
    bool printMAC(QString cmd);
    bool printerInfo(sBarTend &it);
    QString http_post(const QString &method, const QString &ip, sBarTend &it, int port=80);
protected:
    void delay(int msec);
    bool recvResponse(int sec);

    int sendMsg(const QByteArray &msg, quint16 port,
                const QHostAddress &host=QHostAddress::LocalHost);
    bool RawDataToPrinter(LPSTR szPrinterName, LPBYTE lpData, DWORD dwCount);

protected slots:
    void recvSlot();


private:
    bool mRes;
    QString mRcvData;
    QUdpSocket *mSocket;
};

#endif // PRINTER_BARTENDER_H
