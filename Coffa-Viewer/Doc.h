#ifndef DOC_H
#define DOC_H

#include "InteractiveShape.h"

//Std C++
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <concurrent_vector.h>

#include <QtWidgets/QMdiArea>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFormLayout>
#include <QtCore\qobject.h>
#include <QtCore\qlist.h>
#include <QtCore/QStringList>
#include <QtCore/QFileInfo>
#include <AIS_InteractiveContext.hxx>
#include <AIS_SequenceOfInteractive.hxx>
#include <V3d_Viewer.hxx>
#include <AIS_Shape.hxx>
#include <AIS_Plane.hxx>
#include <Graphic3d_MaterialAspect.hxx>
#include <gp_EulerSequence.hxx>
#include <gp_Mat.hxx>
#include <gp_Quaternion.hxx>
#include <QtDataVisualization/q3dsurface.h>
//#include <QtDataVisualization>
#include <StlAPI.hxx>
#include <StlAPI_Reader.hxx>
#include <RWStl.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_DataMapofIntegerShape.hxx>
#include <QtWidgets/qprogressdialog.h>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Compound.hxx>
#include <AIS_Trihedron.hxx>
#include <Geom_Axis2Placement.hxx>
#include <GeomAPI_ExtremaCurveCurve.hxx>
#include <QtCore\qthread.h>
#include <QtCore\qelapsedtimer.h>

using namespace concurrency;
using namespace QtDataVisualization;

class OrientationProject;

class /*__declspec(dllexport)*/  Doc:public QThread
{
	Q_OBJECT
public:
	Doc(const int, OrientationProject*);
	~Doc();
	OrientationProject* myApp;
	int ComputerThreads;
	int ThreadsToUse;
	void				AddPart(QString PartName);
	void				RemovePart();
	QList<InteractiveShape*>				getListOfShapes();
	void               SomePartisPressed();
	void				displayCAD(InteractiveShape* pShape);
	void				onDesirabilityComputation(int aSequence, double angStep1, double angStep2, QList<bool> ARofInterest);
	void				onCustDesirabilityComputation(int aSequence, double angX, double angY, double angZ, QList<bool> ARofInterest);
	void				drawResponseSurfaces(int aSequence, double angStep1, double angStep2);
	void				rotateCurrentShape(int aSequence, double angX, double angY, double angZ);
	void translateCurrentShape(double mx, double my, double mz);
	void				Platform();
	void				PlatformOn();
	void				PlatformOff();

	void checkPartFitnessTool();

	OrientationProject*       getApplication();
	Handle(AIS_InteractiveContext) getContext();
	Handle(AIS_Shape)              AISPart;
	void                           fitAll();
	void						ExportFileSTEP(Standard_CString aFile3, int theShapeID);
	void				WriteBRepFile(Standard_CString aFile3, int theShapeID);
	void						ExportFileSTL(Standard_CString aFile3, int theShapeID);
	void						SaveProject(Standard_CString aFile3);
	
	void generateRTS(double ang, double space1, double space2);

	void simulateSSDR(double d, double ang);

	QList<QList<QList<QPointF>>>		aValueDesiList;
	bool						getIdCheck();

	Handle(V3d_Viewer)		myViewer;
	
	void							RedisplayAction();

	void SelFromViewer();

	bool PlatformVisible;
	QList<QList<QList<QList<double>>>> *theListofDesiValues;
	QList<QList<QList<int>>> *theFidList;

	int IOid, Shapeid;

	void				SetupSelectionModes();
	void				RestoreSelectionModes();

	//Orientation Computation Tools
	int									theAxis1;
	int									theAxis2;

	Handle(AIS_Shape) CompAISRed, CompAISYellow, CompAISGreen, CompAISBlue, CompAISRoad;

	
	double PrinterX = 300;
	double PrinterY = 300;
	double PrinterZ = 300;
	double PrinterThreshold;

private:
	Handle(V3d_Viewer)             Viewer(const Standard_ExtString theName,
	const Standard_CString theDomain,
	const Standard_Real theViewSize, const V3d_TypeOfOrientation theViewProj);

signals:
	void                           selectionChanged();
	void                           sendCloseDocument(Doc*);
Q_SIGNALS:
	void							progressChanged(int info);
	void							progressFinished(int info);
public slots:
	virtual void                   onDelete();
	void onCheckFitness();

	

protected:
	
	
	Handle(AIS_InteractiveContext)			myContext;
	Handle(AIS_Trihedron) aTrihedron;
	int										myIndex;
	int										myNbViews;
	double									myTotArea;
	double									EstimTime;
	Standard_CString						aFile;
	TopoDS_Shape							aShape;
	Handle(AIS_InteractiveObject)		anIOAISPart, anIO2, PlatformIO;
	Handle(AIS_Shape) PAISWire2, PAISEdge1, PAISEdge2, PAISEdge3, PAISEdge4, PAISEdgeA, PAISEdgeB, PAISEdgeC;
	

	AIS_SequenceOfInteractive SeqAll;
	QList<AIS_SequenceOfInteractive> ListofSeqAll;
	
	Q3DSurface *SurfResponse;
	QSurface3DSeries *myResultSeries;

	QList<QStringList> theFStringList;
	QList<TopTools_IndexedMapOfShape> ListofMaps;
	TopTools_IndexedMapOfShape indexMap2;
	TopTools_IndexedMapOfShape indexMap3;
	TopTools_IndexedMapOfShape indexMap8;//SSDR
	TopTools_IndexedMapOfShape indexMapSupport;
	TopTools_IndexedMapOfShape indexSLI;
	TopTools_IndexedMapOfShape indexDF;
	TopTools_IndexedMapOfShape indexMapTriangle;
	TopoDS_ListOfShape myShapeList;
	TopoDS_ListOfShape DatumList;
	TopoDS_ListOfShape AsIsList;
	TopoDS_ListOfShape SuppList, SliList, SliListBuf, SliListBuf2, DFList;
	TopoDS_ListOfShape SSDRList, SSDRListRed, SSDRListYellow, SSDRListGreen, SSDRListBlue;
	TopoDS_ListOfShape SSDRListEdge;

	concurrent_vector<double>		DistSSDR;
	QList<double>		AngSSDR;
	QList<double>		AreaSSDR;
	QList<gp_Dir>		DirSSDR;
	double myAreaSSDR;
	TopoDS_Compound SSDR_RoadComp, SSDR_FaceComp, SSDR_FaceCompRed, SSDR_FaceCompYellow, SSDR_FaceCompGreen, SSDR_FaceCompBlue;

	Handle(AIS_InteractiveObject) CompIOSSDR, CompIOSSDR1;
	Handle(AIS_InteractiveObject) CompIOSupport;
	
	

	QList<QList<TopTools_IndexedMapOfShape>> *theListofIndexMap;
	QList<QList<AIS_SequenceOfInteractive>> *theListofISequence;
	

	QList<double> TabAng1;
	QList<double> TabAng2;
	QProgressDialog	*myProgressRead;
	int idSame1, idSame2;
	bool idCheck, idCheck2, idCheck3, idCheck4, idCheck41;
	TopoDS_Shape uShape;
	TopoDS_Shape PlateShape; TopoDS_Face Plateface;
	double Pxx;
	double Pyy;
	double Pzz;
	double TabShad[1296];
	
	
	QList<QList<TopoDS_Wire>> *OrgaSliWires;
	QList<QList<QList<int>>> *RiskyLayers;
	QList<double> MaxLengthList;
	QList<int> *NAboveThis;
	QList<QList<double>> *ShadowValues;
	QList<double > ShadowValues2;
	QList<double >PredictedShadowVal;

	QList<QList<double>> *ListofMaxLengthList;

	double TotalLength;
	double MaxLengthOfi;
	Handle(AIS_InteractiveObject) LayerIO;
	bool STOP_COMPUTE;
	bool SSDR_EXIST;
	bool DEFORM_EXIST;
	
	//int loopOc=0;
	int PressedOccurence = 1;
	int FacePressedOccurence = 0;

	QDialog* fitnessDialog;
	QFormLayout* fitnessLayout;

	QList<InteractiveShape*> listOfShapes;

	QLabel* dimLabel, *msgLabelX, *msgLabelY, * msgLabelZ;
};
#endif