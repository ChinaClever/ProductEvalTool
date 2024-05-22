/*
 * msgbox.cpp
 * 信息提示框
 *  Created on: 2016年10月11日
 *      Author: Lzy
 */
#include "msgbox.h"

bool MsgBox::critical(QWidget *parent,  const QString &str)
{
    bool ret = false;
    QMessageBox::StandardButton reply;
    reply = QMessageBox::critical(parent,  QObject::tr("错误提示"), str);
    if (reply == QMessageBox::Abort) ret = true;
    else if (reply == QMessageBox::Retry) ret = false;

    return ret;
}

bool MsgBox::information(QWidget *parent,  const QString &str)
{
    bool ret = false;
    QMessageBox::StandardButton reply;
    reply = QMessageBox::information(parent,  QObject::tr("信息提示"), str);
    if (reply == QMessageBox::Ok) ret = true;

    return ret;
}

bool MsgBox::question(QWidget *parent,  const QString &str)
{
    bool ret = false;
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(parent,  QObject::tr("信息询问"), str);
    if (reply == QMessageBox::Yes) ret = true;

    return ret;
}

bool MsgBox::warning(QWidget *parent,  const QString &str)
{
    bool ret = false;
    QMessageBox::StandardButton reply;
    reply = QMessageBox::warning(parent,  QObject::tr("警告信息"), str);
    if (reply == QMessageBox::Ok) ret = true;

    return ret;
}



