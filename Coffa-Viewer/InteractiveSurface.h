#pragma once

#include "CF_Triangle.h"

#include <QtCore/QString>
#include <QtConcurrent/qtconcurrentrun.h>
#include <QtCore\qthread.h>
#include <QtCore\QFuture>

#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Vec.hxx>
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

	InteractiveSurface(TopoDS_Face aShape);

	void	setFace(TopoDS_Face aFace);
	void	setName(QString aName);
	void	setID(QString id);
	TopoDS_Face	 getFace();
	QString getName();
	QString			 getID();
	Handle(AIS_InteractiveObject) getInteractiveObject();

	double theArea;
	QList<CF_Triangle> theTriangles;

	double getArea();

	QList<CF_Triangle> Triangulate();
	void setTriangles(QList<CF_Triangle> allTriangles);
	QList<CF_Triangle> getTriangles();

	double getXmin();

	double getXmax();

	double getYmin();

	double getYmax();

	double getZmin();

	double getZmax();

	void Pixelize(double stepElement);

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
	
	bool alreadyComputedArea;
};

