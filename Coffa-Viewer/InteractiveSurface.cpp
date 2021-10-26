#include "InteractiveSurface.h"

#include <omp.h>
#include <qdebug.h>
#include <QtConcurrent/qtconcurrentrun.h>
#include <QtCore\qthread.h>
#include <QtCore\QFuture>
#include <concurrent_vector.h>

using namespace std;
using namespace concurrency;
using namespace QtConcurrent;

InteractiveSurface::InteractiveSurface()
{
	alreadyComputedArea = false;
	alreadyComputedAbsArea = false;
	alreadyComputedDist = false;
	accessProblem = false;
	theArea = 0;
	ID = "";
}

void InteractiveSurface::setAvailableThreads(int threadsCount)
{
	threadCount = threadsCount;
}

InteractiveSurface::InteractiveSurface(TopoDS_Face aFace)
{
	theFace = aFace;
	alreadyComputedArea = false;
	alreadyComputedAbsArea = false;
	alreadyComputedDist = false;
	accessProblem = false;
	theArea = 0;
	ID = "";
	//Triangulate();

}

void InteractiveSurface::setFace(TopoDS_Face aFace)
{
	theFace = aFace;
	//theGeomSurf = BRep_Tool::Surface(theFace);
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

void InteractiveSurface::setShape(TopoDS_Shape aShape)
{
	theShape = aShape;
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

TopoDS_Shape InteractiveSurface::getShape()
{
	return theShape;
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

void InteractiveSurface::setAttribute(QString anAttribute)
{

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

double InteractiveSurface::getAbsoluteArea()
{
	return theAbsArea;
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
				triang.owner = ID;
				triang.id = i;
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

void InteractiveSurface::computeMinDistanceAround(QList<InteractiveSurface*> surfAround)
{	
	if (alreadyComputedDist == false)
	{
		QList<double> dist;
		QList<CF_Pixel> pixels=surfaceMap;

		if (pixels.size()>0)
		{
			for (int n = 0; n < pixels.size(); n++)
			{
				QList<double> distOfThisPix;
				for (int m = 0; m < surfAround.size(); m++)
				{
					if (surfAround[m]->getID() != ID)
					{
						QList<CF_Triangle> triang2 = surfAround[m]->theTriangles;
						//qDebug() << "Pixels" << pixels.size();
						/*omp_set_dynamic(0);
					#pragma omp parallel default (none)
						{
					#pragma omp for*/
						for (int j = 0; j < triang2.size(); j++)
						{
							/*if (pixels[n].theCenter.X() >= triang2[j].xmin && pixels[n].theCenter.X() <= triang2[j].xmax
								&& pixels[n].theCenter.Y() >= triang2[j].ymin && pixels[n].theCenter.Y() <= triang2[j].ymax)
							{*/
							gp_Pnt intPoint = triang2[j].getIntersectionPoint(pixels[n].theDirection, pixels[n].theCenter);

							if (triang2[j].theUnitNormal.Dot(pixels[n].theDirection) < 0 && triang2[j].isPointInside(intPoint) && !intPoint.IsEqual(pixels[n].theCenter, 0.1))
							{
								gp_Vec VMagn(pixels[n].theCenter, intPoint);
								double myMagn = VMagn.Magnitude();
								if (myMagn >= 0.5)
								{
									if (VMagn.Dot(triang2[j].theUnitNormal) < 0 && VMagn.Dot(pixels[n].theDirection) > 0)
									{
										if (myMagn <= 50)
										{
											dist.push_back(myMagn);
											distOfThisPix.push_back(myMagn);
											if (myMagn <= 2.5)
											{
												break;
											}
										}
									}
								}
							}
						}
						//for (int j = 0; j < triang2.size(); j++)
						//{
						//	gp_Pnt Pnt11 = triang2[j].Pn1;
						//	//gp_Dir myNormal2 = pixels2[j].getDirection();
						//	double d = (-triang2[j].theUnitNormal.X() * Pnt11.X()) + (-triang2[j].theUnitNormal.Y() * Pnt11.Y()) + (-triang2[j].theUnitNormal.Z() * Pnt11.Z());
						//	double a = triang2[j].theUnitNormal.X(); double b = triang2[j].theUnitNormal.Y(); double c = triang2[j].theUnitNormal.Z();
						//	double a1 = pixels[n].theDirection.X(); double b1 = pixels[n].theDirection.Y(); double c1 = pixels[n].theDirection.Z();

						//	double tTest = (a * a1) + (b * b1) + (c * c1);

						//	if (tTest != 0)
						//	{
						//		double t = (-1 * (d + (a * pixels[n].theCenter.X()) + (b * pixels[n].theCenter.Y()) + (c * pixels[n].theCenter.Z()))) / ((a * a1) + (b * b1) + (c * c1));
						//		double ix = pixels[n].theCenter.X() + (a1 * t); double iy = pixels[n].theCenter.Y() + (b1 * t); double iz = pixels[n].theCenter.Z() + (c1 * t);
						//		gp_Pnt IntPnt(ix, iy, iz);

						//		if (triang2[j].isPointInside(IntPnt))
						//		{
						//			gp_Vec VMagn(pixels[n].theCenter, IntPnt);
						//			double myMagn = VMagn.Magnitude();
						//			if (myMagn >= 0.10)
						//			{
						//				if (triang2[j].theUnitNormal.Dot(pixels[n].theDirection) < 0 && VMagn.Dot(triang2[j].theUnitNormal) < 0 && VMagn.Dot(pixels[n].theDirection) > 0)
						//				{
						//					if (myMagn <= 50)
						//					{
						//						dist.push_back(myMagn);
						//						//qDebug() << myMagn;
						//						distOfThisPix.push_back(myMagn);

						//						if (myMagn <= 1.0)
						//						{
						//							break;
						//						}
						//					}
						//				}
						//			}
						//		}
						//	}
						//}
					}
				}
				if (!distOfThisPix.isEmpty())
				{
					std::sort(distOfThisPix.begin(), distOfThisPix.end());
					surfaceMap[n].setMinDistanceAround(distOfThisPix.first());
				}
			}
		}

		distPixels = pixels;
		//qDebug()<<"Finished";

		if (!dist.isEmpty())
		{
			std::sort(dist.begin(), dist.end());
			theMinDistanceAround = dist.first();
			theMaxDistanceAround = dist.last();
			qDebug() << "Minimum Around" << theMinDistanceAround;

			//if (theMinDistanceAround>0)
			accessProblem = true;
		}

		else
		{
			accessProblem = false;
		}
		//qDebug() << "Distances Around" << theMinDistanceAround << " " << theMaxDistanceAround;
		alreadyComputedDist = true;
	}
	

}

void InteractiveSurface::computeMinDistanceAround2(QList<InteractiveSurface*> surfAround)
{
	if (alreadyComputedDist == false)
	{
		omp_set_num_threads(threadCount);
		omp_set_dynamic(1);
		concurrent_vector<double> dist;
		concurrent_vector<CF_Triangle> arroundTriangles;

		for (int m = 0; m < surfAround.size(); m++)
		{
			if (surfAround[m]->getID() != ID)
			{
				for (int p = 0; p < surfAround[m]->theTriangles.size(); p++)
				{
					arroundTriangles.push_back(surfAround[m]->theTriangles[p]);
				}
			}
		}
		
		//QList<int> relatedTriangles;
		for (int i = 0; i < theTriangles.size(); i++)
		{
			QList<gp_Pnt> map/*= theTriangles[i].copyNodes()*/;
			//map.insert(0, theTriangles[i].inCenter);
			map.push_back(theTriangles[i].inCenter);
			concurrent_vector<double> distOfThisPix;
			for (int j = 0; j < map.size(); j++)
			{
				bool tooSmall = false;

#pragma omp parallel default (none)
				{
#pragma omp for
					for (int p = 0; p < arroundTriangles.size(); p++)
					{
						/*if (arroundTriangles[p].compartiment == theTriangles[i].compartiment ||
							arroundTriangles[p].compartiment == theTriangles[i].compartiment2)
						{*/

						if (arroundTriangles[p].theUnitNormal.IsOpposite(theTriangles[i].theUnitNormal, 1.55))
						{
							gp_Pnt intPoint = arroundTriangles[p].getIntersectionPoint(theTriangles[i].theUnitNormal, map[j]);

							if (arroundTriangles[p].isPointInside(intPoint) /*|| arroundTriangles[p].isPointOnContour(intPoint)*/)
							{
								gp_Vec myVec(map[j], intPoint);

								double leng = myVec.Magnitude();

								if (leng >= 1.0 && leng <= 30.0)
								{
									if (myVec.Angle(theTriangles[i].theUnitNormal) <= 0.00174 && myVec.Dot(theTriangles[i].theUnitNormal) > 0)
									{
										//relatedTriangles.push_back(p);
										//qDebug() << "in some compartiment" << leng;
										dist.push_back(leng);
										distOfThisPix.push_back(leng);
										if (leng <= 4.0)
										{
											tooSmall = true;
											break;
										}
									}
								}
							}
						}
						//}
					}
				}
				if (tooSmall)
					break;

			}
			if (distOfThisPix.size()>0)
			{
				std::sort(distOfThisPix.begin(), distOfThisPix.end());
				theTriangles[i].minDistance = distOfThisPix[0];
				//qDebug() << "For this Triangle" << distOfThisPix.first();
			}
		}


		if (dist.size()>0)
		{
			std::sort(dist.begin(), dist.end());
			theMinDistanceAround = dist[0];
			theMaxDistanceAround = dist[dist.size()-1];

			accessProblem = true;
		}

		else
		{
			accessProblem = false;
		}
		alreadyComputedDist = true;
	}
}

void InteractiveSurface::computeMinDistanceAround3(QList<InteractiveSurface*> surfAround)
{
	if (alreadyComputedDist == false)
	{
		omp_set_num_threads(threadCount);
		omp_set_dynamic(1);
		concurrent_vector<double> dist;
		concurrent_vector<CF_Triangle> arroundTriangles;
		concurrent_vector<Handle(Geom_Surface)> arroundSurf;

		
		for (int m = 0; m < surfAround.size(); m++)
		{
			if (surfAround[m]->getID() != ID)
			{
				Handle(Geom_Surface) mySurf = BRep_Tool::Surface(theFace);
				arroundSurf.push_back(mySurf);
				/*for (int p = 0; p < surfAround[m]->theTriangles.size(); p++)
				{
					
					arroundTriangles.push_back(surfAround[m]->theTriangles[p]);
				}*/
			}
		}

		for (int i = 0; i < theTriangles.size(); i++)
		{
			QList<gp_Pnt> map /*= theTriangles[i].copyNodes()*/;
			//map.insert(0, theTriangles[i].inCenter);
			map.push_back(theTriangles[i].inCenter);
			concurrent_vector<double> distOfThisPix;
			for (int j = 0; j < map.size(); j++)
			{
				Handle(Geom_Line) aLine = new Geom_Line(map[j], theTriangles[i].theUnitNormal);
				Handle(Geom_Curve) aCurv;
				aCurv=aLine;

#pragma omp parallel default (none)
				{
#pragma omp for
					for (int p = 0; p < arroundSurf.size(); p++)
					{
						/*if (arroundSurf[p]->ID != ID)
						{*/
							GeomAPI_IntCS anInt(aCurv, arroundSurf[p]/*->theGeomSurf*/);
							bool tooSmall = false;
							if (anInt.IsDone())
							{
								//qDebug()<< anInt.NbPoints();
								int nb = anInt.NbPoints();
								if (nb > 0)
								{
									for (int n = 1; n <= nb; n++)
									{
										gp_Vec myVec(map[j], anInt.Point(n));

										double leng = myVec.Magnitude();

										if (leng >= 1.0 && leng <= 40)
										{
											if (myVec.Angle(theTriangles[i].theUnitNormal) <= 0.00174 && myVec.Dot(theTriangles[i].theUnitNormal) > 0)
											{
												//qDebug() << "in some compartiment" << leng;
												dist.push_back(leng);
												distOfThisPix.push_back(leng);
												tooSmall = true;
												if (leng <= 3)
												{

													break;
												}
											}
										}
									}

									/*if (tooSmall)
										break;*/
									//qDebug() << anInt.NbPoints();

								}
							}
						//}
					}
				}
			}
			if (distOfThisPix.size() > 0)
			{
				std::sort(distOfThisPix.begin(), distOfThisPix.end());
				theTriangles[i].minDistance = distOfThisPix[0];
				//qDebug() << "For this Triangle" << distOfThisPix.first();
			}
		}


		if (dist.size() > 0)
		{
			std::sort(dist.begin(), dist.end());
			theMinDistanceAround = dist[0];
			theMaxDistanceAround = dist[dist.size() - 1];

			accessProblem = true;
		}

		else
		{
			accessProblem = false;
		}
		alreadyComputedDist = true;
	}
}

QList<CF_Pixel> InteractiveSurface::simulateMinDistanceAround(QList<InteractiveSurface*> surfAround,
	double ang, double toolSize)
{
	QList<double> dist;
	Handle(Geom_Surface) mySurf = BRep_Tool::Surface(theFace);
	double uMin, uMax, vMin, vMax;
	BRepTools::UVBounds(theFace, uMin, uMax, vMin, vMax);

	double stepElement = 1;

	QList<CF_Pixel> pixelsOfInterest;
	QList<CF_Pixel> pixels;

	gp_Vec zdir(0, 0, -1);
	Handle(Geom_Curve) aCurv1 = mySurf->VIso(vMin);
	if (!aCurv1.IsNull())
	{
		Standard_Real aLength = GCPnts_AbscissaPoint::Length(GeomAdaptor_Curve(aCurv1, uMin, uMax));
		if (aLength >= 1)
		{
			int spaceu = std::round(aLength / stepElement);

			for (int u = 0; u <= spaceu; u++)
			{
				double udim = uMin + ((uMax - uMin) * u * stepElement) / aLength;
				Handle(Geom_Curve) aCurv = mySurf->UIso(udim);
				if (!aCurv.IsNull())
				{
					Standard_Real totalLength = GCPnts_AbscissaPoint::Length(GeomAdaptor_Curve(aCurv, vMin, vMax));
					if (totalLength >= 1)
					{
						int spacev = std::round(totalLength / stepElement);
						QList<gp_Pnt> pnts;
						for (int v = 0; v <= spacev; v++)
						{
							double vdim = vMin + ((vMax - vMin) * v * stepElement) / totalLength;

								gp_Pnt p;
								gp_Vec uVec;
								gp_Vec vVec;
								mySurf->D1(udim, vdim, p, uVec, vVec);
								gp_Vec aNorm = uVec.Crossed(vVec);

								if (theFace.Orientation() == TopAbs_REVERSED)
									aNorm.Reverse();
								if (aNorm.Magnitude() > 0.15)
								{
									if (aNorm.Angle(zdir) <= (ang * 3.14 / 180))
									{
										CF_Pixel aPix(p, 1);
										aPix.setDirection(gp_Dir(aNorm));
										pixels.push_back(aPix);
									}
								}
						}
					}
				}
			}
		}
	}

	QList<CF_Pixel> pixels2;
	if (pixels.size() > 0)
	{
		for (int su = 0; su < surfAround.size(); su++)
		{
			if (surfAround[su]->getID() != ID)
			{
				Handle(Geom_Surface) mySurf2 = BRep_Tool::Surface(surfAround[su]->getFace());
				double uMin, uMax, vMin, vMax;
				BRepTools::UVBounds(surfAround[su]->getFace(), uMin, uMax, vMin, vMax);

				double stepElement = 1;

				Handle(Geom_Curve) aCurv1 = mySurf2->VIso(vMin);
				if (!aCurv1.IsNull())
				{
					Standard_Real aLength = GCPnts_AbscissaPoint::Length(GeomAdaptor_Curve(aCurv1, uMin, uMax));
					if (aLength >= 1)
					{
						int spaceu = std::round(aLength / stepElement);

						for (int u = 0; u <= spaceu; u++)
						{
							double udim = uMin + ((uMax - uMin) * u * stepElement) / aLength;
							Handle(Geom_Curve) aCurv = mySurf2->UIso(udim);
							if (!aCurv.IsNull())
							{
								Standard_Real totalLength = GCPnts_AbscissaPoint::Length(GeomAdaptor_Curve(aCurv, vMin, vMax));
								if (totalLength >= 1)
								{
									int spacev = std::round(totalLength / stepElement);
									QList<gp_Pnt> pnts;
									for (int v = 0; v <= spacev; v++)
									{
										double vdim = vMin + ((vMax - vMin) * v * stepElement) / totalLength;
										gp_Pnt p;
										gp_Vec uVec;
										gp_Vec vVec;
										mySurf2->D1(udim, vdim, p, uVec, vVec);
										gp_Vec aNorm = uVec.Crossed(vVec);

										if (surfAround[su]->getFace().Orientation() == TopAbs_REVERSED)
											aNorm.Reverse();
										if (aNorm.Magnitude() > 0.15)
										{
											CF_Pixel aPix(p, 1);
											aPix.setDirection(gp_Dir(aNorm));
											pixels2.push_back(aPix);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}


	if (pixels2.size() > 0)
	{
		for (int n = 0; n < pixels.size(); n++)
		{
			QList<double> distOfThisPix;
			//qDebug() << "Pixels" << pixels.size();
			/*omp_set_dynamic(0);
		#pragma omp parallel default (none)
			{
		#pragma omp for*/
			for (int j = 0; j < pixels2.size(); j++)
			{
				gp_Pnt Pnt11 = pixels2[j].getCenter();
				gp_Dir myNormal2 = pixels2[j].getDirection();
				double d = (-myNormal2.X() * Pnt11.X()) + (-myNormal2.Y() * Pnt11.Y()) + (-myNormal2.Z() * Pnt11.Z());
				double a = myNormal2.X(); double b = myNormal2.Y(); double c = myNormal2.Z();
				double a1 = pixels[n].getDirection().X(); double b1 = pixels[n].getDirection().Y(); double c1 = pixels[n].getDirection().Z();

				double tTest = (a * a1) + (b * b1) + (c * c1);

				if (tTest != 0)
				{
					double t = (-1 * (d + (a * pixels[n].getCenter().X()) + (b * pixels[n].getCenter().Y()) + (c * pixels[n].getCenter().Z()))) / ((a * a1) + (b * b1) + (c * c1));
					double ix = pixels[n].getCenter().X() + (a1 * t); double iy = pixels[n].getCenter().Y() + (b1 * t); double iz = pixels[n].getCenter().Z() + (c1 * t);
					gp_Pnt IntPnt(ix, iy, iz);

					if (IntPnt.IsEqual(pixels2[j].getCenter(), stepElement / 2))
					{
						gp_Vec VMagn(pixels[n].getCenter(), IntPnt);
						double myMagn = VMagn.Magnitude();
						if (myMagn >= 0.15)
						{
							if (pixels2[j].getDirection().Dot(pixels[n].getDirection()) < 0 && VMagn.Dot(pixels2[j].getDirection()) < 0 && VMagn.Dot(pixels[n].getDirection()) > 0)
							{
								//qDebug() << "Finished";
								//qDebug() << myMagn;
								if (myMagn <= toolSize)
								{
									//dist.push_back(myMagn);
									distOfThisPix.push_back(myMagn);
								}
							}
						}
					}
				}
			}

			if (!distOfThisPix.isEmpty())
			{
				std::sort(distOfThisPix.begin(), distOfThisPix.end());
				pixels[n].setMinDistanceAround(distOfThisPix.first());
				pixelsOfInterest.push_back(pixels[n]);
			}
		}
	}

	return pixelsOfInterest;
}


double InteractiveSurface::getMinDistanceAround(QList<InteractiveSurface*> surfAround)
{
	if (!alreadyComputedDist)
	{
		computeMinDistanceAround2(surfAround);
	}
	return theMinDistanceAround;
}

bool InteractiveSurface::hasAccessProblems()
{
	return accessProblem;
}

QList<CF_Triangle> InteractiveSurface::getInvolvedTriangles(double thresholdDistance)
{
	QList<CF_Triangle> tri;
	
	for (int i = 0; i < theTriangles.size(); i++)
	{
		//qDebug()<< theTriangles[i].minDistance;
		if (theTriangles[i].minDistance <= thresholdDistance)
		{
			//qDebug() << theTriangles[i].minDistance;
			tri.push_back(theTriangles[i]);
			//break;
		}
	}
	
	return tri;
}

QList<CF_Pixel> InteractiveSurface::getInvolvedPixels(double thresholdDistance)
{
	QList<CF_Pixel> pix;
	//qDebug()<< surfaceMap.size();
	for (int i = 0; i < theTriangles.size(); i++)
	{
		for (int j = 0; j < theTriangles[i].map.size(); j++)
		{
			//qDebug()<<surfaceMap[i].getMinDistanceAround();
			if (theTriangles[i].map[j].getMinDistanceAround() <= thresholdDistance)
			{
				pix.push_back(theTriangles[i].map[j]);
				//break;
			}
		}
	}

	return pix;
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

QList<CF_Pixel> InteractiveSurface::getDistPixels()
{
	return distPixels;
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
		concurrent_vector<CF_Pixel> aList;
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
											
											CF_Pixel aPix(p, stepElement, aNorm);
											aPix.setUVPoint(gp_Pnt(udim, vdim, p.Z()));
											aList.push_back(aPix);
											//somePoints.push_back(surfaceMap.size()-1);
										}
									}
								}
								//arrangedSurfMap.push_back(somePoints);
							}
						}
					}
				}
			}

			for (int i = 0; i < aList.size(); i++)
			{
				surfaceMap.push_back(aList[i]);
				
			}
		}

		alreadyPixelized = true;
		if (surfaceMap.size() != 0)
			theAbsArea = getArea() / surfaceMap.size();

		else
			theAbsArea = 1;
	}
	
}

QList<CF_Pixel> InteractiveSurface::FinerPixelization(double elementSize)
{
	QList<CF_Pixel> finerPix;
	Handle(Geom_Surface) mySurf = BRep_Tool::Surface(theFace);
	double uMin, uMax, vMin, vMax;
	BRepTools::UVBounds(theFace, uMin, uMax, vMin, vMax);

	double stepElement = elementSize;

	Handle(Geom_Curve) aCurv1 = mySurf->VIso(vMin);
	if (!aCurv1.IsNull())
	{
		Standard_Real aLength = GCPnts_AbscissaPoint::Length(GeomAdaptor_Curve(aCurv1, uMin, uMax));
		if (aLength >= stepElement)
		{
			int spaceu = std::round(aLength / stepElement);

			for (int u = 0; u <= spaceu; u++)
			{
				double udim = uMin + ((uMax - uMin) * u * stepElement) / aLength;
				Handle(Geom_Curve) aCurv = mySurf->UIso(udim);

				if (!aCurv.IsNull())
				{
					Standard_Real totalLength = GCPnts_AbscissaPoint::Length(GeomAdaptor_Curve(aCurv, vMin, vMax));
					if (totalLength >= stepElement)
					{
						int spacev = std::round(totalLength / stepElement);
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
								if (aNorm.Magnitude() > 0.1)
								{
									CF_Pixel aPix(gp_Pnt(udim, vdim, 0), stepElement, aNorm);
									finerPix.push_back(aPix);

								}
							}
						}
					}
				}
			}
		}
	}

	return finerPix;
}

void InteractiveSurface::transformPixels(gp_Trsf myTransform)
{
	if (surfaceMap.size() != 0)
	{
		for (int i = 0; i < surfaceMap.size(); i++)
		{
			surfaceMap[i].rotatePixel(myTransform);
		}
	}	
}

void InteractiveSurface::transformTriangles(gp_Trsf myTransform)
{
	for (int i = 0; i < theTriangles.size(); i++)
	{
		theTriangles[i].transformTriangle(myTransform);
	}
}