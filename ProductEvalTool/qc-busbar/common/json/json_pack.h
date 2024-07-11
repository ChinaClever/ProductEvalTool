#ifndef JSONPACK_H
#define JSONPACK_H

// #include "datapacket.h"
#include "http/httpclient.h"
#include "config.h"

class Json_Pack:public QObject
{
    Q_OBJECT
    explicit Json_Pack(QObject *parent = nullptr);
public:
    static Json_Pack *bulid(QObject *parent = nullptr);
    void http_post(const QString &method, const QString &ip, QJsonObject json, int port=48088);
    void stephttp_post(const QString &method, const QString &ip,QJsonObject &json, int port=48088);
    void getJson(QJsonObject &json, QByteArray &ba);
    void stepData();
    void stepData_Eng();
    void SendJson_Safe();
    void SafeData();
    void SafeData_Lan();
    void FuncData(int num);
    void FuncData_Lan(int num);
signals:
    void httpSig(const QString &msg, bool pass);
protected:
    void head(QJsonObject &obj);
    void pduInfo(QJsonObject &json);
    int objData(QJsonObject &obj);
    void head_English(QJsonObject &obj);
private:
    sProgress *mPro;
    sProgress *ePro;
    sBusData *mBusData;
    sCfgItem *mItem;
};

#endif // JSONPACK_H
