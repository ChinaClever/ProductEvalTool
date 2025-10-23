#include "face_Volinsul.h"
#include "ui_face_Volinsul.h"
#include "printer_bartender.h"

Face_Volinsul::Face_Volinsul(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Face_Volinsul)
{
    ui->setupUi(this);
    mItem = TestConfig::bulid()->item;
    mPacket = sDataPacket::bulid();
    mPro = mPacket->getPro();
    mDataSave = new TestDataSave(this);
    mCfg = Cfg::bulid()->item;
    Printer_BarTender::bulid(this);
    mSn = Sn_SerialNum::bulid(this);
    mDev = mPacket->getDev();
    mPacketEng = datapacket_English::bulid();
    ePro = mPacketEng->getPro();

    initWid();
}

Face_Volinsul::~Face_Volinsul()
{
    delete ui;
}

void Face_Volinsul::initWid()
{
    timer = new QTimer(this);
    timer->start(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(progressSlot()));
}

void Face_Volinsul::startSlot()
{
    QString name = mItem->sn.name;
    mPro->productType = mItem->sn.name;
    QString sn = mItem->sn.sn;
    mItem->progress.errNum = mItem->progress.finishNum = mItem->progress.okNum = 0;
    if(mItem->work_mode == 0) {
        ui->progressBar->setValue(0);
        ui->progressBar->setStyleSheet("QProgressBar {border:2px solid;background-color:transparent;border-radius: 5px;text-align: center;color:green;}" );
    }else if(mItem->work_mode == 1) {
        ui->progressBar2->setValue(0);
        ui->progressBar2->setStyleSheet("QProgressBar {border:2px solid;background-color:transparent;border-radius: 5px;text-align: center;color:green;}" );
    }

}

bool Face_Volinsul::printer(QString ip ,QString url, int port)
{
    // QString ip = "192.168.1.13";
    qDebug()<<"printer";
    bool ret = true;
    QString str = tr("标签打印 "); QString str1;
        // if(mPro->result != Test_Fail){
        sBarTend it;
        QString mPn = mCfg->pn;//订单号+成品代码
        QStringList list = mPn.split("+");
        for(int i = 0; i < list.count(); i++)
        {
            if(i == 0) it.on = list.at(i);
            if(i == 1) it.pn = list.at(i);
        }

        QString mSn = mDev->devType.sn;//模块序列号
        it.sn =  mSn.remove(QRegExp("\\s"));
        qDebug()<<"mSn: "<<mSn;
        it.fw = "V0.0";
        it.hw = "V1.0";
        // if(it.sn.isEmpty()){
        //     mPro->result = Test_Fail;
        //     ret  = false;
        //     if(it.sn.isEmpty()) str += tr(" 读取到序列号SN为空 ");
        // }
        if(ret){
            str1 = Printer_BarTender::bulid(this)->http_post(url, ip, it, port);
            if(str1 == "Success") {
                ret = true;
            }else {
                str1 = Printer_BarTender::bulid(this)->http_post(url, ip, it, port);
                if(str1 == "Success") {
                    ret = true;
                }else ret = false;
            }
        }
        if(ret) str += tr("正常"); else str += tr("错误");
    // } else str = tr("因测试未通过，标签未打印");
    return mPacket->updatePro(str, ret);
}

void Face_Volinsul::resultSlot()
{
    bool p = true;
    if(mItem->progress.errNum)  p = false;

    mItem->progress.allNum = mItem->progress.finishNum;
    if(mItem->progress.allNum == 0) {mItem->progress.okNum = 0; mItem->progress.allNum = 1; p = false;}
    int ok = (mItem->progress.okNum * 100.0) / mItem->progress.allNum;
    QString str1 = tr("测试项目数:%1  失败项目数：%2  项目测试通过率：%3%").arg(mItem->progress.allNum).arg(mItem->progress.errNum).arg(ok);
    mPacket->updatePro(str1);

    if(mCfg->modeId == 2 || mCfg->modeId == BASIC_TYPE) //测试
    {
        if(mDev->devType.sn.isEmpty() || mCfg->modeId == 2 || mCfg->modeId == BASIC_TYPE) mSn->createSn();//设置序列号
        QString str = mDev->devType.sn;
        mPro->moduleSN = str.remove(QRegExp("\\s"));
        mCfg->moduleSn = mPro->moduleSN; Cfg::bulid()->writeQRcode();
    }



    if(mCfg->modeId == 2 && mPro->result != Test_Fail && mItem->work_mode == 0) emit finshSig();
    if(mCfg->modeId == 2 && mPro->result != Test_Fail && mItem->work_mode == 0){
        while(1)
        {
            mPacket->delay(1);
            if(mPro->issure)
            {
                break;
            }
        }
    }

    updateData();
    if(mPro->online) {
        mPacket->delayMs(5);
        Json_Pack::bulid()->SendJson_Safe();
    }

    if(mPro->result != Test_Fail && mItem->work_mode == 1 && mPro->type == 0) emit overSig();
    if(mPro->result != Test_Fail && mItem->work_mode == 1 && mPro->type == 0){
        while(1)
        {
            mPacket->delay(1);
            if(mPro->issure)
            {
                break;
            }
        }
    }



    bool res = false;

    QString str = tr("测试结果 ");
    if(!p)
    {
        if(mItem->work_mode == 0) ui->progressBar->setStyleSheet("QProgressBar {border:2px solid;background-color:transparent;border-radius: 5px;text-align: center;color:red;}" );
        else if(mItem->work_mode == 1) ui->progressBar2->setStyleSheet("QProgressBar {border:2px solid;background-color:transparent;border-radius: 5px;text-align: center;color:red;}" );

        res = false; str += tr("失败");
        mPro->uploadPassResult = 0;
    } else {
        qDebug()<<"mCfg->modeId: "<<mCfg->modeId;
        qDebug()<<"mPro->word_mode: "<< mPro->work_mode;
        qDebug()<<"mPro->type: "<<mPro->type;
        if((mCfg->modeId == 2 && mPro->work_mode == 0))//母线槽与基本型始端箱和插接箱只需安规测试，接地测试成功打印标签
        {                                                                           //mPro->type == 0  测试
            QString method; int port = 0; QString ip;
            if(mCfg->modeId == 2) {method = "Integration/Busbar-Busway/Execute"; port = 81; ip = "127.0.0.1"; }
            else { ip = "127.0.0.1"; method = "Integration/Busbar-Product/Execute"; port = 80;}

            res = printer(ip, method, port);
            if(res) {
                str += tr("通过");mPro->uploadPassResult = 1;
            }else {
                str += tr("失败");mPro->uploadPassResult = 0;
            }
            if(mCfg->cnt.num > 0) {
                mCfg->cnt.num -= 1;
            }
        }else {
            res = true; str += tr("成功");
            mPro->uploadPassResult = 1;
        }
        // res = true; str += tr("成功");
        // mPro->uploadPassResult = 1;
    }
    mPacket->updatePro(str, res);

    mDataSave->saveTestData();

    if(mPro->result == Test_Fail) res = false;
    else {res = true;}
    emit StatusSig(res);
    mPro->step = Test_Over;
}

void Face_Volinsul::updateData()
{
    if(mPro->work_mode == 0){
         if((mPro->acwParm.size()) && (mPro->irParm.size())){
             mPro->itemRequest = tr("绝缘测试，分别对以下测试点输入电压 %1V，%2s：").arg(mPro->irParm.at(2)).arg(mPro->irParm.at(7))
                             + tr("N-PE/L1/L2/L3，绝缘电阻 >10MΩ。")
                             + tr("交流耐压测试，分别对以下测试点输入电压 %1V，%2s：").arg(mPro->acwParm.at(2)).arg(mPro->acwParm.at(6))
                                 + tr("1、PE-N/L1/L2/L3，2、N-PE/L1/L2/L3，3、L1-PE，4、L2-PE，5、L3-PE，漏电流≤10mA。");

            if(mCfg->modeId != 2) mPro->itemRequest += tr("检查极性测试模块指示灯显示应正常，火、零、地接线应正确。");

            ePro->itemRequest = tr("Insulation resistance test,input voltage to the following test points respectively %1V，%2s：").arg(mPro->irParm.at(2)).arg(mPro->irParm.at(7))
                             + tr("N-PE/L1/L2/L3,insulation resistance >10MΩ。")
                             + tr("AC withstand voltage test, input voltage to the following test points respectively %1V，%2s：").arg(mPro->acwParm.at(2)).arg(mPro->acwParm.at(6))
                             + tr("1、PE-N/L1/L2/L3,2、N-PE/L1/L2/L3,3、L1-PE,4、L2-PE,5、L3-PE,Leakage current <10mA。");

            if(mCfg->modeId != 2) ePro->itemRequest += tr("Check that the indicator light on the polarity test module should be normal and the live, neutral and ground connections should be correct。");

         }else {
            mPro->itemRequest = tr("绝缘测试，分别对以下测试点输入电压 500V，5s：")
                             + tr("N-PE/L1/L2/L3，绝缘电阻 >10MΩ。")
                             + tr("交流耐压测试，分别对以下测试点输入电压 3000V，5s：")
                             + tr("1、PE-N/L1/L2/L3，2、N-PE/L1/L2/L3，3、L1-PE，4、L2-PE，5、L3-PE，漏电流≤10mA。");

                            if(mCfg->modeId != 2) mPro->itemRequest += tr("检查极性测试模块指示灯显示应正常，火、零、地接线应正确。");

            ePro->itemRequest = tr("Insulation resistance test,input voltage to the following test points respectively 500V，5s：")
                             + tr("N-PE/L1/L2/L3,insulation resistance >10MΩ。")
                             + tr("AC withstand voltage test, input voltage to the following test points respectively 3000V，5s：")
                             + tr("1、PE-N/L1/L2/L3,2、N-PE/L1/L2/L3,3、L1-PE,4、L2-PE,5、L3-PE,Leakage current <10mA.");

                            if(mCfg->modeId != 2) ePro->itemRequest += tr("Check that the indicator light on the polarity test module should be normal and the live, neutral and ground connections should be correct。");
         }
    }

    if(mPro->work_mode == 1){
         if(mPro->gndParm.size())
         {
                mPro->itemRequest = tr("接地测试，分别对以下测试点输入电流 %1A，%2s：").arg(mPro->gndParm.at(2)).arg(mPro->gndParm.at(6))
                             + tr("PE-箱体面壳接地螺钉，接地电阻<100mΩ。");

                ePro->itemRequest =tr("Grounding test, input current to the following test points respectively %1A，%2s：").arg(mPro->gndParm.at(2)).arg(mPro->gndParm.at(8))
                             + tr("PE - box surface shell grounding screw,grounding resistance <100mΩ.");
         }else {
                mPro->itemRequest = tr("接地测试，分别对以下测试点输入电流 25A，5s：")
                             + tr("PE-箱体面壳接地螺钉，接地电阻<100mΩ。");

                ePro->itemRequest =tr("Grounding test, input current to the following test points respectively 25A，5s：")
                             + tr("PE - box surface shell grounding screw,grounding resistance <100mΩ.");
         }
    }

}
void Face_Volinsul::progressSlot()
{
    sTestProgress *arg = &(mItem->progress);

    int progress = (arg->finishNum * 100.0) / arg->allNum;
    if(mItem->work_mode == 0) ui->progressBar->setValue(progress);
    else if(mItem->work_mode == 1) ui->progressBar2->setValue(progress);

    if(arg->errNum)
    {
        if(mItem->work_mode == 0) ui->progressBar->setStyleSheet("QProgressBar {border:2px solid;background-color:transparent;border-radius: 5px;text-align: center;color:red;}" );
        else if(mItem->work_mode == 1) ui->progressBar2->setStyleSheet("QProgressBar {border:2px solid;background-color:transparent;border-radius: 5px;text-align: center;color:red;}" );
    }


}
