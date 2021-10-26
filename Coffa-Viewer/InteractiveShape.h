#pragma once
#include "InteractiveSurface.h"
#include "SupportStructure.h"
#include "CF_Triangle.h"
#include "CF_Pixel.h"
#include "CF_Edge.h"
#include "ActionRule.h"
#include "VarARValues.h"
#include "VarARData.h"
#include "VarIDValue.h"
#include "VarAttribute.h"
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

#include <QtDataVisualization/Q3DSurface>
#include <QtDataVisualization/QSurface3DSeries>

#include <concurrent_vector.h>

using namespace QtDataVisualization;
using namespace concurrency;

class Doc;
class ActionRule;

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

	void setAvailableThreads(int threadsCount);

	void organizeByCompartiments();

	void organizeByCloseTriangles();

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

	void createResultWidget();

	QWidget* getResultWidget();

	void setActionRules(QList<ActionRule*> someRules);

	VarAttribute getAttributedSurfaces(QString arID);

	void setAttributedSurfaces(VarAttribute aVarAttrib);

	void showAttributedSurfaces(QString arID);

	void hideAttributedSurfaces(QString arID);

	void hideAllAttributedSurfaces();

	void evaluateRules(QList<bool> interest);
	void rotateAndEvaluateRules(gp_Quaternion aQuaternion, double xang, double yang, double zang, QList<bool> interest,
		AxisSequence aseq, int it1, int it2, int compMode);

	double currentRx();
	double currentRy();
	double currentRz();

	void genRTS(double thresholdAngle, double space);
	void genRTS2(double thresholdAngle, double space, double space2);
	void genRTS3(double thresholdAngle, double space, double space2);
	void genRTS(double thresholdAngle, double space, double space2);
	void generateSupport(double h, double w, SupportStructure::ConnectionType  ctype,
		SupportStructure::PerforationType  ptype);

	void simulateSSDR(double d, double ang);

	QList<VarIDValue> evaluateBySurfAngle(ActionRule* aRule, int whichFunc);
	QList<VarIDValue> evaluateByDistance(ActionRule* aRule, int whichFunc);
	QList<VarIDValue> evaluateBySurfAngleAndDistance(ActionRule* aRule, int whichFunc);
	double evaluateByShadow(ActionRule* aRule, int whichFunc);
	double evaluateByStackSliceLength(ActionRule* aRule, int whichFunc);

	void visualizeDistribution(QString id);

	QList<VarARValues> theComputedValues;
	QList<VarARValues> theCustComputedValues;

	///
	
	QList<gp_Pnt> organizePointCCWiseUV(QList<gp_Pnt> aList);

	QList<gp_Pnt> outerBoundOfPixels(QList<CF_Pixel> aList);

	double getTotalArea();
	double getLimitedTotalArea(ActionRule* aRule);
	void rotateShape2(gp_Quaternion aQuaternion, double angx, double angy, double angz);
	void rotateShape(gp_Quaternion aQuaternion, double angx, double angy, double angz);
	void translatePart(double mx, double my, double mz);
	QList<QList<gp_Pnt>> sliceTriangle(gp_Pnt Pnt1, gp_Pnt Pnt2, gp_Pnt Pnt3, gp_Dir aNormal,
		gp_Dir SliceDirection, double zMin, double zMax, double space);
	double sliceIntersectMeasure();

	QList<gp_Pnt> EdgeSorting(concurrent_vector<CF_Edge> thelistIn, int LayerId);

	QList<gp_Pnt> contourIntersection(QList<gp_Pnt> PointList1, QList<gp_Pnt> PointList2);

	QList<double> hatchMeasurement(QList<gp_Pnt> PointList);

	double Tx, Ty, Tz;

	Handle(AIS_InteractiveObject) IObject;
	Handle(AIS_InteractiveObject) RTSObject;
	Handle(AIS_InteractiveObject) SupportObject;
	Handle(AIS_InteractiveObject) SSDRObject;
	Handle(AIS_InteractiveObject) VisuObject;

	bool someIsSelected=false;

	double ssdrPrepTime=0;
	QElapsedTimer *ssdrPrepTimer;
	int t1 = 1;

	QList<QList<VarIDValue>> theCompartiments;
	bool organizedByXCompartiments=false;
	bool organizedByYCompartiments=false;
	bool organizedByZCompartiments=false;

	QList<InteractiveSurface*> theSurfaces;
	int threadCount = 1;
private:
	TopoDS_Shape theShape;
	TopoDS_Shape theProcessingShape;
	TopoDS_Shape theOriginShape;
	Doc* theDoc;
	QString ID;
	QString theName;
	QList<InteractiveSurface*> theOriginSurfaces;
	
	double Rx, Ry, Rz;
	

	QList<ActionRule*> theActionRules;
	bool alreadyComputedArea;
	double thetotalArea;

	QStandardItem *mainItem, *geomItem, *suppItem;

	QList<QList<gp_Pnt>> RTSPath;
	QList<VarAttribute> theAttributes;
	QDialog* attributeDialog;

	int ThreadsToUse=0;

	gp_Trsf rotateTrsf;
	gp_Trsf transtateTrsf;
	gp_Quaternion rotQuaternion;
	gp_Vec trlVector;

	int layPosx=1, layPosy = 0;
	QGridLayout* ResultLayout;
	QWidget* ResultWidget;
};

