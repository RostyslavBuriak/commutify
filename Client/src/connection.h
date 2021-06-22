#pragma once

#include <QObject>
#include <QString>
#include <QTcpSocket>

#include "networktypes.h"
#include "package.h"


class Connection : public QObject
{
    Q_OBJECT
public:
    Connection(QString, qint16);

    constexpr static std::chrono::milliseconds reconnectionTime() {
        return std::chrono::seconds {5};
    }

    int sendLogin(const QByteArray &);
    int sendMessage(const QByteArray &);
    void disconnectFromServer();

signals:
    void serverResponse(QString data);
    void connectionStateChanged(net::connectionState state);
    void reconnectionTimerStarted();
    void loginResponseReceived(QString &);
    void loginTimedOut();
    void messageReceived(QString &);

private slots:
    void onSocketError(QAbstractSocket::SocketError socketError);
    void onSocketStateChanged(QAbstractSocket::SocketState socketState);
    void onDataReceived();

private:
    void connectToServer();
    void connectSocketSignals();

    QString m_host;
    qint16 m_port;
    net::connectionState m_state;
    QTcpSocket m_serverSocket;

};


