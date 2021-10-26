#pragma once
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Dir.hxx>
#include <QtCore/QList>

#include <algorithm>
//#include <qdebug.h>

using namespace std;


class  CF_Triangle
{
public:
	CF_Triangle(gp_Pnt& P1, gp_Pnt& P2, gp_Pnt& P3);
	CF_Triangle();
	~CF_Triangle();

	gp_Vec					getNormal();
	gp_Dir					getUnitNormal();
	QList<gp_Pnt>			getNodes();
	double						getArea();
	gp_Pnt					getInCenter();
	bool						isCounterClkWise();

	int id;
	double minDistance = 200;
	double getXmin();

	double getXmax();

	double getYmin();

	double getYmax();

	double getZmin();

	double getZmax();

	gp_Pnt getIntersectionPoint(gp_Dir aDir, gp_Pnt aLoc);

	bool isPointInside(gp_Pnt aPoint);

	bool isPointOnContour(gp_Pnt aPoint);

	void transformTriangle(gp_Trsf aTransformer);

	bool isTooSmall();

	gp_Dir theUnitNormal;
	gp_Pnt Pn1, Pn2, Pn3;
	gp_Pnt inCenter;
	double theArea;
	double xmin, xmax, ymin, ymax, zmin, zmax;

private:
	gp_Pnt originPn1, originPn2, originPn3;
	bool alreadyComputedUnitNormal;
	gp_Dir theOriginUnitNormal;
	gp_Pnt originInCenter;
	bool tooSmall = true;
};

