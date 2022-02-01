#include "database.h"

dataBase::dataBase() {}

QString dataBase::getById(int id){
    if (db.contains(id))
        return db[id];
    else
        return "";
}

int dataBase::add(QString value){
    db[last_id++] = value;

    history += "item with id " + QString::number(last_id - 1) + " has been added<br>";

    return last_id - 1;
}

QString dataBase::edit(int id, QString value) {
    if (db.contains(id)) {
        db[id] = value;

        history += "item with id " + QString::number(last_id - 1) + " has been changed<br>";

        return value;
    }
    else
        return "";
}

bool dataBase::deleteById(int id){
    if (!db.contains(id)) return false;

    db.remove(id);
    history += "item with id " + QString::number(last_id - 1) + " has been deleted<br>";

    return true;
}

QString dataBase::formHtmlData(){
    QString temp = "<table border=1>";

    temp += "<tr><td>id</td><td>data</td></tr>";
    for (int it : db.keys())
        temp += "<tr><td>" + QString::number(it) + "</td><td>" + db[it] + "</td></tr>";

    temp += "</table>";

    return temp + "<br><hr>" + history;
}

