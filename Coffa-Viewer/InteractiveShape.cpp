#include "InteractiveShape.h"
#include "Doc.h"

#include <TopoDS_Compound.hxx>
#include <BRep_Builder.hxx>

#include <BRepTools.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepAlgoAPI_Section.hxx>

#include <BRepAdaptor_Surface.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_Line.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Geom_Curve.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_BezierSurface.hxx>
#include <GeomFill_NSections.hxx>
#include <GeomFill_Pipe.hxx>

#include <GCPnts_AbscissaPoint.hxx>
#include <GeomAPI_PointsToBSplineSurface.hxx>
#include <GeomConvert_CompCurveToBSplineCurve.hxx>
#include <GeomConvert_CompBezierSurfacesToBSplineSurface.hxx>
#include <GeomFill_BezierCurves.hxx>
#include <GeomAPI_Interpolate.hxx>
#include <GeomFill.hxx>
#include <GeomAPI_IntSS.hxx>
#include <GeomAPI_IntCS.hxx>
#include <GeomLProp_SLProps.hxx>

#include <TColGeom_SequenceOfCurve.hxx>
#include <TColGeom_Array2OfBezierSurface.hxx>

#include <gp_Circ.hxx>

using namespace std;
using namespace concurrency;


InteractiveShape::InteractiveShape()
{
	alreadyComputedArea = false;
}

InteractiveShape::InteractiveShape(TopoDS_Shape aShape, Doc* aDoc)
{
	theShape = aShape;
	theOriginShape = aShape;
	theDoc = aDoc;
	alreadyComputedArea = false;

	Handle(AIS_Shape) AISPart = new AIS_Shape(theShape);

	IObject = AISPart;

	theOriginSurfaces = theSurfaces;
	mainItem = new QStandardItem(theName);
	QIcon iconPart;
	iconPart.addPixmap(QPixmap(QString::fromUtf8(":/CoffaViewer/Resources/Part.png")), QIcon::Normal, QIcon::Off);
	mainItem->setIcon(iconPart);
	mainItem->setEditable(false);

	geomItem = new QStandardItem("Geometry");
	geomItem->setEditable(false);

	mainItem->appendRow(geomItem);
}

void InteractiveShape::parseFaces()
{
	TopExp_Explorer ExpFace;
	TopTools_IndexedMapOfShape myIndexMap;
	TopExp Mapper;
	Mapper.MapShapes(theShape, TopAbs_FACE, myIndexMap);

	for (int m = 1; m <= myIndexMap.Size(); m++)
	{
		TopoDS_Shape iShape = myIndexMap.FindKey(m);
		TopoDS_Face iFace = TopoDS::Face(iShape);
		InteractiveSurface *aSurf = new InteractiveSurface(iFace);
		aSurf->setName("Face " + QString::number(m));
		aSurf->setID(QString::number(m));
		aSurf->Triangulate();
        theSurfaces.push_back(aSurf);

		QStandardItem *surfItem = new QStandardItem(aSurf->getName());
		surfItem->setEditable(false);
		geomItem->appendRow(surfItem);
	}

}

QList<InteractiveSurface*> InteractiveShape::getSurfaces()
{
	return theSurfaces;
}

QList<int> InteractiveShape::getSelectedSurfaces()
{
	QList<int> iList;

		for (theDoc->getContext()->InitSelected(); theDoc->getContext()->MoreSelected(); theDoc->getContext()->NextSelected())
		{
			const TopoDS_Shape& mySelShape = theDoc->getContext()->SelectedShape();
			
			int idSearch = 0;
			for (idSearch = 0; idSearch < theSurfaces.size(); idSearch++)
			{
				if (mySelShape.IsPartner(theSurfaces[idSearch]->getFace()))
				{
					iList.push_back(idSearch);
					break;
				}
					
			}
			
		}

	return iList;
}

bool InteractiveShape::isOneFaceSelected()
{
	bool eq=false;

	for (theDoc->getContext()->InitSelected(); theDoc->getContext()->MoreSelected(); theDoc->getContext()->NextSelected())
	{
		const TopoDS_Shape& mySelShape = theDoc->getContext()->SelectedShape();

		int idSearch = 0;
		for (idSearch = 0; idSearch < theSurfaces.size(); idSearch++)
		{
			if (mySelShape.IsPartner(theSurfaces[idSearch]->getFace()))
			{
				eq=true;
				break;
			}

		}

		if (eq)
			break;
	}
	return eq;
}

bool InteractiveShape::isSelected()
{
	bool eq=false;

	for (theDoc->getContext()->InitSelected(); theDoc->getContext()->MoreSelected(); theDoc->getContext()->NextSelected())
	{
		const TopoDS_Shape& mySelShape = theDoc->getContext()->SelectedShape();

		if (mySelShape.IsPartner(theShape))
		{
			eq = true;
		}

	}
	return eq;
}

void InteractiveShape::setShape(TopoDS_Shape aShape)
{
	theShape = aShape;
	parseFaces();
	alreadyComputedArea = false;
	Handle(AIS_Shape) AISPart = new AIS_Shape(theShape);

	IObject = AISPart;
}

TopoDS_Shape InteractiveShape::getShape()
{
	return theShape;
}

TopoDS_Shape InteractiveShape::getOriginShape()
{
	return theOriginShape;
}

void InteractiveShape::setID(QString id)
{
	ID = id;
}

QString InteractiveShape::getID()
{
	return ID;
}

void InteractiveShape::setName(QString aName)
{
	theName = aName;
	mainItem->setText(theName);
}

QString InteractiveShape::getName()
{
	return theName;
}

QStandardItem* InteractiveShape::getTreeItem()
{
	return mainItem;
}

Handle(AIS_InteractiveObject) InteractiveShape::getInteractiveObject()
{
	return IObject;
}

void InteractiveShape::removeAllInteractives()
{
	theDoc->getContext()->Remove(IObject, true);
}

double InteractiveShape::currentRx()
{
	return Rx;
}

double InteractiveShape::currentRy()
{
	return Ry;
}

double InteractiveShape::currentRz()
{
	return Rz;
}

//////////////////////////////////////
///////  Useful Algorithms  //////////
/////////////////////////////////////

double InteractiveShape::getTotalArea()
{
	if (!alreadyComputedArea)
	{
		thetotalArea = 0;

		for each (InteractiveSurface* var in theSurfaces)
		{
			thetotalArea = thetotalArea + var->getArea();
		}
		alreadyComputedArea = true;
	}


	return thetotalArea;
}

void InteractiveShape::rotateShape(gp_Quaternion aQuaternion, double angx, double angy, double angz)
{
	Rx = angx;
	Ry = angy;
	Rz = angz;

	//Get Initial Position
	Bnd_Box B0;
	BRepBndLib::Add(theShape, B0);
	double Bxmin0, Bymin0, Bzmin0, Bxmax0, Bymax0, Bzmax0;
	B0.Get(Bxmin0, Bymin0, Bzmin0, Bxmax0, Bymax0, Bzmax0);
	double Px0 = ((Bxmax0)+(Bxmin0)) / 2;
	double Py0 = ((Bymax0)+(Bymin0)) / 2;
	gp_Pnt initcenter(Px0, Py0, Bzmin0);

	//Transform the shape in background
	rotQuaternion = aQuaternion;
	gp_Trsf rotTrsf;
	rotTrsf.SetRotation(rotQuaternion);
	BRepBuilderAPI_Transform myPartRotation(theOriginShape, rotTrsf, Standard_False);
	theShape = myPartRotation.Shape();

	//Get Final Position
	Bnd_Box B;
	BRepBndLib::Add(theShape, B);
	double Bxmin, Bymin, Bzmin, Bxmax, Bymax, Bzmax;
	B.Get(Bxmin, Bymin, Bzmin, Bxmax, Bymax, Bzmax);
	double Px = ((Bxmax)+(Bxmin)) / 2;
	double Py = ((Bymax)+(Bymin)) / 2;
	gp_Pnt Pcenter(Px, Py, Bzmin);

	trlVector = gp_Vec(Pcenter, initcenter);
	gp_Trsf transTrsf;
	transTrsf.SetTranslation(trlVector);
	BRepBuilderAPI_Transform myPartTranslation(theShape, transTrsf, Standard_False);
	theShape = myPartTranslation.Shape();

	//TransformInteractives
	gp_Trsf myTransform;
	myTransform.SetTransformation(rotQuaternion, trlVector);
	IObject->SetLocalTransformation(myTransform);

	//Parse Faces of Rotated Shape
	TopExp_Explorer ExpFace;
	TopTools_IndexedMapOfShape myIndexMap;
	TopExp Mapper;
	Mapper.MapShapes(theShape, TopAbs_FACE, myIndexMap);

	for (int m = 1; m <= myIndexMap.Size(); m++)
	{
		if (m - 1 < theSurfaces.size())
		{
			theSurfaces[m - 1]->transformTriangles(myTransform);
			if (theSurfaces[m - 1]->getFace().IsSame(TopoDS::Face(myIndexMap.FindKey(m))));
			theSurfaces[m - 1]->setFace(TopoDS::Face(myIndexMap.FindKey(m)));
		}
	}
}

void InteractiveShape::translatePart(double mx, double my, double mz)
{
	Tx = mx;
	Ty = my;
	Tz = mz;

	Bnd_Box B0;
	BRepBndLib::Add(theOriginShape, B0);
	double Bxmin0, Bymin0, Bzmin0, Bxmax0, Bymax0, Bzmax0;
	B0.Get(Bxmin0, Bymin0, Bzmin0, Bxmax0, Bymax0, Bzmax0);
	double Px = ((Bxmax0)+(Bxmin0)) / 2;
	double Py = ((Bymax0)+(Bymin0)) / 2;
	gp_Pnt PA(Px, Py, Bzmin0);
	gp_Pnt PB(Px+Tx, Py+Ty, Bzmin0 +Tz);

	trlVector = gp_Vec(PA, PB);
	gp_Trsf aTransformer;
	aTransformer.SetTranslation(trlVector);
	BRepBuilderAPI_Transform myPartTranslation(theOriginShape, aTransformer, Standard_False);
	theShape = myPartTranslation.Shape();

	//Transform the shape in background
	gp_Trsf rotTrsf;
	rotTrsf.SetRotation(rotQuaternion);
	BRepBuilderAPI_Transform myPartRotation(theShape, rotTrsf, Standard_False);
	theShape = myPartRotation.Shape();

	////Get Final Position
	Bnd_Box B2;
	BRepBndLib::Add(theShape, B2);
	double Bxmin2, Bymin2, Bzmin2, Bxmax2, Bymax2, Bzmax2;
	B2.Get(Bxmin2, Bymin2, Bzmin2, Bxmax2, Bymax2, Bzmax2);
	gp_Pnt PC(((Bxmax2)+(Bxmin2)) / 2, ((Bymax2)+(Bymin2)) / 2, Bzmin2);

	trlVector = gp_Vec(PC, PB);
	gp_Trsf transTrsf;
	transTrsf.SetTranslation(trlVector);
	BRepBuilderAPI_Transform myPartTranslation2(theShape, transTrsf, Standard_False);
	theShape = myPartTranslation2.Shape();

	//To restore the geometry Rotation state
	rotateShape(rotQuaternion, Rx, Ry, Rz);

	theDoc->getContext()->Update(IObject, true);
}

QList<QList<gp_Pnt>> InteractiveShape::sliceTriangle(gp_Pnt Pnt1, gp_Pnt Pnt2, gp_Pnt Pnt3, gp_Dir theNormal, gp_Dir SliceDirection, double zMin, double zMax, double space)
{
	QList<QList<gp_Pnt>> aList;

	if (!Pnt1.IsEqual(Pnt2, 0.01) && !Pnt2.IsEqual(Pnt3, 0.01) && !Pnt1.IsEqual(Pnt3, 0.01))
	{
		if (!theNormal.IsParallel(SliceDirection, 0.0348))
		{
			double H = zMin;
			while (H<=zMax)
			{
				gp_Pnt P1;
				gp_Pnt P2;
				QList<gp_Pnt> myPoints;
				H = H+space;
				//Case a && c
				//
				//Case b & e
				if (Pnt1.Z() == H && Pnt2.Z() == H)
				{
					P1 = Pnt1;
					P2 = Pnt2;

					myPoints.push_back(P1);
					myPoints.push_back(P2);
				}

				else if (Pnt2.Z() == H && Pnt3.Z() == H)
				{
					P1 = Pnt2;
					P2 = Pnt3;

					myPoints.push_back(P1);
					myPoints.push_back(P2);
				}

				else if (Pnt1.Z() == H && Pnt3.Z() == H)
				{
					P1 = Pnt1;
					P2 = Pnt3;

					myPoints.push_back(P1);
					myPoints.push_back(P2);
				}

				//Case d
				else if ((Pnt1.Z() > H && Pnt2.Z() < H && Pnt3.Z() < H) || (Pnt1.Z() < H && Pnt2.Z() > H && Pnt3.Z() > H))
				{
					gp_Dir aDir1(gp_Vec(Pnt1, Pnt2));
					gp_Dir aDir2(gp_Vec(Pnt1, Pnt3));

					double d = -1 * (SliceDirection.Z() * H);
					double t1 = -(d + ((SliceDirection.X() * Pnt1.X()) + (SliceDirection.Y() * Pnt1.Y()) + (SliceDirection.Z() * Pnt1.Z()))) /
						((SliceDirection.X() * aDir1.X()) + (SliceDirection.Y() * aDir1.Y()) + (SliceDirection.Z() * aDir1.Z()));

					double t2 = -(d + ((SliceDirection.X() * Pnt1.X()) + (SliceDirection.Y() * Pnt1.Y()) + (SliceDirection.Z() * Pnt1.Z()))) /
						((SliceDirection.X() * aDir2.X()) + (SliceDirection.Y() * aDir2.Y()) + (SliceDirection.Z() * aDir2.Z()));
					//
					double myX1 = Pnt1.X() + (aDir1.X() * t1);
					double myY1 = Pnt1.Y() + (aDir1.Y() * t1);
					double myZ1 = Pnt1.Z() + (aDir1.Z() * t1);

					double myX2 = Pnt1.X() + (aDir2.X() * t2);
					double myY2 = Pnt1.Y() + (aDir2.Y() * t2);
					double myZ2 = Pnt1.Z() + (aDir2.Z() * t2);

					P1 = gp_Pnt(myX1, myY1, myZ1);
					P2 = gp_Pnt(myX2, myY2, myZ2);

					myPoints.push_back(P1);
					myPoints.push_back(P2);
				}

				else if ((Pnt1.Z() < H && Pnt2.Z() > H && Pnt3.Z() < H) || (Pnt1.Z() > H && Pnt2.Z() < H && Pnt3.Z() > H))
				{
					gp_Dir aDir1(gp_Vec(Pnt2, Pnt1));
					gp_Dir aDir2(gp_Vec(Pnt2, Pnt3));

					double d = -1 * (SliceDirection.Z() * H);
					double t1 = -(d + ((SliceDirection.X() * Pnt2.X()) + (SliceDirection.Y() * Pnt2.Y()) + (SliceDirection.Z() * Pnt2.Z()))) /
						((SliceDirection.X() * aDir1.X()) + (SliceDirection.Y() * aDir1.Y()) + (SliceDirection.Z() * aDir1.Z()));

					double t2 = -(d + ((SliceDirection.X() * Pnt2.X()) + (SliceDirection.Y() * Pnt2.Y()) + (SliceDirection.Z() * Pnt2.Z()))) /
						((SliceDirection.X() * aDir2.X()) + (SliceDirection.Y() * aDir2.Y()) + (SliceDirection.Z() * aDir2.Z()));
					//
					double myX1 = Pnt2.X() + (aDir1.X() * t1);
					double myY1 = Pnt2.Y() + (aDir1.Y() * t1);
					double myZ1 = Pnt2.Z() + (aDir1.Z() * t1);

					double myX2 = Pnt2.X() + (aDir2.X() * t2);
					double myY2 = Pnt2.Y() + (aDir2.Y() * t2);
					double myZ2 = Pnt2.Z() + (aDir2.Z() * t2);

					P1 = gp_Pnt(myX1, myY1, myZ1);
					P2 = gp_Pnt(myX2, myY2, myZ2);

					myPoints.push_back(P1);
					myPoints.push_back(P2);
				}

				else if ((Pnt1.Z() < H && Pnt2.Z() < H && Pnt3.Z() > H) || (Pnt1.Z() > H && Pnt2.Z() > H && Pnt3.Z() < H))
				{
					gp_Dir aDir1(gp_Vec(Pnt3, Pnt1));
					gp_Dir aDir2(gp_Vec(Pnt3, Pnt2));

					double d = -1 * (SliceDirection.Z() * H);
					double t1 = -(d + ((SliceDirection.X() * Pnt3.X()) + (SliceDirection.Y() * Pnt3.Y()) + (SliceDirection.Z() * Pnt3.Z()))) /
						((SliceDirection.X() * aDir1.X()) + (SliceDirection.Y() * aDir1.Y()) + (SliceDirection.Z() * aDir1.Z()));

					double t2 = -(d + ((SliceDirection.X() * Pnt3.X()) + (SliceDirection.Y() * Pnt3.Y()) + (SliceDirection.Z() * Pnt3.Z()))) /
						((SliceDirection.X() * aDir2.X()) + (SliceDirection.Y() * aDir2.Y()) + (SliceDirection.Z() * aDir2.Z()));
					//
					double myX1 = Pnt3.X() + (aDir1.X() * t1);
					double myY1 = Pnt3.Y() + (aDir1.Y() * t1);
					double myZ1 = Pnt3.Z() + (aDir1.Z() * t1);

					double myX2 = Pnt3.X() + (aDir2.X() * t2);
					double myY2 = Pnt3.Y() + (aDir2.Y() * t2);
					double myZ2 = Pnt3.Z() + (aDir2.Z() * t2);

					P1 = gp_Pnt(myX1, myY1, myZ1);
					P2 = gp_Pnt(myX2, myY2, myZ2);

					myPoints.push_back(P1);
					myPoints.push_back(P2);
				}

				//Case f
				else if ((Pnt1.Z() == H && Pnt2.Z() < H && Pnt3.Z() > H) || (Pnt1.Z() == H && Pnt2.Z() > H && Pnt3.Z() < H))
				{
					gp_Dir aDir2(gp_Vec(Pnt2, Pnt3));

					double d = -1 * (SliceDirection.Z() * H);
					double t2 = -(d + ((SliceDirection.X() * Pnt2.X()) + (SliceDirection.Y() * Pnt2.Y()) + (SliceDirection.Z() * Pnt2.Z()))) /
						((SliceDirection.X() * aDir2.X()) + (SliceDirection.Y() * aDir2.Y()) + (SliceDirection.Z() * aDir2.Z()));

					double myX2 = Pnt2.X() + (aDir2.X() * t2);
					double myY2 = Pnt2.Y() + (aDir2.Y() * t2);
					double myZ2 = Pnt2.Z() + (aDir2.Z() * t2);

					P1 = Pnt1;
					P2 = gp_Pnt(myX2, myY2, myZ2);

					myPoints.push_back(P1);
					myPoints.push_back(P2);
				}

				else if ((Pnt1.Z() < H && Pnt2.Z() == H && Pnt3.Z() > H) || (Pnt1.Z() > H && Pnt2.Z() == H && Pnt3.Z() < H))
				{
					gp_Dir aDir2(gp_Vec(Pnt1, Pnt3));

					double d = -1 * (SliceDirection.Z() * H);
					double t2 = -(d + ((SliceDirection.X() * Pnt1.X()) + (SliceDirection.Y() * Pnt1.Y()) + (SliceDirection.Z() * Pnt1.Z()))) /
						((SliceDirection.X() * aDir2.X()) + (SliceDirection.Y() * aDir2.Y()) + (SliceDirection.Z() * aDir2.Z()));

					double myX2 = Pnt1.X() + (aDir2.X() * t2);
					double myY2 = Pnt1.Y() + (aDir2.Y() * t2);
					double myZ2 = Pnt1.Z() + (aDir2.Z() * t2);

					P1 = Pnt2;
					P2 = gp_Pnt(myX2, myY2, myZ2);

					myPoints.push_back(P1);
					myPoints.push_back(P2);
				}

				else if ((Pnt1.Z() < H && Pnt2.Z() > H && Pnt3.Z() == H) || (Pnt1.Z() > H && Pnt2.Z() < H && Pnt3.Z() == H))
				{
					gp_Dir aDir2(gp_Vec(Pnt1, Pnt2));

					double d = -1 * (SliceDirection.Z() * H);
					double t2 = -(d + ((SliceDirection.X() * Pnt1.X()) + (SliceDirection.Y() * Pnt1.Y()) + (SliceDirection.Z() * Pnt1.Z()))) /
						((SliceDirection.X() * aDir2.X()) + (SliceDirection.Y() * aDir2.Y()) + (SliceDirection.Z() * aDir2.Z()));

					double myX2 = Pnt1.X() + (aDir2.X() * t2);
					double myY2 = Pnt1.Y() + (aDir2.Y() * t2);
					double myZ2 = Pnt1.Z() + (aDir2.Z() * t2);

					P1 = Pnt3;
					P2 = gp_Pnt(myX2, myY2, myZ2);

					myPoints.push_back(P1);
					myPoints.push_back(P2);
				}
				
				if (!myPoints.isEmpty())
				{
					aList.push_back(myPoints);
				}
			}
			
		}
	}

	return aList;
}