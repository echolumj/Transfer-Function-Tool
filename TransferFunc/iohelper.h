#ifndef IOHELPER
#define IOHELPER

#include <vector>
class vtkDataArray;
class vtkIntArray;
class vtkFloatArray;
class vtkUnstructuredGrid;


std::vector<float>* getVarArray( const int varID, const char* filename);
vtkDataArray* getVarArrayVTKFormat(const int varID, const char* filename);

void addVarArrayToFile(const char* inFilename, const char* outFilename, vtkIntArray * typeArray, vtkFloatArray *scaleArray = NULL);

vtkUnstructuredGrid * readVTKUnsturctureGridFile(const char* filename);

#endif // IOHELPER

