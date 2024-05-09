
INCLUDEPATH += $$PWD

include(json/json.pri)
include(excel/excel.pri)
include(dbcom/dbcom.pri)
include(sqlcom/sqlcom.pri)
include(logcom/logcom.pri)
include(cfgcom/cfgcom.pri)
include(msgcom/msgcom.pri)
include(tabcom/tabcom.pri)
include(modbus/modbus.pri)
include(RK9800/RK9800.pri)
include(network/network.pri)
include(websocket/websocket.pri)
include(smtpclient/smtpclient.pri)
include(snmpclient/snmpclient.pri)
include(datapacket/datapacket.pri)
include(serialport/serialport.pri)
include(backcolour/backcolour.pri)
include(qtsingleapplication/qtsingleapplication.pri)
include(printer/printer.pri)
include(http/http.pri)

HEADERS += \	
    $$PWD/configbase.h \
    $$PWD/sysconfigfile.h \
    $$PWD/common.h
	
SOURCES += \
    $$PWD/configbase.cpp \
    $$PWD/sysconfigfile.cpp \
    $$PWD/common.cpp

FORMS += \




