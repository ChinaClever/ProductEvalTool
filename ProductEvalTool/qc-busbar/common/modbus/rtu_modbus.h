#ifndef RTU_MODBUS_H
#define RTU_MODBUS_H

#include "rtu_write.h"
#include "common.h"
#define RTU_RW_SIZE 3

class Rtu_Modbus : public QObject
{
    Q_OBJECT
    explicit Rtu_Modbus(QObject *parent = nullptr);
public:
    static Rtu_Modbus *bulid(QObject *parent = nullptr);
    RtuRw *get(int id=1);

private:
    RtuRw *mRtuRw[RTU_RW_SIZE];
};

#endif // RTU_MODBUS_H
