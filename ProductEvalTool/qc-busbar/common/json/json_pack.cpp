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
    ePro = datapacket_English::bulid()->getPro();
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
    QDateTime t = QDateTime::currentDateTime();
    mPro->testEndTime = t.toString("yyyy-MM-dd HH:mm:ss");

    obj.insert("product_sn", mPro->product_sn);
    obj.insert("soft_version", "");
    obj.insert("start_time", mPro->testStartTime);
    obj.insert("end_time", mPro->testEndTime);
    obj.insert("test_type", "");
    obj.insert("test_step", mPro->test_step);
    obj.insert("test_item", mPro->test_item);
    obj.insert("test_request", mPro->itemRequest);
    obj.insert("tool_name", "qc-busbar");
    obj.insert("order_id", mPro->order_id);
    obj.insert("test_num", mPro->test_num);
    obj.insert("dev_name", mPro->dev_name);
    obj.insert("language_select", 0);
    obj.insert("order_num", mPro->order_num);
    int num = mPro->itPass.size();
    mPro->uploadPassResult = 1;
    for(int i=0; i<num; ++i)
    {
        qDebug()<<"mPro->pass.at(i)"<<mPro->itPass.at(i);
        if(mPro->itPass.at(i) == 0) {
            mPro->uploadPassResult = 0; break;
        }
    }
    obj.insert("test_result", mPro->uploadPassResult);

    if(mPro->work_mode >=2) {
        QString str1 = mPro->itemContent.join(";");
        obj.insert("test_cfg" ,str1);
    }

    QString str = mPro->itemData.join(";");
    obj.insert("test_process" ,str);

    // pduInfo(obj);
}

void Json_Pack::pduInfo(QJsonObject &obj)
{
    objData(obj);
}

int Json_Pack::objData(QJsonObject &obj)
{
    int num = mPro->uploadPass.size();

    QString str1 = mPro->itemData.join(";");
    obj.insert("test_process" ,str1);

    return num;
}

void Json_Pack::getJson(QJsonObject &json , QByteArray &ba)
{
    head(json);
    QJsonDocument jsonDoc(json);
    ba = jsonDoc.toJson();
}

void Json_Pack::head_English(QJsonObject &obj)
{
    QDateTime t = QDateTime::currentDateTime();
    ePro->testEndTime = t.toString("yyyy-MM-dd HH:mm:ss");

    obj.insert("product_sn", ePro->product_sn);
    obj.insert("soft_version", "");
    obj.insert("start_time", ePro->testStartTime);
    obj.insert("end_time", ePro->testEndTime);
    obj.insert("test_type", "");
    obj.insert("test_step", ePro->test_step);
    obj.insert("test_item", ePro->test_item);
    obj.insert("test_request", ePro->itemRequest);
    obj.insert("tool_name", "qc-busbar");
    obj.insert("order_id", ePro->order_id);
    obj.insert("test_num", ePro->test_num);
    obj.insert("dev_name", ePro->dev_name);
    obj.insert("language_select", 1);
    obj.insert("order_num", ePro->order_num);

    int num = ePro->itPass.size();
    ePro->uploadPassResult = 1;
    for(int i=0; i<num; ++i)
    {
        qDebug()<<"mPro->pass.at(i)"<<ePro->itPass.at(i);
        if(ePro->itPass.at(i) == 0) {
            ePro->uploadPassResult = 0; break;
        }
    }
    obj.insert("test_result", ePro->uploadPassResult);

    if(mPro->work_mode >=2) {
        QString str1 = ePro->itemContent.join(";");
        obj.insert("test_cfg" ,str1);
    }

    QString str = ePro->itemData.join(";");
    obj.insert("test_process" ,str);

    // pduInfo(obj);
}

void Json_Pack::SendJson_Safe()
{
    QJsonObject json;
    head(json); http_post("busbarreport/add",mPro->Service,json);
    json.empty();
    sDataPacket::bulid()->delay(2);
    head_English(json);http_post("busbarreport/add",mPro->Service,json);//安规测试的英文版本
}

void Json_Pack::http_post(const QString &method, const QString &ip, QJsonObject json, int port)
{
    qDebug()<<"json"<<json;
    AeaQt::HttpClient http;
    http.clearAccessCache();
    http.clearConnectionCache();
    QString url = "http://%1:%2/%3";
    http.post(url.arg(ip).arg(port).arg(method))
        .header("content-type", "application/json")
        .onSuccess([&](QString result) {qDebug()<<"result"<<result; mPro->result = Test_Over; emit httpSig("数据发送成功",true);})
        .onFailed([&](QString error) {qDebug()<<"error"<<error; mPro->result = Test_Fail; emit httpSig("数据发送失败",true); })
        .onTimeout([&](QNetworkReply *) {qDebug()<<"http_post timeout"; mPro->result = Test_Fail; emit httpSig("http_post timeout",true); }) // 超时处理
        .timeout(1) // 1s超时
        .block()
        .body(json)
        .exec();
}

void Json_Pack::stepData()
{
    QJsonObject obj;
    QDateTime t = QDateTime::currentDateTime();
    mPro->testEndTime = t.toString("yyyy-MM-dd HH:mm:ss");
    obj.insert("product_sn", mPro->product_sn);
    obj.insert("soft_version", "");
    obj.insert("start_time", mPro->testStartTime);
    obj.insert("end_time", mPro->testEndTime);
    obj.insert("test_type", "");
    obj.insert("order_id", mPro->order_id);
    obj.insert("order_num", mPro->order_num);
    obj.insert("test_num", mPro->test_num);
    obj.insert("dev_name", mPro->dev_name);
    obj.insert("language_select", 0);

    obj.insert("tool_name", "qc-busbar");

    QString str1 = mPro->itemContent.join(";");
    obj.insert("test_cfg" ,str1);

    int num = mPro->stepResult.size();
    QString str;
    for(int i=0; i<num; ++i)
    {
        obj.insert("test_item", mPro->test_function.at(i));
        obj.insert("test_step", mPro->test_step);
        obj.insert("test_process" ,mPro->itemData.at(i));
        obj.insert("test_result" ,mPro->stepResult.at(i));
        obj.insert("test_request" ,mPro->stepRequest.at(i));
        stephttp_post("busbarreport/add",mPro->Service,obj);
    }
}

void Json_Pack::stepData_Eng()//功能测试的英文版本
{
    QJsonObject obj;
    QDateTime t = QDateTime::currentDateTime();
    ePro->testEndTime = t.toString("yyyy-MM-dd HH:mm:ss");
    obj.insert("product_sn", ePro->product_sn);
    obj.insert("soft_version", "");
    obj.insert("start_time", ePro->testStartTime);
    obj.insert("end_time", ePro->testEndTime);
    obj.insert("test_type", "");
    obj.insert("order_id", mPro->order_id);
    obj.insert("order_num", mPro->order_num);
    obj.insert("test_num", mPro->test_num);
    obj.insert("dev_name", mPro->dev_name);
    obj.insert("language_select", 1);

    obj.insert("tool_name", "qc-busbar");

    QString str1 = ePro->itemContent.join(";");
    obj.insert("test_cfg" ,str1);

    int num = ePro->stepResult.size();
    QString str;
    for(int i=0; i<num; ++i)
    {
        obj.insert("test_item", ePro->test_function.at(i));
        obj.insert("test_step", ePro->test_step);
        obj.insert("test_process" ,ePro->itemData.at(i));
        obj.insert("test_result" ,ePro->stepResult.at(i));
        obj.insert("test_request" ,ePro->stepRequest.at(i));
        stephttp_post("busbarreport/add",mPro->Service,obj);
    }
}

void Json_Pack::stephttp_post(const QString &method, const QString &ip,QJsonObject &json, int port)
{
    qDebug()<<"json"<<json;
    AeaQt::HttpClient http;
    http.clearAccessCache();
    http.clearConnectionCache();
    QString url = "http://%1:%2/%3";
    http.post(url.arg(ip).arg(port).arg(method))
        .header("content-type", "application/json")
        .onSuccess([&](QString result) {qDebug()<<"result"<<result; mPro->result = Test_Over;})
        .onFailed([&](QString error) {qDebug()<<"error"<<error; mPro->result = Test_Fail;})
        .onTimeout([&](QNetworkReply *) {qDebug()<<"http_post timeout"; mPro->result = Test_Fail;}) // 超时处理
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

