#include "CF_Triangle.h"


using namespace std;


//Constructs a triangle from points P1, P2 and P3
CF_Triangle::CF_Triangle(gp_Pnt& P1, gp_Pnt& P2, gp_Pnt& P3)
{
	Pn1 = P1;
	Pn2 = P2;
	Pn3 = P3;

	originPn1 = Pn1;
	originPn2 = Pn2;
	originPn3 = Pn3;

	QList<double> allx;
	allx.push_back(P1.X());
	allx.push_back(P2.X());
	allx.push_back(P3.X());
	std::sort(allx.begin(), allx.end());
	xmin = allx.first(); xmax = allx.last();

	QList<double> ally;
	ally.push_back(P1.Y());
	ally.push_back(P2.Y());
	ally.push_back(P3.Y());
	std::sort(ally.begin(), ally.end());
	ymin = ally.first(); ymax = ally.last();

	QList<double> allz;
	allz.push_back(P1.Z());
	allz.push_back(P2.Z());
	allz.push_back(P3.Z());
	std::sort(allz.begin(), allz.end());
	zmin = allz.first(); zmax = allz.last();

	gp_Vec Vec1(Pn1, Pn2);
	gp_Vec Vec2(Pn1, Pn3);
	gp_Vec myNorm = Vec1.Crossed(Vec2);

	theArea = 0.5 * (myNorm.Magnitude());
	if (theArea >= 0.01)
	{
		theUnitNormal = gp_Dir(myNorm);
		theOriginUnitNormal = theUnitNormal;

		alreadyComputedUnitNormal = true;
		tooSmall = false;

		double vec1x = Pn1.X() - Pn2.X(); double vec1y = Pn1.Y() - Pn2.Y(); double vec1z = Pn1.Z() - Pn2.Z();
		double vec2x = Pn2.X() - Pn3.X(); double vec2y = Pn2.Y() - Pn3.Y(); double vec2z = Pn2.Z() - Pn3.Z();
		double vec3x = Pn3.X() - Pn1.X(); double vec3y = Pn3.Y() - Pn1.Y(); double vec3z = Pn3.Z() - Pn1.Z();

		double aa = sqrt((vec1x * vec1x) + (vec1y * vec1y) + (vec1z * vec1z));
		double bb = sqrt((vec2x * vec2x) + (vec2y * vec2y) + (vec2z * vec2z));
		double cc = sqrt((vec3x * vec3x) + (vec3y * vec3y) + (vec3z * vec3z));
		double Xin = ((aa * Pn1.X()) + (bb * Pn2.X()) + (cc * Pn3.X())) / (aa + bb + cc);
		double Yin = ((aa * Pn1.Y()) + (bb * Pn2.Y()) + (cc * Pn3.Y())) / (aa + bb + cc);
		double Zin = ((aa * Pn1.Z()) + (bb * Pn2.Z()) + (cc * Pn3.Z())) / (aa + bb + cc);

		inCenter = gp_Pnt(Xin, Yin, Zin);
		originInCenter = inCenter;
		//MapUV(1.0);
	}
	

}

//Default constructor
CF_Triangle::CF_Triangle()
{
	Pn1.SetX(0); Pn1.SetY(0); Pn1.SetZ(0);
	Pn2.SetX(0); Pn2.SetY(0); Pn2.SetZ(0);
	Pn3.SetX(0); Pn3.SetY(0); Pn3.SetZ(0);
	originPn1 = Pn1;
	originPn2 = Pn2;
	originPn3 = Pn3;

	alreadyComputedUnitNormal = false;
}

CF_Triangle::~CF_Triangle()
{

}

//Returns a vector normal to thisTriangle
gp_Vec CF_Triangle::getNormal()
{
	gp_Vec Vec1(Pn1, Pn2);
	gp_Vec Vec2(Pn1, Pn3);
	gp_Vec myNorm = Vec1.Crossed(Vec2);

	return myNorm;
}

gp_Dir CF_Triangle::getUnitNormal()
{
	return theUnitNormal;
}

//Returns all Triangle nodes
QList<gp_Pnt> CF_Triangle::getNodes()
{
	QList<gp_Pnt> aPList;
	aPList.push_back(Pn1);
	aPList.push_back(Pn2);
	aPList.push_back(Pn3);
	return aPList;
}

//Returns the surface area of this Triangle
double CF_Triangle::getArea()
{
	return theArea;
}

//Returns the Incenter of this Triangle
gp_Pnt CF_Triangle::getInCenter()
{
	double vec1x = Pn1.X() - Pn2.X(); double vec1y = Pn1.Y() - Pn2.Y(); double vec1z = Pn1.Z() - Pn2.Z();
	double vec2x = Pn2.X() - Pn3.X(); double vec2y = Pn2.Y() - Pn3.Y(); double vec2z = Pn2.Z() - Pn3.Z();
	double vec3x = Pn3.X() - Pn1.X(); double vec3y = Pn3.Y() - Pn1.Y(); double vec3z = Pn3.Z() - Pn1.Z();

	double aa = sqrt((vec1x * vec1x) + (vec1y * vec1y) + (vec1z * vec1z));
	double bb = sqrt((vec2x * vec2x) + (vec2y * vec2y) + (vec2z * vec2z));
	double cc = sqrt((vec3x * vec3x) + (vec3y * vec3y) + (vec3z * vec3z));
	double Xin = ((aa * Pn1.X()) + (bb * Pn2.X()) + (cc * Pn3.X())) / (aa + bb + cc);
	double Yin = ((aa * Pn1.Y()) + (bb * Pn2.Y()) + (cc * Pn3.Y())) / (aa + bb + cc);
	double Zin = ((aa * Pn1.Z()) + (bb * Pn2.Z()) + (cc * Pn3.Z())) / (aa + bb + cc);

	return gp_Pnt(Xin, Yin, Zin);
}

//returns true if the nodes are Counter ClockWise
bool CF_Triangle::isCounterClkWise()
{
	gp_Vec Vec1(Pn1, Pn2);
	gp_Vec Vec2(Pn1, Pn3);
	double u1 = Vec1.X();
	double u2 = Vec1.Y();
	double u3 = Vec1.Z();

	double v1 = Vec2.X();
	double v2 = Vec2.Y();
	double v3 = Vec2.Z();

	double sArea = ((u2 * v3) - (u3 * v2)) + ((u3 * v1) - (u1 * v3)) + ((u1 * v2) - (u2 * v1));

	bool eq = false;
	if (sArea > 0)
		eq = true;


	return eq;
}

double CF_Triangle::getXmin()
{
	return xmin;
}

double CF_Triangle::getXmax()
{
	return xmax;
}

double CF_Triangle::getYmin()
{
	return ymin;
}

double CF_Triangle::getYmax()
{
	return ymax;
}

double CF_Triangle::getZmin()
{
	return zmin;
}

double CF_Triangle::getZmax()
{
	return zmax;
}

//Returns the intersection point between the lying plane of the triangle 
//and the line given by the unit vector aDir and the point aLoc
gp_Pnt CF_Triangle::getIntersectionPoint(gp_Dir aDir, gp_Pnt aLoc)
{
	double d = (-theUnitNormal.X() * Pn1.X()) + (-theUnitNormal.Y() * Pn1.Y()) + (-theUnitNormal.Z() * Pn1.Z());
	double a = theUnitNormal.X(); double b = theUnitNormal.Y(); double c = theUnitNormal.Z();
	double a1 = aDir.X(); double b1 = aDir.Y(); double c1 = aDir.Z();
	double t = (-1 * (d + (a * aLoc.X()) + (b * aLoc.Y()) + (c * aLoc.Z()))) / ((a * a1) + (b * b1) + (c * c1));
	double ix = aLoc.X() + (a1 * t); double iy = aLoc.Y() + (b1 * t); double iz = aLoc.Z() + (c1 * t);
	gp_Pnt IntPnt(ix, iy, iz);

	return IntPnt;
}

//Returns true if the point aPoint is inside the triangle
bool CF_Triangle::isPointInside(gp_Pnt aPoint)
{
	bool eq = false;
	gp_Vec aVec(aPoint, Pn1);
	gp_Vec bVec(aPoint, Pn2);
	gp_Vec cVec(aPoint, Pn3);

	//Check whether inside of over the contour
	if (aVec.Magnitude() >= 0.02 && bVec.Magnitude() >= 0.02 && cVec.Magnitude() >= 0.02)
	{
		double A = aVec.Angle(bVec);
		double B = bVec.Angle(cVec);
		double C = cVec.Angle(aVec);
		double AngTot = A + B + C;

		if (abs(AngTot - 6.28) <= 0.02)
		{
			eq = true;
		}
	}

	return eq;
}

//Returns true if the point aPoint is on the contour of the triangle
bool CF_Triangle::isPointOnContour(gp_Pnt aPoint)
{
	bool eq = false;

	if (aPoint.IsEqual(Pn1, 0.01) || aPoint.IsEqual(Pn2, 0.01) || aPoint.IsEqual(Pn3, 0.01))
	{
		eq = true;
	}

	else
	{
		gp_Vec aVec(aPoint, Pn1);
		gp_Vec bVec(aPoint, Pn2);
		gp_Vec cVec(aPoint, Pn3);

		//Check whether inside of over the contour
		if (aVec.Magnitude() >= 0.02 && bVec.Magnitude() >= 0.02 && cVec.Magnitude() >= 0.02)
		{
			if (aVec.CrossMagnitude(bVec) <= 0.0001 ||
				aVec.CrossMagnitude(cVec) <= 0.0001 ||
				cVec.CrossMagnitude(bVec) <= 0.0001)
			{
				//qDebug()<<"Magni" << aVec.CrossMagnitude(bVec);
				eq = true;
			}

		}
	}


	return eq;
}

void CF_Triangle::transformTriangle(gp_Trsf aTransformer)
{
	Pn1= originPn1.Transformed(aTransformer);
	Pn2= originPn2.Transformed(aTransformer);
	Pn3= originPn3.Transformed(aTransformer);
	inCenter= originInCenter.Transformed(aTransformer);

	theUnitNormal = theOriginUnitNormal.Transformed(aTransformer);

	QList<double> allx;
	allx.push_back(Pn1.X());
	allx.push_back(Pn2.X());
	allx.push_back(Pn3.X());
	std::sort(allx.begin(), allx.end());
	xmin = allx.first(); xmax = allx.last();

	QList<double> ally;
	ally.push_back(Pn1.Y());
	ally.push_back(Pn2.Y());
	ally.push_back(Pn3.Y());
	std::sort(ally.begin(), ally.end());
	ymin = ally.first(); ymax = ally.last();

	QList<double> allz;
	allz.push_back(Pn1.Z());
	allz.push_back(Pn2.Z());
	allz.push_back(Pn3.Z());
	std::sort(allz.begin(), allz.end());
	zmin = allz.first(); zmax = allz.last();
}

bool CF_Triangle::isTooSmall()
{
	return tooSmall;
}
