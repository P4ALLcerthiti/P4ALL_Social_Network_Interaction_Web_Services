#include "ggraphdrawing.h"
#include<QDebug>
#include<QApplication>
#include<QTime>
#include<QMenu>
#include<QSignalMapper>
#include<QFileDialog>
#include<QMessageBox>
#include<QInputDialog>
#include<QFormLayout>
#include<QDialogButtonBox>
#include<QLabel>
#include<QComboBox>

gGraphDrawing::gGraphDrawing(gGraph *graph) :QObject()
{
    this->setGraph(graph);
    //    this->qualityVsSpeed=gGraphDrawing::NiceAndIncredibleSpeed;
    //    this->qualityVsSpeed=gGraphDrawing::GorgeousAndEfficient;

    this->borderScale=0.8;
    this->showEdges=true;
    this->showVertexNames=false;
    this->showEdgeWeights=false;
    this->showLegend=false;
    this->animatingPositions=false;

    this->edgeWeightThresshold=0;

    this->moveOnReleaseMouse=false;

    this->vertexOpacity=150;
    this->edgeOpacity=10;

    this->scaleVerticesWeights=2;
    this->scaleEdgesWeights=1;

    //    this->maxVertexWeight=70;
    //    this->minVertexWeight=6;
    //    this->maxEdgeWeight=30;
    //    this->minEdgeWeight=1;


    this->maxVertexWeight=20;
    this->minVertexWeight=1;
    this->maxEdgeWeight=1;
    this->minEdgeWeight=1;


    //    this->maxVertexWeight=6;
    //    this->minVertexWeight=6;
    //    this->maxEdgeWeight=1;
    //    this->minEdgeWeight=1;

    this->lastScreenSize.push_back(0);
    this->lastScreenSize.push_back(0);


    this->min.setX(0);
    this->min.setY(0);
    this->max.setX(0);
    this->max.setY(0);

    this->HitVertex=-1;
    this->drawSelectionBox=false;



    this->animatePositionSet=false;
    this->maxVertexWeightFromData=-1;


    this->moving=false;
    this->doubleClickSelection=false;

    this->canMoveVertices=true;

    this->minusVertexSize=0;


    this->showLabelsForSelectedVertices=false;

    setupClassColors();
    this->scaleFactor=1;
    this->moveD.setX(0);
    this->moveD.setY(0);
}

gGraphDrawing::~gGraphDrawing()
{

}

void gGraphDrawing::reCalculatePositions(gGraph::layout method)
{
    this->graph->setPositions(method);
    this->scale();
}

void gGraphDrawing::setShowEdges(bool showEdges)
{
    this->showEdges=showEdges;
}

void gGraphDrawing::setShowVertexNames(bool showVertexNames)
{
    this->showVertexNames=showVertexNames;
}

void gGraphDrawing::setShowEdgeWeights(bool showEdgeWeights)
{
    this->showEdgeWeights=showEdgeWeights;
}

void gGraphDrawing::setShowLegend(bool showLegend)
{
    this->showLegend=showLegend;
}

void gGraphDrawing::changeMaxEdgeWeight(float maxEdgeWeightDiff)
{
    if(this->maxEdgeWeight+maxEdgeWeightDiff<1)
    {
        return;
    }
    this->maxEdgeWeight=this->maxEdgeWeight+maxEdgeWeightDiff;
    this->scaleEdgeSizes();
}

void gGraphDrawing::changeVertexWeight(float vertexWeightDiff)
{
    if(this->minVertexWeight+vertexWeightDiff<1)
    {
        minusVertexSize--;
        return;
    }
    else if(minusVertexSize<0)
    {
        minusVertexSize++;
        return;
    }
    this->maxVertexWeight=this->maxVertexWeight+vertexWeightDiff;
    this->minVertexWeight=this->minVertexWeight+vertexWeightDiff;
    this->scaleVertexSizes();
}

void gGraphDrawing::setMaxVertexWeight(float maxVertexWeight)
{
    if(maxVertexWeight<this->minVertexWeight) return;
    this->maxVertexWeight=maxVertexWeight;
    this->scaleVertexSizes();
}

void gGraphDrawing::setEdgeOpacity(int edgeOpacity)
{
    this->edgeOpacity=edgeOpacity;
    if(this->edgeOpacity<0) this->edgeOpacity=0;
    if(this->edgeOpacity>255) this->edgeOpacity=255;
}

void gGraphDrawing::setVertexOpacity(int vertexOpacity)
{
    this->vertexOpacity=vertexOpacity;
    if(this->vertexOpacity<10) this->vertexOpacity=10;
    if(this->vertexOpacity>255) this->vertexOpacity=255;
}

void gGraphDrawing::positionsSet()
{
    this->animatingPositions=false;
    QApplication::processEvents();
    this->animatePositionSet=false;
    disconnect(this->graph,SIGNAL(positionsSet()), this, SLOT(positionsSet()));
    disconnect(this->graph,SIGNAL(positionsUpdated()), this, SLOT(positionsUpdated()));
}

void gGraphDrawing::positionsUpdated()
{
    this->animatingPositions=true;
    QApplication::processEvents();
    this->graph->setPositions(gGraph::FR,5);
}

void gGraphDrawing::showHideLabelsSlot()
{
    QAction * action=qobject_cast<QAction *>(QObject::sender());
    if(!action) return;
    int index=this->graph->getLabels().indexOf(action->text());
    if(index>=0 && index<showLabels.size())
    {
        showLabels[index]=action->isChecked();
    }
}

void gGraphDrawing::setGraph(gGraph *graph)
{
    this->graph=graph;
    showLabels.clear();
    showVertices.clear();
    if(this->graph!=NULL && this->graph)
    {
        QVector<QString> labels=this->graph->getLabels();
        for(int i=0;i<labels.size();i++)
        {
            showLabels.push_back(true);
            showVertices.push_back(true);
        }
    }
}

void gGraphDrawing::calculateGraphPhysicalBorders()
{
    int N = 0;
    QVector<gVertex *> vertices=this->graph->getVertices();
    for (int i = 0; i < vertices.size(); i++)
    {
        if ( !isVisible(vertices[i]))
            continue;

        this->min.setX(vertices[i]->getPos()[0]);
        this->max.setX(vertices[i]->getPos()[0]);
        this->min.setY(vertices[i]->getPos()[1]);
        this->max.setY(vertices[i]->getPos()[1]);
        N = i;
        break;
    }


    for (int i = N; i < vertices.size(); i++)
    {
        if ( !isVisible(vertices[i]))
            continue;

        if (vertices[i]->getPos()[0] < this->min.x())
            this->min.setX(vertices[i]->getPos()[0]);
        else if (vertices[i]->getPos()[0]> this->max.x())
            this->max.setX(vertices[i]->getPos()[0]);

        if (vertices[i]->getPos()[1] < this->min.y())
            this->min.setY(vertices[i]->getPos()[1]);
        else if (vertices[i]->getPos()[1] > this->max.y())
            this->max.setY(vertices[i]->getPos()[1]);
    }
}


void gGraphDrawing::scaleVertexSizes()
{
    //    if(maxVertexWeightFromData==-1)
    //    {
    int N=0;
    long numOfDrawingVertices=0;
    QVector<gVertex *> vertices=this->graph->getVertices();
    for (int i = 0; i < vertices.size(); i++)
    {
        if ( !isVisible(vertices[i]))
            continue;
        numOfDrawingVertices++;
        maxVertexWeightFromData=vertices[i]->getWeight();
        N = i;
        break;
    }
    for (int i = N; i < vertices.size(); i++)
    {
        if ( !isVisible(vertices[i]))
            continue;
        numOfDrawingVertices++;
        if(vertices[i]->getWeight()>maxVertexWeightFromData)
        {
            maxVertexWeightFromData=vertices[i]->getWeight();
        }
    }
    //    }
    this->scaleVerticesWeights=this->maxVertexWeight/maxVertexWeightFromData;
}

void gGraphDrawing::scaleEdgeSizes()
{
    int N=0;
    float maxEdgeWeight;
    long numOfDrawingEdges=0;
    QVector<gEdge *> edges=this->graph->getEdges();
    for (int i = 0; i < edges.size(); i++)
    {
        if(!isVisible(edges[i]->getSource()))
            continue;
        if(!isVisible(edges[i]->getTarget()))
            continue;
        if (!isVisible(edges[i]))
            continue;
        maxEdgeWeight=edges[i]->getWeight();
        N = i;
        numOfDrawingEdges++;
        break;
    }
    for (int i = N; i < edges.size(); i++)
    {
        if(!isVisible(edges[i]->getSource()))
            continue;
        if(!isVisible(edges[i]->getTarget()))
            continue;
        if (!isVisible(edges[i]))
            continue;
        numOfDrawingEdges++;
        if(edges[i]->getWeight()>maxEdgeWeight)
        {
            maxEdgeWeight=edges[i]->getWeight();
        }
    }
    //    double reductionFactor=numOfDrawingEdges/500;
    //    if(reductionFactor<1) reductionFactor=1;
    //    this->maxEdgeWeight=30/reductionFactor;
    this->scaleEdgesWeights=this->maxEdgeWeight/maxEdgeWeight;
}


QVector<float> gGraphDrawing::getScaledVertexPos(gVertex * vertex)
{
    QVector<float> pos=vertex->getPos();

    if(width<height)
    {
        if(max.x()!=min.x())
        {
            pos[0]=(pos[0] -min.x())/(max.x()-min.x())*width*borderScale + 0;
            pos[0]+=(width-width*borderScale)/2.0;
        }
        else
        {
            pos[0]=width/2;
        }
        if(max.y()!=min.y())
        {
            pos[1]=(pos[1] -min.y())/(max.y()-min.y())*width*borderScale + 0;
            pos[1]+=(height-width*borderScale)/2.0;
        }
        else
        {
            pos[1]=height/2;
        }
        //        vertices[i]->setPos(pos);
    }
    else
    {
        if(max.x()!=min.x())
        {
            pos[0]=(pos[0] -min.x())/(max.x()-min.x())*height*borderScale + 0;
            pos[0]+=(width-height*borderScale)/2.0;
        }
        else
        {
            pos[0]=width/2;
        }

        if(max.y()!=min.y())
        {
            pos[1]=(pos[1] -min.y())/(max.y()-min.y())*height*borderScale + 0;
            pos[1]+=(height-height*borderScale)/2.0;
        }
        else
        {
            pos[1]=height/2;
        }

        //        vertices[i]->setPos(pos);
    }
    return pos;
}

QVector<float> gGraphDrawing::getScaledVertexPos(QVector<float> pos)
{
    double borderScale=0.8;
    if(width<height)
    {
        pos[0]=(pos[0] -min.x())/(max.x()-min.x())*width*borderScale + 0;
        pos[1]=(pos[1] -min.y())/(max.y()-min.y())*width*borderScale + 0;
        pos[0]+=(width-width*borderScale)/2.0;
        pos[1]+=(height-width*borderScale)/2.0;
        //        vertices[i]->setPos(pos);
    }
    else
    {
        pos[0]=(pos[0] -min.x())/(max.x()-min.x())*height*borderScale + 0;
        pos[1]=(pos[1] -min.y())/(max.y()-min.y())*height*borderScale + 0;
        pos[0]+=(width-height*borderScale)/2.0;
        pos[1]+=(height-height*borderScale)/2.0;
        //        vertices[i]->setPos(pos);
    }
    return pos;
}


QVector<float> gGraphDrawing::getScaledVertexPosInverse(QVector<float> pos)
{
    double borderScale=0.7;
    if(width<height)
    {
        pos[0]-=(width-width*borderScale)/2.0;
        pos[1]-=(height-width*borderScale)/2.0;
        pos[0]=pos[0]*(max.x()-min.x())/width/borderScale+min.x();
        pos[1]=pos[1]*(max.y()-min.y())/width/borderScale+min.y();

    }
    else
    {
        pos[0]-=(width-height*borderScale)/2.0;
        pos[1]-=(height-height*borderScale)/2.0;
        pos[0]=pos[0]*(max.x()-min.x())/height/borderScale+min.x();
        pos[1]=pos[1]*(max.y()-min.y())/height/borderScale+min.y();
    }
    return pos;
}


void gGraphDrawing::scale()
{
    calculateGraphPhysicalBorders();
    QVector<gVertex *> vertices=this->graph->getVertices();

    if(!this->animatingPositions)
    {
        for (int i = 0; i < vertices.size(); i++)
        {
            vertices[i]->setPos(getScaledVertexPos(vertices[i]));
        }
    }
    scaleVertexSizes();
    scaleEdgeSizes();
}

void gGraphDrawing::printScreen(QString fileName,int width,int height)
{
    if(fileName.size()==0) return;
    if(this->graph==NULL)
        return;

    this->width=width;
    this->height=height;

    if ((this->lastScreenSize[0] != width || this->lastScreenSize[1] != height) || this->animatePositionSet)
    {
        this->scale();
        this->lastScreenSize[0] = width;
        this->lastScreenSize[1] = height;
    }


    QImage img(QSize(this->width,this->height), QImage::Format_RGB32);
    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(this->moveD);
    painter.scale(this->scaleFactor, this->scaleFactor);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(QColor(255,255,255)));
    QRect backgroundRect=QRect(0,0,width,height);
    backgroundRect.moveTo(-moveD.x()/scaleFactor,-moveD.y()/scaleFactor);
    painter.drawRect(backgroundRect);
    paint(&painter,false);
    this->paintLegend(&painter);
    painter.end();

    img.save(fileName);
}


void gGraphDrawing::paintVertex(QPainter *painter,gVertex* vertex,bool rawDrawing)
{
    if(!isVisible(vertex))
    {
        return;
    }
    {
        QColor brushColor=*this->classbrushColor[vertex->getLabel()%this->classNoColors];
        brushColor.setAlpha(this->vertexOpacity);
        painter->setBrush(brushColor);
        QColor penColor=*this->classpenColor[vertex->getLabel()%this->classNoColors];
        penColor.setAlpha(this->vertexOpacity);
        QPen pen;
        pen.setColor(penColor);
        pen.setWidth(2);
        painter->setPen(pen);
    }

    if(!rawDrawing)
    {
        if(vertex->getIsSelected())
        {
            QColor color=QColor(255,50,50);
            painter->setPen(Qt::black);
            color.setAlpha(220);
            painter->setBrush(color);
        }
    }

    QVector<float> pos;
    if(this->animatingPositions)
    {
        pos=this->getScaledVertexPos(vertex);
    }
    else
    {
        pos=vertex->getPos();
    }


    float weight=getVertexWeight(vertex);
    painter->drawEllipse(pos[0]-weight/2.0,pos[1]-weight/2.0,weight,weight);
}


void gGraphDrawing::paintLegend(QPainter *painter)
{
    QVector<QString> labels=this->graph->getLabels();
    int size=10*2;
    int pixelSize=15*2;
    painter->save();
    painter->scale(1/this->scaleFactor,1/this->scaleFactor);

    int maxWidth=-1;
    for(int i=0;i<labels.size();i++)
    {
        QFont font;
        font.setPixelSize(pixelSize);
        painter->setFont(font);
        painter->setPen(Qt::black);
        QRectF rect=painter->boundingRect(0,0,0,0,Qt::TextSingleLine,labels[i]);
        if(maxWidth<rect.width())
        {
            maxWidth=rect.width();
        }
    }

    int i_actual=-1;
    for(int i=0;i<labels.size();i++)
    {
        if(labels[i]=="*") continue;
        if(!showVertices[i]) continue;
        i_actual++;
        double posX=(width-maxWidth-50-this->moveD.x());
        double posY=(100+i_actual*2*size-this->moveD.y());
        QColor brushColor=*this->classbrushColor[i%this->classNoColors];
        brushColor.setAlpha(220);
        painter->setBrush(brushColor);
        QColor penColor=*this->classpenColor[i%this->classNoColors];
        penColor.setAlpha(220);
        QPen pen;
        pen.setColor(penColor);
        pen.setWidth(1);
        painter->setPen(pen);
        painter->drawEllipse(posX,posY,size,size);
        painter->setPen(Qt::black);
        QFont font;
        font.setPixelSize(pixelSize);
        painter->setFont(font);
        painter->drawText(posX+2*size,posY+size,labels[i]);
    }
    painter->restore();
}

float gGraphDrawing::getVertexWeight(gVertex* vertex)
{
    float weight=vertex->getWeight()*this->scaleVerticesWeights;
    if(weight>this->maxVertexWeight) weight=this->maxVertexWeight;
    if(weight<this->minVertexWeight) weight=this->minVertexWeight;
    return weight;
}

float gGraphDrawing::getEdgeWeight(gEdge* edge)
{
    float weight=edge->getWeight()*this->scaleEdgesWeights;
    if(weight>this->maxEdgeWeight) weight=this->maxEdgeWeight;
    if(weight<this->minEdgeWeight) weight=this->minEdgeWeight;
    return weight;
}


bool gGraphDrawing::isVisible(gVertex *vertex)
{
    if(!vertex->getIsVisible())
        return false;
    if(!showVertices[vertex->getLabel()])
        return false;
    return true;
}

bool gGraphDrawing::isVisible(gEdge *edge)
{
    if(!edge->getSource()->getIsVisible())
        return false;
    if(!edge->getTarget()->getIsVisible())
        return false;
    if(!edge->getIsVisible())
        return false;

    if(!showVertices[edge->getTarget()->getLabel()])
        return false;
    if(!showVertices[edge->getSource()->getLabel()])
        return false;

    return true;
}

void gGraphDrawing::paintEdge(QPainter *painter,gEdge* edge, bool rawDrawing)
{
    if(!isVisible(edge))
        return;
    float weight=getEdgeWeight(edge);
    QPen pen;
    QColor penColor=*this->classbrushColor[edge->getSource()->getLabel()%this->classNoColors];
    penColor.setAlpha(this->edgeOpacity);
    if(!rawDrawing)
    {
        if(edge->getSource()->getIsSelected() || edge->getTarget()->getIsSelected())
        {
            penColor=Qt::red;
            penColor.setAlpha(180);
        }
    }


    pen.setColor(penColor);

    pen.setWidth(weight);
    painter->setPen(pen);
    QVector<float> sourcePos;
    QVector<float> targetPos;
    if(this->animatingPositions)
    {
        sourcePos=this->getScaledVertexPos(edge->getSource());
        targetPos=this->getScaledVertexPos(edge->getTarget());

    }
    else
    {
        sourcePos=edge->getSource()->getPos();
        targetPos=edge->getTarget()->getPos();

    }
    painter->drawLine(sourcePos[0],sourcePos[1],targetPos[0],targetPos[1]);
}

void gGraphDrawing::paintEdgeWeight(QPainter *painter,gEdge* edge)
{
    if(!isVisible(edge))
        return;
    float weight=getEdgeWeight(edge);
    weight=weight*3;
    if(weight<3) weight=3;
    QFont font;
    font.setPixelSize(weight);
    painter->setPen(Qt::black);
    painter->setFont(font);
    //    QVector<float> sourcePos=edge->getSource()->getPos();
    //    QVector<float> targetPos=edge->getTarget()->getPos();
    QVector<float> sourcePos;
    QVector<float> targetPos;
    if(this->animatingPositions)
    {
        sourcePos=this->getScaledVertexPos(edge->getSource());
        targetPos=this->getScaledVertexPos(edge->getTarget());

    }
    else
    {
        sourcePos=edge->getSource()->getPos();
        targetPos=edge->getTarget()->getPos();
    }
    painter->drawText((sourcePos[0]+targetPos[0])/2.0,(sourcePos[1]+targetPos[1])/2.0,QVariant(edge->getWeight()).toString());
}


void gGraphDrawing::paint(QPainter *painter,bool rawDrawing)
{
    if(this->showEdges)
    {
        paintEdges(painter,rawDrawing);
    }
    paintVertices(painter,rawDrawing);
    if(this->showVertexNames)
    {
        paintVerticesNames(painter);
    }
    if(showLabelsForSelectedVertices)
    {
        foreach(gVertex *ver,this->selectedVertices)
        {
            paintVertexName(painter,ver);
        }
    }
    if(this->showEdgeWeights && this->showEdges)
    {
        for(int i=0;i<this->selectedEdges.size();i++)
        {
            this->paintEdgeWeight(painter,this->selectedEdges[i]);
        }
    }
}

void gGraphDrawing::paintVertices(QPainter *painter,bool rawDrawing)
{
    QVector<gVertex *> vertices=this->graph->getVertices();
    for(int i=0;i<vertices.size();i++)
    {
        //        if(i%10000==0) QApplication::processEvents();
        paintVertex(painter,vertices[i],rawDrawing);
        //        exit(110);
    }
    //    if(this->showVertexNames) paintVerticesNames(painter);
}



void gGraphDrawing::paintVertices(QPainter *painter,bool rawDrawing,int start, int end)
{
    QVector<gVertex *> vertices=this->graph->getVertices();
    for(int i=start;i<vertices.size() && i<=end;i++)
    {
        //        if(i%10000==0) QApplication::processEvents();
        paintVertex(painter,vertices[i],rawDrawing);
    }
}


void gGraphDrawing::paintVerticesNames(QPainter *painter)
{
    QVector<gVertex *> vertices=this->graph->getVertices();
    for(int i=0;i<vertices.size();i++)
    {
        //        if(i%10000==0) QApplication::processEvents();
        paintVertexName(painter,vertices[i]);
    }
}

void gGraphDrawing::setShowLabel(QString label,bool show)
{
    int index=this->graph->getLabels().indexOf(label);
    if(index==-1) return;
    showLabels[index]=show;
}

void gGraphDrawing::paintVertexName(QPainter *painter,gVertex *ver)
{
    if(!isVisible(ver))
        return;
    if(showLabelsForSelectedVertices && this->selectedVertices.contains(ver))
    {
        //ok
    }
    else if(!showLabels[ver->getLabel()])
    {
        return;
    }
    painter->setPen(Qt::black);
    QVector<float> pos;
    if(this->animatingPositions)
    {
        pos=this->getScaledVertexPos(ver);
    }
    else
    {
        pos=ver->getPos();
    }

    //    QVector<float> pos=this->getScaledVertexPos(ver);
    //    QVector<float> pos=ver->getPos();
    //qDebug() << ver->getLabel();
    float weight=getVertexWeight(ver);
    QFont font;
    if(weight<3) weight=3;
    font.setPixelSize(weight);
    painter->setFont(font);
    painter->drawText(pos[0]-weight/2.0,pos[1]-weight/2.0,ver->getName().replace("*",""));

}


void gGraphDrawing::paintEdges(QPainter *painter,bool rawDrawing)
{
    QVector<gEdge *> edges=this->graph->getEdges();
    for(int i=0;i<edges.size();i++)
    {
        //        if(i%10000==0) QApplication::processEvents(QEventLoop::AllEvents);
        paintEdge(painter,edges[i],rawDrawing);
    }
}

void gGraphDrawing::recalulatePositions()
{
    this->scale();
}

void gGraphDrawing::setupClassColors()
{
    this->classNoColors = 20;

    this->classbrushColor.resize(classNoColors);
    this->classpenColor.resize(classNoColors);


    this->classbrushColor[0] = new QColor(60, 100, 148);
    this->classbrushColor[1] = new QColor(150, 61, 59);
    this->classbrushColor[2] = new QColor(121, 146, 68);
    this->classbrushColor[3] = new QColor(99, 77, 126);
    this->classbrushColor[4] = new QColor(57, 134, 155);
    this->classbrushColor[5] = new QColor(194, 117, 53);
    this->classbrushColor[6] = new QColor(73, 120, 177);
    this->classbrushColor[7] = new QColor(179, 74, 71);
    this->classbrushColor[8] = new QColor(146, 175, 83);

    this->classbrushColor[9] = new QColor(50, 205, 50);			//Lime Green
    this->classbrushColor[10] = new QColor(0, 0, 255);			//Blue
    this->classbrushColor[11] = new QColor(65, 105, 225);		//Royal Blue
    this->classbrushColor[12] = new QColor(25, 25, 112);		//Midnight Blue
    this->classbrushColor[13] = new QColor(217, 247, 255);		//Very Light Blue
    this->classbrushColor[14] = new QColor(124, 252, 0);		//Lawn Green
    this->classbrushColor[15] = new QColor(0, 100, 0);			//Dark Green
    this->classbrushColor[16] = new QColor(205, 92, 92);		//Indian Red
    this->classbrushColor[17] = new QColor(210, 105, 30);		//Chocolate
    this->classbrushColor[18] = new QColor(255, 165, 0);		//Orange
    this->classbrushColor[19] = new QColor(176, 48, 96);		//Maroon

    float darken = 0.85f;

    this->classpenColor[0] = new QColor(180, 180, 180);
    for (int i = 1; i < this->classNoColors; i++){
        QColor penColor((*this->classbrushColor[i]));
        this->classpenColor[i] = new QColor(penColor.red()*darken,penColor.green()*darken,penColor.blue()*darken);
    }
}
