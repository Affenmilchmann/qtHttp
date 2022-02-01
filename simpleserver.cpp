#include "simpleserver.h"

SimpleServer::SimpleServer(QObject *parent) : QTcpServer(parent)
{
    // starting the server on port 1234
    if (listen(QHostAddress::Any, 1234))
        qDebug() << "Server started";
    else
        qDebug() << "Error while starting: " << errorString();

    // dummy data filling in db
    db.add("first test value");
    db.add("second test value");
    db.add("batman is cool");
    db.add("ive changed my mind spiderman is superior");
}
// overriding incomingConnection so it's easier to handle requests
// incomingConnection calls on every incoming connection
void SimpleServer::incomingConnection(qintptr handle) {
    qDebug() << "[New connection]";
    // creating now socket to recieve connection there
    QTcpSocket *socket = new QTcpSocket();
    // linking our socket to native socket
    socket->setSocketDescriptor(handle);
    // linking signals with our methods
    connect(socket, &QTcpSocket::readyRead, this, &SimpleServer::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &SimpleServer::onDisconnected);
}
// this one is called on every eradyRead signal
void SimpleServer::onReadyRead() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    QString response;

    QMap<QString, QString> data;
    // filling QMap with data we need from request
    formRequestBodyMap(&data, socket->readAll());

    if (data["Url"].startsWith("/test")) {
        doResponse(200, db.formHtmlData(), socket);
    }
    else if (data["Url"].startsWith("/api")) {
        // all api handling is here
        apiHandler(&data, socket);
    }
    else {
        doResponse(404, "<html>404 not found</html>", socket);
    }

    socket->disconnectFromHost();
}
// closing connection and deleting socket
void SimpleServer::onDisconnected() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    socket->close();
    socket->deleteLater();
}
// making sure that our id is a number
bool idValidation(QString id) {
    int int_id = id.toInt();

    if (id != "0" && int_id == 0)
        return false;

    return true;
}
// all api handling
void SimpleServer::apiHandler(QMap<QString, QString> *data, QTcpSocket *socket) {
    QMap<QString, QString> body_args = getArgsFromString(data->value("Body"));
    QMap<QString, QString> url_args = getArgsFromString(data->value("Url"));

    // this if else tree is pretty straightforward
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
// making our request easier to use with only needed data
void SimpleServer::formRequestBodyMap(QMap<QString, QString> *data, QByteArray body) {
    QStringList temp_list = QString(body).split("\n");

    qDebug() << "\n***\n" << body << "\n***\n";

    if (temp_list[0].startsWith("GET") ||
        temp_list[0].startsWith("POST") ||
        temp_list[0].startsWith("PUT") ||
        temp_list[0].startsWith("DELETE")) {
        QStringList get_url_temp = temp_list[0].split(" ");

        // format is "<Method> <Url> <HTTP version>\r\n\r\n<Body>"
        data->insert("Method", get_url_temp[0]);
        data->insert("Url", get_url_temp[1]);
        data->insert("Body", temp_list[2]);
    }
    else
        qDebug() << "[Error] Invalid method";
}
// retrieving args from given string into QMap
// it could be url string like "/api?id=1&val=jeb_" then it will return map {"id":"1", "val":"jeb_"}
// or it could be body string like "id=1&val=jeb_" then it will also return map {"id":"1", "val":"jeb_"}
QMap<QString, QString> SimpleServer::getArgsFromString(QString str) {
    // looking for '?'  in order to cut url path
    int start_idx = str.indexOf('?');
    if (start_idx != -1)
        str = str.mid(start_idx + 1);
    // splitting into string list so example from above will become ["id=1", "val=jeb_"]
    QStringList var_list = str.split("&");
    // output map
    QMap<QString, QString> vars;
    // if args string is empty OR if it was empty url path like "/api" so it wasnt cut off bc there is no '?' char
    if (str == "" || str.startsWith("/")) return vars;

    qDebug() << "Body variables:";
    for (QString it : var_list) {
        // and here we are splitting each list element with '='
        QStringList temp = it.split("=");
        // and decoding it from format "jeb_%20is%20cool" to regulat string "jeb_ is cool"
        temp[0] = QUrl::fromPercentEncoding(temp[0].toUtf8());
        temp[1] = QUrl::fromPercentEncoding(temp[1].toUtf8());

        vars[temp[0]] = temp[1];
        qDebug() << temp[0] << " : " << temp[1];
    }

    return vars;
}

void SimpleServer::doResponse(int code, QString text, QTcpSocket *socket) {
    QString response = QString("HTTP/1.1 %1 \r\n\r\n%2").arg(QString::number(code)).arg(text);

    socket->write(response.toUtf8());
}
