#ifndef AD_MACADDR_H
#define AD_MACADDR_H
#include <QtCore>

class MacAddr
{
    MacAddr();
public:
    static MacAddr *bulid();
    bool isMacAddress(const QString &mac);

    QString macAdd(const QString &in, int step);
    int macToChar(const QString &in, uchar *buf);

protected:
    QString intToMac(quint64 v);
    int intToChar(quint64 v, uchar *buf);
    quint64 macToInt(const QString &in);
    QByteArray toLittleEndian(quint64 v);
    quint64 fromBigEndian(const QByteArray &bytes);
    QByteArray get_mac_array_from_QString(const QString &in);
};

#endif // AD_MACADDR_H
