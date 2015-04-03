#ifndef PARSER_H
#define PARSER_H

#include <QObject>
#include <QHash>

class Parser : public QObject
{
    Q_OBJECT
public:
    explicit Parser(QObject *parent = 0);
    ~Parser();
    bool parseXML(QByteArray ba);
    QList<QMap<QString, QString> > parseXMLtoMaps(QByteArray ba);

    QHash<QString, QString> results() const;
    void setResults(const QHash<QString, QString> &results);

signals:
    void xmlParsed();

public slots:
    void parseUpnpReply();
    void parseAnswer(QByteArray ba);

private:
    QHash<QString, QString> m_results;
};

#endif // PARSER_H
