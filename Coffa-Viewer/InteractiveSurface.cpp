#include "InteractiveSurface.h"
#include <omp.h>
#include <concurrent_vector.h>
using namespace std;
using namespace concurrency;

InteractiveSurface::InteractiveSurface()
{
	alreadyComputedArea = false;
	theArea = 0;
	ID = "";
}

InteractiveSurface::InteractiveSurface(TopoDS_Face aFace)
{
	theFace = aFace;
	alreadyComputedArea = false;
	theArea = 0;
	ID = "";

}

void InteractiveSurface::setFace(TopoDS_Face aFace)
{
	theFace = aFace;

	QList<double> allx;
	QList<double> ally;
	QList<double> allz;

	for (int i = 0; i < theTriangles.size(); i++)
	{
		allx.push_back(theTriangles[i].getXmax());
		allx.push_back(theTriangles[i].getXmin());

		ally.push_back(theTriangles[i].getYmax());
		ally.push_back(theTriangles[i].getYmin());

		allz.push_back(theTriangles[i].getZmax());
		allz.push_back(theTriangles[i].getZmin());
	}

	if (!allx.isEmpty())
	{
		std::sort(allx.begin(), allx.end());
		xmin = allx.first(); xmax = allx.last();
	}
	
	if (!ally.isEmpty())
	{
		std::sort(ally.begin(), ally.end());
		ymin = ally.first(); ymax = ally.last();
	}

	if (!allz.isEmpty())
	{
		std::sort(allz.begin(), allz.end());
		zmin = allz.first(); zmax = allz.last();
	}
}

void InteractiveSurface::setName(QString aName)
{
	theName = aName;
}

void InteractiveSurface::setID(QString id)
{
	ID = id;
}

TopoDS_Face InteractiveSurface::getFace()
{
	return theFace;
}

QString InteractiveSurface::getName()
{
	return theName;
}

QString InteractiveSurface::getID()
{
	return ID;
}

Handle(AIS_InteractiveObject) InteractiveSurface::getInteractiveObject()
{
	return IObject;
}

double InteractiveSurface::getArea()
{
	if (!alreadyComputedArea)
	{
		TopLoc_Location bLocation;
		Handle_Poly_Triangulation bTr = BRep_Tool::Triangulation(getFace(), bLocation);
		const Poly_Array1OfTriangle& triangles = bTr->Triangles();

		theArea = 0;

		if (!bTr.IsNull())
		{
			Standard_Integer Nb_Tr = bTr->NbTriangles();
			Standard_Integer Nt = 0, n1, n2, n3;
			const TColgp_Array1OfPnt& bNodes = bTr->Nodes();

			for (Nt = 1; Nt < Nb_Tr + 1; Nt++)
			{
				triangles(Nt).Get(n1, n2, n3);

				gp_Pnt Pnt1 = bNodes(n1).Transformed(bLocation);
				gp_Pnt Pnt2 = bNodes(n2).Transformed(bLocation);
				gp_Pnt Pnt3 = bNodes(n3).Transformed(bLocation);

				gp_Vec Vec_A(Pnt1, Pnt2);
				gp_Vec Vec_B(Pnt1, Pnt3);
				theArea = theArea + (0.5 * (Vec_A.CrossMagnitude(Vec_B)));
			}
		}
		alreadyComputedArea = true;
	}

	return theArea;
}

QList<CF_Triangle> InteractiveSurface::Triangulate()
{
	theTriangles.clear();

	TopLoc_Location bLocation;
	Handle_Poly_Triangulation bTr = BRep_Tool::Triangulation(theFace, bLocation);
	const Poly_Array1OfTriangle& triangles1 = bTr->Triangles();
	
	bool RevCheck = false;

	if (theFace.Orientation() == TopAbs_REVERSED)
	{
		RevCheck = true;
	}
	
	if (!bTr.IsNull())
	{
		const TColgp_Array1OfPnt& bNodes = bTr->Nodes();
		Standard_Integer Nb_Tr = triangles1.Size();
		for (int i = 1; i <= Nb_Tr; i++)
		{
			Standard_Integer n1, n2, n3;
			gp_Pnt Pnt1; gp_Pnt Pnt2; gp_Pnt Pnt3;
			if (RevCheck == false)
			{
				triangles1(i).Get(n1, n2, n3);
				Pnt1 = bNodes(n1).Transformed(bLocation);
				Pnt2 = bNodes(n2).Transformed(bLocation);
				Pnt3 = bNodes(n3).Transformed(bLocation);
			}

			else
			{
				triangles1(i).Get(n1, n3, n2);
				Pnt1 = bNodes(n1).Transformed(bLocation);
				Pnt2 = bNodes(n2).Transformed(bLocation);
				Pnt3 = bNodes(n3).Transformed(bLocation);
			}

			CF_Triangle triang(Pnt1, Pnt2, Pnt3);
			
			if (!triang.isTooSmall())
			{
				theTriangles.push_back(triang);
			}
		}
	}
	
	QList<double> allx;
	QList<double> ally;
	QList<double> allz;

	for (int i = 0; i < theTriangles.size(); i++)
	{
		allx.push_back(theTriangles[i].getNodes()[0].X());
		allx.push_back(theTriangles[i].getNodes()[1].X());
		allx.push_back(theTriangles[i].getNodes()[2].X());
		
		ally.push_back(theTriangles[i].getNodes()[0].Y());
		ally.push_back(theTriangles[i].getNodes()[1].Y());
		ally.push_back(theTriangles[i].getNodes()[2].Y());
		
		allz.push_back(theTriangles[i].getNodes()[0].Z());
		allz.push_back(theTriangles[i].getNodes()[1].Z());
		allz.push_back(theTriangles[i].getNodes()[2].Z());
	}

	if (!allx.isEmpty())
	{
		std::sort(allx.begin(), allx.end());
		xmin = allx.first(); xmax = allx.last();
	}

	if (!ally.isEmpty())
	{
		std::sort(ally.begin(), ally.end());
		ymin = ally.first(); ymax = ally.last();
	}

	if (!allx.isEmpty())
	{
		std::sort(allz.begin(), allz.end());
		zmin = allz.first(); zmax = allz.last();
	}
	return theTriangles;
}

void InteractiveSurface::setTriangles(QList<CF_Triangle> allTriangles)
{
	theTriangles = allTriangles;
}

QList<CF_Triangle> InteractiveSurface::getTriangles()
{
	return theTriangles;
}

double InteractiveSurface::getXmin()
{
	return xmin;
}

double InteractiveSurface::getXmax()
{
	return xmax;
}

double InteractiveSurface::getYmin()
{
	return ymin;
}

double InteractiveSurface::getYmax()
{
	return ymax;
}

double InteractiveSurface::getZmin()
{
	return zmin;
}

double InteractiveSurface::getZmax()
{
	return zmax;
}

void InteractiveSurface::Pixelize(double stepElement)
{
	if (!alreadyPixelized)
	{
		Handle(Geom_Surface) mySurf = BRep_Tool::Surface(theFace);
		double uMin, uMax, vMin, vMax;
		BRepTools::UVBounds(theFace, uMin, uMax, vMin, vMax);

		double minSize = stepElement;

		if (stepElement < 1)
			minSize = 0.7;
		Handle(Geom_Curve) aCurv1 = mySurf->VIso(vMin);
		if (!aCurv1.IsNull())
		{
			Standard_Real aLength = GCPnts_AbscissaPoint::Length(GeomAdaptor_Curve(aCurv1, uMin, uMax));
			if (aLength >= minSize)
			{
				int spaceu = std::round(aLength / stepElement);

				omp_set_dynamic(0);
#pragma omp parallel default (none)
				{
#pragma omp for
					for (int u = 0; u <= spaceu; u++)
					{
						double udim = uMin + ((uMax - uMin) * u * stepElement) / aLength;
						Handle(Geom_Curve) aCurv = mySurf->UIso(udim);

						if (!aCurv.IsNull())
						{
							Standard_Real totalLength = GCPnts_AbscissaPoint::Length(GeomAdaptor_Curve(aCurv, vMin, vMax));
							if (totalLength >= minSize)
							{
								int spacev = std::round(totalLength / stepElement);
								QList<int> somePoints;
								for (int v = 0; v <= spacev; v++)
								{
									double vdim = vMin + ((vMax - vMin) * v * stepElement) / totalLength;

									BRepTopAdaptor_FClass2d topologyChecker(theFace, Precision::Approximation());
									TopAbs_State state = topologyChecker.Perform(gp_Pnt2d(udim, vdim));

									if (state != TopAbs_OUT)
									{
										gp_Pnt p;
										gp_Vec uVec;
										gp_Vec vVec;
										mySurf->D1(udim, vdim, p, uVec, vVec);
										gp_Vec aNorm = uVec.Crossed(vVec);

										if (theFace.Orientation() == TopAbs_REVERSED)
											aNorm.Reverse();
										if (aNorm.Magnitude() > 0.05)
										{

										}
									}
								}
							}
						}
					}
				}
			}
		}

		alreadyPixelized = true;
	}
	
}

void InteractiveSurface::transformPixels(gp_Trsf myTransform)
{
}

void InteractiveSurface::transformTriangles(gp_Trsf myTransform)
{
	for (int i = 0; i < theTriangles.size(); i++)
	{
		theTriangles[i].transformTriangle(myTransform);
	}
}