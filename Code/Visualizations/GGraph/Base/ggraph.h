#ifndef GGRAPH_H
#define GGRAPH_H

#include<QString>
#include<QVector>
#include<QMap>
#include<QSet>
#include<QVariant>

class gVertex
{
public:
    gVertex(QString name,int label=0);
    QVector<float> getPos(){return pos;}
    void setPos(QVector<float> pos){if(pos.size()==2) this->pos=pos;}
    void setPosX(float x){this->pos[0]=x;}
    void setPosY(float y){this->pos[1]=y;}
    int getLabel(){return label;}
    void setLabel(int label){this->label=label;}
    QString getName(){return name;}
    bool getIsVisible(){return isVisible;}
    void setIsVisible(bool isVisible){this->isVisible=isVisible;}


    bool getIsSelected(){return isSelected;}
    void setIsSelected(bool isSelected){this->isSelected=isSelected;}

    float getWeight(){return this->weight;}
    void setWeight(float weight){this->weight=weight;}

    static QString uniqueId(gVertex *ver){return ver->getName()+"_$_"+QVariant(ver->getLabel()).toString();}
private:

    QString name;
    int label;
    QVector<float> pos;
    float weight;
    bool isVisible;
    bool isSelected;
};



class gEdge
{
public:
    gEdge(gVertex *source, gVertex * target,float weight=1);
    gVertex *getSource(){return this->source;}
    gVertex *getTarget(){return this->target;}
    float getWeight(){return this->weight;}
    void setWeight(float weight){this->weight=weight;}
    bool getIsVisible(){return isVisible;}
    void setIsVisible(bool isVisible){this->isVisible=isVisible;}

    static QString uniqueId_st(gEdge *edge){return gVertex::uniqueId(edge->source)+"_$$_"+gVertex::uniqueId(edge->target);}
    static QString uniqueId_ts(gEdge *edge){return gVertex::uniqueId(edge->target)+"_$$_"+gVertex::uniqueId(edge->source);}


private:
    gVertex *source;
    gVertex *target;
    float weight;
    bool isVisible;

};




class gGraph : public QObject
{
    Q_OBJECT
public:

    enum graphStorageMethod{Static,Dynamic};

    enum layout{FMMM,FR};

    gGraph(bool directed=false, graphStorageMethod graphstoragemethod=gGraph::Static);
    ~gGraph();

    //graph creation
    bool addEdge(QString sourceName, QString sourceLabel, QString targetName, QString targetLabel, float weight=1);



    //graph access
    QVector<gVertex *> & getVertices(){setAdditionMode(false); return this->vertices;}
    QVector<gEdge *> & getEdges(){setAdditionMode(false); return this->edges;}
    QString getLabel(gVertex *ver);
    QString getLabel(int index);
    QVector<QString> &getLabels();


    //graph manipulation
    bool setPositions(layout method, int maxIter=300, bool useEdgeWeights=false);
    void savePositionsToFile(QString filename);
    void saveStructureToFile(QString filename);




    //layout parameters
    void setFRruns(int FRruns){this->FRruns=FRruns;}
    void setmaxFRruns(int maxFRruns){this->maxFRruns=maxFRruns;}
    int getFRruns(){return FRruns;}
    int getmaxFRruns(){return maxFRruns;}

    //graph parameters
    QVector<bool> showVertices;

signals:
    void positionsUpdated();
    void positionsSet();
    void labelsChanged();

protected:
    //protected functions
    void setAdditionMode(bool additionMode);
    bool addEdge(gVertex *source, gVertex * target,float weight=1);
    bool FRLayout(bool useEdgeWeights=false);

    //protected graph definition variables
    QVector<gVertex *> vertices;
    QVector<gEdge *> edges;
    QVector<QString> labels;
    QMap<QString,int> labelsMap;
    bool additionMode;
    QMap<QString,gEdge *> edgesMap;
    QMap<QString,gVertex *> verticesMap;
    bool directed;

    //protected graph parameters variables
    int FRruns;
    int maxFRruns;
    QVector< QVector<float> > force;
    QVector< QVector<float> > velocity;
    graphStorageMethod graphstoragemethod;
};

#endif // GGRAPH_H
