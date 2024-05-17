/*
 *
 *
 *  Created on: 2018年10月1日
 *      Author: Lzy
 */
#include "json_pack.h"


Json_Pack::Json_Pack(QObject *parent)
{
    mPro = sDataPacket::bulid()->getPro();
    mBusData = get_share_mem();
    mItem = Cfg::bulid()->item;
}

Json_Pack *Json_Pack::bulid(QObject *parent)
{
    static Json_Pack* sington = NULL;
    if(sington == NULL) {
        sington = new Json_Pack(parent);
    }
    return sington;
}

void Json_Pack::head(QJsonObject &obj)
{
    obj.insert("product_sn", mPro->product_sn);
    obj.insert("start_time", mPro->testStartTime);
    obj.insert("test_step", mPro->test_step);
    obj.insert("test_item", mPro->test_item);
    obj.insert("tool_name", "qc-busbar");
    int num = mPro->pass.size();
    mPro->uploadPassResult = 1;
    for(int i=0; i<num; ++i)
    {
        if(mPro->pass.at(i) == 0) {
            mPro->uploadPassResult = 0; break;
        }
    }

    obj.insert("test_result", mPro->uploadPassResult);
    pduInfo(obj);
}

void Json_Pack::pduInfo(QJsonObject &obj)
{
    objData(obj);
}

int Json_Pack::objData(QJsonObject &obj)
{
    QJsonArray jsonArray;
    QJsonObject DataObj;
    int num = mPro->uploadPass.size();

    QString str1 = mPro->itemContent.join(";");
    obj.insert("test_process" ,str1);

    return num;
}

void Json_Pack::getJson(QJsonObject &json , QByteArray &ba)
{
    head(json);
    QJsonDocument jsonDoc(json);
    ba = jsonDoc.toJson();
}
void Json_Pack::http_post(const QString &method, const QString &ip, int port)
{
    QJsonObject json; head(json);
    qDebug()<<"json"<<json;
    AeaQt::HttpClient http;
    http.clearAccessCache();
    http.clearConnectionCache();
    QString url = "http://%1:%2/%3";
    http.post(url.arg(ip).arg(port).arg(method))
        .header("content-type", "application/json")
        .onSuccess([&](QString result) {qDebug()<<"result"<<result; mPro->result = Test_Over; emit httpSig("数据发送成功",true);})
        .onFailed([&](QString error) {qDebug()<<"error"<<error; mPro->result = Test_Fail; emit httpSig("数据发送失败",false); })
        .onTimeout([&](QNetworkReply *) {qDebug()<<"http_post timeout"; mPro->result = Test_Fail; emit httpSig("http_post timeout",false); }) // 超时处理
        .timeout(1) // 1s超时
        .block()
        .body(json)
        .exec();
}

//bool Json_Build::saveJson( QJsonObject &json)
//{
//    QJsonDocument jsonDoc(json);
//    QByteArray ba = jsonDoc.toJson();
//    QString path = CfgCom::bulid()->pathOfData("pdu_id.json");
//    QFile file(path);
//    bool ret = false;
//    if(file.exists())//文件存在则不需要再写
//    {
//        ret = true;
//    }
//    else//文件不存在则写入初始数据
//    {
//        ret = file.open(QIODevice::WriteOnly);
//        if(ret) {
//            file.write(ba);
//            file.close();
//        } else {
//            qDebug() << "write json file failed";
//        }
//    }

//    return ret;
//}

