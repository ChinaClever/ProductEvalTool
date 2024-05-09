#ifndef MSGBOX_H
#define MSGBOX_H

#include <QtCore>
#include <QObject>
#include <QtGui>
#include <QMessageBox>
#include <QPushButton>
#include <QInputDialog>
#include <QGridLayout>
#include "backcolourcom.h"

class MsgBox
{
public:
    static bool critical(QWidget *parent = 0, const QString &str = "");
    static bool information(QWidget *parent = 0, const QString &str = "");
    static bool question(QWidget *parent = 0, const QString &str = "");
    static bool warning(QWidget *parent = 0, const QString &str = "");
};


#endif // MSGBOX_H
