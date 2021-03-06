#ifndef SIMPLESERVER_H
#define SIMPLESERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QUrl>
#include <QDebug>

#include "database.h"

class SimpleServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit SimpleServer(QObject *parent = 0);
    // all methods are descripted in .cpp file
    void incomingConnection(qintptr handle);

public slots:
    void onReadyRead();
    void onDisconnected();

private:
    // our little "data base" which is stored in RAM
    dataBase db;

    void apiHandler(QMap<QString, QString> *data, QTcpSocket *socket);

    void formRequestBodyMap(QMap<QString, QString> *data, QByteArray body);
    QMap<QString, QString> getArgsFromString(QString str);

    void doResponse(int code, QString text, QTcpSocket *socket);
};

#endif // SIMPLESERVER_H
