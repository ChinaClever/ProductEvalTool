/*
 * email_setdlg.cpp
 *  邮件配置界面
 *
 *  Created on: 2016年10月11日
 *      Author: Lzy
 */
#include "email_setdlg.h"
#include "ui_email_setdlg.h"
#include "common.h"
#include "email.h"

static email_setInfo *gSet = NULL;
email_setInfo *email_get_setInfo()
{
    return gSet;
}

Email_SetDlg::Email_SetDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Email_SetDlg)
{
    ui->setupUi(this);
    setWindowTitle(tr("SMTP设置"));
    groupBox_background_icon(this);

    gSet = mSet = new email_setInfo;
    initData();
    initWidget();
}

Email_SetDlg::~Email_SetDlg()
{
    delete ui;
}


/**
 * @brief 初始化数据
 */
void Email_SetDlg::initData(void)
{
    CfgCom *cfg = CfgCom::bulid();
    mSet->server = cfg->read("Server", Email_Server, "Email").toString();
    mSet->usr = cfg->read("UsrName", Email_UsrName, "Email").toString();
    mSet->pwd = cfg->read("Password", Email_Password, "Email").toString();
    mSet->to = cfg->read("To", Email_ToUsrName, "Email").toString();
    mSet->port = cfg->read("Port", Email_Port, "Email").toInt();
    mSet->ssl = cfg->read("ssl", 0, "Email").toInt();
}

/**
 * @brief 窗口初始
 */
void Email_SetDlg::initWidget(void)
{
    ui->usrEdit->setText(mSet->usr);
    ui->pwdEdit->setText(mSet->pwd);
    ui->serEdit->setText(mSet->server);
    ui->portEdit->setText(QString::number(mSet->port));
    ui->comboBox->setCurrentIndex(mSet->ssl);
}

/**
 * @brief 数据验证
 * @return
 */
bool Email_SetDlg::dataCheck(void)
{
    QString str = ui->portEdit->text();

    bool ret = cm_isDigitStr(str);
    if(ret == false)
        MsgBox::critical(this, "发件箱端口号错误!");

    return ret;
}

/**
 * @brief 保存数据
 */
void Email_SetDlg::saveData(void)
{
    CfgCom *cfg = CfgCom::bulid();
    mSet->usr =  ui->usrEdit->text();
    cfg->write("UsrName", mSet->usr, "Email");

    mSet->pwd = ui->pwdEdit->text();
    cfg->write("Password", mSet->pwd, "Email");

    mSet->server = ui->serEdit->text();
    cfg->write("Server", mSet->server, "Email");

    mSet->to = ui->toEdit->text();
    cfg->write("To", mSet->to, "Email");

    mSet->port = ui->portEdit->text().toInt();
    cfg->write("Port", mSet->port, "Email");

    mSet->ssl = ui->comboBox->currentIndex();
    cfg->write("ssl", mSet->ssl, "Email");
}

/**
 * @brief 保存按键
 */
void Email_SetDlg::on_saveBtn_clicked()
{
    bool ret = dataCheck();
    if(ret) {
        this->close();
        saveData();
    }
}


void Email_SetDlg::on_testBtn_clicked()
{
    bool ret = dataCheck();
    if(ret) {
        saveData();

        QString subject = tr("PDU测试邮件");
        QString body = tr("这是一份测试邮件!");

        Email email;
        QStringList recipient;
        recipient << mSet->to;
        email.sentEmail(recipient,subject,body);

        MsgBox::information(this,tr("邮件已发送至：%1，请等待!").arg(mSet->to));
    } else {

    }
}
