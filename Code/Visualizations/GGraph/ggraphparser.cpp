#include "ggraphparser.h"
#include<QFile>
#include<QTextStream>
#include<QDebug>



void gGraphParser::createFromFile(gGraph *graph, QString filename,QVector<QString> headersToAdd,QString weightHeader,QString split)
{
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        qDebug()<<"Error: Cannot open file"<<filename;
        return;
    }
    QTextStream in(&file);
    in.setCodec("UTF-8");
    QList<int> VertexIndeces;
    int weightHeaderIndex=-1;
    QStringList headers;
    long lineIndex=-1;
    while(!in.atEnd())
    {
        QString line=in.readLine();
                //qDebug()<<"line"<<line;
        lineIndex++;
        if(lineIndex==0)
        {
            headers=line.split(split);
            for(int i=0;i<headersToAdd.size();i++)
            {
                int index=headers.indexOf(headersToAdd[i]);
                if(index>=0 && index<headers.size())
                    VertexIndeces.push_back(index);
                if(weightHeader.size()!=0)
                {
                    weightHeaderIndex = headers.indexOf(weightHeader);
                }
            }
            continue;
        }
        QStringList splitted=line.split(split);
        for(int i=0;i<VertexIndeces.size();i++)
        {
            for(int j=i+1;j<VertexIndeces.size();j++)
            {
                if(weightHeaderIndex==-1)
                {
                    graph->addEdge(splitted[VertexIndeces[i]],headers[VertexIndeces[i]],splitted[VertexIndeces[j]],headers[VertexIndeces[j]]);
                }
                else
                {
                    graph->addEdge(splitted[VertexIndeces[i]],headers[VertexIndeces[i]],splitted[VertexIndeces[j]],headers[VertexIndeces[j]],QVariant(splitted[weightHeaderIndex]).toFloat());
                }
            }
        }
    }
    file.close();
}














