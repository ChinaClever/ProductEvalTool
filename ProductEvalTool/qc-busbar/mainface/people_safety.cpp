#include "people_safety.h"
#include "ui_people_safety.h"

People_Safety::People_Safety(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::People_Safety)
{
    ui->setupUi(this);

    this->setWindowTitle(tr("人工确认"));
    groupBox_background_icon(this);
    initData();
}

People_Safety::~People_Safety()
{
    delete ui;
}

void People_Safety::initData()
{
    mPacket = sDataPacket::bulid();
    mItem = Cfg::bulid()->item;
    mPro = mPacket->getPro();
    mPacketEng = datapacket_English::bulid();
    ePro = mPacketEng->getPro();
}

void People_Safety::writeData(const QString &str1,const QString &str2, const QString &str3,bool pass)
{
    mPro->stepRequest << str1; mPro->itemData << str2;
    mPro->test_function << str3;   
    if(pass) {
        mPro->stepResult << "1";
    } else {
        mPro->stepResult << "0";
    }

}

void People_Safety::writeData_Eng(const QString &str1,const QString &str2, const QString &str3,bool pass)
{
    ePro->stepRequest << str1; ePro->itemData << str2;
    ePro->test_function << str3;   
    if(pass) {
        ePro->stepResult << "1";
    } else {
        ePro->stepResult << "0";
    }

}

void People_Safety::on_sureButton_clicked()
{
    mPacket->delayMs(10);
    mPro->issure = 1; this->close();

    QString str2 = tr("符合要求"); QString eng2 = tr("Meet a requirement");
    QString str3 = tr("不符合要求"); QString eng3 = tr("Not Satisfiable");
    QString str4 = tr("外观检查"); QString eng4 = tr("Inspection");

    mPro->sureItem << str4; ePro->sureItem << eng4;
    QString str = tr("产品外观"); QString eng = tr("Appearance");
    QString str1 = ui->appearBox->text();
    QString eng1 = tr("At a distance of 30cm from the product, check for scratches, dirt, paint peeling, exposed bottom, deformation, and cracking. Standard reference: New Busbar General Inspection Standard for Appearance");
    bool ret = ui->appearBox->isChecked();
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else  {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

    mPro->sureItem << str4; ePro->sureItem << eng4;
    str = tr("外观丝印"); eng = tr("Appearance screen printing");
    str1 = ui->appearBox_2->text();
    eng1 = tr("All silk and mold prints on the components are clear and visible, without any defects, blurriness, or other abnormal phenomena");
    ret = ui->appearBox_2->isChecked();
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else  {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

    mPro->sureItem << str4; ePro->sureItem << eng4;
    str = tr("内观"); eng = tr("Vipassana");
    str1 = ui->appearBox_3->text();
    eng1 = tr("The interior of the product is clean and free of foreign objects. It is not allowed to have any abnormal items such as wire or screws inside the product, and there should be no abnormal noise when shaken");
    ret = ui->appearBox_3->isChecked();
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else  {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

    mPro->sureItem << str4; ePro->sureItem << eng4;
    str = tr("安装位置"); eng = tr("Installation position");
    str1 = ui->appearBox_4->text();
    eng1 = tr("Reserved installation position and reserved length for copper bars (50-53mm)");
    ret = ui->appearBox_4->isChecked();
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else  {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }
//==================================================================================//
    str4 = tr("包装检查"); eng4 = tr("Packaging inspection");

    mPro->sureItem << str4; ePro->sureItem << eng4;
    str = tr("包装方式"); eng = tr("Packaging method");
    str1 = ui->packBox->text();
    eng1 = tr("The product placement and stacking method are correct");
    ret = ui->packBox->isChecked();
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else  {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

    mPro->sureItem << str4; ePro->sureItem << eng4;
    str = tr("配件"); eng = tr("Accessory");
    str1 = ui->packBox_2->text();
    eng1 = tr("No missing or incorrectly installed accessories");
    ret = ui->packBox_2->isChecked();
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else  {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

    mPro->sureItem << str4; ePro->sureItem << eng4;
    str = tr("包装标签"); eng = tr("Labeling");
    str1 = ui->packBox_2->text();
    eng1 = tr("The contents of packaging labels, color box labels, and pallet labels are correct and consistent");
    ret = ui->packBox_2->isChecked();
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else  {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

//==================================================================================//
    str4 = tr("装配检查"); eng4 = tr("Assembly inspection");

    mPro->sureItem << str4; ePro->sureItem << eng4;
    str = tr("配件型号"); eng = tr("Accessory model");
    str1 = ui->assemBox->text();
    eng1 = tr("The product component model should be consistent with the BOM list");
    ret = ui->assemBox->isChecked();
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else  {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

    mPro->sureItem << str4; ePro->sureItem << eng4;
    str = tr("产品一致性"); eng = tr("Product consistency");
    str1 = ui->assemBox_2->text();
    eng1 = tr("Maintain consistency in the assembly position and direction of each group of components in the product");
    ret = ui->assemBox_2->isChecked();
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else  {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

    mPro->sureItem << str4; ePro->sureItem << eng4;
    str = tr("装配效果"); eng = tr("Assembly effect");
    str1 = ui->assemBox_3->text();
    eng1 = tr("All components are installed in place without any abnormal gaps or gaps");
    ret = ui->assemBox_3->isChecked();
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else  {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

    mPro->sureItem << str4; ePro->sureItem << eng4;
    str = tr("组装"); eng = tr("Assemble");
    str1 = ui->assemBox_4->text();
    eng1 = tr("There are no abnormal phenomena such as missing screws, slipping teeth, floating height, or loose components in the entire machine");
    ret = ui->assemBox_4->isChecked();
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else  {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

    mPro->sureItem << str4; ePro->sureItem << eng4;
    str = tr("线缆"); eng = tr("Cable");
    str1 = ui->assemBox_5->text();
    eng1 = tr("There are no abnormal phenomena such as incorrect connection, missed connection, cable damage, or copper leakage in the internal cables of the entire machine");
    ret = ui->assemBox_5->isChecked();
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else  {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

    mPro->sureItem << str4; ePro->sureItem << eng4;
    str = tr("扭力"); eng = tr("Torque force");
    str1 = ui->assemBox_6->text();
    eng1 = tr("The torque of each screw meets the SOP requirements");
    ret = ui->assemBox_6->isChecked();
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else  {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }


}
