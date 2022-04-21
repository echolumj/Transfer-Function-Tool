#include "datainfo.h"
#include "iohelper.h"
#include "shared/trianglerenderer.h"

#include <vtkXMLMultiBlockDataReader.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkAppendFilter.h>
#include <vtkRectilinearGrid.h>
#include <vtkDataSet.h>

#include <vtkUnstructuredGrid.h>
#include <vtkSmartPointer.h>
#include <vtkPointData.h>
#include <vtkIntArray.h>
#include <vtkDataArray.h>
#include <vtkFloatArray.h>
#include <vtkDataArray.h>

#include <QDir>
#include <thread>
#include <mutex>
#include <QCollator>

#include <algorithm>
using namespace std;

DataInfo *globalData;

#define QString2CharStr(str) str.toLocal8Bit().data()
#define GetInputFileName(n) (inputDir+'/'+fileNameList.at(n)).toLatin1().data()


DataInfo::DataInfo():QObject(){
    currentTimeStep=0;
    totalTimeStep = 0;
    data = nullptr;
    datatype = RAWDATA;

    //fileNameList = QStringList();
}

StructureDataInfo::StructureDataInfo()
{
    data = nullptr;
    datatype = VTKUNSTRUCTUREDDATA;
}

StructureDataInfo::~StructureDataInfo()
{
    if (data)
        STRUCTDATA(data)->Delete(); //wait to verify

}


void DataInfo::calcNameOfFileSeries(QString fileName)
{
    fileNameList.clear();
    //fileNameList = QStringList();
    int pos1 = fileName.lastIndexOf('/');
    int pos2 = fileName.lastIndexOf('\\');
    pos1 = max(pos1,pos2);

    inputDir = fileName.mid(0,pos1+1);

    if (fileName.contains('*')){
        int pos = fileName.indexOf('*');

        QStringList filters;
        filters << fileName.mid(pos+1);
        filters << fileName.mid(pos1+1,pos);

        QDir dir(inputDir);
        dir.setNameFilters(filters);

        fileNameList = dir.entryList(QDir::Files|QDir::NoDotAndDotDot);

        QCollator collator;
        collator.setNumericMode(true);

        std::sort(
            fileNameList.begin(),
            fileNameList.end(),
            [&](const QString &file1, const QString &file2)
            {
                return collator.compare(file1, file2) < 0;
            });


        totalTimeStep = fileNameList.size();
    }
    else{

        totalTimeStep = 1;
        fileNameList.push_back(fileName.mid(pos1+1));
    }

    currentTimeStep = 0;
}

void DataInfo::setCurrentTime(int n)
{
    if (n>=totalTimeStep || n <0){
        qWarning(QString("timestep %d outside the range 0-%d").arg(n).arg(totalTimeStep).toLocal8Bit().data());
        return;
    }
    currentTimeStep = n;
    readTimeStep(n);
    emit currentTimeChanged();
}

QString DataInfo::getInputFileName(int n)
{
    return inputDir+'/'+fileNameList.at(n);
}

QString DataInfo::getoutputFileName(int n, QString postfix)
{
    QString name =  outputDir+'/'+fileNameList.at(n);
    name.insert(name.lastIndexOf('.'),postfix);
    return name;
}

//mutex locker;
//input all arrays of multiblock to rectilinearGrid
void StructureDataInfo::threadSpeed(vtkMultiBlockDataSet *multiblock, uint startblock, uint endblock)
{
    for (uint i = startblock; i <= endblock; i++)
    {
        auto block = vtkRectilinearGrid::SafeDownCast(multiblock->GetBlock(i));
        auto extent = block->GetBounds();
        auto pointData = STRUCTDATA(data)->GetPointData();

        for(int x = (extent[0] - dimenMin.x)/2; x < (extent[1] - dimenMin.x)/2; x++)
            for(int y = (extent[2] - dimenMin.y)/2; y < (extent[3] - dimenMin.y)/2; y++)
                 for(int z = (extent[4] - dimenMin.z)/2; z < (extent[5] - dimenMin.z)/2; z++)
                {
                     /*
                     int ijk[] = { x, y, z};
                     vtkIdType rectiId = STRUCTDATA(data)->ComputePointId(ijk);
                     ijk[0] = x + dimenMin.x - extent[0];
                     ijk[1] = y + dimenMin.y - extent[2];
                     ijk[2] = z + dimenMin.z - extent[4];
                     vtkIdType blockId = block->ComputePointId(ijk);
                     */

                     int current[] = {static_cast<int>(x-extent[0]/2+ dimenMin.x/2), static_cast<int>(y-extent[2]/2 + dimenMin.y/2) , static_cast<int>(z-extent[4]/2 + dimenMin.z/2)};
                     vtkIdType id = z * dim.x * dim.y+ y * dim.x + x;
                     vtkIdType currentID = block->ComputePointId(current);

                     for(int i = 0; i < pointData->GetNumberOfArrays(); i++)
                     {
                         //auto name = pointData->GetArrayName(i);
                         pointData->GetArray(i)->SetTuple(id, block->GetPointData()->GetArray(i)->GetTuple(currentID));
                     }
                 }
    }
}

void StructureDataInfo::readTimeStep(int n)
{
    vtkSmartPointer<vtkXMLMultiBlockDataReader> reader =
    vtkSmartPointer<vtkXMLMultiBlockDataReader>::New();
    reader->SetFileName(QString2CharStr((inputDir+fileNameList.at(n))));
    reader->Update();

    //auto multiBlockReader = vtkMultiBlockDataSet::SafeDownCast(reader->GetOutput());
    auto multiBlockReader = vtkMultiBlockDataSet::SafeDownCast(reader->GetOutput());
    auto blockNum = multiBlockReader->GetNumberOfBlocks();

    auto midData = vtkRectilinearGrid::SafeDownCast(multiBlockReader->GetBlock(0));
    auto min = vtkRectilinearGrid::SafeDownCast(multiBlockReader->GetBlock(0))->GetBounds();
    auto max = vtkRectilinearGrid::SafeDownCast(multiBlockReader->GetBlock(blockNum-1))->GetBounds();

    //pos 100 0 96
    this->dimenMin = Vec3i(min[0], min[2], min[4]);

    this->dim = Vec3i(static_cast<int>((max[1] - min[0])/2), static_cast<int>((max[3] - min[2])/2),static_cast<int>((max[5] - min[4])/2));
    if (data)
        STRUCTDATA(data)->Delete(); // may be data.Delete(), check later
    data = vtkRectilinearGrid::New();

    auto pointData = midData->GetPointData();
    STRUCTDATA(data)->SetDimensions(dim.x, dim.y, dim.z);
    //STRUCTDATA(data)
    for(int i = 0; i < midData->GetPointData()->GetNumberOfArrays(); i++)
    {
        vtkFloatArray* array = vtkFloatArray::New();
        array->SetName(pointData->GetArrayName(i));
        array->SetNumberOfComponents(pointData->GetArray(i)->GetNumberOfComponents());
        array->SetNumberOfTuples(dim.x * dim.y * dim.z);
        
        STRUCTDATA(data)->GetPointData()->AddArray(array);
        array->Delete();
    }
    
    uint part =  blockNum / THREAD_NUM;
    std::thread thread[THREAD_NUM];
    thread[0] = std::thread(&StructureDataInfo::threadSpeed, this, multiBlockReader, 0, part-1);
    for(size_t i = 1; i < THREAD_NUM-1; i++)
    {
        thread[i] = std::thread(&StructureDataInfo::threadSpeed, this, multiBlockReader, part * i, part * (i+1)-1);
    }
    thread[THREAD_NUM-1] = std::thread(&StructureDataInfo::threadSpeed, this, multiBlockReader, part*(THREAD_NUM-1), blockNum-1);

    for(size_t i = 0; i < THREAD_NUM; i++)
        thread[i].join();

    auto xRange = STRUCTDATA(data)->GetPointData()->GetArray(scalarName.c_str())->GetRange();
    auto yRange = STRUCTDATA(data)->GetPointData()->GetArray(temporalName.c_str())->GetRange();

    axisRange = glm::vec4(xRange[0], xRange[1],yRange[0], yRange[1]);
}

Vec3i StructureDataInfo::getDim()
{
    return this->dim;
}

int StructureDataInfo::getPointNum()
{

     return STRUCTDATA(data)->GetNumberOfPoints();
}



vector<float> * StructureDataInfo::getVarArray(int varID, int timeStep)
{
    if (inputDir.isNull() || inputDir.isEmpty())
        return nullptr;
    if (timeStep==currentTimeStep || timeStep == -1){
        vtkPointData *pointdata   = STRUCTDATA(data)->GetPointData ();
        vtkDataArray *curVar      = pointdata->GetArray(varID);
        int           numPnts     = curVar->GetNumberOfTuples();
        vector<float> *p          = new vector<float>(numPnts);

        if (QString("vtkFloatArray") == curVar->GetClassName()){
            float * pCurVar = ( float *)curVar->GetVoidPointer(0);
            p->assign(pCurVar, pCurVar+numPnts);
        }
        else {
            for (int i=0; i<numPnts; ++i){
                p->at(i) = curVar->GetTuple1(i);
            }
        }
        return p;

    }
    else{
        return  ::getVarArray(varID, GetInputFileName(timeStep));
    }

}


vtkDataArray* StructureDataInfo::getVarArrayVTKFormat(int varID, int timeStep)
{

    if (inputDir.isNull() || inputDir.isEmpty())
        return nullptr;
    if (timeStep==currentTimeStep || timeStep == -1){

        vtkPointData * 	pointdata = STRUCTDATA(data)->GetPointData ();
        vtkDataArray *curVar= pointdata->GetArray(varID);
        return curVar;

    }
    else{
        return  ::getVarArrayVTKFormat(varID, GetInputFileName(timeStep));
    }

}

void *StructureDataInfo::getCurrentVarArrayPointer(int varID)
{
    if(!data)
        return nullptr;


    vtkPointData * 	pointdata = STRUCTDATA(data)->GetPointData ();
    vtkDataArray *curVar= pointdata->GetArray(varID);

    void * pCurVar = curVar->GetVoidPointer(0);
    return pCurVar;

}

vtkDataArray* StructureDataInfo::getCurrentVarArrayVTKFormat(int varID)
{
    if(!data)
        return nullptr;


    vtkPointData * 	pointdata = STRUCTDATA(data)->GetPointData ();
    vtkDataArray *curVar= pointdata->GetArray(varID);

    return curVar;

}


Vec2f getRange(vtkDataArray* data)
{

    double* range = data->GetRange();
    return Vec2f(range[0],range[1]);

}


double getMax(vtkDataArray* data)
{
    Q_ASSERT(data!=NULL);

    //     float max = data->GetTuple1(0);
    //     for (int i=0; i< data->GetNumberOfTuples(); ++i){
    //         if (max<*data->GetTuple(i))
    //             max=*data->GetTuple(i);
    //     }
    //     return max;

    return data->GetRange()[1];
}
double getMin(vtkDataArray* data)
{
    Q_ASSERT(data!=NULL);

    //     float min = *data->GetTuple(0);
    //     for (int i=0; i < data->GetNumberOfTuples(); ++i){
    //         if (min > *data->GetTuple(i))
    //             min = *data->GetTuple(i);
    //     }
    //     return min;
    return data->GetRange()[0];
}

vector<int> calcHist(vtkDataArray* data, int histBinNum)
{
    vector<int> histgram;
    histgram.resize(histBinNum,0);
    double maxVar = getMax(data);
    double minVar = getMin(data);
    int length = data->GetNumberOfTuples();
    double range = maxVar - minVar;
    if (range<1e-06){
        qWarning("empty data");
        return histgram;
    }

    for (int i=0; i<length; ++i)
    {
        int index = floor((data->GetTuple(i)[0]-minVar)/range*histBinNum);
        if (index>=histBinNum)
            index = histBinNum-1;
        ++histgram[index];
    }

    //Q_ASSERT( accumulate(histgram.begin(),histgram.end(),0) == length);

    return histgram;
}

vector<float> calcGradientHist(vtkDataArray* scalar, vtkDataArray* gradient, int histBinNum)
{
    vector<float> histgram(histBinNum,0.0f);
    vector<int> histgramNum(histBinNum,0);

    double maxVar = getMax(scalar);
    double minVar = getMin(scalar);
    int length = scalar->GetNumberOfTuples();
    double range = maxVar - minVar;
    if (range<1e-06){
        qWarning("empty data");
        return histgram;
    }

    for (int i=0; i<length; ++i)
    {
        int index = floor((scalar->GetTuple(i)[0]-minVar)/range*histBinNum);
        if (index>=histBinNum)
            index = histBinNum-1;

        histgram[index] = (histgram[index] * histgramNum[index] + gradient->GetTuple(i)[0])/(histgramNum[index]+1);
        histgramNum[index]++;
    }

    //Q_ASSERT( accumulate(histgram.begin(),histgram.end(),0) == length);

    return histgram;
}

void normalizeArray(vtkFloatArray * var, double minVar, double maxVar){

    double extent = maxVar-minVar;
    if (extent < 1e-5){
        qWarning("the vars extent smaller than 1e-5");
        return;
    }

    float * p = var->GetPointer(0);
    for_each (p,p+var->GetNumberOfTuples()-1, [minVar,extent](float &it){it=(it-minVar)/extent;} );

}
