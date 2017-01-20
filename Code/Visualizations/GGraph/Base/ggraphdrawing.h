#ifndef GGRAPHDRAWING_H
#define GGRAPHDRAWING_H

#include<GGraph/Base/ggraph.h>
#include<QSet>
#include<QRect>
#include<QFile>
#include<QTextStream>
#include<QPainter>

class gGraphDrawing:public QObject
{
    Q_OBJECT
public:

    gGraphDrawing(gGraph * graph=NULL);
    ~gGraphDrawing();

    gGraph * getGraph(){return this->graph;}
    void setGraph(gGraph * graph);

    void reCalculatePositions(gGraph::layout method=gGraph::FMMM);

    void setShowEdges(bool showEdges);
    void setShowVertexNames(bool showVertexNames);
    void setMaxVertexWeight(float maxVertexWeight);
    float getMaxVertexWeight(){return this->maxVertexWeight;}
    void changeVertexWeight(float vertexWeightDiff);
    void setEdgeOpacity(int edgeOpacity);
    int getEdgeOpacity(){return this->edgeOpacity;}
    int getVertexOpacity(){return this->vertexOpacity;}


    void setAnimatePositionSet(bool animatePositionSet);
    void setShowLegend(bool showLegend);




    bool isVisible(gEdge *edge);
    bool isVisible(gVertex *vertex);

    void setVertexOpacity(int vertexOpacity);

    void changeMaxEdgeWeight(float maxEdgeWeightDiff);

    float getEdgeWeightThresshold(){return edgeWeightThresshold;}
    void setShowEdgeWeights(bool showEdgeWeights);

    void animateLastIterations(int numOfLastIterations=50);

    void setCanMoveVertices(bool canMoveVertices){this->canMoveVertices=canMoveVertices;}
    bool getCanMoveVertices(){return canMoveVertices;}

    void setBorderScale(double borderScale){this->borderScale=borderScale;}
    double getBorderScale(){return this->borderScale;}

    long getNumOfSelectedVertices();

    void printScreen(QString fileName, int width=2000, int height=2000);
    void setShowLabel(QString label, bool show);
protected slots:
    void positionsSet();
    void positionsUpdated();
    void showHideLabelsSlot();

    void recalulatePositions();
signals:
    void paintEventFinished();



protected:

    QPoint moveD;
    float scaleFactor;

    void setupClassColors();

    int classNoColors;
    std::vector<QColor*> classbrushColor;
    std::vector<QColor*> classpenColor;

    bool mousePressed;
    QPoint lastPos;
    QPoint lastClickedPos;



    int width;
    int height;

    bool showLabelsForSelectedVertices;

    double borderScale;
    float edgeWeightThresshold;


    QVector<bool> showLabels;
    QVector<bool> showVertices;

    float maxVertexWeightFromData;

    void paint(QPainter *painter, bool rawDrawing);
    void paintVertices(QPainter *painter, bool rawDrawing);
    void paintEdges(QPainter *painter, bool rawDrawing);

    QSet<gVertex *> selectedVertices;
    QVector<gEdge *> selectedEdges;



    Qt::MouseButtons mouseButtonPressed;
    bool moving;

    bool animatePositionSet;
    bool moveOnReleaseMouse;

    bool animatingPositions;
    bool showEdges;
    bool showVertexNames;
    bool showEdgeWeights;
    bool showLegend;
    int vertexOpacity;
    int edgeOpacity;
    float scaleVerticesWeights;
    float scaleEdgesWeights;
    bool doubleClickSelection;

    float maxVertexWeight;
    float minVertexWeight;

    float maxEdgeWeight;
    float minEdgeWeight;

    bool canMoveVertices;

    QVector<int> lastScreenSize;
    QPoint min;
    QPoint max;

    long HitVertex;


    gGraph * graph;

    bool drawSelectionBox;
    QRect selectionBox;


    void scale();
    void calculateGraphPhysicalBorders();
    void scaleVertexSizes();
    void scaleEdgeSizes();
    void paintVertex(QPainter *painter, gVertex *vertex, bool rawDrawing);
    void paintEdge(QPainter *painter, gEdge *edge, bool rawDrawing);
    float getVertexWeight(gVertex *vertex);
    float getEdgeWeight(gEdge *edge);

    void paintVertexName(QPainter *painter, gVertex *ver);
    void paintVerticesNames(QPainter *painter);
    void paintVertices(QPainter *painter, bool rawDrawing, int start, int end);
    virtual void paintLegend(QPainter *painter);
    QVector<float> getScaledVertexPos(gVertex *vertex);
    QVector<float> getScaledVertexPos(QVector<float> pos);
    QVector<float> getScaledVertexPosInverse(QVector<float> pos);




    void paintEdgeWeight(QPainter *painter, gEdge *edge);
    int minusVertexSize;

    QSet<gVertex *> hiddenVertsDueToThresshold;
    QSet<gVertex *> hiddenVertsDueToSelection;


    void revertHiddenVerticesDueToEdgeWeightThresshold();
};

#endif // GGRAPHDRAWING_H
