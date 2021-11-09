#ifndef DOC_H
#define DOC_H


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

#include <QtCore\qthread.h>
#include <QtCore\qelapsedtimer.h>

class CoffaViewer;
class InteractiveShape;

class  Doc:public QObject
{
	Q_OBJECT
public:
	Doc(CoffaViewer*);
	~Doc();

	void									fitAll();
	void									AddPart(QString PartName);
	void									RemovePart();
	QList<InteractiveShape*>				getListOfShapes();
	void									displayCAD(InteractiveShape* pShape);
	void									rotateCurrentShape(int aSequence, double angX, double angY, double angZ);
	void									translateCurrentShape(double mx, double my, double mz);
	void									Platform();
	void									PlatformOn();
	void									PlatformOff();
	void									onUpdatePlate(double plateX, double plateY, double plateZ);

	CoffaViewer*							getApplication();
	Handle(AIS_InteractiveContext)			getContext();

	
	void									ExportFileSTEP(Standard_CString aFile3, int theShapeID);
	void									ExportFileBRep(Standard_CString aFile3, int theShapeID);
	void									ExportFileSTL(Standard_CString aFile3, int theShapeID);
	
	void									SetupSelectionModes();
	void									RestoreSelectionModes();

	CoffaViewer* myApp;
	Handle(V3d_Viewer)						myViewer;
	
public:
	bool PlatformVisible;
	int IOid, Shapeid;

	double PlateX = 300;
	double PlateY = 300;
	double PlateZ = 300;

private:
	Handle(V3d_Viewer)             Viewer(const Standard_ExtString theName,
	const Standard_CString theDomain,
	const Standard_Real theViewSize, const V3d_TypeOfOrientation theViewProj);

protected:
		
	Handle(AIS_InteractiveContext)			myContext;
	Handle(AIS_Trihedron) aTrihedron;

	Handle(AIS_InteractiveObject)		anIOAISPart, anIO2, PlatformIO;
	Handle(AIS_Shape) PAISWire2, PAISEdge1, PAISEdge2, PAISEdge3, PAISEdge4, PAISEdgeA, PAISEdgeB, PAISEdgeC;


	QDialog* fitnessDialog;
	QFormLayout* fitnessLayout;

	QList<InteractiveShape*> listOfShapes;


	QLabel* dimLabel, *msgLabelX, *msgLabelY, * msgLabelZ;
};
#endif