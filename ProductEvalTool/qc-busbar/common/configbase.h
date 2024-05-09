#ifndef CONFIGBASE
#define CONFIGBASE
#include "sysconfigfile.h"
#include "datapacket.h"

class ConfigBase
{
public:
    ConfigBase();

    sTestConfigItem *item;
    void initConfig(sSafetyItem *it=nullptr);
    void saveConfig(sSafetyItem *it=nullptr);

    QString getSerialName(const QString &prefix="test");
    void setSerialName(const QString &name,const QString &prefix="test");

protected:
    QString getName();
    void setName(const QString &arg);

    QString getOp();
    void setOp(const QString &arg);

    QString getCn();
    void setCn(const QString &arg);

    QString getBarCode();
    void setBarCode(const QString &arg);

    QString getBatch();
    void setBatch(const QString &arg);

    QString getPurpose();
    void setPurpose(const QString &arg);

    bool getSnClear();
    void setSnClear(bool mode);

    virtual QString getPrefix()=0;
};

#endif // CONFIGBASE
