#ifndef GGRAPHPARSER_H
#define GGRAPHPARSER_H
#include<GGRaph/Base/ggraph.h>
#include<GGraph/Base/ggraphdrawing.h>
#include<QVariant>



class gGraphParser
{
public:
    gGraphParser(){;}
    ~gGraphParser(){;}

    static void createFromFile(gGraph *graph, QString filename, QVector<QString> headersToAdd, QString weightHeader="", QString split=",");
};

#endif // GGRAPHPARSER_H
