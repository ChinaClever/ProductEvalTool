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
    mPro->order_num = mItem->cnt.all;

    obj.insert("product_sn", mPro->product_sn);
    obj.insert("soft_version", mPro->softwareVersion);
    obj.insert("start_time", mPro->testStartTime);
    obj.insert("end_time", mPro->testEndTime);
    obj.insert("module_sn", mPro->moduleSN);
    obj.insert("test_step", mPro->test_step);
    obj.insert("test_item", mPro->test_item);
    obj.insert("test_request", mPro->itemRequest);
    obj.insert("tool_name", "qc-busbar");
    obj.insert("order_id", mPro->order_id);
    obj.insert("test_num", "");
    obj.insert("dev_name", mPro->dev_name);
    obj.insert("language_select", 0);
    obj.insert("order_num", mPro->order_num);
    int num = mPro->itPass.size();
    mPro->uploadPassResult = 1;
    for(int i=0; i<num; ++i)
    {
        if(mPro->itPass.at(i) == 0) {
            mPro->uploadPassResult = 0; break;
        }
    }
    obj.insert("test_result", mPro->uploadPassResult);

    if(mPro->work_mode >=2) {
        QString str1 = mPro->itemContent.join(";");
        obj.insert("test_cfg" ,str1);
    }

    QString str = mPro->itemData.join("；");
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
    ePro->order_num = mItem->cnt.all;

    ePro->testStartTime = mPro->testStartTime;
    obj.insert("product_sn", ePro->product_sn);
    obj.insert("soft_version", ePro->softwareVersion);
    obj.insert("start_time", ePro->testStartTime);
    obj.insert("end_time", ePro->testEndTime);
    obj.insert("module_sn", ePro->moduleSN);
    obj.insert("test_step", ePro->test_step);
    obj.insert("test_item", ePro->test_item);
    obj.insert("test_request", ePro->itemRequest);
    obj.insert("tool_name", "qc-busbar");
    obj.insert("order_id", ePro->order_id);
    obj.insert("test_num", "");
    obj.insert("dev_name", ePro->dev_name);
    obj.insert("language_select", 1);
    obj.insert("order_num", ePro->order_num);

    int num = ePro->itPass.size();
    ePro->uploadPassResult = 1;
    for(int i=0; i<num; ++i)
    {
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
    head(json); http_post("admin-api/bus/testData",mPro->Service,json);
    json.empty();
    sDataPacket::bulid()->delay(1);
    head_English(json);http_post("admin-api/bus/testData",mPro->Service,json);//安规测试的英文版本
    json.empty();
    sDataPacket::bulid()->delay(1);
    SafeData(); sDataPacket::bulid()->delay(1);
    SafeData_Lan();
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
        .onSuccess([&](QString result) {qDebug()<<"result"<<result; mPro->flag = 1;})
        .onFailed([&](QString error) {qDebug()<<"error"<<error; mPro->flag = 0;})
        .onTimeout([&](QNetworkReply *) {qDebug()<<"http_post timeout"; mPro->flag = 0;}) // 超时处理
        .timeout(1) // 1s超时
        .block()
        .body(json)
        .exec();
}

void Json_Pack::SafeData()
{
    QJsonObject obj;
    QDateTime t = QDateTime::currentDateTime();
    mPro->testEndTime = t.toString("yyyy-MM-dd HH:mm:ss");

    mPro->order_num = mItem->cnt.all;

    obj.insert("product_sn", mPro->product_sn);
    obj.insert("soft_version", mPro->softwareVersion);
    obj.insert("start_time", mPro->testStartTime);
    obj.insert("end_time", mPro->testEndTime);
    obj.insert("module_sn", mPro->moduleSN);
    obj.insert("order_id", mPro->order_id);
    obj.insert("order_num", mPro->order_num);
    obj.insert("test_num", "");
    obj.insert("dev_name", mPro->dev_name);
    obj.insert("language_select", 0);

    obj.insert("tool_name", "qc-busbar");

    if(mPro->work_mode >=2) {
        QString str1 = mPro->itemContent.join("；");
        obj.insert("test_cfg" ,str1);
    }

    int num = mPro->stepResult.size();
    QString str;
    for(int i=0; i<num; ++i)
    {
        obj.insert("test_item", mPro->test_function.at(i));
        obj.insert("test_step", mPro->sureItem.at(i));
        obj.insert("test_process" ,mPro->itemData.at(i));
        obj.insert("test_result" ,mPro->stepResult.at(i));
        obj.insert("test_request" ,mPro->stepRequest.at(i));
        stephttp_post("admin-api/bus/testData",mPro->Service,obj);
    }
}

void Json_Pack::SafeData_Lan()
{
    QJsonObject obj;
    QDateTime t = QDateTime::currentDateTime();
    ePro->testEndTime = t.toString("yyyy-MM-dd HH:mm:ss");

    ePro->order_num = mItem->cnt.all;

    obj.insert("product_sn", ePro->product_sn);
    obj.insert("soft_version", ePro->softwareVersion);
    obj.insert("start_time", ePro->testStartTime);
    obj.insert("end_time", ePro->testEndTime);
    obj.insert("module_sn", ePro->moduleSN);
    obj.insert("order_id", ePro->order_id);
    obj.insert("order_num", ePro->order_num);
    obj.insert("test_num", "");
    obj.insert("dev_name", ePro->dev_name);
    obj.insert("language_select", 0);

    obj.insert("tool_name", "qc-busbar");

    if(mPro->work_mode >=2) {
        QString str1 = ePro->itemContent.join("；");
        obj.insert("test_cfg" ,str1);
    }

    int num = ePro->stepResult.size();
    QString str;
    for(int i=0; i<num; ++i)
    {
        obj.insert("test_item", ePro->test_function.at(i));
        obj.insert("test_step", ePro->sureItem.at(i));
        obj.insert("test_process" ,ePro->itemData.at(i));
        obj.insert("test_result" ,ePro->stepResult.at(i));
        obj.insert("test_request" ,ePro->stepRequest.at(i));
        stephttp_post("admin-api/bus/testData",ePro->Service,obj);
    }
}

void Json_Pack::stepData()
{
    QJsonObject obj;
    QDateTime t = QDateTime::currentDateTime();
    mPro->testEndTime = t.toString("yyyy-MM-dd HH:mm:ss");

    mPro->order_num = mItem->cnt.all;

    obj.insert("product_sn", mPro->product_sn);
    obj.insert("soft_version", mPro->softwareVersion);
    obj.insert("start_time", mPro->testStartTime);
    obj.insert("end_time", mPro->testEndTime);
    obj.insert("module_sn", mPro->moduleSN);
    obj.insert("order_id", mPro->order_id);
    obj.insert("order_num", mPro->order_num);
    obj.insert("test_num", "");
    obj.insert("dev_name", mPro->dev_name);
    obj.insert("language_select", 0);

    obj.insert("tool_name", "qc-busbar");

    if(mPro->work_mode >=2) {
        QString str1 = mPro->itemContent.join("；");
        obj.insert("test_cfg" ,str1);
    }

    int num = mPro->stepResult.size();
    QString str;
    for(int i=0; i<num; ++i)
    {
        obj.insert("test_item", mPro->test_function.at(i));
        obj.insert("test_step", mPro->test_step);
        obj.insert("test_process" ,mPro->itemData.at(i));
        obj.insert("test_result" ,mPro->stepResult.at(i));
        obj.insert("test_request" ,mPro->stepRequest.at(i));
        stephttp_post("admin-api/bus/testData",mPro->Service,obj);
    }
}

void Json_Pack::stepData_Eng()//功能测试的英文版本
{
    QJsonObject obj;
    QDateTime t = QDateTime::currentDateTime();
    ePro->testEndTime = t.toString("yyyy-MM-dd HH:mm:ss");
    ePro->testStartTime = mPro->testStartTime;

    ePro->order_num = mItem->cnt.all;

    obj.insert("product_sn", ePro->product_sn);
    obj.insert("soft_version", ePro->softwareVersion);
    obj.insert("start_time", ePro->testStartTime);
    obj.insert("end_time", ePro->testEndTime);
    obj.insert("module_sn", ePro->moduleSN);
    obj.insert("order_id", ePro->order_id);
    obj.insert("order_num", ePro->order_num);
    obj.insert("test_num", "");
    obj.insert("dev_name", ePro->dev_name);
    obj.insert("language_select", 1);

    obj.insert("tool_name", "qc-busbar");

    if(mPro->work_mode >=2) {
        QString str1 = ePro->itemContent.join(";");
        obj.insert("test_cfg" ,str1);
    }

    int num = ePro->stepResult.size();
    QString str;
    for(int i=0; i<num; ++i)
    {
        obj.insert("test_item", ePro->test_function.at(i));
        obj.insert("test_step", ePro->test_step);
        obj.insert("test_process" ,ePro->itemData.at(i));
        obj.insert("test_result" ,ePro->stepResult.at(i));
        obj.insert("test_request" ,ePro->stepRequest.at(i));
        stephttp_post("admin-api/bus/testData",mPro->Service,obj);
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
        .onSuccess([&](QString result) {qDebug()<<"result"<<result; mPro->flag = 1; })
        .onFailed([&](QString error) {qDebug()<<"error"<<error; mPro->flag = 0; })
        .onTimeout([&](QNetworkReply *) {qDebug()<<"http_post timeout"; mPro->flag = 0;}) // 超时处理
        .timeout(1) // 1s超时
        .block()
        .body(json)
        .exec();
}

void Json_Pack::FuncData()
{
    QJsonObject obj; QJsonObject obj_en;
    QDateTime t = QDateTime::currentDateTime();
    mPro->testEndTime = t.toString("yyyy-MM-dd HH:mm:ss");

    mPro->order_num = mItem->cnt.all;

    obj.insert("product_sn", mPro->product_sn);
    obj.insert("soft_version", mPro->softwareVersion);
    obj.insert("start_time", mPro->testStartTime);
    obj.insert("end_time", mPro->testEndTime);
    obj.insert("module_sn", mPro->moduleSN);
    obj.insert("order_id", mPro->order_id);
    obj.insert("order_num", mPro->order_num);
    obj.insert("test_num", "");
    obj.insert("dev_name", mPro->dev_name);
    obj.insert("language_select", 0);

    obj.insert("tool_name", "qc-busbar");

    if(mPro->work_mode >= 2) {
        QString str1 = mPro->itemContent.join("；");
        obj.insert("test_cfg" ,str1);
    }

    int num = mPro->stepResult.size();
    int fag = 0;
    if(num){
        for(int i=0; i<num; ++i)
        {
            if(mPro->test_function.at(i).contains("极性检查"))
        {
                fag = i; break;
            }
        }
        for(int i=0; i<=fag; ++i)
        {
            obj.insert("test_item", mPro->test_function.at(i));
            obj.insert("test_step", mPro->test_step);
            obj.insert("test_process" ,mPro->itemData.at(i));
            obj.insert("test_result" ,mPro->stepResult.at(i));
            obj.insert("test_request" ,mPro->stepRequest.at(i));
            stephttp_post("admin-api/bus/testData",mPro->Service,obj);
        }
        int j = 0;
        if((fag != 0) && ((fag +1) < num)){
            for(int i= fag +1; i<num; ++i)
            {
                obj.insert("test_item", mPro->test_function.at(i));
                obj.insert("test_step", mPro->sureItem.at(j));
                obj.insert("test_process" ,mPro->itemData.at(i));
                obj.insert("test_result" ,mPro->stepResult.at(i));
                obj.insert("test_request" ,mPro->stepRequest.at(i));
                stephttp_post("admin-api/bus/testData",mPro->Service,obj);
                j++;
            }
        }
    }

}

void Json_Pack::FuncData_Lan()
{
    QJsonObject obj;
    QDateTime t = QDateTime::currentDateTime();
    ePro->testEndTime = t.toString("yyyy-MM-dd HH:mm:ss");

    ePro->order_num = mItem->cnt.all;

    obj.insert("product_sn", ePro->product_sn);
    obj.insert("soft_version", ePro->softwareVersion);
    obj.insert("start_time", ePro->testStartTime);
    obj.insert("end_time", ePro->testEndTime);
    obj.insert("module_sn", ePro->moduleSN);
    obj.insert("order_id", ePro->order_id);
    obj.insert("order_num", ePro->order_num);
    obj.insert("test_num", "");
    obj.insert("dev_name", ePro->dev_name);
    obj.insert("language_select", 1);

    obj.insert("tool_name", "qc-busbar");

    if(mPro->work_mode >=2) {
        QString str1 = ePro->itemContent.join("；");
        obj.insert("test_cfg" ,str1);
    }

    int num = mPro->stepResult.size();
    int fag = 0;
    if(num){
        for(int i=0; i<num; ++i)
        {
            if(mPro->test_function.at(i).contains("极性检查"))
        {
                fag = i; break;
            }
        }
        for(int i=0; i<=fag; ++i)
        {
            obj.insert("test_item", ePro->test_function.at(i));
            obj.insert("test_step", ePro->test_step);
            obj.insert("test_process" ,ePro->itemData.at(i));
            obj.insert("test_result" ,ePro->stepResult.at(i));
            obj.insert("test_request" ,ePro->stepRequest.at(i));
            stephttp_post("admin-api/bus/testData",mPro->Service,obj);
        }
        int j = 0;
        if((fag != 0) && ((fag +1) < num)){
            for(int i= fag +1; i<num; ++i)
            {
                obj.insert("test_item", ePro->test_function.at(i));
                obj.insert("test_step", ePro->sureItem.at(j));
                obj.insert("test_process" ,ePro->itemData.at(i));
                obj.insert("test_result" ,ePro->stepResult.at(i));
                obj.insert("test_request" ,ePro->stepRequest.at(i));
                stephttp_post("admin-api/bus/testData",mPro->Service,obj);
                j++;
            }
        }
    }

}
