#pragma once

#include <QByteArray>
#include <QDataStream>
#include <QVariant>


class Package
{
public:
    Package();
    Package(const QVariant& data);


    QVariant data() const;
    QByteArray rawData() const;

    friend QDataStream &operator>>(QDataStream &stream, Package &package);
private:
    QByteArray m_rawData = nullptr;

    QVariant m_data;

};


