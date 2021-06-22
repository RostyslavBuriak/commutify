#include "connection.h"

#include <QTextStream>
#include <QtGlobal>
#include <QTimer>


Connection::Connection(QString host, qint16 port)
    : m_host(host), m_port(port)
{
    connectSocketSignals();
    connectToServer();
}

int Connection::sendLogin(const QByteArray &data) {
    int r = m_serverSocket.write(data);

    bool wait = m_serverSocket.waitForReadyRead(3000);
    if (!wait) {
        emit loginTimedOut();
    }

    return r;
}

int Connection::sendMessage(const QByteArray &data)
{
    int r = m_serverSocket.write(data);
    return r;
}

void Connection::onSocketError(QAbstractSocket::SocketError socketError)
{
    QTextStream(stderr) << "Socket error: " << socketError << '\n';

    QTimer::singleShot(reconnectionTime(), this, &Connection::connectToServer);

    emit reconnectionTimerStarted();
}

void Connection::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{
    QTextStream(stdout) << "Socket state changed: " << socketState << '\n';

    switch (socketState) {
    case QAbstractSocket::SocketState::BoundState:
    case QAbstractSocket::SocketState::ConnectedState:
    {
        QTextStream(stdout) << "connection established\n";
        m_state = net::connectionState::Connected;
        break;
    }
    case QAbstractSocket::SocketState::ClosingState:
    case QAbstractSocket::SocketState::UnconnectedState:
    {
        m_state = net::connectionState::Disconnected;
        break;
    }
    default:
    {
        m_state = net::connectionState::Connecting;
    }
    }

    emit connectionStateChanged(m_state);
}

void Connection::onDataReceived()
{
    QTextStream(stdout) << "ON DATA REECEIVED HANDLER\n";
    QByteArray inputData;
    inputData = m_serverSocket.readAll();

    QString dataText = inputData;

    QTextStream(stdout) << "INPUT DATA:\n" << dataText.section(' ', 0, 0) << "\nEND INPUT DATA\n";

    int msg_length = dataText.section(' ', 0, 0).toInt();
    dataText = dataText.section(' ', 1);


    QTextStream(stdout) << "dt len: " << dataText.length() << " msg len: " << msg_length << '\n';

    if (dataText.length() > msg_length) {
        dataText.resize(msg_length);
    }

    QTextStream(stdout) << "INPUT DATA:\n" << dataText << "\nEND INPUT DATA\n";

    if (dataText.startsWith("SERVER OK")) {
        QTextStream(stdout) << "SERVER OK EMITTING RESP\n";
        emit loginResponseReceived(dataText);
    } else if (dataText.startsWith("SERVERSYNC")) {
        emit messageReceived(dataText);
    }
    QTextStream(stdout) << "ON DATA RECEIVED FINISHED";
}

void Connection::connectToServer()
{
    QTextStream(stdout) << "Connecting to server at " << m_host << " port " << m_port << '\n';

    m_serverSocket.connectToHost(m_host, m_port);
}

void Connection::disconnectFromServer()
{
    m_serverSocket.disconnectFromHost();

    QTextStream(stdout) << "Disconnected from server\n";
}

void Connection::connectSocketSignals()
{
    connect(&m_serverSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this, &Connection::onSocketError);
    connect(&m_serverSocket, &QAbstractSocket::stateChanged, this, &Connection::onSocketStateChanged);
    connect(&m_serverSocket, &QAbstractSocket::readyRead, this, &Connection::onDataReceived);
}
