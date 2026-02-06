#include "people_judg.h"
#include "ui_people_judg.h"

People_judg::People_judg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::People_judg)
{
    ui->setupUi(this);

    this->setWindowTitle(tr("人工确认"));
    groupBox_background_icon(this);

    mPacket = sDataPacket::bulid();
    mItem = Cfg::bulid()->item;
    mPro = mPacket->getPro();
    mPacketEng = datapacket_English::bulid();
    ePro = mPacketEng->getPro();

    timer = new QTimer(this);
    timer->start(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeoutDone()));
}

People_judg::~People_judg()
{
    delete ui;
}

void People_judg::timeoutDone()
{
    initData();
}

void People_judg::initData()
{
    if(mItem->modeId == START_BUSBAR)
    {
        ui->assemBox_10->show();
        ui->funcBox_2->show();
        ui->funcBox_3->hide();
        ui->funcBox_4->hide();
        ui->funcBox_5->hide();
        ui->funcBox->show();

        ui->lcdBox->show(); ui->keyBox->show();
        ui->alarmBox->show(); ui->beepBox->show();
        ui->assemBox_9->show();
        // ui->testBox->show();
        // ui->label_5->show();
    }else if( (mItem->modeId == INSERT_BUSBAR || mItem->modeId == TEMPERATURE_BUSBAR) && mPro->type == 1 ) {
        ui->assemBox_10->hide();
        ui->funcBox_2->hide();
        ui->funcBox_3->show();
        ui->funcBox_4->show();
        ui->funcBox_5->show();
        ui->funcBox->show();

        ui->lcdBox->show(); ui->keyBox->show();
        ui->alarmBox->show(); ui->beepBox->show();
        ui->assemBox_9->show();
        // ui->testBox->show();
        // ui->label_5->show();
    } else if(mItem->modeId == BASIC_TYPE) {//基本型没有极性检查
        // ui->testBox->hide();
        // ui->label_5->hide();
        ui->assemBox_10->hide();
        ui->funcBox_2->hide();
        ui->funcBox->hide();

        ui->lcdBox->hide(); ui->keyBox->hide();
        ui->alarmBox->hide(); ui->beepBox->hide();
        ui->assemBox_9->hide();
    }
}
void People_judg::writeData(const QString &str1,const QString &str2, const QString &str3,bool pass)
{
    mPro->stepRequest << str1; mPro->itemData << str2;
    mPro->test_function << str3;   
    if(pass) {
        mPro->stepResult << "1";
    } else {
        mPro->stepResult << "0";
    }
    if(mPro->online)
    {
        mPacket->delayMs(5);
        Json_Pack::bulid()->FuncData(mPro->stepNum,str1,str2,str3,pass);
        mPro->stepNum++;
    }
}

void People_judg::writeData_Eng(const QString &str1,const QString &str2, const QString &str3,bool pass)
{
    ePro->stepRequest << str1; ePro->itemData << str2;
    ePro->test_function << str3;
    if(pass) {
        ePro->stepResult << "1";
    } else {
        ePro->stepResult << "0";
    }
    if(mPro->online)
    {
        mPacket->delayMs(5);
        Json_Pack::bulid()->FuncData_Lan(mPro->stepNumEng , str1,str2,str3,pass);
        mPro->stepNumEng++;
    }
}

void People_judg::on_sureButton_clicked()
{
    // mPro->stepResult.clear(); ePro->stepResult.clear();
    // mPro->stepRequest.clear(); ePro->stepRequest.clear();
    // mPro->test_function.clear(); ePro->test_function.clear();
    // mPro->itemData.clear(); ePro->itemData.clear();

    mPacket->delayMs(10);
    this->close();
    QString str2 = tr("符合要求"); QString eng2 = tr("Meet a requirement");
    QString str3 = tr("不符合要求"); QString eng3 = tr("Not Satisfiable");
    mPro->test_step = "功能测试"; ePro->test_step = "Functional testing";

    QString str = tr("指示灯检查"); QString eng = tr("Indicator light inspection");
    QString str1 = ui->lcdBox->text();
    QString eng1 = tr("Generate an alarm, the indicator light turns red and flashes");
    bool ret = false;
    if(mItem->modeId == START_BUSBAR || mItem->modeId == INSERT_BUSBAR){
        bool ret = ui->lcdBox->isChecked();
        if(ret) {
            writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
        }else  {
            writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
        }
    }

    ret = ui->keyBox->isChecked();
    str1 = ui->keyBox->text();
    eng1 = tr("Release the alarm, the indicator light will turn green and flash");
    if(mItem->modeId == START_BUSBAR || mItem->modeId == INSERT_BUSBAR){
        if(ret) {
            writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
        }else {
            writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
        }
    }

    str = tr("蜂鸣器开关检查"); eng = tr("Buzzer switch inspection");
    mPro->test_step = "功能测试"; ePro->test_step = "Functional testing";
    ret = ui->alarmBox->isChecked();
    str1 = ui->alarmBox->text();
    eng1 = tr("Generate an alarm, the buzzer sounds loud and clear");
    if(mItem->modeId == START_BUSBAR || mItem->modeId == INSERT_BUSBAR){
        if(ret) {
            writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
        }else {
            writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
        }
    }

    str = tr("蜂鸣器开关检查");
    mPro->test_step = "功能测试"; ePro->test_step = "Functional testing";
    ret = ui->beepBox->isChecked();
    str1 = ui->beepBox->text();
    eng1 = tr("Release the alarm and stop the buzzer sound");
    if(mItem->modeId == START_BUSBAR || mItem->modeId == INSERT_BUSBAR){
        if(ret) {
            writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
        }else {
            writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
        }
    }

    //if(mPro->type == 1 && (mPro->work_mode == 0 || mPro->work_mode == 1))
    {
        str = tr("极性检查"); eng = tr("Polarity check");
        ret = ui->testBox->isChecked();
        str1 = ui->testBox->text();
        eng1 = tr("Connect the output end to the load and check that the polarity test module indicator lights 1 and 2 are on");
        if(ret) {
            writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
        }else {
            writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
        }
    }

    // Cfg::bulid()->writeJudgItem();

//=================================================================================//
    QString str4 = tr("外观检查"); QString eng4 = tr("Inspection");
    // mPro->sureItem << str4; ePro->sureItem << eng4;
    mPro->test_step = "外观检查"; ePro->test_step = "Inspection";

    str = tr("产品外观"); eng = tr("Appearance");
    ret = ui->appearBox->isChecked();
    str1 = ui->appearBox->text();
    eng1 = tr("At a distance of 30cm from the product, check for scratches, dirt, paint peeling, exposed bottom, deformation, and cracking. Standard reference: New Busbar General Inspection Standard for Appearance");
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

    // mPro->sureItem << str4; ePro->sureItem << eng4;
    str = tr("外观丝印"); eng = tr("Appearance screen printing");
    str1 = ui->appearBox_2->text();
    eng1 = tr("All silk and mold prints on the components are clear and visible, without any defects, blurriness, or other abnormal phenomena");
    ret = ui->appearBox_2->isChecked();
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else  {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

    // mPro->sureItem << str4; ePro->sureItem << eng4;
    str = tr("内观"); eng = tr("Vipassana");
    str1 = ui->appearBox_3->text();
    eng1 = tr("The interior of the product is clean and free of foreign objects. It is not allowed to have any abnormal items such as wire or screws inside the product, and there should be no abnormal noise when shaken");
    ret = ui->appearBox_3->isChecked();
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else  {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

    // mPro->sureItem << str4; ePro->sureItem << eng4;
    str = tr("产品标识"); eng = tr("Product identification");
    str1 = ui->appearBox_4->text();
    eng1 = tr("Labels, without any errors or omissions, with consistent placement and clear content, and no lifting or falling off of labels");
    ret = ui->appearBox_4->isChecked();
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else  {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

//============================================================================================//
    str4 = tr("包装检查"); eng4 = tr("Packaging inspection");
    // mPro->sureItem << str4; ePro->sureItem << eng4;
    mPro->test_step = "包装检查"; ePro->test_step = "Packaging inspection";

    str = tr("包装方式"); eng = tr("Packaging method");
    str1 = ui->packBox->text();
    eng1 = tr("The product placement and stacking method are correct");
    ret = ui->packBox->isChecked();
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else  {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

    // mPro->sureItem << str4; ePro->sureItem << eng4;
    str = tr("配件"); eng = tr("Accessory");
    str1 = ui->packBox_2->text();
    eng1 = tr("No missing or incorrectly installed accessories");
    ret = ui->packBox_2->isChecked();
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else  {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

    // mPro->sureItem << str4; ePro->sureItem << eng4;
    str = tr("包装标签"); eng = tr("Labeling");
    str1 = ui->packBox_3->text();
    eng1 = tr("The contents of packaging labels, color box labels, and pallet labels are correct and consistent");
    ret = ui->packBox_3->isChecked();
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else  {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

//==========================================================================//
    str4 = tr("装配检查"); eng4 = tr("Assembly inspection");
    mPro->test_step = "装配检查"; ePro->test_step = "Assembly inspection";

    // mPro->sureItem << str4; ePro->sureItem << eng4;
    str = tr("配件型号"); eng = tr("Accessory model");
    str1 = ui->assemBox->text();
    eng1 = tr("The product component model should be consistent with the BOM list");
    ret = ui->assemBox->isChecked();
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else  {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

    // mPro->sureItem << str4; ePro->sureItem << eng4;
    str = tr("产品一致性"); eng = tr("Product consistency");
    str1 = ui->assemBox_2->text();
    eng1 = tr("Maintain consistency in the assembly position and direction of each group of components in the product");
    ret = ui->assemBox_2->isChecked();
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else  {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

    // mPro->sureItem << str4; ePro->sureItem << eng4;
    str = tr("装配效果"); eng = tr("Assembly effect");
    str1 = ui->assemBox_3->text();
    eng1 = tr("All components are installed in place without any abnormal gaps or gaps");
    ret = ui->assemBox_3->isChecked();
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else  {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

    // mPro->sureItem << str4; ePro->sureItem << eng4;
    str = tr("组装"); eng = tr("Assemble");
    str1 = ui->assemBox_4->text();
    eng1 = tr("There are no abnormal phenomena such as missing screws, slipping teeth, floating height, or loose components in the entire machine");
    ret = ui->assemBox_4->isChecked();
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else  {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

    // mPro->sureItem << str4; ePro->sureItem << eng4;
    str = tr("线缆"); eng = tr("Cable");
    str1 = ui->assemBox_5->text();
    eng1 = tr("There are no abnormal phenomena such as incorrect connection, missed connection, cable damage, or copper leakage in the internal cables of the entire machine");
    ret = ui->assemBox_5->isChecked();
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else  {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

    // mPro->sureItem << str4; ePro->sureItem << eng4;
    str = tr("扭力"); eng = tr("Torque force");
    str1 = ui->assemBox_6->text();
    eng1 = tr("The torque of each screw meets the SOP requirements");
    ret = ui->assemBox_6->isChecked();
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else  {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

    // mPro->sureItem << str4; ePro->sureItem << eng4;
    str = tr("端子"); eng = tr("Terminal");
    str1 = ui->assemBox_7->text();
    eng1 = tr("The internal cable terminal connections of the product are not allowed to be loose, detached, not installed properly, and the crimping screws are not tightened");
    ret = ui->assemBox_7->isChecked();
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else  {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

    // mPro->sureItem << str4; ePro->sureItem << eng4;
    str = tr("接地"); eng = tr("Grounding");
    str1 = ui->assemBox_8->text();
    eng1 = tr("The main body, surface shell and other related components need to be grounded. The grounding screw and the surrounding area of the main panel position should not be obstructed by foreign objects");
    ret = ui->assemBox_8->isChecked();
    if(ret) {
        writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
    }else  {
        writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
    }

    // mPro->sureItem << str4; ePro->sureItem << eng4;
    if(mItem->modeId == START_BUSBAR || mItem->modeId == INSERT_BUSBAR){
        str = tr("表头插拔"); eng = tr("Meter head insertion and removal");
        str1 = ui->assemBox_9->text();
        eng1 = tr("After inserting the meter head, no abnormal phenomena such as shaking, loosening, or inability to insert are allowed. The pulling force is (50 ± 5N)");
        ret = ui->assemBox_9->isChecked();
        if(ret) {
            writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
        }else  {
            writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
        }
    }
    if(mItem->modeId == START_BUSBAR)
    {
        ui->assemBox_10->show();
        // mPro->sureItem << str4; ePro->sureItem << eng4;
        str = tr("防雷"); eng = tr("Lightning protection");
        str1 = ui->assemBox_10->text();
        eng1 = tr("There is no shortage or omission of lightning protection insurance. The lid is closed without any looseness or detachment");
        ret = ui->assemBox_10->isChecked();
        if(ret) {
            writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
        }else  {
            writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
        }
    }

//===============================================================================================//
    str4 = tr("基本使用功能"); eng4 = tr("Basic usage functions");
    mPro->test_step = "基本使用功能"; ePro->test_step = "Basic usage functions";

    // mPro->sureItem << str4; ePro->sureItem << eng4;
    str = tr("表头按键"); eng = tr("Meter head button");
    str1 = ui->funcBox->text();
    eng1 = tr("The buttons can be used normally and have a consistent feel");
    ret = ui->funcBox->isChecked();
    if(mItem->modeId == START_BUSBAR || mItem->modeId == INSERT_BUSBAR){
        if(ret) {
            writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
        }else  {
            writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
        }
    }

    if(mItem->modeId == START_BUSBAR)
    {
        // mPro->sureItem << str4; ePro->sureItem << eng4;
        str = tr("面板圆锁"); eng = tr("Panel circular lock");
        str1 = ui->funcBox_2->text();
        eng1 = tr("Both keys of the panel lock can be opened and closed normally");
        ret = ui->funcBox_2->isChecked();
        if(ret) {
            writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
        }else  {
            writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
        }
    }else if(mItem->modeId == START_BUSBAR||mItem->modeId == INSERT_BUSBAR || mItem->modeId == BASIC_TYPE) {

        // mPro->sureItem << str4; ePro->sureItem << eng4;
        str = tr("卡爪"); eng = tr("Claw");
        str1 = ui->funcBox_3->text();
        eng1 = tr("Press the claw button, and the claw can be unfolded and locked to both sides normally. Press the locking button, the claw can rebound normally");
        ret = ui->funcBox_3->isChecked();
        if(ret) {
            writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
        }else  {
            writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
        }

        // mPro->sureItem << str4; ePro->sureItem << eng4;
        str = tr("取电模块"); eng = tr("Power intake module");
        str1 = ui->funcBox_4->text();
        eng1 = tr("The handle of the power module is pressed smoothly without any abnormal interference or jamming, and the locking mechanism can be activated. After the locking mechanism is activated, the power module cannot sink");
        ret = ui->funcBox_4->isChecked();
        if(ret) {
            writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
        }else  {
            writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
        }

        // mPro->sureItem << str4; ePro->sureItem << eng4;
        str = tr("插座模块"); eng = tr("Socket module");
        str1 = ui->funcBox_5->text();
        eng1 = tr("Copper sleeve tin, standard plug can be used normally without any inability to insert, protective cover can automatically rebound");
        ret = ui->funcBox_5->isChecked();
        if(mItem->modeId == START_BUSBAR||mItem->modeId == INSERT_BUSBAR){
            if(ret) {
                writeData(str1,str2,str,ret); writeData_Eng(eng1,eng2,eng,ret);
            }else  {
                writeData(str1,str3,str,ret); writeData_Eng(eng1,eng3,eng,ret);
            }
        }
    }
    mPro->issure = 1;


}

