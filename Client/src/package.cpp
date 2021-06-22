#include "package.h"

#include <QTextStream>


Package::Package()
{
}

Package::Package(const QVariant& data)
    : Package()
{
    QTextStream(stdout) << "m_data set to " << data.toString();
    m_data = data;
    m_rawData.clear();
    m_rawData = m_data.toByteArray();
}

QVariant Package::data() const
{
    return m_data;
}

QByteArray Package::rawData() const
{
    return m_rawData;
}

QDataStream &operator>>(QDataStream &stream, Package &package)
{
    QByteArray data;
    stream >> data;
    QTextStream(stdout) << "data is null - " << data.isNull() << '\n';
    package.rawData() = data;

    package.data() = QVariant(package.rawData());

    QTextStream(stdout) << "package data is now: " << package.data().toString() << '\n';

    return stream;
}
