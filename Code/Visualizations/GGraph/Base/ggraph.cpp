#include "ggraph.h"

#include<QDebug>
#include<QFile>
#include<QTextStream>
#include<QVariant>
#include "omp.h"
#include<limits>


gGraph::gGraph(bool directed, graphStorageMethod graphstoragemethod)
{
    this->directed=directed;
    this->additionMode=true;
    this->FRruns=0;
    this->maxFRruns=300;
    this->graphstoragemethod=graphstoragemethod;
}

gGraph::~gGraph()
{
    this->setAdditionMode(false);
    for(int i=0;i<this->edges.size();i++)
    {
        delete this->edges[i];
    }
    for(int i=0;i<this->vertices.size();i++)
    {
        delete this->vertices[i];
    }
    this->edges.clear();
    this->vertices.clear();
    this->edgesMap.clear();
    this->verticesMap.clear();
}

void gGraph::saveStructureToFile(QString filename)
{
    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        qDebug()<<"Cannot open file:"<<filename;
        return;
    }
    QTextStream out(&file);
    out<<"SourceVertexName,SourceVertexLabel,TargetVertexName,TargetVertexLabel,EdgeWeight\n";
    for(int i=0;i<this->edges.size();i++)
    {
        gVertex *source=this->edges[i]->getSource();
        gVertex *target=this->edges[i]->getTarget();
        out<<source->getName()<<","<<this->getLabel(source)<<",";
        out<<target->getName()<<","<<this->getLabel(target)<<",";
        out<<edges[i]->getWeight()<<"\n";
    }
    file.close();
}

void gGraph::savePositionsToFile(QString filename)
{
    this->setAdditionMode(false);
    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        qDebug()<<"ERROR: Cannot open file"<<filename;
        return;
    }
    QTextStream out(&file);
    out<<"VertexName,VertexLabel,posX,posY\n";
    for(int i=0;i<this->vertices.size();i++)
    {
        QVector<float> pos=this->vertices[i]->getPos();
        out<<this->vertices[i]->getName()<<",";
        out<<this->getLabel(this->vertices[i])<<",";
        out<<QVariant(pos[0]).toString()<<",";
        out<<QVariant(pos[1]).toString()<<"\n";
    }
    file.close();
}


bool gGraph::addEdge(gVertex *source, gVertex *target, float weight)
{
    if(!additionMode)
    {
        this->setAdditionMode(true);
    }

    gEdge * edgeToAdd=new gEdge(source,target,weight);
    QString uniqueId_st=gEdge::uniqueId_st(edgeToAdd); //source/target
    QString uniqueId_ts=gEdge::uniqueId_ts(edgeToAdd); //target/source
    if(edgesMap.contains(uniqueId_st))
    {
        gEdge * edge=edgesMap[uniqueId_st];
        edge->setWeight(edge->getWeight()+weight);
        delete edgeToAdd;
        return false;

    }
    else if(edgesMap.contains(uniqueId_ts))
    {
        gEdge * edge=edgesMap[uniqueId_ts];
        edge->setWeight(edge->getWeight()+weight);
        delete edgeToAdd;
        return false;
    }
    else
    {
        edgesMap[uniqueId_st]=edgeToAdd;
        if(graphstoragemethod==gGraph::Dynamic)
        {
            this->edges.push_back(edgeToAdd);
        }
        if(!directed)
        {
            edgesMap[uniqueId_ts]=edgeToAdd;
        }

        return true;
    }
    return false;
}


bool gGraph::addEdge(QString sourceName, QString sourceLabel,QString targetName, QString targetLabel,float weight)
{
    if(!additionMode)
    {
        this->setAdditionMode(true);
    }
    if(!labelsMap.contains(sourceLabel))
    {
        labels.push_back(sourceLabel);
        this->showVertices.push_back(true);
        labelsMap[sourceLabel]=labels.size()-1;
    }
    if(!labelsMap.contains(targetLabel))
    {
        labels.push_back(targetLabel);
        this->showVertices.push_back(true);
        labelsMap[targetLabel]=labels.size()-1;
    }
    gVertex * source=new gVertex(sourceName, labelsMap[sourceLabel]);
    gVertex * target=new gVertex(targetName, labelsMap[targetLabel]);
    QString uniqueSourceId=gVertex::uniqueId(source);
    QString uniqueTargetId=gVertex::uniqueId(target);
    if(verticesMap.contains(uniqueSourceId))
    {
        delete source;
        source=verticesMap[uniqueSourceId];
    }
    else
    {
        verticesMap[uniqueSourceId]=source;
        if(graphstoragemethod==gGraph::Dynamic)
        {
            this->vertices.push_back(source);
        }
    }
    if(verticesMap.contains(uniqueTargetId))
    {
        delete target;
        target=verticesMap[uniqueTargetId];
    }
    else
    {
        verticesMap[uniqueTargetId]=target;
        if(graphstoragemethod==gGraph::Dynamic)
        {
            this->vertices.push_back(target);
        }
    }

    if(!addEdge(source,target,weight))
    {
        return false;
    }
    else
    {
        verticesMap[uniqueTargetId]->setWeight(verticesMap[uniqueTargetId]->getWeight()+weight);
        verticesMap[uniqueSourceId]->setWeight(verticesMap[uniqueSourceId]->getWeight()+weight);
    }
    return true;
}






QString gGraph::getLabel(gVertex *ver)
{
    return getLabel(ver->getLabel());
}

QString gGraph::getLabel(int index)
{
    if(index<this->labels.size() && index>=0)
    {
        return this->labels[index];
    }
    return "";
}

QVector<QString> & gGraph::getLabels()
{
    return this->labels;
}




bool gGraph::setPositions(gGraph::layout method, int maxIter,bool useEdgeWeights)
{
    this->setAdditionMode(false);
    if(method==gGraph::FR)
    {
        bool toReturn=false;
        for(int i=0;i<maxIter;i++)
        {
            toReturn=FRLayout(useEdgeWeights);
        }
        if(toReturn==true)
        {
            emit this->positionsSet();
        }
        else
        {
            emit this->positionsUpdated();
        }
        return toReturn;
    }
    return true;
}


bool gGraph::FRLayout(bool useEdgeWeights)
{
    this->setAdditionMode(false);
    if(this->FRruns>=this->maxFRruns)
    {
        this->force.clear();
        this->velocity.clear();
        return true;
    }
    qDebug()<<"FRLayout"<<this->FRruns;
    int numVerts = this->vertices.size();

    float dampenLast = 0.65f;

    float Dimension = 3000.0f;
    float area = Dimension * Dimension;
    float k = sqrt(area/numVerts);
    float repulseConstant = pow(k, 2);
    float springConstant = 1.0/k;
    //float restDistance = 40.0f;
    float restDistance = 10.0f;
    float epsilon = 0.0000001f;
    //    float minD = 100 * pow(k, 2);
    //    minD /= 5;
    float minD=std::numeric_limits<float>::max();
    if(useEdgeWeights)
    {
        repulseConstant=pow((double)repulseConstant,1.2);
    }

    //float T = pow((300 - FRRuns)/float(301), 2) * numVerts (10 + 0.5*FRRuns);
    //float T = sqrt(k) * (300 - FRRuns)/float(301);
    //float T = k * pow((300 - FRRuns)/float(301), 2);
    float FRRatio = (this->maxFRruns - this->FRruns) / (float)(this->maxFRruns + 1);
    float T = k * FRRatio;
    //float T = 1000;
    //fprintf(stderr, "T: %f\n", T);fflush(stderr);fflush(stderr);

    int i, j, l;
    float deltaX, deltaY, distSquared, dist, prod, force, w;
    float Fo;
    float first, second;
    float a, b;
    QPoint **Force;
    int tid, nthreads;
    int chunk = 10;
    //    omp_set_num_threads(1);

    QMap<gVertex *,int> map;
    for (j = 0; j < numVerts; ++j)
    {
        map[this->vertices[j]]=j;
        if(this->force.size()<=j)
        {
            QVector<float> temp;
            temp.push_back(0);
            temp.push_back(0);
            this->force.push_back(temp);
        }
        if(this->velocity.size()<=j)
        {
            QVector<float> temp;
            temp.push_back(0);
            temp.push_back(0);
            this->velocity.push_back(temp);
        }
    }


#pragma omp parallel
    {
        nthreads = omp_get_num_threads();
    }
    Force = new QPoint*[nthreads];
    for (i = 0; i < nthreads; i++)
    {
        Force[i] = new QPoint[numVerts];
        for (j = 0; j < numVerts; ++j)
        {
            Force[i][j].setX(0);
            Force[i][j].setY(0);
        }
    }


#pragma omp parallel shared(Force) private(tid, i, j, l, deltaX, deltaY, distSquared, dist, prod, force, w, first, second, a, b)
    {

#pragma omp for schedule (static, chunk)
        for (i = 0; i < numVerts - 1; ++i){
            if (!this->vertices[i]->getIsVisible())
                continue;
            if(!showVertices[this->vertices[i]->getLabel()])
                continue;

            tid = omp_get_thread_num();

            for (j = i + 1; j < numVerts; ++j)
            {
                if (!this->vertices[j]->getIsVisible())
                    continue;
                if(!showVertices[this->vertices[j]->getLabel()])
                    continue;
                //fprintf(stderr," fr layout %d\n", i);fflush(stderr);
                deltaX = this->vertices[i]->getPos()[0] - this->vertices[j]->getPos()[0];
                deltaY = this->vertices[i]->getPos()[1] - this->vertices[j]->getPos()[1];

                distSquared = (float)deltaX*(float)deltaX + (float)deltaY*(float)deltaY + epsilon;
                // Avoid divide by zero
                //if (distSquared == 0)
                //distSquared += epsilon;

                if ( distSquared < minD ){
                    //a = log10(this->Vertex[i]->Map->NoEdges);
                    //b = log10(this->Vertex[j]->Map->NoEdges);
                    a = this->vertices[i]->getWeight();
                    b = this->vertices[j]->getWeight();
                    a = 1;
                    b = 1;
                    //a = 1 + (this->Vertex[i]->NormalSize - 1) * (1 - FRRatio);
                    //b = 1 + (this->Vertex[j]->NormalSize - 1) * (1 - FRRatio);
                    a = a > b ? a : b;
                    //a = a * a;

                    prod = a * (float)repulseConstant / (float)distSquared;
                    first = (float)prod * (float)deltaX;
                    second = (float)prod * (float)deltaY;
                    Force[tid][i].setX(Force[tid][i].x()+first);
                    Force[tid][i].setY(Force[tid][i].y()+second);
                    Force[tid][j].setX(Force[tid][j].x()-first);
                    Force[tid][j].setY(Force[tid][j].y()-second);
                }

            }
        }

#pragma omp for schedule (static, chunk)
        for (l = 0; l < this->edges.size(); ++l)
        {
            if (!this->edges[l]->getIsVisible())
                continue;
            if (!this->edges[l]->getSource()->getIsVisible())
                continue;
            if (!this->edges[l]->getTarget()->getIsVisible())
                continue;
            if(!showVertices[this->edges[l]->getSource()->getLabel()])
                continue;
            if(!showVertices[this->edges[l]->getTarget()->getLabel()])
                continue;
            tid = omp_get_thread_num();


            w=0.5f;
            if(useEdgeWeights)
            {
                w=pow((double)this->edges[l]->getWeight(),1.5)/2;
            }


            deltaX = this->edges[l]->getSource()->getPos()[0] - this->edges[l]->getTarget()->getPos()[0];
            deltaY = this->edges[l]->getSource()->getPos()[1] - this->edges[l]->getTarget()->getPos()[1];

            distSquared = (float)deltaX*(float)deltaX + (float)deltaY*(float)deltaY;
            dist = sqrt(distSquared) + epsilon;
            //dist = sqrt(distSquared) + epsilon - restDistance;

            force = springConstant * pow( (dist - 1 * restDistance), 2);
            prod = (float)w * (float)force / (float)dist;

            //prod = (float)w * (float)springConstant * (float)dist;
            first = (float)prod * (float)deltaX;
            second = (float)prod * (float)deltaY;
            i=map[this->edges[l]->getSource()];
            j=map[this->edges[l]->getTarget()];

            Force[tid][i].setX(Force[tid][i].x()-first);
            Force[tid][i].setY(Force[tid][i].y()-second);
            Force[tid][j].setX(Force[tid][j].x()+first);
            Force[tid][j].setY(Force[tid][j].y()+second);
        }
    }
#pragma omp parallel shared(numVerts) private(i, j, Fo, dampenLast)
    {
#pragma omp for schedule (static, chunk)
        for (i = 0; i < numVerts; ++i)
        {
            if (!this->vertices[i]->getIsVisible())
                continue;
            if(!showVertices[this->vertices[i]->getLabel()])
                continue;

            for (j = 0; j < nthreads; j++)
            {
                this->force[i][0] += Force[j][i].x();
                this->force[i][1] += Force[j][i].y();
            }

            Fo = sqrt((float)this->force[i][0] *(float)this->force[i][0] + (float)this->force[i][1]*(float)this->force[i][1]) + epsilon;
            //T = this->Impl->F[u].first;
            this->force[i][0] = (fabs(this->force[i][0]) < T? fabs(this->force[i][0]) : T) * this->force[i][0] / Fo;
            //T = this->Impl->F[u].second;
            this->force[i][1] = (fabs(this->force[i][1]) < T? fabs(this->force[i][1]) : T) * this->force[i][1] / Fo;

            if (this->FRruns == 0)
                dampenLast = 0;

            this->velocity[i][0] = dampenLast * this->velocity[i][0] + (1 - dampenLast) * this->force[i][0];
            this->velocity[i][1] = dampenLast * this->velocity[i][1]+ (1 - dampenLast) * this->force[i][1];

            this->vertices[i]->setPosX( this->vertices[i]->getPos()[0]+ 1.00 * this->velocity[i][0]);
            this->vertices[i]->setPosY( this->vertices[i]->getPos()[1]+ 1.00 * this->velocity[i][1]);

            this->force[i][0] = 0;
            this->force[i][1] = 0;
            for (j = 0; j < nthreads; j++)
            {
                Force[j][i].setX(0);
                Force[j][i].setY(0);
            }
        }
    }

    for (i = 0; i < nthreads; i++)
        delete []Force[i];
    //free(Force);
    delete []Force;



    this->FRruns += 1;


    if(this->FRruns>=this->maxFRruns)
    {
        this->force.clear();
        this->velocity.clear();
        return true;
    }

    return false;
}


void gGraph::setAdditionMode(bool additionMode)
{
    if(this->additionMode==additionMode)
    {
        return;
    }
    this->additionMode=additionMode;
    if(graphstoragemethod==gGraph::Static)
    {
        if(this->additionMode==true)
        {
            this->edgesMap.clear();
            for(int i=0;i<this->edges.size();i++)
            {
                this->edgesMap[gEdge::uniqueId_st(this->edges[i])]=this->edges[i];
                if(!directed) this->edgesMap[gEdge::uniqueId_st(this->edges[i])]=this->edges[i];
            }
            for(int i=0;i<this->vertices.size();i++)
            {
                verticesMap[gVertex::uniqueId(this->vertices[i])]=this->vertices[i];
            }
        }
        else if(this->additionMode==false)
        {
            QSet<gEdge *> edgesSet;
            this->edges.clear();
            this->vertices.clear();

            //convert edgesMap to edges
            QMapIterator<QString,gEdge *> edIter(this->edgesMap);
            while (edIter.hasNext())
            {
                edIter.next();
                edgesSet.insert(edIter.value());
                //            this->edges.push_back(edIter.value());
            }
            this->edgesMap.clear();
            this->edges=edgesSet.toList().toVector();
            edgesSet.clear();


            //convert verticesMap to vertices
            QMapIterator<QString,gVertex *> verIter(verticesMap);
            while (verIter.hasNext())
            {
                verIter.next();
                this->vertices.push_back(verIter.value());
            }
            this->verticesMap.clear();
        }
    }
    else //if(graphstoragemethod==gGraph::Dynamic)
    {
        //do nothing;
    }
}


gVertex::gVertex(QString name, int label)
{
    this->name=name;
    this->label=label;
    this->pos.push_back(rand()%1000);
    this->pos.push_back(rand()%1000);
    this->isVisible=true;
    this->weight=1;
    this->isSelected=false;
}


gEdge::gEdge(gVertex *source, gVertex *target, float weight)
{
    this->source=source;
    this->target=target;
    this->weight=weight;
    this->isVisible=true;

}



