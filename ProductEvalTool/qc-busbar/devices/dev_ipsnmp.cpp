/*
 *
 *  Created on: 2021年1月1日
 *      Author: Lzy
 */
#include "dev_ipsnmp.h"

Dev_IpSnmp::Dev_IpSnmp(QObject *parent) : Dev_Object(parent)
{
    mPacket = sDataPacket::bulid();
    mItem = Cfg::bulid()->item;
    mPro = mPacket->getPro();
    mDev = mPacket->getDev();
    mSnmp = new SnmpClient(this);
    mBusData = get_share_mem();
}

Dev_IpSnmp *Dev_IpSnmp::bulid(QObject *parent)
{
    static Dev_IpSnmp* sington = nullptr;
    if(sington == nullptr)
        sington = new Dev_IpSnmp(parent);
    return sington;
}

void Dev_IpSnmp::resetSnmp()
{
    //delete mSnmp; msleep(150);
    mSnmp = new SnmpClient(this);
}

int Dev_IpSnmp::getItemByOid(const QString &oid, int id)
{
    QString res = QString(oid).remove(BUSBAR_MIB_OID).simplified();
    QStringList list = res.split(".");
    return list.at(id).toInt();
}

bool Dev_IpSnmp::lineNumV3()
{
    bool ret = true;
    QString oid(MIB_OID_CLEVER); oid += "10.3.1.4";
    QtSnmpDataList values = mSnmp->requestValue(oid);
    if(values.size()) {
        QtSnmpData value = values.first();
        sObjData *obj = &(mDev->line);
        obj->size = value.data().toHex().toInt(&ret,16);
        if(obj->size == 2) obj->size = 3;
        obj->vol.size = obj->cur.size = obj->size;
    } else {
        ret = false;
    }

    return ret;
}

bool Dev_IpSnmp::devDataV3()
{
    qDebug() << "before Error initializing v3MP: ";
    bool ret = false;
    QString ipAddr = "192.168.1.163";
    QString roCom = "public";
    QString v3PrivPass = "privatekey1";
    QString v3AuthPass = "authkey123";
    QString name = "Busbar";
    int num = 1;
    int readVersion = version3;

    Snmp::socket_startup();
    char ipaddr[255];
    memset(ipaddr,0,sizeof(char)*255);
    strncpy_s(ipaddr,sizeof(char)*255,ipAddr.toStdString().c_str(),sizeof(char)*255);
    UdpAddress address(ipaddr);
    if ( !address.valid()) {
        Snmp::socket_cleanup();
        return ret;
    }
    QString o = QString("1.3.6.1.4.1.30966.12.%1").arg(num);
    Oid oid(o.toStdString().c_str());
    if ( !oid.valid()) {
        Snmp::socket_cleanup();
        return ret;
    }

    char com[255];
    memset(com,0,sizeof(char)*255);
    strncpy_s(com,sizeof(char)*255,roCom.toStdString().c_str(),sizeof(char)*255);

    snmp_version version = version2c;
    if(readVersion == 1) version = version1;
    else if(readVersion == 3) version = version3;
    int retries = 1;
    int timeout = 1000;
    u_short port = 161;
    OctetStr community(com);

    int status;
    Snmp snmp(status, 0, (address.get_ip_version() == Address::version_ipv6));
    //Snmp snmp(status, 0);

    if ( status != SNMP_CLASS_SUCCESS) {
        Snmp::socket_cleanup();
        return ret;
    }
    Pdu pdu;
    Vb vb;
    vb.set_oid(oid);
    pdu += vb;

    address.set_port(port);
#ifdef _SNMPv3
    memset(com,0,sizeof(char)*255);
    strncpy_s(com,sizeof(char)*255,v3PrivPass.toStdString().c_str(),sizeof(char)*255);
    OctetStr privPassword(com);
    memset(com,0,sizeof(char)*255);
    strncpy_s(com,sizeof(char)*255,v3AuthPass.toStdString().c_str(),sizeof(char)*255);
    OctetStr authPassword(com);
    memset(com,0,sizeof(char)*255);
    strncpy_s(com,sizeof(char)*255,name.toStdString().c_str(),sizeof(char)*255);
    OctetStr securityName(com);
    int securityModel = SNMP_SECURITY_MODEL_USM;
    int securityLevel = SNMP_SECURITY_LEVEL_AUTH_PRIV;
    OctetStr contextName("");
    OctetStr contextEngineID("");
    long authProtocol = SNMP_AUTHPROTOCOL_HMACMD5;
    long privProtocol = SNMP_PRIVPROTOCOL_DES;
    v3MP *v3_MP;
    if (version == version3) {
        const char *engineId = "snmpNext";
        const char *filename = "snmpv3_boot_counter";
        unsigned int snmpEngineBoots = 0;
        int status;
        status = getBootCounter(filename, engineId, snmpEngineBoots);
        if ((status != SNMPv3_OK) && (status < SNMPv3_FILEOPEN_ERROR)){
            qDebug() << "Error loading snmpEngineBoots counter: " << status << endl;
            return ret;
        }
        snmpEngineBoots++;
        status = saveBootCounter(filename, engineId, snmpEngineBoots);
        if (status != SNMPv3_OK){
            qDebug() << "Error saving snmpEngineBoots counter: " << status << endl;
            return ret;
        }

        int construct_status;
        v3_MP = new v3MP(engineId, snmpEngineBoots, construct_status);
        if (construct_status != SNMPv3_MP_OK){
            qDebug() << "Error initializing v3MP: " << construct_status << endl;
            return ret;
        }
        USM *usm = v3_MP->get_usm();
        usm->add_usm_user(securityName,
                          authProtocol, privProtocol,
                          authPassword, privPassword);
    }
    else{
        // MUST create a dummy v3MP object if _SNMPv3 is enabled!
        int construct_status;
        v3_MP = new v3MP("dummy", 0, construct_status);
    }
    snmp.set_mpv3(v3_MP);
#endif
    CTarget ctarget(address);
    UTarget utarget(address);
    if (version == version3) {
        utarget.set_version( version);          // set the SNMP version SNMPV1 or V2 or V3
        utarget.set_retry( retries);            // set the number of auto retries
        utarget.set_timeout( timeout);          // set timeout
        utarget.set_security_model( securityModel);
        utarget.set_security_name( securityName);
        pdu.set_security_level( securityLevel);
        pdu.set_context_name (contextName);
        pdu.set_context_engine_id(contextEngineID);
    }
    else {
        ctarget.set_version(version);
        ctarget.set_retry(retries);
        ctarget.set_timeout(timeout);
        ctarget.set_readcommunity( community);
    }

    SnmpTarget *target;

    if (version == version3){
        target = &utarget;
    }
    else target = &ctarget;
    do{
        status = snmp.get_next(pdu,*target);
        if (status == SNMP_CLASS_SUCCESS){
            for ( int z = 0 ; z < pdu.get_vb_count() ; z++) {
                pdu.get_vb(vb,z);
                Oid tmp;
                vb.get_oid(tmp);
                if (oid.nCompare(oid.len(), tmp) != 0){
                    Snmp::socket_cleanup();
                    return ret;
                }
                if ( vb.get_syntax() != sNMP_SYNTAX_ENDOFMIBVIEW){
                    QString id = vb.get_printable_oid();
                    QString id_value = vb.get_printable_value();
                    //1.3.6.1.4.1.30966.12.1
                    if(id == "1.3.6.1.4.1.30966.12.1.1.2.18.0" || id == "1.3.6.1.4.1.30966.12.1.1.2.19.0") continue;
                    praseMasterVal(id , id_value);
                    ret = true;
                    qDebug()<<id  <<"       id_value   " <<id_value;
                }
                else{
                    Snmp::socket_cleanup();
                    return ret;
                }
            }
            pdu.set_vblist(&vb, 1);
        }
    }while(status == SNMP_CLASS_SUCCESS);
    qDebug() << "after Error initializing v3MP: ";
    Snmp::socket_cleanup();
    if(v3_MP) {
        delete v3_MP;
        v3_MP = NULL;
    }
    return ret;
}

void Dev_IpSnmp::praseMasterVal(QString id , QString value)
{
    int item = getItemByOid(id , 2);
    switch(item){
    case 1: baseMsInformation(id , value); break;
    case 2: loopMsInformation(id , value); break;
    case 3: temMsInformation(id , value); break;
    default: break;
    }
}

void Dev_IpSnmp::startBoxBaseInfo(QString id , QString val)
{
    int item = getItemByOid(id , 4);
    sBoxData *t= & mBusData->box[0];
    bool ok;
    switch(item){
    case 1: break;
    case 2: {
        t->dc = 1;
        t->proNum = (val.toUInt())&0x0f;
        t->loopNum = t->data.lineNum = 3;
    }break;
    case 3: t->curSpecification = val.toUInt(&ok);break;
    case 4: t->version = val.remove(".").toUInt(&ok);break;
    case 5: t->workMode = val.toUInt(&ok);break;
    case 6: mBusData->boxNum = val.toUInt(&ok);break;
    case 7: t->online1 = val.toUInt(&ok);break;
    case 8: t->online2 = val.toUInt(&ok);break;
    case 9: t->buzzerStatus = val.toUInt(&ok);break;
    case 10: t->alarmTime = val.toUInt(&ok);break;
    case 11: t->lightning = val.toUInt(&ok);break;
    case 12: t->iOF = val.toUInt(&ok);break;
    case 13: t->isd = val.toUInt(&ok);break;
    case 14: t->shuntRelease = val.toUInt(&ok);break;
    case 15: t->reState = val.toUInt(&ok);break;
    default: break;
    }
}

void Dev_IpSnmp::startBoxEleParaInfo(QString id ,QString val)
{
    int item = getItemByOid(id , 4);
    sBoxData *t= & mBusData->box[0];
    bool ok;
    switch(item){
    case 1: t->reCur.svalue = (val.toFloat()*(short)COM_RATE_CUR);break;
    case 2: t->reCur.supalarm = val.toUInt(&ok);break;
    case 3: t->zeroLineCur.svalue = (uint)(val.toFloat()*(short)COM_RATE_CUR);break;
    case 4: t->zeroLineCur.supalarm = val.toUInt(&ok);break;
    case 5: t->totalApPow = (unsigned long long)(val.toFloat()*(short)COM_RATE_POW);break;
    case 6: t->totalPow.ivalue = (unsigned long long)(val.toFloat()*(short)COM_RATE_POW);break;
    case 7: t->totalPow.iupalarm = val.toUInt(&ok);break;
    case 8:
    {
        t->data.sw[0] = val.toUInt(&ok);
        t->data.sw[1] = val.toUInt(&ok);
        t->data.sw[2] = val.toUInt(&ok);
        break;
    }
    case 9: t->rate.svalue = val.toUInt(&ok);break;
    case 10: t->rate.supalarm = val.toUInt(&ok);break;
    case 11: t->volUnbalance = (uint)val.toUInt(&ok);break;
    case 12: t->curUnbalance = (uint)val.toUInt(&ok);break;
    case 13: t->reCur.smax = (val.toFloat()*(short)COM_RATE_CUR);break;
    case 14: t->zeroLineCur.smax = (uint)(val.toFloat()*(short)COM_RATE_CUR);break;
    case 15:
    case 16: t->totalPow.imin = (unsigned long long)(val.toFloat()*(short)COM_RATE_POW);break;
    case 17: t->totalPow.imax = (unsigned long long)(val.toFloat()*(short)COM_RATE_POW);break;
    case 18: t->rate.smin = (val.toUInt(&ok));break;
    case 19: t->rate.smax = (val.toUInt(&ok));break;
    default: break;
    }
}

void Dev_IpSnmp::startBoxNetInfo(QString id , QString val)
{
    int item = getItemByOid(id , 4);
    switch(item){
    case 1:mBusData->box[mItem->addr-1].mac = val;break;
    default:break;
    }
}

void Dev_IpSnmp::baseMsInformation(QString id , QString val)
{
    int item = getItemByOid(id , 3);
    switch(item){
    case 1:startBoxNetInfo(id , val);break;
    case 2:startBoxBaseInfo(id , val);break;
    case 3:startBoxEleParaInfo(id , val);break;
    default:break;
    }
}

void Dev_IpSnmp::parseVolHar(QString val, int line)
{
    QString str = val;
    QStringList strList = str.split("/");
    int len = strList.length();
    sThdData *t= & mBusData->thdData;
    bool ok;
    for(int i = 0 ; i < len ; i++){
        t->volThd[line][i] = strList.at(i).toInt(&ok);
    }
}

void Dev_IpSnmp::parseCurHar(QString val, int line)
{
    QString str = val;
    QStringList strList = str.split("/");
    int len = strList.length();
    sThdData *t= & mBusData->thdData;
    bool ok;
    for(int i = 0 ; i < len ; i++){
        t->curThd[line][i] = strList.at(i).toInt(&ok);
    }
}

void Dev_IpSnmp::loopMsInformation(QString id ,QString val)
{
    int line = 1;
    if( getItemByOid(id , 3) >= 1 && getItemByOid(id , 3) <= 3 )
        line = getItemByOid(id , 3) - 1;
    else return;
    int item = getItemByOid(id , 4);
    sObjectData *t= & mBusData->box[0].data;
    bool ok;
    if(t){
        switch(item){
        case 1:  t->lineVol.value[line] = val.toFloat(&ok)*(short)COM_RATE_VOL;break;
        case 2:  t->lineVol.upalarm[line] = val.toUInt(&ok);break;
        case 3:  t->vol.value[line] = val.toFloat(&ok)*(short)COM_RATE_VOL;break;
        case 4:  t->vol.upalarm[line] = val.toUInt(&ok);break;
        case 5:  t->cur.value[line] = (uint)(val.toFloat(&ok)*(short)COM_RATE_CUR);break;
        case 6:  t->cur.upalarm[line] = val.toUInt(&ok);break;
        case 7:  t->pow.value[line] = (uint)(val.toFloat(&ok)*(short)COM_RATE_POW);break;
        case 8:  t->pow.upalarm[line] = val.toUInt(&ok);break;
        case 9:  t->reactivePower[line] = (uint)(val.toFloat(&ok)*(short)COM_RATE_POW);break;
        case 10:  t->apPow[line] = (uint)(val.toFloat(&ok)*(short)COM_RATE_POW);break;
        case 11:  t->pf[line] = (val.toFloat(&ok)*(short)COM_RATE_PF);break;
        case 12:  t->ele[line] = (uint)(val.toFloat(&ok)*(short)COM_RATE_ELE);break;

        case 13: parseVolHar( val , line );break;
        case 14: parseCurHar( val , line );break;
        case 15:  t->pl[line] = (val.toFloat(&ok)*COM_RATE_PF);break;

        case 16:  t->lineVol.min[line] = (val.toFloat(&ok)*(short)COM_RATE_VOL);break;
        case 17:  t->lineVol.max[line] = (val.toFloat(&ok)*(short)COM_RATE_VOL);break;
        case 18:  t->vol.min[line] = (val.toFloat(&ok)*(short)COM_RATE_VOL);break;
        case 19:  t->vol.max[line] = (val.toFloat(&ok)*(short)COM_RATE_VOL);break;
        case 20:  t->cur.min[line] = (uint)(val.toFloat(&ok)*(short)COM_RATE_CUR);break;
        case 21:  t->cur.max[line] = (uint)(val.toFloat(&ok)*(short)COM_RATE_CUR);break;
        case 22:  t->pow.min[line] = (uint)(val.toFloat(&ok)*(short)COM_RATE_POW);break;
        case 23:  t->pow.max[line] = (uint)(val.toFloat(&ok)*(short)COM_RATE_POW);break;
        default: break;
        }
    }
}


void Dev_IpSnmp::temMsInformation(QString id ,QString val)
{
    int item = getItemByOid(id ,3);
    sDataValue *t= & mBusData->box[0].env.tem;
    bool ok;
    if(t){
        switch(item){
        case 1:  t->value[0] = val.toUInt(&ok);break;
        case 2:  t->upalarm[0] = val.toUInt(&ok);break;
        case 3:  t->value[1] = val.toUInt(&ok);break;
        case 4:  t->upalarm[1] = val.toUInt(&ok);break;
        case 5:  t->value[2] = val.toUInt(&ok);break;
        case 6:  t->upalarm[2] = val.toUInt(&ok);break;
        case 7:  t->value[3] = val.toUInt(&ok);break;
        case 8:  t->upalarm[3] = val.toUInt(&ok);break;
        case 9:  t->min[0] = val.toUInt(&ok);break;
        case 10:  t->max[0] = val.toUInt(&ok);break;
        case 11:  t->min[1] = val.toUInt(&ok);break;
        case 12:  t->max[1] = val.toUInt(&ok);break;
        case 13:  t->min[2] = val.toUInt(&ok);break;
        case 14:  t->max[2] = val.toUInt(&ok);break;
        case 15:  t->min[3] = val.toUInt(&ok);break;
        case 16:  t->max[3] = val.toUInt(&ok);break;
        default: break;
        }
    }
}

bool Dev_IpSnmp::devDataV1()
{
    bool ret; QStringList oids;
    for(int k=1; k<12; k++)  oids << IP_MIB_OID + QString(".%1").arg(k);
    QtSnmpDataList values = mSnmp->requestValue(oids);

    for(const auto& value : values) {
        sObjData *obj = &(mDev->line);
        int res = value.data().toHex().toInt(&ret,16);
        int id = getItemByOid(value.address(), 1);
        switch (id%12) {
        case 1: obj->vol.value[0] = res; break;
        case 2: obj->vol.value[1] = res; break;
        case 3: obj->vol.value[2] = res; break;

        case 4: obj->cur.value[0] = res; break;
        case 5: obj->cur.value[1] = res; break;
        case 6: obj->cur.value[2] = res; break;

        case 7: obj->ele[0] = res; break;
        case 8: obj->ele[1] = res; break;
        case 9: obj->ele[2] = res; break;

        case 10: mDev->env.tem.value[0] = res; break;
//        case 11: mDev->env.hum.value[0] = res; break;
        default:         break;
        }
    }

    return values.size();
}

bool Dev_IpSnmp::readPduData()
{
    bool ret = true;
    qDebug()<<"devDataV3";
    ret = devDataV3();
    return ret;
}

// determine the smi type and get a value from the user
bool Dev_IpSnmp::determine_vb(SmiUINT32 val, Vb &vb, QString str)
{
    char buffer[255];
    if (val == sNMP_SYNTAX_NOSUCHINSTANCE) {
        vb.set_syntax(val);
    }

    switch (val) {
    // octet string
    case sNMP_SYNTAX_OCTETS:
    {
        qDebug() << "Octet String\n";
        qDebug() << "Please enter new value: ";
        //cin >> buffer;
        memset(buffer,0,sizeof(char)*255);
        strncpy_s(buffer,sizeof(char)*255,str.toStdString().c_str(),sizeof(char)*250);
        OctetStr octetstr(buffer);
        if (octetstr.valid()) {
            vb.set_value(octetstr);
            return true;
        }
        else {
            qDebug() << "Invalid OctetStr\n";
            return false;
        }
    }
        // IP Address
    case sNMP_SYNTAX_IPADDR:
    {
        qDebug() << "IP Address\n";
        qDebug() << "Please enter new value: ";
        //cin >> buffer;
        IpAddress ipaddress(buffer);
        if (ipaddress.valid()) {
            vb.set_value(ipaddress);
            return true;
        }
        else {
            qDebug() << "Invalid IP Address\n";
            return false;
        }
    }
        // Oid
    case sNMP_SYNTAX_OID:
    {
        qDebug() << "Oid\n";
        qDebug() << "Please enter new value: ";
        //cin >> buffer;
        Oid oid(buffer);
        if (oid.valid()) {
            vb.set_value(oid);
            return true;
        }
        else {
            qDebug() << "Invalid Oid\n";
            return false;
        }
    }
        // TimeTicks
    case sNMP_SYNTAX_TIMETICKS:
    {
        qDebug() << "TimeTicks\n";
        qDebug() << "Please enter new value: ";
        //cin >> buffer;
        unsigned long i;
        i = atol(buffer);
        TimeTicks timeticks(i);
        if (timeticks.valid()) {
            vb.set_value(timeticks);
            return true;
        }
        else {
            qDebug() << "Invalid TimeTicks\n";
            return false;
        }
    }
        // Gauge32
    case sNMP_SYNTAX_GAUGE32:
    {
        qDebug() << "Gauge32\n";
        qDebug() << "Please enter new value: ";
        //cin >> buffer;
        unsigned long i;
        i = atol(buffer);
        Gauge32 gauge32(i);
        if (gauge32.valid()) {
            vb.set_value(gauge32);
            return true;
        }
        else {
            qDebug() << "Invalid Gauge32\n";
            return false;
        }
    }
    case sNMP_SYNTAX_CNTR32:
    {
        qDebug() << "Counter32\n";
        qDebug() << "Please enter new value: ";
        //cin >> buffer;
        unsigned long i;
        i = atol(buffer);
        Counter32 counter32(i);
        if (counter32.valid()) {
            vb.set_value(counter32);
            return true;
        }
        else {
            qDebug() << "Invalid Counter32\n";
            return false;
        }
    }
    case sNMP_SYNTAX_CNTR64:
    {
        qDebug() << "Counter64\n";
        qDebug() << "Please enter value (low 32 bit): ";
        //cin >> buffer;
        unsigned long i;
        i = atol(buffer);
        Counter64 counter64;
        counter64.set_low(i);

        qDebug() << "Please enter value (high 32 bit): ";
        //cin >> buffer;
        i = atol(buffer);
        counter64.set_high(i);

        if (counter64.valid()) {
            vb.set_value(counter64);
            return true;
        }
        else {
            qDebug() << "Invalid Counter64\n";
            return false;
        }
    }
    case sNMP_SYNTAX_INT:
    {
        qDebug() << "Integer\n";
        qDebug() << "Please enter new value: ";
        //cin >> buffer;
        memset(buffer,0,sizeof(char)*255);
        strncpy_s(buffer,sizeof(char)*255,str.toStdString().c_str(),sizeof(char)*255);
        unsigned long i;
        i = atol(buffer);
        long l ;
        l = (long) i;
        vb.set_value(l);
        return true;
    }
    case sNMP_SYNTAX_NOSUCHOBJECT:
    {
        //        cout << "NO SUCH OBJECT\n";
        //        cout << "Object cannot be created.\n";
        //        return false;
        qDebug() << "Integer\n";
        qDebug() << "Please enter new value: ";
        //cin >> buffer;
        memset(buffer,0,sizeof(char)*255);
        strncpy_s(buffer,sizeof(char)*255,str.toStdString().c_str(),sizeof(char)*255);
        unsigned long i;
        i = atol(buffer);
        long l ;
        l = (long) i;
        vb.set_value(l);
        return true;
    }
    default:
        qDebug() << "Unknown Data Type " << val << "\n";
        return false;
    }
}

bool Dev_IpSnmp::SetInfo(QString o , QString val)
{
    bool ret = false;
    QString ipAddr = "192.168.1.163";
    QString rwCom = "private";
    QString roCom = "public";
    QString v3PrivPass = "privatekey1";
    QString v3AuthPass = "authkey123";
    QString name = "Busbar";

    int setVersion = version3;
    Snmp::socket_startup();  // Initialize socket subsystem
    //---------[ make a GenAddress and Oid object to retrieve ]---------------
    UdpAddress address(ipAddr.toStdString().c_str());      // make a SNMP++ Generic address

    Oid oid(o.toStdString().c_str());      // default is sysName

    //---------[ determine options to use ]-----------------------------------
    snmp_version version=version2c;                 // default is v1
    if(setVersion == 1) version = version1;
    else if(setVersion == 3) version = version3;
    int retries=1;                                  // default retries is 1
    int timeout=2000;                                // default is 1 second
    u_short port=161;                               // default snmp port is 161
    OctetStr community(rwCom.toStdString().c_str());                   // community name
    OctetStr get_community;

    if (get_community.len() == 0)
        get_community = community;

    //----------[ create a SNMP++ session ]-----------------------------------
    int status;
    // bind to any port and use IPv6 if needed
    Snmp snmp(status, 0, (address.get_ip_version() == Address::version_ipv6));


    //--------[ build up SNMP++ object needed ]-------------------------------
    Pdu pdu;                               // construct a Pdu object
    Vb vb;                                 // construct a Vb object
    vb.set_oid(oid);                      // set the Oid portion of the Vb
    pdu += vb;                             // add the vb to the Pdu

    address.set_port(port);

    OctetStr privPassword(v3PrivPass.toStdString().c_str());
    OctetStr authPassword(v3AuthPass.toStdString().c_str());
    OctetStr securityName(name.toStdString().c_str());
    int securityModel = SNMP_SECURITY_MODEL_USM;
    int securityLevel = SNMP_SECURITY_LEVEL_AUTH_PRIV;
    OctetStr contextName("");
    OctetStr contextEngineID("");
    long authProtocol = SNMP_AUTHPROTOCOL_HMACMD5;
    long privProtocol = SNMP_PRIVPROTOCOL_DES;
    v3MP *v3_MP;
    if (version == version3) {
        const char *engineId = "snmpSet";
        const char *filename = "snmpv3_boot_counter";
        unsigned int snmpEngineBoots = 0;
        int status;
        status = getBootCounter(filename, engineId, snmpEngineBoots);
        if ((status != SNMPv3_OK) && (status < SNMPv3_FILEOPEN_ERROR)){
            qDebug() << "Error loading snmpEngineBoots counter: " << status << endl;
            return ret;
        }
        snmpEngineBoots++;
        status = saveBootCounter(filename, engineId, snmpEngineBoots);
        if (status != SNMPv3_OK){
            qDebug() << "Error saving snmpEngineBoots counter: " << status << endl;
            return ret;
        }
        int construct_status;
        v3_MP = new v3MP(engineId, snmpEngineBoots, construct_status);
        if (construct_status != SNMPv3_MP_OK){
            qDebug() << "Error initializing v3MP: " << construct_status << endl;
            return ret;
        }
        USM *usm = v3_MP->get_usm();
        usm->add_usm_user(securityName,
                          authProtocol, privProtocol,
                          authPassword, privPassword);
    }
    else{
        // MUST create a dummy v3MP object if _SNMPv3 is enabled!
        int construct_status;
        v3_MP = new v3MP("dummy", 0, construct_status);
    }
    snmp.set_mpv3(v3_MP);
    CTarget ctarget(address);
    UTarget utarget(address);
    if (version == version3) {
        utarget.set_version( version);          // set the SNMP version SNMPV1 or V2 or V3
        utarget.set_retry( retries);            // set the number of auto retries
        utarget.set_timeout( timeout);          // set timeout
        utarget.set_security_model( securityModel);
        utarget.set_security_name( securityName);
        pdu.set_security_level( securityLevel);
        pdu.set_context_name (contextName);
        pdu.set_context_engine_id(contextEngineID);
    }
    else {
        ctarget.set_version(version);
        ctarget.set_retry(retries);
        ctarget.set_timeout(timeout);
        ctarget.set_readcommunity(get_community); // set the read community name
        ctarget.set_writecommunity(community);// set the write community name
    }

    SnmpTarget *target;

    if (version == version3){
        target = &utarget;
    }
    else
        target = &ctarget;
    // first get the variabel to determine its type
    if ((status = snmp.get(pdu,*target))== SNMP_CLASS_SUCCESS)
    {
        int ret = pdu.get_vb(vb,0);
        qDebug() << "Oid = " << vb.get_printable_oid() << endl
                 << "Current Value = " << vb.get_printable_value() << endl
                 <<ret<<endl;

        if (determine_vb(vb.get_syntax(), vb , val)) {
            //if (determine_vb(sNMP_SYNTAX_INT, vb , val)) {
            // do the Set
            Pdu setpdu;
            setpdu.set_security_level( securityLevel);
            setpdu.set_context_name (contextName);
            setpdu.set_context_engine_id(contextEngineID);
            vb.set_oid(oid);           // use the same oid as the inquire
            setpdu += vb;
            status = snmp.set(setpdu, *target);
            qDebug() << "Set Status = " << snmp.error_msg(status) << "\n";
            if(SNMP_CLASS_SUCCESS == status) ret = true;
        }
    }
    else qDebug() << "SNMP++ Set Error, " << snmp.error_msg(status) << "\n";

    Snmp::socket_cleanup();  // Shut down socket subsystem
    if(v3_MP) {
        delete v3_MP;
        v3_MP = NULL;
    }
    return ret;
}

