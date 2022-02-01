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
    return last_id - 1;
}

QString dataBase::edit(int id, QString value) {
    if (db.contains(id)) {
        db[id] = value;
        return value;
    }
    else
        return "";
}

bool dataBase::deleteById(int id){
    if (!db.contains(id)) return false;

    db.remove(id);

    return true;
}

QString dataBase::formHtmlData(){
    QString temp = "";

    for (int it : db.keys())
        temp += QString::number(it) + "\t" + db[it] + "<br>";

    return temp;
}

