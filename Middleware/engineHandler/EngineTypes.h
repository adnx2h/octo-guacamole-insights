#ifndef ENGINETYPES_H
#define ENGINETYPES_H

//## dependency QObject - Not needed
#include <QMap>
#include <QStringList>
#include <QString>
//class EngyneTypes  - You don't need a class

// Structure to hold the parsed PGN data
namespace EngineTypes{
struct PgnData {
    QMap<QString, QString> tags;
    QStringList moves;
    QString result;
};
}
//#endif // ENGINETYPES_H  - Wrong endif
#endif
