#include "iohelper.h"
#include <vtkXMLMultiBlockDataReader.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkUnstructuredGrid.h>
#include <vtkSmartPointer.h>
#include <vtkPointData.h>
#include <vtkIntArray.h>
#include <vtkFloatArray.h>

#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkRectilinearGrid.h>
#include <vtkAppendFilter.h>

#include <qglobal.h>
#include <QString>
#include <algorithm>
using namespace std;


/**
 * @brief get an array from vtkXMLStructureGrid file
*/
vector<float>* getVarArray(const int varID, const char* filename)
{
    vtkUnstructuredGrid * data = readVTKUnsturctureGridFile(filename);
    vtkPointData * 	pointdata = data->GetPointData ();
    vtkDataArray *curVar= pointdata->GetArray(varID);
    int numPnts = static_cast<int>(curVar->GetNumberOfTuples());
    vector<float> *p = new vector<float>(numPnts);

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
/** useless. because this data will not be kept when return . */
vtkDataArray* getVarArrayVTKFormat(const int varID, const char* filename)
{
    vtkUnstructuredGrid * data = readVTKUnsturctureGridFile(filename);
    vtkPointData * 	pointdata = data->GetPointData ();
    vtkDataArray *curVar= pointdata->GetArray(varID);

    return curVar;

}

vtkUnstructuredGrid * readVTKUnsturctureGridFile(const char* filename)
{
    vtkSmartPointer<vtkXMLMultiBlockDataReader> reader =
    vtkSmartPointer<vtkXMLMultiBlockDataReader>::New();

    reader->SetFileName(filename);
    reader->Update();

    //auto multiBlockReader = vtkMultiBlockDataSet::SafeDownCast(reader->GetOutput());
    auto multiBlockReader = vtkMultiBlockDataSet::SafeDownCast(vtkMultiBlockDataSet::SafeDownCast(reader->GetOutput())->GetBlock(0));
    //Merge Blocks
    auto blockNum = multiBlockReader->GetNumberOfBlocks();
    auto append = vtkSmartPointer<vtkAppendFilter>::New();
    for (unsigned int i = 0; i < blockNum; i++)
        append->AddInputData(vtkRectilinearGrid::SafeDownCast(multiBlockReader->GetBlock(i)));
    append->Update();

    vtkUnstructuredGrid * data = append->GetOutput();
    return data;
}


/**
 * @brief vtkXMLStructureGrid files output. Write an intarray to outfilename with inFilename as a template.
 *  @param parameter's names define their meaning
*/
void addVarArrayToFile(const char* inFilename, const char* outFilename, vtkIntArray * typeArray, vtkFloatArray *scaleArray)
{
    vtkUnstructuredGrid * data = readVTKUnsturctureGridFile(inFilename);
    vtkPointData * 	pointdata = data->GetPointData ();

    pointdata->AddArray(typeArray);

    if(scaleArray)
         pointdata->AddArray(scaleArray);


    vtkSmartPointer<vtkXMLUnstructuredGridWriter> writer =
            vtkSmartPointer<vtkXMLUnstructuredGridWriter>::New();
    writer->SetInputData(data);
    writer->SetFileName(outFilename);
    writer->Write();

}



