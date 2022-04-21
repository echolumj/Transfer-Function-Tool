#ifndef DATAINFO_H
#define DATAINFO_H

#include <vector>
#include <algorithm>
#include <QString>
#include <QStringList>
#include <vtkRectilinearGrid.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>
#include "Vec.h"
#include <QObject>

using namespace std;

#define THREAD_NUM 10

//the attribute name of vtm file data
const std::string gradientName = "Gradient";
const std::string temporalName = "Temporal_Gradient";
const std::string scalarName = "press_equil_CC/0";


class vtkAppendFilter;
class vtkMultiBlockDataSet;

class DataInfo: public QObject{
Q_OBJECT

public:
    enum DATATYPE{VTKUNSTRUCTUREDDATA, RAWDATA};
    DataInfo();
    ~DataInfo(){  }

    void calcNameOfFileSeries(QString fileName);

    QString getInputFileName(int n = 0);
    QString getoutputFileName(int n=0, QString postfix="");
    virtual std::vector<float> *getVarArray(int varID=0, int timeStep=-1)=0;
    virtual vtkDataArray *getVarArrayVTKFormat(int varID, int timeStep)=0;
    virtual void* getCurrentVarArrayPointer(int varID=0)=0;
    virtual vtkDataArray* getCurrentVarArrayVTKFormat(int varID)=0;
    virtual Vec3i getDim()=0;
    virtual int getPointNum()=0;

    virtual void readTimeStep(int n=0 ) =0;

public:
    DATATYPE datatype;
    Vec3i dim;
    Vec3i dimenMin;
    QString inputDir;
    QStringList fileNameList;
    int currentTimeStep;
    int totalTimeStep;

    QString outputDir;

    void *data;

public slots:
    void setCurrentTime(int n);

signals:
    void currentTimeChanged();
    void setTFSignal(vtkDataArray*, QString);
    void setTFSignal2D(vtkDataArray*, vtkDataArray*);
};


#define STRUCTDATA(data) static_cast<vtkRectilinearGrid *>(data)

//TODO: add support for VTI format. utilize dynamic_cast inside the getFilenameSeries to change global_data to corresponding subclass

class StructureDataInfo : public DataInfo
{


public:
    StructureDataInfo();

    ~StructureDataInfo();

    /** Three type of get data. getVarArray will get a data and its pointer.
     * Other two only get a pointer of current time.
     **/ 
    virtual std::vector<float> *getVarArray(int varID=0, int timeStep=0);
    virtual vtkDataArray *getVarArrayVTKFormat(int varID, int timeStep);
    virtual void* getCurrentVarArrayPointer(int varID=0);
    virtual vtkDataArray* getCurrentVarArrayVTKFormat(int varID);
   
    
    virtual void readTimeStep(int n =0 );
    virtual Vec3i getDim();
     virtual int getPointNum();

private:
    void threadSpeed(vtkMultiBlockDataSet *multiblock, uint startblock, uint endblock);

    vtkFloatArray* attributeArray;
};




extern DataInfo *globalData;


// the following are some help function to deal with the data
/**
 * @brief normalized an vector<T> to [0,1]
*/
template <typename T>
void normalizeArray(vector<T> * var){
    T maxVar = *max_element(var->begin(),var->end());
    T minVar = *min_element(var->begin(),var->end());
    T extent = maxVar-minVar;
    Q_ASSERT(extent > 1e-6);

    for_each (var->begin(),var->end(), [minVar,extent](T &it){it=(it-minVar)/extent;} );

}

void normalizeArray(vtkFloatArray * var, double minVar, double maxVar);


template  <typename T>
vector<int> calcHist(const vector<T>& data, int histBinNum = 256)
{
    vector<int> histgram;
    histgram.resize(histBinNum,0);
    T maxVar = *max_element(data->begin(),data->end());
    T minVar = *min_element(data->begin(),data->end());
    float range = maxVar - minVar + 1e-2; //little wider to simplify the index cal
    for (uint i=0; i<data.size(); ++i)
    {
         ++histgram[floor((data[i]-minVar)/range*histBinNum)];
    }

    Q_ASSERT( std::accumulate(histgram.begin(),histgram.end(),0) == data.size());

    return histgram;
}



template  <typename T>
T getMax(const vector<T>& data)
{
    return *max_element(data->begin(),data->end());
}


template  <typename T>
T getMin(const vector<T>& data)
{
    return *min_element(data->begin(),data->end());
}


double getMax(vtkDataArray* data);
double getMin(vtkDataArray* data);
vector<int> calcHist(vtkDataArray* data, int histBinNum = 256);
vector<float> calcGradientHist(vtkDataArray* scalar, vtkDataArray* gradient, int histBinNum = 256);
#endif // STRUCTUREDATAINFO_H

