#include <QApplication>

#include<GGraph/Base/ggraph.h>
#include<GGraph/Base/ggraphdrawing.h>
#include<GGraph/ggraphparser.h>

#include<QDebug>


void loadParameters(QString filename,QMap<QString,QString> &parameters)
{
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        qDebug()<<"Cannot open file"<<filename;
        return;
    }
    QTextStream in(&file);
    while(!in.atEnd())
    {
        QString line=in.readLine();
        QStringList splitted=line.split("=");
        if(splitted.size()!=2) continue;
        parameters[splitted[0]]=splitted[1];
    }
    file.close();
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if(argc>=2)
    {
        QMap<QString,QString> parameters ;
        loadParameters(argv[1],parameters);
        if(!parameters.contains("datasetFilename") || !parameters.contains("attributesToLoad")
                ||!parameters.contains("runExample") || !parameters.contains("scriptName")
                ||!parameters.contains("datasetFilename2") || !parameters.contains("attributesToLoad2") || !parameters.contains("scriptName2")
                ||!parameters.contains("datasetFilename3") || !parameters.contains("attributesToLoad3") || !parameters.contains("scriptName3")
                ||!parameters.contains("datasetFilename4") || !parameters.contains("attributesToLoad4") ||!parameters.contains("datasetFilename5")
                || !parameters.contains("attributesToLoad5"))return 0;


        //data configuration
        QString filename;
        QString filename2;
        QVector<QString> headers;
        QVector<QString> headers2;
        QString scriptName;
        bool showVertexNames = true;
        bool showLegend = true;
        int edgeOpacity=100;
        QString py_command="python ";

        if(parameters["runExample"] == "1")
        {
            qDebug()<< "Starting Example1\n";
            filename=parameters["datasetFilename"];
            headers=parameters["attributesToLoad"].split(",").toVector();
            scriptName=parameters["scriptName"];
        }
        else if(parameters["runExample"] == "2")
        {
            qDebug()<< "Starting Example2\n";
            filename=parameters["datasetFilename2"];
            headers=parameters["attributesToLoad2"].split(",").toVector();
            scriptName=parameters["scriptName2"];
        }
        else if(parameters["runExample"] == "3")
        {
            qDebug()<< "Starting Example3\n";
            filename=parameters["datasetFilename3"];
            headers=parameters["attributesToLoad3"].split(",").toVector();
            scriptName=parameters["scriptName3"];
        }
        else if(parameters["runExample"] == "4")
        {
            qDebug()<< "Starting Example4\n";
            filename=parameters["datasetFilename4"];
            headers=parameters["attributesToLoad4"].split(",").toVector();
            filename2=parameters["datasetFilename5"];
            headers2=parameters["attributesToLoad5"].split(",").toVector();
            scriptName=parameters["scriptName4"];
        }

        if(parameters.contains("showVertexNames"))
        {
            if(parameters["showVertexNames"] == "false")
            {
                showVertexNames = false;
            }
            else
            {
                showVertexNames = true;
            }

        }
        if(parameters.contains("showLegend"))
        {
            if(parameters["showLegend"] == "false")
            {
                showLegend = false;
            }
            else
            {
                showLegend = true;
            }

        }

        if(parameters.contains("edgeOpacity")) {edgeOpacity =parameters["edgeOpacity"].toInt();}

        qDebug() << filename << "\n";
        qDebug() << headers <<"\n";

        py_command = py_command.append(scriptName);
        //Run python script
        system(py_command.toUtf8().constData());

        gGraph *graph=new gGraph();
        gGraphParser::createFromFile(graph,filename,headers);
        if(parameters["runExample"] == "4")
        {
            gGraphParser::createFromFile(graph,filename2,headers2);
        }
        graph->setPositions(gGraph::FR);
        gGraphDrawing *graphDrawing=new gGraphDrawing(graph);
        graphDrawing->setShowVertexNames(showVertexNames);
        graphDrawing->setShowLegend(showLegend);
        graphDrawing->changeVertexWeight(20);
        QSize size(2000,2000);
        if(parameters.contains("pngSize"))
        {
            QStringList splitted=parameters["pngSize"].split(",");
            if(splitted.size()==2)
            {
                size.setWidth(QVariant(splitted[0]).toInt());
                size.setHeight(QVariant(splitted[1]).toInt());
            }
        }
        graphDrawing->setShowLabel("followers",false);
        graphDrawing->setShowLabel("followings",false);
        graphDrawing->setEdgeOpacity(edgeOpacity);
        graphDrawing->printScreen("graph.png",size.width(),size.height());
        graph->saveStructureToFile("graph_structure.csv");
        graph->savePositionsToFile("vertex_positions.csv");


        return 0;
    }
    return 0;
}
