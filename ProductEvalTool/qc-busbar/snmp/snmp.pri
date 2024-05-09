
INCLUDEPATH += $$PWD



HEADERS += \
    $$PWD/des.h \
    $$PWD/des_locl.h \
    $$PWD/des_ver.h \
    $$PWD/libsnmp.h \
    $$PWD/podd.h \
    $$PWD/sk.h \
    $$PWD/snmp_pp/IPv6Utility.h \
    $$PWD/snmp_pp/address.h \
    $$PWD/snmp_pp/asn1.h \
    $$PWD/snmp_pp/auth_priv.h \
    $$PWD/snmp_pp/collect.h \
    $$PWD/snmp_pp/config_snmp_pp.h \
    $$PWD/snmp_pp/counter.h \
    $$PWD/snmp_pp/ctr64.h \
    $$PWD/snmp_pp/eventlist.h \
    $$PWD/snmp_pp/eventlistholder.h \
    $$PWD/snmp_pp/gauge.h \
    $$PWD/snmp_pp/idea.h \
    $$PWD/snmp_pp/integer.h \
    $$PWD/snmp_pp/log.h \
    $$PWD/snmp_pp/md5.h \
    $$PWD/snmp_pp/mp_v3.h \
    $$PWD/snmp_pp/msec.h \
    $$PWD/snmp_pp/msgqueue.h \
    $$PWD/snmp_pp/notifyqueue.h \
    $$PWD/snmp_pp/octet.h \
    $$PWD/snmp_pp/oid.h \
    $$PWD/snmp_pp/oid_def.h \
    $$PWD/snmp_pp/pdu.h \
    $$PWD/snmp_pp/reentrant.h \
    $$PWD/snmp_pp/sha.h \
    $$PWD/snmp_pp/smi.h \
    $$PWD/snmp_pp/smival.h \
    $$PWD/snmp_pp/snmp_pp.h \
    $$PWD/snmp_pp/snmperrs.h \
    $$PWD/snmp_pp/snmpmsg.h \
    $$PWD/snmp_pp/target.h \
    $$PWD/snmp_pp/timetick.h \
    $$PWD/snmp_pp/userdefined.h \
    $$PWD/snmp_pp/usertimeout.h \
    $$PWD/snmp_pp/usm_v3.h \
    $$PWD/snmp_pp/uxsnmp.h \
    $$PWD/snmp_pp/v3.h \
    $$PWD/snmp_pp/vb.h

	
SOURCES +=\
    $$PWD/cbc_enc.c \
    $$PWD/des_enc.c \
    $$PWD/ecb_enc.c \
    $$PWD/ede_enc.c \
    $$PWD/fcrypt.c \
    $$PWD/ncbc_enc.c \
    $$PWD/set_key.c


FORMS +=

win32: LIBS += -L$$PWD/./ -lsnmpdev

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/./snmpdev.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/./libsnmpdev.a

LIBS += -lws2_32


