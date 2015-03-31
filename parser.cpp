#include "parser.h"

#include <QFile>
#include <QDebug>
#include <QStringList>

Parser::Parser(QObject *parent) : QObject(parent)
{

}

Parser::~Parser()
{

}

void Parser::parseUpnpReply()
{
    //TODO delete
    QFile *answer = new QFile("/home/simon/code/QtHttp/massiveAnswer");
    if (!answer->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Loading File Problem";
        exit(-1);
    }
    QByteArray ba = answer->readAll();
    QString s(ba);
    s.replace(QString("&quot"), QString("\""));
    s.replace(QString("&lt"), QString("<"));
    s.replace(QString("&gt"), QString(">"));
    s.remove(';');
    qDebug() << s;
//    QStringList strings = s.split("<");
//    foreach(QString s, strings)
//    {
//        qDebug() << s;
//    }

}
