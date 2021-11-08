#pragma once
#include "InteractiveSurface.h"
#include "Doc.h"

#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS.hxx>
#include <TopExp_Explorer.hxx>
#include <TopExp.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepTools.hxx>
#include <BRepBndLib.hxx>
#include <AIS_InteractiveObject.hxx>
#include <gp_Quaternion.hxx>

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtGui/QStandardItem>
#include <QtWidgets/QDialog>

#include <concurrent_vector.h>

using namespace concurrency;

class Doc;

class InteractiveShape : public TopoDS_Shape
{
public:
	enum CADType {volumeType, surfaceType};
	enum VARType {normalAngle, slicePerimeter, sliceMaxLength, surfaceArea, volumeShadow, surfaceShadow, minimumDistAround};
	enum AXIS {xAxis, yAxis, zAxis};
	enum AxisSequence { XYZ, XZY, YXZ, YZX, ZXY, ZYX };
	InteractiveShape();
	InteractiveShape(TopoDS_Shape aShape, Doc* aDoc);

	void parseFaces();

	QList<InteractiveSurface*> getSurfaces();

	QList<int> getSelectedSurfaces();

	bool isOneFaceSelected();

	bool isSelected();

	void setShape(TopoDS_Shape aShape);
	TopoDS_Shape getShape();

	TopoDS_Shape getOriginShape();

	void setID(QString id);
	QString getID();

	void setName(QString aName);
	QString getName();

	QStandardItem *getTreeItem();

	Handle(AIS_InteractiveObject) getInteractiveObject();

	void removeAllInteractives();

	double currentRx();
	double currentRy();
	double currentRz();

	double getTotalArea();
	void rotateShape(gp_Quaternion aQuaternion, double angx, double angy, double angz);
	void translatePart(double mx, double my, double mz);
	QList<QList<gp_Pnt>> sliceTriangle(gp_Pnt Pnt1, gp_Pnt Pnt2, gp_Pnt Pnt3, gp_Dir aNormal,
		gp_Dir SliceDirection, double zMin, double zMax, double space);

	
	Handle(AIS_InteractiveObject) IObject;

	bool someIsSelected=false;
	QList<InteractiveSurface*> theSurfaces;

	double Rx, Ry, Rz;
	double Tx, Ty, Tz;
private:
	TopoDS_Shape theShape;
	TopoDS_Shape theProcessingShape;
	TopoDS_Shape theOriginShape;
	Doc* theDoc;
	QString ID;
	QString theName;
	QList<InteractiveSurface*> theOriginSurfaces;
	
	

	bool alreadyComputedArea;
	double thetotalArea;

	QStandardItem *mainItem, *geomItem;

	gp_Quaternion rotQuaternion;
	gp_Vec trlVector;

};

