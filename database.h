#ifndef DATABASE_H
#define DATABASE_H

#include <QMap>

class dataBase
{
public:
    dataBase();

    QString getById(int id);
    int add(QString value);
    QString edit(int id, QString value);
    bool deleteById(int id);

    QString formHtmlData();

private:
    QMap<int, QString> db;
    int last_id = 0;
};

#endif // DATABASE_H
