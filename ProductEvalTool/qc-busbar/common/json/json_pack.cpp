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

    obj.insert("productSn", mPro->product_sn);
    obj.insert("softVersion", mPro->softwareVersion);
    obj.insert("startTime", mPro->testStartTime);
    obj.insert("endTime", mPro->testEndTime);
    obj.insert("moduleSn", mPro->moduleSN);
    obj.insert("testStep", mPro->test_step);
    obj.insert("toolName", "qc-busbar");
    obj.insert("orderId", mPro->order_id);
    obj.insert("testNum", "");
    obj.insert("devName", mPro->dev_name);
    obj.insert("languageSelect", 0);
    obj.insert("orderNum", mPro->order_num);

    QStringList list = mPro->itemRequest.split("。");

    QStringList step;
    if(mPro->work_mode == 0) {step.clear(); step << "绝缘测试";  step << "交流耐压测试";}
            else {step.clear(); step << "接地测试";}

    int num = mPro->safe_result.size();
    for(int i=0; i<num; ++i)
    {
        obj.insert("testResult", QString::number(mPro->safe_result.at(i)));
        obj.insert("testRequest",list.at(i));
        obj.insert("testItem", step.at(i));
        obj.insert("testProcess" ,mPro->itemData.at(i));
        obj.insert("testResult" ,mPro->safe_result.at(i));
        stephttp_post("admin-api/bus/testData",mPro->Service,obj);
    }


    if(mPro->work_mode >=2) {
        QString str1 = mPro->itemContent.join(";");
        obj.insert("testCfg" ,str1);
    }

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
    obj.insert("testProcess" ,str1);

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

    ePro->moduleSN = mPro->moduleSN;
    ePro->testStartTime = mPro->testStartTime;
    obj.insert("productSn", ePro->product_sn);
    obj.insert("softVersion", ePro->softwareVersion);
    obj.insert("startTime", ePro->testStartTime);
    obj.insert("endTime", ePro->testEndTime);
    obj.insert("moduleSn", ePro->moduleSN);
    obj.insert("testStep", ePro->test_step);
    // obj.insert("testItem", ePro->test_item);
    // obj.insert("testRequest", ePro->itemRequest);
    obj.insert("toolName", "qc-busbar");
    obj.insert("orderId", ePro->order_id);
    obj.insert("testNum", "");
    obj.insert("devName", ePro->dev_name);
    obj.insert("languageSelect", 1);
    obj.insert("orderNum", ePro->order_num);

    QStringList list = ePro->itemRequest.split(".");

    QStringList step;
    if(mPro->work_mode == 0) {step << "Insulation test";  step << "Communication voltage withstand test";}
    else step << "Grounding test";

    int num = mPro->safe_result.size();
    for(int i=0; i<num; ++i)
    {
        obj.insert("testResult", ePro->safe_result.at(i));
        obj.insert("testRequest",list.at(i));
        obj.insert("testItem", step.at(i));
        obj.insert("testProcess" ,ePro->itemData.at(i));
        obj.insert("testResult" ,ePro->safe_result.at(i));
        stephttp_post("admin-api/bus/testData",mPro->Service,obj);
    }

    if(mPro->work_mode >=2) {
        QString str1 = ePro->itemContent.join(";");
        obj.insert("testCfg" ,str1);
    }
    // pduInfo(obj);
}

void Json_Pack::SendJson_Safe()
{
    QJsonObject json;
    head(json);
    // http_post("admin-api/bus/testData",mPro->Service,json);
    json.empty();
    sDataPacket::bulid()->delayMs(5);
    head_English(json);
    // http_post("admin-api/bus/testData",mPro->Service,json);//安规测试的英文版本

    if(mItem->modeId == 2)
    {
        json.empty();
        sDataPacket::bulid()->delayMs(3);
        SafeData(); sDataPacket::bulid()->delayMs(3);
        SafeData_Lan();
    }
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
        .timeoutMs(200) // 1s超时
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

    obj.insert("productSn", mPro->product_sn);
    obj.insert("softVersion", mPro->softwareVersion);
    obj.insert("startTime", mPro->testStartTime);
    obj.insert("endTime", mPro->testEndTime);
    obj.insert("moduleSn", mPro->moduleSN);
    obj.insert("orderId", mPro->order_id);
    obj.insert("orderNum", mPro->order_num);
    obj.insert("testNum", "");
    obj.insert("devName", mPro->dev_name);
    obj.insert("languageSelect", 0);

    obj.insert("toolName", "qc-busbar");

    if(mPro->work_mode >=2) {
        QString str1 = mPro->itemContent.join("；");
        obj.insert("testCfg" ,str1);
    }

    int num = mPro->stepResult.size();
    QString str;
    for(int i=0; i<num; ++i)
    {
        obj.insert("testItem", mPro->test_function.at(i));
        obj.insert("testStep", mPro->sureItem.at(i));
        obj.insert("testProcess" ,mPro->itemData.at(i));
        obj.insert("testResult" ,mPro->stepResult.at(i));
        obj.insert("testRequest" ,mPro->stepRequest.at(i));
        stephttp_post("admin-api/bus/testData",mPro->Service,obj);
    }
}

void Json_Pack::SafeData_Lan()
{
    QJsonObject obj;
    QDateTime t = QDateTime::currentDateTime();
    ePro->testEndTime = t.toString("yyyy-MM-dd HH:mm:ss");

    ePro->order_num = mItem->cnt.all;

    obj.insert("productSn", ePro->product_sn);
    obj.insert("softVersion", ePro->softwareVersion);
    obj.insert("startTime", ePro->testStartTime);
    obj.insert("endTime", ePro->testEndTime);
    obj.insert("moduleSn", ePro->moduleSN);
    obj.insert("orderId", ePro->order_id);
    obj.insert("orderNum", ePro->order_num);
    obj.insert("testNum", "");
    obj.insert("devName", ePro->dev_name);
    obj.insert("languageSelect", 0);

    obj.insert("toolName", "qc-busbar");

    if(mPro->work_mode >=2) {
        QString str1 = ePro->itemContent.join("；");
        obj.insert("testCfg" ,str1);
    }

    int num = ePro->stepResult.size();
    QString str;
    for(int i=0; i<num; ++i)
    {
        obj.insert("testItem", ePro->test_function.at(i));
        obj.insert("testStep", ePro->sureItem.at(i));
        obj.insert("testProcess" ,ePro->itemData.at(i));
        obj.insert("testResult" ,ePro->stepResult.at(i));
        obj.insert("testRequest" ,ePro->stepRequest.at(i));
        stephttp_post("admin-api/bus/testData",mPro->Service,obj);
    }
}

void Json_Pack::stepData()
{
    QJsonObject obj;
    QDateTime t = QDateTime::currentDateTime();
    mPro->testEndTime = t.toString("yyyy-MM-dd HH:mm:ss");

    mPro->order_num = mItem->cnt.all;

    obj.insert("productSn", mPro->product_sn);
    obj.insert("softVersion", mPro->softwareVersion);
    obj.insert("startTime", mPro->testStartTime);
    obj.insert("endTime", mPro->testEndTime);
    obj.insert("moduleSn", mPro->moduleSN);
    obj.insert("orderId", mPro->order_id);
    obj.insert("orderNum", mPro->order_num);
    obj.insert("testNum", "");
    obj.insert("devName", mPro->dev_name);
    obj.insert("languageSelect", 0);

    obj.insert("toolName", "qc-busbar");

    if(mPro->work_mode >=2) {
        QString str1 = mPro->itemContent.join("；");
        obj.insert("testCfg" ,str1);
    }

    int num = mPro->stepResult.size();
    QString str;
    for(int i=0; i<num; ++i)
    {
        obj.insert("testItem", mPro->test_function.at(i));
        obj.insert("testStep", mPro->test_step);
        obj.insert("testProcess" ,mPro->itemData.at(i));
        obj.insert("testResult" ,mPro->stepResult.at(i));
        obj.insert("testRequest" ,mPro->stepRequest.at(i));
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

    obj.insert("productSn", ePro->product_sn);
    obj.insert("softVersion", ePro->softwareVersion);
    obj.insert("startTime", ePro->testStartTime);
    obj.insert("endTime", ePro->testEndTime);
    obj.insert("moduleSn", ePro->moduleSN);
    obj.insert("orderId", ePro->order_id);
    obj.insert("orderNum", ePro->order_num);
    obj.insert("testNum", "");
    obj.insert("devName", ePro->dev_name);
    obj.insert("languageSelect", 1);

    obj.insert("toolName", "qc-busbar");

    if(mPro->work_mode >=2) {
        QString str1 = ePro->itemContent.join(";");
        obj.insert("testCfg" ,str1);
    }

    int num = ePro->stepResult.size();
    QString str;
    for(int i=0; i<num; ++i)
    {
        obj.insert("testItem", ePro->test_function.at(i));
        obj.insert("testStep", ePro->test_step);
        obj.insert("testProcess" ,ePro->itemData.at(i));
        obj.insert("testResult" ,ePro->stepResult.at(i));
        obj.insert("testRequest" ,ePro->stepRequest.at(i));
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
        .timeoutMs(200) // 1s超时
        .block()
        .body(json)
        .exec();
}

void Json_Pack::FuncData(int num)
{
    QJsonObject obj; QJsonObject obj_en;
    QDateTime t = QDateTime::currentDateTime();
    mPro->testEndTime = t.toString("yyyy-MM-dd HH:mm:ss");

    mPro->order_num = mItem->cnt.all;

    obj.insert("productSn", mPro->product_sn);
    obj.insert("softVersion", mPro->softwareVersion);
    obj.insert("startTime", mPro->testStartTime);
    obj.insert("endTime", mPro->testEndTime);
    obj.insert("moduleSn", mPro->moduleSN);
    obj.insert("orderId", mPro->order_id);
    obj.insert("orderNum", mPro->order_num);
    obj.insert("testNum", "");
    obj.insert("devName", mPro->dev_name);
    obj.insert("languageSelect", 0);

    obj.insert("toolName", "qc-busbar");

    if(mPro->work_mode >= 2) {
        QString str1 = mPro->itemContent.join("；");
        obj.insert("testCfg" ,str1);
    }

    obj.insert("testItem", mPro->test_function.at(num));
    obj.insert("testStep", mPro->test_step);
    obj.insert("testProcess" ,mPro->itemData.at(num));
    obj.insert("testResult" ,mPro->stepResult.at(num));
    obj.insert("testRequest" ,mPro->stepRequest.at(num));
    stephttp_post("admin-api/bus/testData",mPro->Service,obj);

    // int num = mPro->stepResult.size();
    // int fag = 0;
    // if(num){
    //     for(int i=0; i<num; ++i)
    //     {
    //         if(mPro->test_function.at(i).contains("极性检查"))
    //     {
    //             fag = i; break;
    //         }
    //     }
    //     for(int i=0; i<=fag; ++i)
    //     {
    //         obj.insert("test_item", mPro->test_function.at(i));
    //         obj.insert("test_step", mPro->test_step);
    //         obj.insert("test_process" ,mPro->itemData.at(i));
    //         obj.insert("test_result" ,mPro->stepResult.at(i));
    //         obj.insert("test_request" ,mPro->stepRequest.at(i));
    //         stephttp_post("admin-api/bus/testData",mPro->Service,obj);
    //     }
    //     int j = 0;
    //     if((fag != 0) && ((fag +1) < num)){
    //         for(int i= fag +1; i<num; ++i)
    //         {
    //             obj.insert("test_item", mPro->test_function.at(i));
    //             obj.insert("test_step", mPro->sureItem.at(j));
    //             obj.insert("test_process" ,mPro->itemData.at(i));
    //             obj.insert("test_result" ,mPro->stepResult.at(i));
    //             obj.insert("test_request" ,mPro->stepRequest.at(i));
    //             stephttp_post("admin-api/bus/testData",mPro->Service,obj);
    //             j++;
    //         }
    //     }
    // }

}

void Json_Pack::FuncData_Lan(int num)
{
    QJsonObject obj;
    QDateTime t = QDateTime::currentDateTime();
    ePro->testEndTime = t.toString("yyyy-MM-dd HH:mm:ss");

    ePro->order_num = mItem->cnt.all;

    obj.insert("productSn", ePro->product_sn);
    obj.insert("softVersion", ePro->softwareVersion);
    obj.insert("startTime", ePro->testStartTime);
    obj.insert("endTime", ePro->testEndTime);
    obj.insert("moduleSn", ePro->moduleSN);
    obj.insert("orderId", ePro->order_id);
    obj.insert("orderNum", ePro->order_num);
    obj.insert("testNum", "");
    obj.insert("devName", ePro->dev_name);
    obj.insert("languageSelect", 1);

    obj.insert("toolName", "qc-busbar");

    if(mPro->work_mode >=2) {
        QString str1 = ePro->itemContent.join("；");
        obj.insert("testCfg" ,str1);
    }

    obj.insert("testItem", ePro->test_function.at(num));
    obj.insert("testStep", ePro->test_step);
    obj.insert("testProcess" ,ePro->itemData.at(num));
    obj.insert("testResult" ,ePro->stepResult.at(num));
    obj.insert("testRequest" ,ePro->stepRequest.at(num));
    stephttp_post("admin-api/bus/testData",mPro->Service,obj);

    // int num = mPro->stepResult.size();
    // int fag = 0;
    // if(num){
    //     for(int i=0; i<num; ++i)
    //     {
    //         if(mPro->test_function.at(i).contains("极性检查"))
    //     {
    //             fag = i; break;
    //         }
    //     }
    //     for(int i=0; i<=fag; ++i)
    //     {
    //         obj.insert("test_item", ePro->test_function.at(i));
    //         obj.insert("test_step", ePro->test_step);
    //         obj.insert("test_process" ,ePro->itemData.at(i));
    //         obj.insert("test_result" ,ePro->stepResult.at(i));
    //         obj.insert("test_request" ,ePro->stepRequest.at(i));
    //         stephttp_post("admin-api/bus/testData",mPro->Service,obj);
    //     }
    //     int j = 0;
    //     if((fag != 0) && ((fag +1) < num)){
    //         for(int i= fag +1; i<num; ++i)
    //         {
    //             obj.insert("test_item", ePro->test_function.at(i));
    //             obj.insert("test_step", ePro->sureItem.at(j));
    //             obj.insert("test_process" ,ePro->itemData.at(i));
    //             obj.insert("test_result" ,ePro->stepResult.at(i));
    //             obj.insert("test_request" ,ePro->stepRequest.at(i));
    //             stephttp_post("admin-api/bus/testData",mPro->Service,obj);
    //             j++;
    //         }
    //     }
    // }

}
