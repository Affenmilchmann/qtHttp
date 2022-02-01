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
    db.add("ive changed my mind spiderman is superior");
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

    if (data["Url"].startsWith("/test")) {
        doResponse(200, db.formHtmlData(), socket);
    }
    else if (data["Url"].startsWith("/api")) {
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

bool idValidation(QString id) {
    int int_id = id.toInt();

    if (id != "0" && int_id == 0)
        return false;

    return true;
}

void SimpleServer::apiHandler(QMap<QString, QString> *data, QTcpSocket *socket) {
    QMap<QString, QString> body_args = getArgsFromString(data->value("Body"));
    QMap<QString, QString> url_args = getArgsFromString(data->value("Url"));

    QString method = data->value("Method");
    if (method == "POST") {
        if (!body_args.contains("val")) {
            doResponse(400, "Missing args", socket);
            return;
        }
        doResponse(200, QString::number(db.add(body_args["val"])), socket);
    }
    else if (method == "GET") {
        if (!url_args.contains("id")) {
            doResponse(400, "Missing args", socket);
            return;
        }

        if (idValidation(url_args["id"])) {
            QString resp = db.getById(url_args["id"].toInt());
            if (resp == "") doResponse(400, "Invalid id", socket);
            else doResponse(200, resp, socket);
        }
        else
            doResponse(400, "Invalid id", socket);
    }
    else if (method == "PUT") {
        if (!body_args.contains("id") || !body_args.contains("val")) {
            doResponse(400, "Missing args", socket);
            return;
        }

        if (idValidation(body_args["id"])) {
            QString resp = db.edit(body_args["id"].toInt(), body_args["val"]);
            if (resp == "") doResponse(400, "Invalid id", socket);
            else doResponse(200, resp, socket);
        }
        else
            doResponse(400, "Invalid id", socket);
    }
    else if (method == "DELETE") {
        if (!body_args.contains("id")) {
            doResponse(400, "Missing args", socket);
            return;
        }

        if (idValidation(body_args["id"])) {
            if (db.deleteById(body_args["id"].toInt()))
                doResponse(200, "deleted", socket);
            else
                doResponse(400, "Invalid id", socket);
        }
        else
            doResponse(400, "Invalid id", socket);
    }
    else
        doResponse(400, "Invalid method", socket);
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

QMap<QString, QString> SimpleServer::getArgsFromString(QString str) {
    int start_idx = str.indexOf('?');
    if (start_idx != -1)
        str = str.mid(start_idx + 1);

    QStringList var_list = str.split("&");
    QMap<QString, QString> vars;

    if (str == "" || str.startsWith("/")) return vars;

    qDebug() << "Body variables:";
    for (QString it : var_list) {
        QStringList temp = it.split("=");
        vars[temp[0]] = temp[1];
        qDebug() << temp[0] << " : " << temp[1];
    }

    return vars;
}

void SimpleServer::doResponse(int code, QString text, QTcpSocket *socket) {
    QString response = QString("HTTP/1.1 %1 \r\n\r\n<html><body>%2</body></html>").arg(QString::number(code)).arg(text);

    socket->write(response.toUtf8());
}
