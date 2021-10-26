#pragma once

#include "CF_Triangle.h"
#include "CF_Pixel.h"
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Vec.hxx>
#include <QtCore/QString>
#include <QtConcurrent/qtconcurrentrun.h>
#include <QtCore\qthread.h>
#include <QtCore\QFuture>

#include <AIS_InteractiveObject.hxx>
#include <BRepTools.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepTopAdaptor_FClass2d.hxx>

#include <BRepAdaptor_Surface.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <GeomAPI_IntCS.hxx>
#include <Geom_Line.hxx>
#include <Geom_Curve.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <TopoDS.hxx>

using namespace QtConcurrent;

class InteractiveSurface: public TopoDS_Face
{
public:
	InteractiveSurface();
	void setAvailableThreads(int threadsCount);
	int threadCount=1;
	InteractiveSurface(TopoDS_Face aShape);

	void	setFace(TopoDS_Face aFace);
	void	setShape(TopoDS_Shape aShape);
	void	setName(QString aName);
	void	setID(QString id);
	void	setAttribute(QString anAttribute);

	TopoDS_Face	 getFace();
	TopoDS_Shape getShape();
	QString getName();
	QString			 getID();
	Handle(AIS_InteractiveObject) getInteractiveObject();

	double theArea;
	bool isFlat=false;
	QList<CF_Triangle> theTriangles;
	QList<CF_Pixel> surfaceMap;
	QList<CF_Pixel> finerMap;
	QList<QList<int>> arrangedSurfMap;
	Handle(Geom_Surface) theGeomSurf;

	double getArea();

	double getAbsoluteArea();

	QList<CF_Triangle> Triangulate();
	void setTriangles(QList<CF_Triangle> allTriangles);
	QList<CF_Triangle> getTriangles();

	void computeMinDistanceAround(QList<InteractiveSurface*> surfAround);
	void computeMinDistanceAround2(QList<InteractiveSurface*> surfAround2);
	void computeMinDistanceAround3(QList<InteractiveSurface*> surfAround);
	QList<CF_Pixel> simulateMinDistanceAround(QList<InteractiveSurface*> surfAround, double ang, double toolSize);
	double getMinDistanceAround(QList<InteractiveSurface*> surfAround);

	bool hasAccessProblems();

	QList<CF_Triangle> getInvolvedTriangles(double thresholdDistance);

	QList<CF_Pixel> getInvolvedPixels(double thresholdDistance);

	double getXmin();

	double getXmax();

	double getYmin();

	double getYmax();

	double getZmin();

	double getZmax();

	QList<CF_Pixel> getDistPixels();

	void Pixelize(double stepElement);

	QList<CF_Pixel> FinerPixelization(double elementSize);

	void transformPixels(gp_Trsf myTransform);

	void transformTriangles(gp_Trsf myTransform);
	bool alreadyPixelized = false;

	double xmin, xmax, ymin, ymax, zmin, zmax;
	QString ID;

private:
	TopoDS_Face theFace;
	TopoDS_Shape theShape;
	
	QString theName;
	Handle(AIS_InteractiveObject) IObject;
	
	
	double theAbsArea;
	bool alreadyComputedArea;
	bool alreadyComputedAbsArea;
	bool alreadyComputedDist;
	bool accessProblem;
	
	double theMinDistanceAround = 0;
	double theMaxDistanceAround = 0;



	QList<CF_Pixel> distPixels;
};

