#include "simpleserver.h"

SimpleServer::SimpleServer(QObject *parent) : QTcpServer(parent)
{
    if (listen(QHostAddress::Any, 1234))
        qDebug() << "Server started";
    else
        qDebug() << "Error while starting: " << errorString();

    // dummy data
    db.add("first test value");
    db.add("second test value");
    db.add("batman is cool");
    db.add("ive changed my mind spiderman is superiour");
}

void SimpleServer::incomingConnection(qintptr handle) {
    qDebug() << "[New connection]";

    QTcpSocket *socket = new QTcpSocket();
    socket->setSocketDescriptor(handle);

    connect(socket, &QTcpSocket::readyRead, this, &SimpleServer::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &SimpleServer::onDisconnected);
}

void SimpleServer::onReadyRead() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    QString response;

    QMap<QString, QString> data;
    formRequestBodyMap(&data, socket->readAll());

    if (data["Url"] == "/test") {
        doResponse(200, db.formHtmlData(), socket);
    }
    else if (data["Url"] == "/api") {
        apiHandler(&data, socket);
    }
    else {
        doResponse(404, "404 not found", socket);
    }

    socket->disconnectFromHost();
}

void SimpleServer::onDisconnected() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    socket->close();
    socket->deleteLater();
}

void SimpleServer::apiHandler(QMap<QString, QString> *data, QTcpSocket *socket) {
    if (data->value("Body") == "") {
        doResponse(400, "Missing args", socket);
        return;
    }
    doResponse(200, "Not ready yet", socket);

    QStringList var_list = data->value("Body").split("&");
    QMap<QString, QString> vars;

    qDebug() << "Body variables:";
    for (QString it : var_list) {
        QStringList temp = it.split("=");
        vars[temp[0]] = temp[1];
        qDebug() << temp[0] << " : " << temp[1];
    }

    QString method = data->value("Method");
    if (method == "GET") {
        if (!vars.contains("id")) {
            doResponse(400, "Missing args", socket);
            return;
        }
        doResponse(200, db.getById(vars[]), socket);
    }
}

void SimpleServer::formRequestBodyMap(QMap<QString, QString> *data, QByteArray body) {
    QStringList temp_list = QString(body).split("\n");

    qDebug() << "\n***\n" << body << "\n***\n";

    if (temp_list[0].startsWith("GET") ||
        temp_list[0].startsWith("POST") ||
        temp_list[0].startsWith("PUT") ||
        temp_list[0].startsWith("DELETE")) {
        QStringList get_url_temp = temp_list[0].split(" ");

        data->insert("Method", get_url_temp[0]);
        data->insert("Url", get_url_temp[1]);
        data->insert("Body", temp_list[2]);
    }
    else
        qDebug() << "[Error] Invalid method";
}

void SimpleServer::doResponse(int code, QString text, QTcpSocket *socket) {
    QString response = QString("HTTP/1.1 %1 \r\n\r\n<html><body>%2</body></html>").arg(QString::number(code)).arg(text);

    socket->write(response.toUtf8());
}
