QT       += core gui
QT       +=  charts concurrent
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets opengl printsupport

CONFIG += c++11


TARGET = TransferFunc
TEMPLATE = app

SOURCES += \
    DiagDataAnalyzer.cpp \
    TFPrimitive.cpp \
    TransferFunction/TFDesigner.cpp \
    TransferFunction/TransferFunction.cpp \
    VulkanWindow.cpp \
    datainfo.cpp \
    iohelper.cpp \
    main.cpp \
    mainwindow.cpp \
    qcustomplot.cpp \
    shader/trianglerenderer.cpp

HEADERS += \
    DiagDataAnalyzer.h \
    Mat.h \
    MathUtil.h \
    TFPrimitive.h \
    TransferFunction/TFDesigner.h \
    TransferFunction/TransferFunction.h \
    VulkanWindow.h \
    datainfo.h \
    iohelper.h \
    mainwindow.h \
    qcustomplot.h \
    shader/trianglerenderer.h

FORMS += \
    DiagDataAnalyzer.ui \
    TransferFunction/TFDesigner.ui \
    TransferFunction/tempTFDesigner.ui \
    mainwindow.ui


Data.files += $$DISTFILES

macx {
    Data.path = Contents/MacOS
    QMAKE_BUNDLE_DATA += Data
    QMAKE_MAC_SDK = macosx10.9
}

win32 {
    Data.path = $$OUT_PWD
    INSTALLS += Data

# namespace lmj
# opengl include

#vulkan
    INCLUDEPATH += D:/vulkan/1.2.176.1/Include
    INCLUDEPATH += D:/vulkan/1.2.176.1/Lib
    DEPENDPATH += D:/vulkan/1.2.176.1/Lib

LIBS += $$quote(D:\vulkan\1.2.176.1\Lib\dxcompiler.lib)
LIBS += $$quote(D:\vulkan\1.2.176.1\Lib\GenericCodeGen.lib)
LIBS += $$quote(D:\vulkan\1.2.176.1\Lib\glslang.lib)
LIBS += $$quote(D:\vulkan\1.2.176.1\Lib\HLSL.lib)
LIBS += $$quote(D:\vulkan\1.2.176.1\Lib\MachineIndependent.lib)
LIBS += $$quote(D:\vulkan\1.2.176.1\Lib\OGLCompiler.lib)
LIBS += $$quote(D:\vulkan\1.2.176.1\Lib\OSDependent.lib)
LIBS += $$quote(D:\vulkan\1.2.176.1\Lib\shaderc.lib)
LIBS += $$quote(D:\vulkan\1.2.176.1\Lib\shaderc_combined.lib)
LIBS += $$quote(D:\vulkan\1.2.176.1\Lib\shaderc_shared.lib)
LIBS += $$quote(D:\vulkan\1.2.176.1\Lib\shaderc_util.lib)
LIBS += $$quote(D:\vulkan\1.2.176.1\Lib\spirv-cross-c-shared.lib)
LIBS += $$quote(D:\vulkan\1.2.176.1\Lib\spirv-cross-c.lib)
LIBS += $$quote(D:\vulkan\1.2.176.1\Lib\spirv-cross-core.lib)
LIBS += $$quote(D:\vulkan\1.2.176.1\Lib\spirv-cross-cpp.lib)
LIBS += $$quote(D:\vulkan\1.2.176.1\Lib\spirv-cross-glsl.lib)
LIBS += $$quote(D:\vulkan\1.2.176.1\Lib\spirv-cross-hlsl.lib)
LIBS += $$quote(D:\vulkan\1.2.176.1\Lib\spirv-cross-msl.lib)
LIBS += $$quote(D:\vulkan\1.2.176.1\Lib\spirv-cross-reflect.lib)
LIBS += $$quote(D:\vulkan\1.2.176.1\Lib\spirv-cross-util.lib)
LIBS += $$quote(D:\vulkan\1.2.176.1\Lib\SPIRV-Tools-link.lib)
LIBS += $$quote(D:\vulkan\1.2.176.1\Lib\SPIRV-Tools-opt.lib)
LIBS += $$quote(D:\vulkan\1.2.176.1\Lib\SPIRV-Tools-reduce.lib)
LIBS += $$quote(D:\vulkan\1.2.176.1\Lib\SPIRV-Tools-shared.lib)
LIBS += $$quote(D:\vulkan\1.2.176.1\Lib\SPIRV-Tools.lib)
LIBS += $$quote(D:\vulkan\1.2.176.1\Lib\SPIRV.lib)
LIBS += $$quote(D:\vulkan\1.2.176.1\Lib\SPVRemapper.lib)
LIBS += $$quote(D:\vulkan\1.2.176.1\Lib\VkLayer_utils.lib)
LIBS += $$quote(D:\vulkan\1.2.176.1\Lib\vulkan-1.lib)

#vtk
    LIBS +=  -LC:/VTK/vtk_debug/lib  -lvtkIOXML-9.1d -lvtkCommonDataModel-9.1d -lvtkCommonCore-9.1d
    INCLUDEPATH += C:/VTK/vtk_debug/include/vtk-9.1
    DEPENDPATH += C:/VTK/vtk_debug/lib

win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkChartsCore-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkCommonColor-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkCommonComputationalGeometry-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkCommonCore-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkCommonDataModel-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkCommonExecutionModel-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkCommonMath-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkCommonMisc-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkCommonSystem-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkCommonTransforms-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkDICOMParser-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkDomainsChemistry-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkdoubleconversion-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkexodusII-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkexpat-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkFiltersAMR-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkFiltersCore-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkFiltersExtraction-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkFiltersFlowPaths-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkFiltersGeneral-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkFiltersGeneric-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkFiltersGeometry-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkFiltersHybrid-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkFiltersHyperTree-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkFiltersImaging-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkFiltersModeling-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkFiltersParallel-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkFiltersParallelImaging-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkFiltersPoints-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkFiltersProgrammable-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkFiltersSelection-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkFiltersSMP-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkFiltersSources-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkFiltersStatistics-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkFiltersTexture-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkFiltersTopology-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkFiltersVerdict-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkfreetype-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkGeovisCore-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkgl2ps-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkglew-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkGUISupportQt-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkhdf5-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkhdf5_hl-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkImagingColor-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkImagingCore-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkImagingFourier-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkImagingGeneral-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkImagingHybrid-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkImagingMath-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkImagingMorphological-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkImagingSources-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkImagingStatistics-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkImagingStencil-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkInfovisCore-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkInfovisLayout-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkInteractionImage-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkInteractionStyle-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkInteractionWidgets-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOAMR-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOAsynchronous-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOCityGML-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOCore-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOEnSight-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOExodus-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOExport-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOExportGL2PS-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOExportPDF-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOGeometry-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOImage-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOImport-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOInfovis-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOLegacy-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOLSDyna-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOMINC-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOMotionFX-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOMovie-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIONetCDF-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOOggTheora-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOParallel-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOParallelXML-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOPLY-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOSegY-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOSQL-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOTecplotTable-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOVeraOut-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOVideo-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOXML-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOXMLParser-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkjpeg-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkjsoncpp-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtklibharu-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtklibproj-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtklibxml2-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkloguru-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtklz4-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtklzma-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkmetaio-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtknetcdf-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkogg-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkParallelCore-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkParallelDIY-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkpng-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkpugixml-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkRenderingAnnotation-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkRenderingContext2D-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkRenderingCore-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkRenderingFreeType-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkRenderingGL2PSOpenGL2-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkRenderingImage-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkRenderingLabel-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkRenderingLOD-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkRenderingOpenGL2-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkRenderingSceneGraph-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkRenderingUI-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkRenderingVolume-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkRenderingVolumeOpenGL2-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkRenderingVtkJS-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtksqlite-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtksys-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkTestingRendering-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtktheora-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtktiff-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkverdict-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkViewsContext2D-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkViewsCore-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkViewsInfovis-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkWrappingTools-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkzlib-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkcgns-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkViewsQt-9.1d.lib)
win32:LIBS += $$quote(C:\VTK\vtk_debug\lib\vtkIOCGNSReader-9.1d.lib)


#itself
    INCLUDEPATH += $$PWD/TransferFunction
    DESTDIR = $$OUT_PWD
}

unix:!macx {
    Data.path = $$OUT_PWD
    INSTALLS += Data
    LIBS += -lGLEW

}

msvc {
  QMAKE_CXXFLAGS += -openmp -arch:AVX -D "_CRT_SECURE_NO_WARNINGS"
  QMAKE_CXXFLAGS_RELEASE *= -O2
}

RESOURCES += \
    shader/hellovulkantriangle.qrc \
    xvrresource.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:LIBS += -lOpenGL32 -luser32 -lGdi32

DISTFILES += \
    shader/color.frag \
    shader/color.vert


