#include "database.h"

dataBase::dataBase() {}

QString dataBase::getById(int id){
    if (db.contains(id))
        return db[id];
    else
        return "";
}

void dataBase::add(QString value){
    db[last_id++] = value;
}

void dataBase::deleteById(int id){
    db.remove(id);
}

QString dataBase::formHtmlData(){
    QString temp = "";

    for (int it : db.keys())
        temp += QString::number(it) + "\t" + db[it] + "<br>";

    return temp;
}

