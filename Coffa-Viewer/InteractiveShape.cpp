#include "InteractiveShape.h"
#include "Doc.h"
#include "ActionRule.h"
#include "VarIDValue.h"
#include "AngCoord.h"
#include "AngPoint.h"
#include "VarPix.h"

//#include <im_Shadow.h>

#include <qdebug.h>
#include <omp.h>
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
	ResultWidget = new QWidget;
}

InteractiveShape::InteractiveShape(TopoDS_Shape aShape, Doc* aDoc)
{
	theShape = aShape;
	theOriginShape = aShape;
	theDoc = aDoc;
	alreadyComputedArea = false;

	Handle(AIS_Shape) AISPart = new AIS_Shape(theShape);

	IObject = AISPart;

	//theDoc->getContext()->Display(IObject, true);

	theOriginSurfaces = theSurfaces;
	mainItem = new QStandardItem(theName);
	QIcon iconPart;
	iconPart.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/Part.png")), QIcon::Normal, QIcon::Off);
	mainItem->setIcon(iconPart);
	mainItem->setEditable(false);

	geomItem = new QStandardItem("Geometry");
	geomItem->setEditable(false);

	suppItem = new QStandardItem("Support Structures");
	suppItem->setEditable(false);

	mainItem->appendRow(geomItem);
	mainItem->appendRow(suppItem);

	//Init Support Aspect

	ThreadsToUse = theDoc->ThreadsToUse;

	ResultWidget = new QWidget;

	
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
		aSurf->setShape(iShape);
		aSurf->Triangulate();
		aSurf->setAvailableThreads(threadCount);
        theSurfaces.push_back(aSurf);
		//QApplication::processEvents();
	}

	//organizeByCompartiments();
}

void InteractiveShape::setAvailableThreads(int threadsCount)
{
	threadCount = threadsCount;
}

void InteractiveShape::organizeByCompartiments()
{
	Bnd_Box B0;
	BRepBndLib::Add(theShape, B0);
	double Bxmin0, Bymin0, Bzmin0, Bxmax0, Bymax0, Bzmax0;
	B0.Get(Bxmin0, Bymin0, Bzmin0, Bxmax0, Bymax0, Bzmax0);

	double dimX = abs(Bxmax0 - Bxmin0);
	double dimY = abs(Bymax0 - Bymin0);
	double dimZ = abs(Bzmax0 - Bzmin0);

	double Px0 = ((Bxmax0)+(Bxmin0)) / 2;
	double Py0 = ((Bymax0)+(Bymin0)) / 2;
	double Pz0 = ((Bzmax0)+(Bzmin0)) / 2;
	//gp_Pnt initcenter(Px0, Py0, Bzmin0);

	QList<VarIDValue> comp1;
	QList<VarIDValue> comp2;
	QList<VarIDValue> comp3;
	int triangleCount = 0;
	if (dimX >= 30)
	{
		double limX1 = Bxmin0;
		double limX2 = Bxmin0 + dimX/2;
		double limX3 = Bxmax0;
		double limX4 = Bxmin0 + dimX / 4;
		double limX5 = Bxmax0 - dimX / 4;

		for (int i = 0; i < theSurfaces.size(); i++)
		{
			for (int j = 0; j < theSurfaces[i]->theTriangles.size(); j++)
			{
				triangleCount++;
				if (theSurfaces[i]->theTriangles[j].xmin >= limX1 && theSurfaces[i]->theTriangles[j].xmax <= limX5)
				{
					VarIDValue var;
					var.id = theSurfaces[i]->theTriangles[j].owner;
					var.extra2 = j;

					if (theSurfaces[i]->theTriangles[j].xmin < limX2)
					{
						theSurfaces[i]->theTriangles[j].compartiment = 1;
						theSurfaces[i]->theTriangles[j].compartiment2 = 2;
						comp1.push_back(var);
					}
					comp2.push_back(var);
				}

				if (theSurfaces[i]->theTriangles[j].xmin >= limX4 && theSurfaces[i]->theTriangles[j].xmax <= limX5)
				{
					VarIDValue var;
					var.id = theSurfaces[i]->theTriangles[j].owner;
					var.extra2 = j;

					if (theSurfaces[i]->theTriangles[j].xmin >= limX2)
					{
						theSurfaces[i]->theTriangles[j].compartiment = 3;
						theSurfaces[i]->theTriangles[j].compartiment2 = 2;

						comp3.push_back(var);
					}
					comp2.push_back(var);
				}
			}
		}
		organizedByXCompartiments = true;
		organizedByYCompartiments = false;
		organizedByZCompartiments = false;
	}

	else if (dimY >= 30)
	{
		double limY1 = Bymin0;
		double limY2 = Bymin0 + dimY / 2;
		double limY3 = Bymax0;
		double limY4 = Bymin0 + dimY / 4;
		double limY5 = Bymax0 - dimY / 4;

		for (int i = 0; i < theSurfaces.size(); i++)
		{
			for (int j = 0; j < theSurfaces[i]->theTriangles.size(); j++)
			{
				if (theSurfaces[i]->theTriangles[j].ymin >= limY1 && theSurfaces[i]->theTriangles[j].ymax <= limY5)
				{
					VarIDValue var;
					var.id = theSurfaces[i]->theTriangles[j].owner;
					var.extra2 = j;

					if (theSurfaces[i]->theTriangles[j].ymin < limY2)
					{
						theSurfaces[i]->theTriangles[j].compartiment = 1;
						theSurfaces[i]->theTriangles[j].compartiment2 = 2;

						comp1.push_back(var);
					}
					comp2.push_back(var);
				}

				if (theSurfaces[i]->theTriangles[j].ymin >= limY4 && theSurfaces[i]->theTriangles[j].ymax <= limY5)
				{
					VarIDValue var;
					var.id = theSurfaces[i]->theTriangles[j].owner;
					var.extra2 = j;

					if (theSurfaces[i]->theTriangles[j].ymin >= limY2)
					{
						theSurfaces[i]->theTriangles[j].compartiment = 3;
						theSurfaces[i]->theTriangles[j].compartiment = 2;

						comp3.push_back(var);
					}
					comp2.push_back(var);
				}
			}
		}
		organizedByXCompartiments = false;
		organizedByYCompartiments = true;
		organizedByZCompartiments = false;
	}

	else if (dimZ >= 30)
	{
		double limZ1 = Bzmin0;
		double limZ2 = Bzmin0 + dimZ / 2;
		double limZ3 = Bzmax0;
		double limZ4 = Bzmin0 + dimZ / 4;
		double limZ5 = Bzmax0 - dimZ / 4;

		for (int i = 0; i < theSurfaces.size(); i++)
		{
			for (int j = 0; j < theSurfaces[i]->theTriangles.size(); j++)
			{
				if (theSurfaces[i]->theTriangles[j].zmin >= limZ1 && theSurfaces[i]->theTriangles[j].zmax <= limZ5)
				{
					VarIDValue var;
					var.id = theSurfaces[i]->theTriangles[j].owner;
					var.extra2 = j;

					if (theSurfaces[i]->theTriangles[j].zmin < limZ2)
					{
						theSurfaces[i]->theTriangles[j].compartiment = 1;
						theSurfaces[i]->theTriangles[j].compartiment2 = 2;

						comp1.push_back(var);
					}
					comp2.push_back(var);
				}

				if (theSurfaces[i]->theTriangles[j].zmin >= limZ4 && theSurfaces[i]->theTriangles[j].zmax <= limZ5)
				{
					VarIDValue var;
					var.id = theSurfaces[i]->theTriangles[j].owner;
					var.extra2 = j;

					if (theSurfaces[i]->theTriangles[j].zmin >= limZ2)
					{
						theSurfaces[i]->theTriangles[j].compartiment = 3;
						theSurfaces[i]->theTriangles[j].compartiment2 = 2;

						comp3.push_back(var);
					}
					comp2.push_back(var);
				}
			}
		}
		organizedByXCompartiments = false;
		organizedByYCompartiments = false;
		organizedByZCompartiments = true;
	}

	else
	{
		for (int i = 0; i < theSurfaces.size(); i++)
		{
			for (int j = 0; j < theSurfaces[i]->theTriangles.size(); j++)
			{
				triangleCount++;

				theSurfaces[i]->theTriangles[j].compartiment = 1;
			}
		}
	}

	theCompartiments.push_back(comp1);
	theCompartiments.push_back(comp2);
	theCompartiments.push_back(comp3);
}

void InteractiveShape::organizeByCloseTriangles()
{
	for (int i = 0; i < theSurfaces.size(); i++)
	{
		for (int j = 0; j < theSurfaces[i]->theTriangles.size(); j++)
		{
			QList<VarIDValue> aList;
			for (int k = 0; k < theSurfaces.size(); k++)
			{
				for (int l = 0; l < theSurfaces[k]->theTriangles.size(); l++)
				{
					if (theSurfaces[i]->ID == theSurfaces[k]->ID &&
						theSurfaces[i]->theTriangles[j].id == theSurfaces[k]->theTriangles[l].id)
					{

					}

					else
					{
						double limxmin = theSurfaces[i]->theTriangles[j].inCenter.X() - 30;
						double limxmax = theSurfaces[i]->theTriangles[j].inCenter.X() + 30;
						double limymin = theSurfaces[i]->theTriangles[j].inCenter.Y() - 30;
						//gp_Vec aVec(theSurfaces[i]->theTriangles[j]
					}
				}
			}
		}
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
	theDoc->getContext()->Remove(RTSObject, true);
	theDoc->getContext()->Remove(SSDRObject, true);
	theDoc->getContext()->Remove(SupportObject, true);
	theDoc->getContext()->Remove(VisuObject, true);

	theDoc->getContext()->Remove(IObject, true);
}

void InteractiveShape::createResultWidget()
{
	ResultLayout = new QGridLayout;
	ResultWidget->setLayout(ResultLayout);
	layPosx = 1; layPosy = 0;

	for (int i = 0; i < theComputedValues.size(); i++)
	{
		if (layPosy >= 2)
		{
			layPosx++;
			layPosy = 0;
			ResultLayout->addWidget(theComputedValues[i].getResponse(),	layPosx, layPosy++, Qt::AlignCenter);
			theComputedValues[i].getResponse()->show();
		}
		else
		{
			ResultLayout->addWidget(theComputedValues[i].getResponse(), layPosx, layPosy++, Qt::AlignCenter);
			theComputedValues[i].getResponse()->show();
		}
	}
}

QWidget* InteractiveShape::getResultWidget()
{
	return ResultWidget;
}

void InteractiveShape::setActionRules(QList<ActionRule*> someRules)
{
	theActionRules = someRules;
	theComputedValues.clear();
	theCustComputedValues.clear();

	if (geomItem->rowCount() > 0)
	{
		geomItem->removeRows(0, geomItem->rowCount());
	}

	theAttributes.clear();

	for each (ActionRule* var in theActionRules)
	{
		VarARValues aVal;
		aVal.setID(var->getID());
		aVal.theResponse->setTitle(var->getName());
		aVal.theResponse->setProject(theDoc->myApp);
		//aVal.theResponse->setFixedSize(350, 350);
		if (var->getAttribute() != "All")
		{
			QStandardItem* anItem = new QStandardItem(var->getAttribute());
			anItem->setEditable(false);
			
			QColor aColor;
			aColor.setNamedColor(var->getColor());
			QVariant colData(aColor);
			anItem->setData(colData, Qt::DecorationRole);
			geomItem->appendRow(anItem);

			VarAttribute anAttrib;
			anAttrib.id = var->getID();
			anAttrib.isDisplayed = false;
			theAttributes.push_back(anAttrib);
		}
		
		theComputedValues.push_back(aVal);

		VarARValues aVal2 = aVal;
		theCustComputedValues.push_back(aVal2);
	}

	createResultWidget();
}

VarAttribute InteractiveShape::getAttributedSurfaces(QString arID)
{
	VarAttribute aVarAttrib;
	for each (VarAttribute var in theAttributes)
	{
		if (var.id == arID)
		{
			aVarAttrib = var;
			break;
		}
	}

	return aVarAttrib;
}

void InteractiveShape::setAttributedSurfaces(VarAttribute aVarAttrib)
{
	//qDebug() << aVarAttrib.items.size();
	for (int i = 0; i < theAttributes.size(); i++)
	{
		if (theAttributes[i].id == aVarAttrib.id)
		{
			theAttributes[i].items = aVarAttrib.items;
			if (theAttributes[i].isDisplayed)
			{
				//theAttributes[i].isDisplayed = false;
				showAttributedSurfaces(theAttributes[i].id);
				//theDoc->getContext()->Remove(theAttributes[i].theDisplayable, true);
			}

			
			break;
		}
	}
}

void InteractiveShape::showAttributedSurfaces(QString arID)
{
	TopoDS_Compound comp;
	BRep_Builder aB;
	aB.MakeCompound(comp);

	for (int i = 0; i < theAttributes.size(); i++)
	{
		if (theAttributes[i].id == arID)
		{
			theDoc->getContext()->Remove(theAttributes[i].theDisplayable, true);

			for (int j = 0; j < theAttributes[i].items.size(); j++)
			{
				int ind = theAttributes[i].items[j];
				if (ind < theSurfaces.size() && ind > -1)
				{
					aB.Add(comp, theSurfaces[ind]->getFace());
				}
			}

			Handle(AIS_Shape) aiss = new AIS_Shape(comp);
			Handle(AIS_InteractiveObject) attribObject = aiss;
			Handle(Prs3d_LineAspect) LineAspect = new Prs3d_LineAspect(Quantity_NOC_BLACK, Aspect_TOL_SOLID, 4);
			Handle(Prs3d_Drawer) rtsStyle = new Prs3d_Drawer();
			rtsStyle->SetLineAspect(LineAspect);
			rtsStyle->SetWireAspect(LineAspect);
			rtsStyle->SetWireDraw(true);

			QColor aColor(theActionRules[i]->getColor());
			attribObject->SetAttributes(rtsStyle);
			Graphic3d_MaterialAspect myMat1(Graphic3d_NOM_BRASS);
			myMat1.SetColor(Quantity_Color(aColor.redF(), aColor.greenF(), aColor.blueF(), Quantity_TOC_RGB));
			attribObject->SetMaterial(myMat1);
			attribObject->SetDisplayMode(AIS_Shaded);
			theAttributes[i].theDisplayable = attribObject;
			theDoc->getContext()->Display(theAttributes[i].theDisplayable, true);
			theDoc->getContext()->Deactivate(theAttributes[i].theDisplayable, TopAbs_FACE);
			theDoc->getContext()->Deactivate(theAttributes[i].theDisplayable, TopAbs_SHAPE);
			theDoc->getContext()->Deactivate(theAttributes[i].theDisplayable, TopAbs_EDGE);
			theAttributes[i].isDisplayed = true;

			break;
		}
	}
}

void InteractiveShape::hideAttributedSurfaces(QString arID)
{
	for (int i = 0; i < theAttributes.size(); i++)
	{
		if (theAttributes[i].id == arID)
		{
			theDoc->getContext()->Remove(theAttributes[i].theDisplayable, true);
			theAttributes[i].isDisplayed = false;
			break;
		}
	}
}

void InteractiveShape::hideAllAttributedSurfaces()
{
	for (int i = 0; i < theAttributes.size(); i++)
	{
		theDoc->getContext()->Remove(theAttributes[i].theDisplayable, true);
		theAttributes[i].isDisplayed = false;
		//break;
	}
}

void InteractiveShape::evaluateRules(QList<bool> interest)
{
	for each (ActionRule* var in theActionRules)
	{
		//var.theAttributed;
		VarARData arData;
		if (var->getCadType() == ActionRule::surfaceType && !var->isComposed())
		{
			if (var->getVarType() == ActionRule::normalAngle)
			{
				QList<VarIDValue> results = evaluateBySurfAngle(var, 1);

				double globalVal = 0;

				if (var->getActionType() == ActionRule::Avoid || var->getActionType() == ActionRule::Require)
				{
					globalVal = 1;
					for (int i = 0; i < results.size(); i++)
					{
						globalVal = globalVal * results[i].value * (results[i].extra / getTotalArea());
					}
				}

				else if (var->getActionType() == ActionRule::Minimize || var->getActionType() == ActionRule::Maximize)
				{
					globalVal = 0;
					for (int i = 0; i < results.size(); i++)
					{
						globalVal = globalVal + results[i].value * (results[i].extra / getTotalArea());
						int h = 0;
					}
				}

				arData.values = results;
				arData.Rx = Rx;
				arData.Ry = Ry;
				arData.Rz = Rz;
				arData.globalValue = globalVal;

				for (int i = 0; i < theComputedValues.size(); i++)
				{
					if (var->getID() == theComputedValues[i].getID())
					{
						theComputedValues[i].theData.push_back(arData);
					}
				}
			}
		}

		else if (var->getCadType() == ActionRule::surfaceType && var->isComposed())
		{
			double globalVal1 = 0;
			double globalVal2 = 0;
			QList<VarIDValue> results1;
			if (var->getVarType() == ActionRule::normalAngle)
			{
				results1 = evaluateBySurfAngle(var, 1);
			}


			QList<VarIDValue> results2;
			if (var->getVarType2() == ActionRule::minimumDistAround)
			{
				results2 = evaluateByDistance(var, 2);
			}
			
			QList<VarIDValue> finalResults;
			if (results1.size() == results2.size())
			{
				for (int i = 0; i < results1.size(); i++)
				{
					if (results1[i].id == results2[i].id)
					{
						double glo = std::max(results1[i].value, results2[i].value);
						VarIDValue aVal;
						aVal.value = glo;
						aVal.id = results1[i].id;
						aVal.extra = results1[i].extra;
						finalResults.push_back(aVal);
					}
				}
			}

			double globalVal;
			if (var->getActionType() == ActionRule::Avoid || var->getActionType() == ActionRule::Require)
			{
				globalVal = 1;
				for (int i = 0; i < finalResults.size(); i++)
				{
					globalVal = globalVal * finalResults[i].value * (finalResults[i].extra / getTotalArea());
				}
			}

			else if (var->getActionType() == ActionRule::Minimize || var->getActionType() == ActionRule::Maximize)
			{
				globalVal = 0;
				for (int i = 0; i < finalResults.size(); i++)
				{
					globalVal = globalVal + finalResults[i].value * (finalResults[i].extra / getTotalArea());
				}
			}

			arData.Rx = Rx;
			arData.Ry = Ry;
			arData.Rz = Rz;
			arData.globalValue = globalVal;

			for (int i = 0; i < theComputedValues.size(); i++)
			{
				if (var->getID() == theComputedValues[i].getID())
				{
					theComputedValues[i].theData.push_back(arData);
				}
			}
		}

		else if (var->getCadType() == ActionRule::volumeType && !var->isComposed())
		{
			if (var->getVarType() == ActionRule::volumeShadow)
			{
				//evaluateByShadow(var, 1);
				arData.Rx = Rx;
				arData.Ry = Ry;
				arData.Rz = Rz;
				arData.globalValue = 1;

				for (int i = 0; i < theComputedValues.size(); i++)
				{
					if (var->getID() == theComputedValues[i].getID())
					{
						theComputedValues[i].theData.push_back(arData);
					}
				}
			}

			else if (var->getVarType() == ActionRule::sliceMaxLength)
			{
				//evaluateByStackSliceLength(var, 1);
				arData.Rx = Rx;
				arData.Ry = Ry;
				arData.Rz = Rz;
				arData.globalValue = 1;

				for (int i = 0; i < theComputedValues.size(); i++)
				{
					if (var->getID() == theComputedValues[i].getID())
					{
						theComputedValues[i].theData.push_back(arData);
					}
				}
			}
		}
	}
}

void InteractiveShape::rotateAndEvaluateRules(gp_Quaternion aQuaternion, double xang, double yang, double zang, QList<bool> interests, AxisSequence aseq, int it1, int it2, int compMode)
{
	t1 = it1;
	
	if (t1 > 0)
	{
		ssdrPrepTime = 0;
	}
	rotateShape2(aQuaternion, xang, yang, zang);

	for (int n=0; n < theActionRules.size(); n++)
	{
		if (interests[n] == true)
		{
			ActionRule* var = theActionRules[n];
			for (int i = 0; i < theComputedValues.size(); i++)
			{
				if (var->getID() == theComputedValues[i].getID())
				{
					theComputedValues[i].getResponse()->shouldUpdate=true;
				}
			}

			
			VarARData arData;
			if (var->getCadType() == ActionRule::surfaceType && !var->isComposed())
			{
				if (var->getVarType() == ActionRule::normalAngle)
				{
					QList<VarIDValue> results = evaluateBySurfAngle(var, 1);

					double globalVal = 0;

					if (var->getActionType() == ActionRule::Avoid || var->getActionType() == ActionRule::Require)
					{
						globalVal = 1;
						for (int i = 0; i < results.size(); i++)
						{
							globalVal = globalVal * pow(results[i].value, (results[i].extra / getLimitedTotalArea(var)));
						}
					}

					else if (var->getActionType() == ActionRule::Minimize || var->getActionType() == ActionRule::Maximize)
					{
						globalVal = 0;
						for (int i = 0; i < results.size(); i++)
						{
							globalVal = globalVal + (results[i].value * (results[i].extra / getLimitedTotalArea(var)));
						}
					}

					arData.values = results;
					arData.Rx = Rx;
					arData.Ry = Ry;
					arData.Rz = Rz;
					arData.globalValue = globalVal;
					arData.iterator1 = it1;
					arData.iterator2 = it2;
					//qDebug() << globalVal;

					for (int i = 0; i < theComputedValues.size(); i++)
					{
						if (var->getID() == theComputedValues[i].getID())
						{
							if (compMode == 0)
							{
								theComputedValues[i].addData(arData);
								theComputedValues[i].setRelativeCalculated(false);
							}
							
							else
							{
								theCustComputedValues[i].addData(arData);
								theCustComputedValues[i].setRelativeCalculated(false);
							}
						}
					}
				}
			}

			else if (var->getCadType() == ActionRule::surfaceType && var->isComposed())
			{
				QList<VarIDValue> results = evaluateBySurfAngleAndDistance(var, 1);

				double globalVal = 0;

				if (var->getActionType() == ActionRule::Avoid || var->getActionType() == ActionRule::Require)
				{
					globalVal = 1;
					double relArea = 0;
					for (int i = 0; i < results.size(); i++)
					{
						relArea += results[i].extra;
					}

					for (int i = 0; i < results.size(); i++)
					{
						globalVal = globalVal * pow(results[i].value, (results[i].extra /relArea /*getLimitedTotalArea(var)*/));

					}
					//qDebug() <<"Global Value"<< globalVal;
				}

				else if (var->getActionType() == ActionRule::Minimize || var->getActionType() == ActionRule::Maximize)
				{
					globalVal = 0;
					for (int i = 0; i < results.size(); i++)
					{
						globalVal = globalVal + (results[i].value * (results[i].extra / getLimitedTotalArea(var)));
					}
				}

				arData.values = results;
				arData.Rx = Rx;
				arData.Ry = Ry;
				arData.Rz = Rz;
				arData.globalValue = globalVal;
				arData.iterator1 = it1;
				arData.iterator2 = it2;
				//qDebug() << globalVal;

				for (int i = 0; i < theComputedValues.size(); i++)
				{
					if (var->getID() == theComputedValues[i].getID())
					{
						if (compMode == 0)
						{
							theComputedValues[i].addData(arData);
							theComputedValues[i].setRelativeCalculated(false);
						}

						else
						{
							theCustComputedValues[i].addData(arData);
							theCustComputedValues[i].setRelativeCalculated(false);
						}
					}
				}

				/*double globalVal1 = 0;
				double globalVal2 = 0;
				QList<VarIDValue> results1;
				if (var->getVarType() == ActionRule::normalAngle)
				{
					results1 = evaluateBySurfAngle(var, 1);
				}

				QList<VarIDValue> results2;
				if (var->getVarType2() == ActionRule::minimumDistAround)
				{
					results2 = evaluateByDistance(var, 2);
				}

				QList<VarIDValue> finalResults;
				if (results1.size() == results2.size())
				{
					for (int i = 0; i < results1.size(); i++)
					{
						if (results1[i].id == results2[i].id)
						{
							double glo = std::max(results1[i].value, results2[i].value);
							VarIDValue aVal;
							aVal.value = glo;
							aVal.id = results1[i].id;
							aVal.extra = results1[i].extra;
							finalResults.push_back(aVal);
						}
					}
				}

				double globalVal;
				if (var->getActionType() == ActionRule::Avoid || var->getActionType() == ActionRule::Require)
				{
					globalVal = 1;
					for (int i = 0; i < finalResults.size(); i++)
					{
						globalVal = globalVal * pow(finalResults[i].value, (finalResults[i].extra / getLimitedTotalArea(var)));
					}
				}

				else if (var->getActionType() == ActionRule::Minimize || var->getActionType() == ActionRule::Maximize)
				{
					globalVal = 0;
					for (int i = 0; i < finalResults.size(); i++)
					{
						globalVal = globalVal + (finalResults[i].value * (finalResults[i].extra / getLimitedTotalArea(var)));
					}
				}

				arData.Rx = Rx;
				arData.Ry = Ry;
				arData.Rz = Rz;
				arData.globalValue = globalVal;
				arData.iterator1 = it1;
				arData.iterator2 = it2;


				for (int i = 0; i < theComputedValues.size(); i++)
				{
					if (var->getID() == theComputedValues[i].getID())
					{
						if (compMode == 0)
						{
							theComputedValues[i].addData(arData);
							theComputedValues[i].setRelativeCalculated(false);
						}

						else
						{
							theCustComputedValues[i].addData(arData);
							theCustComputedValues[i].setRelativeCalculated(false);
						}
					}
				}*/
			}

			else if (var->getCadType() == ActionRule::volumeType && !var->isComposed())
			{
				if (var->getVarType() == ActionRule::volumeShadow)
				{
					//evaluateByShadow(var, 1);
					double globalVal = evaluateByShadow(var, 1);
					arData.Rx = Rx;
					arData.Ry = Ry;
					arData.Rz = Rz;
					arData.globalValue = globalVal;
					arData.iterator1 = it1;
					arData.iterator2 = it2;
					
					for (int i = 0; i < theComputedValues.size(); i++)
					{
						if (var->getID() == theComputedValues[i].getID())
						{
							if (compMode == 0)
							{
								theComputedValues[i].addData(arData);
								theComputedValues[i].setRelativeCalculated(false);
							}

							else
							{
								theCustComputedValues[i].theData.push_back(arData);
								theCustComputedValues[i].setRelativeCalculated(false);
							}
						}
					}
				}

				else if (var->getVarType() == ActionRule::sliceMaxLength)
				{
					double globalVal = evaluateByStackSliceLength(var, 1);
					arData.Rx = Rx;
					arData.Ry = Ry;
					arData.Rz = Rz;
					arData.globalValue = globalVal;
					arData.iterator1 = it1;
					arData.iterator2 = it2;

					for (int i = 0; i < theComputedValues.size(); i++)
					{
						if (var->getID() == theComputedValues[i].getID())
						{
							if (compMode == 0)
							{
								theComputedValues[i].addData(arData);
								theComputedValues[i].setRelativeCalculated(false);
							}

							else
							{
								theCustComputedValues[i].theData.push_back(arData);
								theCustComputedValues[i].setRelativeCalculated(false);
							}
						}
					}
				}
			}

		}
	}
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

QList<VarIDValue> InteractiveShape::evaluateBySurfAngle(ActionRule *aRule, int whichFunc)
{
	QList<int> items;
	for each (VarAttribute var in theAttributes)
	{
		if (var.id == aRule->getID())
		{
			items = var.items;
			break;
		}
	}

	QList<VarIDValue> aList;
	double pi = 3.141592654;
	gp_Pnt Origin(0, 0, 0);
	gp_Dir Zo(0, 0, -1);
	gp_Vec myZfix(Zo);

	for (int i = 0; i < items.size(); i++)
	{
		if (items[i] < theSurfaces.size())
		{
			double myAngle;
			double Area = theSurfaces[items[i]]->getArea();
			double Ang_Pen;
			double totalValue1 = 1;
			double totalValue2 = 0;

			for (int f = 0; f < theSurfaces[items[i]]->theTriangles.size(); f++)
			{
				myAngle = theSurfaces[items[i]]->theTriangles[f].theUnitNormal.Angle(Zo);
				double sArea = theSurfaces[items[i]]->theTriangles[f].theArea;
				Ang_Pen = (myAngle * 180) / (pi);
				double theVal = 0;
				QList<QPointF> funcPoints;
				if (whichFunc == 1)
				{
					funcPoints = aRule->getPoints();
				}

				else
				{
					funcPoints = aRule->getPoints2();
				}

				for (int di = 0; di < funcPoints.size() - 1; di++)
				{
					QPointF aPf = funcPoints.at(di);
					QPointF bPf = funcPoints.at(di + 1);

					if (Ang_Pen >= aPf.x() && Ang_Pen < bPf.x() && aRule->getActionType() == ActionRule::Avoid)
					{
						if (aPf.y() == bPf.y())
						{
							theVal = pow((bPf.y()), (sArea / (Area)));
						}

						else if (aPf.y() < bPf.y())
						{
							double myVal = (((bPf.y() - aPf.y()) * ((Ang_Pen - aPf.x()))) / (bPf.x() - aPf.x())) + aPf.y();
							theVal = pow(myVal, (sArea / (Area)));
						}

						else if (aPf.y() > bPf.y())
						{
							double myVal = (((aPf.y() - bPf.y()) * (bPf.x() - (Ang_Pen))) / (bPf.x() - aPf.x())) + bPf.y();
							theVal = pow(myVal, (sArea / (Area)));
						}
						totalValue1 = totalValue1 * theVal;
					}

					else if (Ang_Pen >= aPf.x() && Ang_Pen < bPf.x() && aRule->getActionType() == ActionRule::Require)
					{
						if (aPf.y() == bPf.y())
						{
							theVal = pow((bPf.y()), (sArea / (Area)));
						}

						else if (aPf.y() < bPf.y())
						{
							double myVal = (((bPf.y() - aPf.y()) * ((Ang_Pen - aPf.x()))) / (bPf.x() - aPf.x())) + aPf.y();
							theVal = pow(myVal, (sArea / (Area)));
						}

						else if (aPf.y() > bPf.y())
						{
							double myVal = (((aPf.y() - bPf.y()) * (bPf.x() - (Ang_Pen))) / (bPf.x() - aPf.x())) + bPf.y();
							theVal = pow(myVal, (sArea / (Area)));
						}
						totalValue1 = totalValue1 * theVal;
					}

					else if (Ang_Pen >= aPf.x() && Ang_Pen < bPf.x() && aRule->getActionType() == ActionRule::Minimize)
					{
						if (aPf.y() == bPf.y())
						{
							theVal = (bPf.y()) * (sArea / (Area));
						}

						else if (aPf.y() < bPf.y())
						{
							double myVal = (((bPf.y() - aPf.y()) * ((Ang_Pen - aPf.x()))) / (bPf.x() - aPf.x())) + aPf.y();
							theVal = myVal * (sArea / (Area));
						}

						else if (aPf.y() > bPf.y())
						{
							double myVal = (((aPf.y() - bPf.y()) * (bPf.x() - (Ang_Pen))) / (bPf.x() - aPf.x())) + bPf.y();
							theVal = myVal * (sArea / (Area));
						}
						totalValue2 = totalValue2 + theVal;
					}

					else if (Ang_Pen >= aPf.x() && Ang_Pen < bPf.x() && aRule->getActionType() == ActionRule::Maximize)
					{
						if (aPf.y() == bPf.y())
						{
							theVal = (bPf.y()) * (sArea / (Area));
						}

						else if (aPf.y() < bPf.y())
						{
							double myVal = (((bPf.y() - aPf.y()) * ((Ang_Pen - aPf.x()))) / (bPf.x() - aPf.x())) + aPf.y();
							theVal = myVal * (sArea / (Area));
						}

						else if (aPf.y() > bPf.y())
						{
							double myVal = (((aPf.y() - bPf.y()) * (bPf.x() - (Ang_Pen))) / (bPf.x() - aPf.x())) + bPf.y();
							theVal = myVal * (sArea / (Area));
						}
						totalValue2 = totalValue2 + theVal;
					}
				}
			}

			if (aRule->getActionType() == ActionRule::Avoid || aRule->getActionType() == ActionRule::Require)
			{
				VarIDValue aval;
				aval.id = theSurfaces[items[i]]->getID();
				aval.value = totalValue1;
				aval.extra = Area;
				aList.push_back(aval);	
			}

			else if (aRule->getActionType() == ActionRule::Minimize || aRule->getActionType() == ActionRule::Maximize)
			{
				VarIDValue aval;
				aval.id = theSurfaces[items[i]]->getID();
				aval.value = totalValue2;
				aval.extra = Area;
				aList.push_back(aval);
			}
		}
	}
	return aList;
}

QList<VarIDValue> InteractiveShape::evaluateByDistance(ActionRule* aRule, int whichFunc)
{
	VarAttribute aVarAttrib;
	QList<int> items;
	for each (VarAttribute var in theAttributes)
	{
		if (var.id == aRule->getID())
		{
			items = var.items;
			break;
		}
	}

	if (t1 == 0)
	{
		ssdrPrepTimer = new QElapsedTimer();
		ssdrPrepTimer->start();
	}

	/*for (int i = 0; i < items.size(); i++)
	{
		if (items[i] < theSurfaces.size())
		{
			theSurfaces[items[i]]->Pixelize(1.0);
		}
	}*/

	if (t1 == 0)
	{
		ssdrPrepTime += ssdrPrepTimer->elapsed();
	}

	QList<VarIDValue> aList;
	for (int i = 0; i < items.size(); i++)
	{
		if (items[i] < theSurfaces.size())
		{
			if (t1 == 0)
			{
				ssdrPrepTimer->restart();
			}
			double mini = theSurfaces[items[i]]->getMinDistanceAround(theSurfaces);

			if (t1 == 0)
			{
				ssdrPrepTime += ssdrPrepTimer->elapsed();
			}

			double ValDist = 0;
			if (theSurfaces[items[i]]->hasAccessProblems())
			{
				if (!aRule->isRelative())
				{
					QList<QPointF> funcPoints;
					if (whichFunc == 1)
					{
						funcPoints = aRule->getPoints();
					}

					else
					{
						funcPoints = aRule->getPoints2();
					}


					for (int di = 0; di < funcPoints.size() - 1; di++)
					{
						QPointF aPf1 = funcPoints.at(di);
						QPointF bPf1 = funcPoints.at(di + 1);
						if (mini >= aPf1.x() && mini <= bPf1.x())
						{
							if (aPf1.y() == bPf1.y())
							{
								ValDist = bPf1.y();
							}

							else if (aPf1.y() < bPf1.y())
							{
								ValDist = (((bPf1.y() - aPf1.y()) * ((mini - aPf1.x()))) / (bPf1.x() - aPf1.x())) + aPf1.y();
							}

							else if (aPf1.y() > bPf1.y())
							{
								ValDist = (((aPf1.y() - bPf1.y()) * (bPf1.x() - mini)) / (bPf1.x() - aPf1.x())) + bPf1.y();
							}
						}
					}
				}

				else
				{
					ValDist = mini;
				}
			}


			else
			{
				ValDist = 1;
			}
			//qDebug() << "Desirability Distance " << ValDist << " " << mini;

			VarIDValue aVal;
			aVal.id = theSurfaces[items[i]]->getID();
			aVal.value = ValDist;
			aVal.extra = theSurfaces[items[i]]->getArea();
			aList.push_back(aVal);

			//qDebug() << mini;
		}


	}
	return aList;
}

QList<VarIDValue> InteractiveShape::evaluateBySurfAngleAndDistance(ActionRule* aRule, int whichFunc)
{
	gp_Dir Zo(0, 0, -1);
	double pi = 3.1415;
	int fun1, fun2;
	if (aRule->getVarType() == ActionRule::normalAngle)
	{
		fun1 = 1;
		fun2 = 2;
	}
	else
	{
		fun1 = 2;
		fun2 = 1;
	}
	VarAttribute aVarAttrib;
	QList<int> items;
	for each (VarAttribute var in theAttributes)
	{
		if (var.id == aRule->getID())
		{
			items = var.items;
			break;
		}
	}

	if (t1 == 0)
	{
		ssdrPrepTimer = new QElapsedTimer();
		ssdrPrepTimer->start();
	}

	QList<VarIDValue> aList;
	for (int i=0; i < items.size(); i++)
	{
		if (items[i] < theSurfaces.size())
		{
			double Area = theSurfaces[items[i]]->getArea();
			double totalValue1 = 1;
			double totalValue2 = 0;
			if (t1 == 0)
			{
				ssdrPrepTimer->restart();
			}
			theSurfaces[items[i]]->getMinDistanceAround(theSurfaces);

			if (t1 == 0)
			{
				ssdrPrepTime += ssdrPrepTimer->elapsed();
			}

			double ValDist = 0;
			if (theSurfaces[items[i]]->hasAccessProblems())
			{
				for (int f = 0; f < theSurfaces[items[i]]->theTriangles.size(); f++)
				{
					double mini = theSurfaces[items[i]]->theTriangles[f].minDistance;
					double maxVal = 1;
					if (!aRule->isRelative())
					{
						QList<QPointF> funcPoints2;
						if (fun2 == 1)
						{
							funcPoints2 = aRule->getPoints();
						}

						else
						{
							funcPoints2 = aRule->getPoints2();
						}
						double d1 = 0, d2 = 0;
						for (int di = 0; di < funcPoints2.size() - 1; di++)
						{
							QPointF aPf1 = funcPoints2.at(di);
							QPointF bPf1 = funcPoints2.at(di + 1);
							if (mini >= aPf1.x() && mini <= bPf1.x())
							{
								if (aPf1.y() == bPf1.y())
								{
									d2 = bPf1.y();
								}

								else if (aPf1.y() < bPf1.y())
								{
									d2 = (((bPf1.y() - aPf1.y()) * ((mini - aPf1.x()))) / (bPf1.x() - aPf1.x())) + aPf1.y();
								}

								else if (aPf1.y() > bPf1.y())
								{
									d2 = (((aPf1.y() - bPf1.y()) * (bPf1.x() - mini)) / (bPf1.x() - aPf1.x())) + bPf1.y();
								}
							}
						}

						QList<QPointF> funcPoints;
						if (fun1 == 1)
						{
							funcPoints = aRule->getPoints();
						}

						else
						{
							funcPoints = aRule->getPoints2();
						}

						double myAngle = theSurfaces[items[i]]->theTriangles[f].theUnitNormal.Angle(Zo);
						double sArea = theSurfaces[items[i]]->theTriangles[f].theArea;
						double Ang_Pen = (myAngle * 180) / (pi);
						double theVal = 0;

						for (int di = 0; di < funcPoints.size() - 1; di++)
						{
							QPointF aPf = funcPoints.at(di);
							QPointF bPf = funcPoints.at(di + 1);

							if (Ang_Pen >= aPf.x() && Ang_Pen < bPf.x())
							{
								if (aPf.y() == bPf.y())
								{
									d1 = bPf.y();
								}

								else if (aPf.y() < bPf.y())
								{
									double myVal = (((bPf.y() - aPf.y()) * ((Ang_Pen - aPf.x()))) / (bPf.x() - aPf.x())) + aPf.y();
									d1 = myVal;
								}

								else if (aPf.y() > bPf.y())
								{
									double myVal = (((aPf.y() - bPf.y()) * (bPf.x() - (Ang_Pen))) / (bPf.x() - aPf.x())) + bPf.y();
									d1 = myVal;
								}
							}
						}

						maxVal = std::max(d1, d2);

						if (aRule->getActionType() == ActionRule::Avoid || aRule->getActionType() == ActionRule::Require)
						{
							totalValue1 = totalValue1 * (pow(maxVal, (sArea / Area)));
						}

						else if (aRule->getActionType() == ActionRule::Minimize || aRule->getActionType() == ActionRule::Maximize)
						{
							totalValue2 = totalValue2 + (maxVal * (sArea / Area));
						}
					}

					else
					{
						totalValue1 = maxVal;
						totalValue2 = maxVal;
					}
				}

				//}
				/*else
				{
					totalValue1 = 1;
					totalValue2 = 1;
				}*/


				if (aRule->getActionType() == ActionRule::Avoid || aRule->getActionType() == ActionRule::Require)
				{
					//qDebug() << totalValue1;
					VarIDValue aval;
					aval.id = theSurfaces[items[i]]->getID();
					aval.value = totalValue1;
					aval.extra = Area;
					aList.push_back(aval);
				}

				else if (aRule->getActionType() == ActionRule::Minimize || aRule->getActionType() == ActionRule::Maximize)
				{
					VarIDValue aval;
					aval.id = theSurfaces[items[i]]->getID();
					aval.value = totalValue2;
					aval.extra = Area;
					aList.push_back(aval);
				}
			}
		}
	}
	return aList;
}

double InteractiveShape::evaluateByShadow(ActionRule* aRule, int whichFunc)
{
	//qDebug() << theSurfaces.size();
	double Shad = 0;

	Bnd_Box B;
	BRepBndLib::Add(theShape, B);
	double Bxmin, Bymin, Bzmin, Bxmax, Bymax, Bzmax;
	B.Get(Bxmin, Bymin, Bzmin, Bxmax, Bymax, Bzmax);
	int Xrange = abs((Bxmax)-(Bxmin));
	int Yrange = abs((Bymax)-(Bymin));

	gp_Dir zVert(0, 0, -1);

	for (int i = 0; i <= Xrange; i++)
	{
		double pX = Bxmin + (i);
		for (int j = 0; j <= Yrange; j++)
		{
			double pY= Bymin + (j);
			for (int s=0; s<theSurfaces.size(); s++)
			{
				InteractiveSurface* var = theSurfaces[s];
				if (pX >= var->xmin && pX <= var->xmax
					&& pY >= var->ymin && pY <= var->ymax)
				{
					bool inside = false;
//#pragma omp parallel default (none)
//					{
//#pragma omp for
						for (int n = 0; n < var->theTriangles.size(); n++)
						{
							if (pX >= var->theTriangles[n].xmin && pX <= var->theTriangles[n].xmax
								&& pY >= var->theTriangles[n].ymin && pY <= var->theTriangles[n].ymax)
							{
								gp_Pnt intPoint = var->theTriangles[n].getIntersectionPoint(zVert, gp_Pnt(pX, pY, Bzmax + 10));

								if (var->theTriangles[n].isPointInside(intPoint) || var->theTriangles[n].isPointOnContour(intPoint))
								{
									Shad = Shad + 1;
									inside = true;
									break;
								}
							}
						}
						
					//}
					if (inside)
						{
							break;
						}
				}
			}
		}
	}

	
	if (aRule->isRelative())
	{
		return Shad;
	}

	else
	{

		return Shad;
	}

}

double InteractiveShape::evaluateByStackSliceLength(ActionRule* aRule, int whichFunc)
{

	double leng=sliceIntersectMeasure();

	double theVal = 0;
	QList<QPointF> funcPoints;
	if (whichFunc == 1)
	{
		funcPoints = aRule->getPoints();
	}

	else
	{
		funcPoints = aRule->getPoints2();
	}


	for (int di = 0; di < funcPoints.size() - 1; di++)
	{
		QPointF aPf = funcPoints.at(di);
		QPointF bPf = funcPoints.at(di + 1);

		if (leng >= aPf.x() && leng < bPf.x())
		{
			if (aPf.y() == bPf.y())
			{
				theVal = bPf.y();
			}

			else if (aPf.y() < bPf.y())
			{
				double myVal = (((bPf.y() - aPf.y()) * ((leng - aPf.x()))) / (bPf.x() - aPf.x())) + aPf.y();
				theVal = myVal;
			}

			else if (aPf.y() > bPf.y())
			{
				double myVal = (((aPf.y() - bPf.y()) * (bPf.x() - (leng))) / (bPf.x() - aPf.x())) + bPf.y();
				theVal = myVal;
			}
		}
	}

	return theVal;
}

void InteractiveShape::visualizeDistribution(QString anIndex)
{
	theDoc->getContext()->Remove(VisuObject, true);
	if (anIndex == "none")
	{
		IObject->SetTransparency(0.0);
		theDoc->myViewer->Redraw();
		theDoc->myViewer->Update();
	}

	else
	{
		BRep_Builder aB;
		TopoDS_Compound comp;
		aB.MakeCompound(comp);

		ActionRule* aRule;
		for (int i = 0; i < theActionRules.size(); i++)
		{
			if (anIndex == theActionRules[i]->getID())
			{
				aRule = theActionRules[i];
				break;
			}
		}

		QList<int> items;
		for each (VarAttribute var in theAttributes)
		{
			if (var.id == aRule->getID())
			{
				items = var.items;
				break;
			}
		}

		if (!aRule->isComposed())
		{
			int whichFunc = 1;
			if (aRule->getVarType() == ActionRule::normalAngle)
			{
				double pi = 3.141592654;
				gp_Pnt Origin(0, 0, 0);
				gp_Dir Zo(0, 0, -1);

				for (int i = 0; i < items.size(); i++)
				{
					if (items[i] < theSurfaces.size())
					{
						double myAngle;
						double Area = theSurfaces[items[i]]->getArea();
						double Ang_Pen;
						Handle(Geom_Surface) mySurf = BRep_Tool::Surface(theSurfaces[items[i]]->getFace());
						GeomAdaptor_Surface anAdaptor(mySurf);
						double uMin, uMax, vMin, vMax;
						BRepTools::UVBounds(theSurfaces[items[i]]->getFace(), uMin, uMax, vMin, vMax);

						if (anAdaptor.GetType() == GeomAbs_Plane)
						{
							GeomLProp_SLProps props(mySurf, uMin, vMin, 1, 0.01);
							gp_Vec aNorm = props.Normal();

							if (theSurfaces[items[i]]->getFace().Orientation() == TopAbs_REVERSED)
								aNorm.Reverse();
							if (aNorm.Magnitude() >= 0.05)
							{
								myAngle = aNorm.Angle(Zo);

								Ang_Pen = (myAngle * 180) / (pi);
								double theVal = 0;
								QList<QPointF> funcPoints = aRule->getPoints();

								for (int di = 0; di < funcPoints.size() - 1; di++)
								{
									QPointF aPf = funcPoints.at(di);
									QPointF bPf = funcPoints.at(di + 1);

									if (Ang_Pen >= aPf.x() && Ang_Pen < bPf.x())
									{
										if (aPf.y() == bPf.y())
										{
											theVal = bPf.y() /*pow((bPf.y()), (Area / (Area)))*/;
										}

										else if (aPf.y() < bPf.y())
										{
											double myVal = (((bPf.y() - aPf.y()) * ((Ang_Pen - aPf.x()))) / (bPf.x() - aPf.x())) + aPf.y();
											theVal = myVal /*pow(myVal, (Area / (Area)))*/;
										}

										else if (aPf.y() > bPf.y())
										{
											double myVal = (((aPf.y() - bPf.y()) * (bPf.x() - (Ang_Pen))) / (bPf.x() - aPf.x())) + bPf.y();
											theVal = myVal /*pow(myVal, (Area / (Area)))*/;
										}
									}
								}

								if (theVal < 0.5)
								{
									aB.Add(comp, theSurfaces[items[i]]->getFace());
								}
							}
						}

						else
						{
							for (int f = 0; f < theSurfaces[items[i]]->theTriangles.size(); f++)
							{
								myAngle = theSurfaces[items[i]]->theTriangles[f].theUnitNormal.Angle(Zo);
								double sArea = theSurfaces[items[i]]->theTriangles[f].theArea;
								Ang_Pen = (myAngle * 180) / (pi);
								double theVal = 0;
								QList<QPointF> funcPoints = aRule->getPoints();

								for (int di = 0; di < funcPoints.size() - 1; di++)
								{
									QPointF aPf = funcPoints.at(di);
									QPointF bPf = funcPoints.at(di + 1);

									if (Ang_Pen >= aPf.x() && Ang_Pen < bPf.x())
									{
										if (aPf.y() == bPf.y())
										{
											theVal = bPf.y();
										}

										else if (aPf.y() < bPf.y())
										{
											double myVal = (((bPf.y() - aPf.y()) * ((Ang_Pen - aPf.x()))) / (bPf.x() - aPf.x())) + aPf.y();
											theVal = myVal;
										}

										else if (aPf.y() > bPf.y())
										{
											double myVal = (((aPf.y() - bPf.y()) * (bPf.x() - (Ang_Pen))) / (bPf.x() - aPf.x())) + bPf.y();
											theVal = myVal;
										}
									}
								}

								if (theVal < 0.52)
								{
									BRepBuilderAPI_MakePolygon aP;
									aP.Add(theSurfaces[items[i]]->theTriangles[f].Pn1);
									aP.Add(theSurfaces[items[i]]->theTriangles[f].Pn2);
									aP.Add(theSurfaces[items[i]]->theTriangles[f].Pn3);
									aP.Close();

									if (!aP.Wire().IsNull())
									{
										BRepBuilderAPI_MakeFace aM(aP.Wire());
										if (aM.IsDone())
											aB.Add(comp, aM.Face());
									}
								}
							}
						}
					}
				}
			}

			else if (aRule->getVarType() == ActionRule::minimumDistAround)
			{
				double pi = 3.141592654;
				gp_Pnt Origin(0, 0, 0);
				gp_Dir Zo(0, 0, -1);

				double thresholdDistance = 10;
				QList<QPointF> funcPoints = aRule->getPoints2();

				for (int di = 0; di < funcPoints.size() - 1; di++)
				{
					QPointF aPf1 = funcPoints.at(di);
					QPointF bPf1 = funcPoints.at(di + 1);
					if (0.51 >= aPf1.y() && 0.51 <= bPf1.y())
					{
						if (aPf1.y() < bPf1.y())
						{
							thresholdDistance = (bPf1.x() - aPf1.x()) * ((0.5 - aPf1.y()) / (bPf1.y() - aPf1.y())) + aPf1.x();
						}
					}
				}
				VarAttribute aVarAttrib;
				QList<int> items;
				for each (VarAttribute var in theAttributes)
				{
					if (var.id == aRule->getID())
					{
						items = var.items;
						break;
					}
				}

				QList<VarIDValue> aList;
				for (int i = 0; i < items.size(); i++)
				{
					if (items[i] < theSurfaces.size())
					{
						double mini = theSurfaces[items[i]]->getMinDistanceAround(theSurfaces);

						double ValDist = 0;
						if (theSurfaces[items[i]]->hasAccessProblems())
						{
							QList<CF_Triangle> someTriangles = theSurfaces[items[i]]->getInvolvedTriangles(thresholdDistance);

							for (int t = 0; t < someTriangles.size(); t++)
							{
								BRepBuilderAPI_MakePolygon aP;
								aP.Add(someTriangles[t].Pn1);
								aP.Add(someTriangles[t].Pn2);
								aP.Add(someTriangles[t].Pn3);
								aP.Close();

								if (!aP.Wire().IsNull())
								{
									BRepBuilderAPI_MakeFace aM(aP.Wire());
									if (aM.IsDone())
										aB.Add(comp, aM.Face());
								}
							}
						}
					}
				}
			}
		}

		if (aRule->isComposed())
		{
			if (aRule->getVarType() == ActionRule::normalAngle && aRule->getVarType2() == ActionRule::minimumDistAround)
			{
				double pi = 3.141592654;
				gp_Pnt Origin(0, 0, 0);
				gp_Dir Zo(0, 0, -1);

				int fun1, fun2;
				if (aRule->getVarType() == ActionRule::normalAngle)
				{
					fun1 = 1;
					fun2 = 2;
				}
				else
				{
					fun1 = 2;
					fun2 = 1;
				}
				VarAttribute aVarAttrib;
				QList<int> items;
				for each (VarAttribute var in theAttributes)
				{
					if (var.id == aRule->getID())
					{
						items = var.items;
						break;
					}
				}

				QList<VarIDValue> aList;
				for (int i = 0; i < items.size(); i++)
				{
					if (items[i] < theSurfaces.size())
					{
						double Area = theSurfaces[items[i]]->getArea();
						double totalValue1 = 1;
						double totalValue2 = 0;

						theSurfaces[items[i]]->getMinDistanceAround(theSurfaces);

						double ValDist = 0;
						if (theSurfaces[items[i]]->hasAccessProblems())
						{
							for (int f = 0; f < theSurfaces[items[i]]->theTriangles.size(); f++)
							{
								double mini = theSurfaces[items[i]]->theTriangles[f].minDistance;
								double maxVal = 1;
								if (!aRule->isRelative())
								{
									QList<QPointF> funcPoints2;
									if (fun2 == 1)
									{
										funcPoints2 = aRule->getPoints();
									}

									else
									{
										funcPoints2 = aRule->getPoints2();
									}
									double d1 = 0, d2 = 0;
									for (int di = 0; di < funcPoints2.size() - 1; di++)
									{
										QPointF aPf1 = funcPoints2.at(di);
										QPointF bPf1 = funcPoints2.at(di + 1);
										if (mini >= aPf1.x() && mini <= bPf1.x())
										{
											if (aPf1.y() == bPf1.y())
											{
												d2 = bPf1.y();
											}

											else if (aPf1.y() < bPf1.y())
											{
												d2 = (((bPf1.y() - aPf1.y()) * ((mini - aPf1.x()))) / (bPf1.x() - aPf1.x())) + aPf1.y();
											}

											else if (aPf1.y() > bPf1.y())
											{
												d2 = (((aPf1.y() - bPf1.y()) * (bPf1.x() - mini)) / (bPf1.x() - aPf1.x())) + bPf1.y();
											}
										}
									}

									QList<QPointF> funcPoints;
									if (fun1 == 1)
									{
										funcPoints = aRule->getPoints();
									}

									else
									{
										funcPoints = aRule->getPoints2();
									}

									double myAngle = theSurfaces[items[i]]->theTriangles[f].theUnitNormal.Angle(Zo);
									double sArea = theSurfaces[items[i]]->theTriangles[f].theArea;
									double Ang_Pen = (myAngle * 180) / (pi);

									for (int di = 0; di < funcPoints.size() - 1; di++)
									{
										QPointF aPf = funcPoints.at(di);
										QPointF bPf = funcPoints.at(di + 1);

										if (Ang_Pen >= aPf.x() && Ang_Pen < bPf.x())
										{
											if (aPf.y() == bPf.y())
											{
												d1 = bPf.y();
											}

											else if (aPf.y() < bPf.y())
											{
												double myVal = (((bPf.y() - aPf.y()) * ((Ang_Pen - aPf.x()))) / (bPf.x() - aPf.x())) + aPf.y();
												d1 = myVal;
											}

											else if (aPf.y() > bPf.y())
											{
												double myVal = (((aPf.y() - bPf.y()) * (bPf.x() - (Ang_Pen))) / (bPf.x() - aPf.x())) + bPf.y();
												d1 = myVal;
											}
										}
									}

									maxVal = std::max(d1, d2);

									if (maxVal < 0.50)
									{
										//qDebug() << theSurfaces[items[i]]->theTriangles[f].minDistance;
										BRepBuilderAPI_MakePolygon aP;
										aP.Add(theSurfaces[items[i]]->theTriangles[f].Pn1);
										aP.Add(theSurfaces[items[i]]->theTriangles[f].Pn2);
										aP.Add(theSurfaces[items[i]]->theTriangles[f].Pn3);
										aP.Close();

										if (!aP.Wire().IsNull())
										{
											BRepBuilderAPI_MakeFace aM(aP.Wire());
											if (aM.IsDone())
												aB.Add(comp, aM.Face());
										}
									}
								}
							}
						}
					}
				}
			}

			else if (aRule->getVarType2() == ActionRule::normalAngle && aRule->getVarType() == ActionRule::minimumDistAround)
			{
				double pi = 3.141592654;
				gp_Pnt Origin(0, 0, 0);
				gp_Dir Zo(0, 0, -1);

				int fun1, fun2;
				if (aRule->getVarType() == ActionRule::normalAngle)
				{
					fun1 = 1;
					fun2 = 2;
				}
				else
				{
					fun1 = 2;
					fun2 = 1;
				}
				VarAttribute aVarAttrib;
				QList<int> items;
				for each (VarAttribute var in theAttributes)
				{
					if (var.id == aRule->getID())
					{
						items = var.items;
						break;
					}
				}

				QList<VarIDValue> aList;
				for (int i = 0; i < items.size(); i++)
				{
					if (items[i] < theSurfaces.size())
					{
						double Area = theSurfaces[items[i]]->getArea();
						double totalValue1 = 1;
						double totalValue2 = 0;

						theSurfaces[items[i]]->getMinDistanceAround(theSurfaces);

						double ValDist = 0;
						if (theSurfaces[items[i]]->hasAccessProblems())
						{
							for (int f = 0; f < theSurfaces[items[i]]->theTriangles.size(); f++)
							{
								double mini = theSurfaces[items[i]]->theTriangles[f].minDistance;
								double maxVal = 1;
								if (!aRule->isRelative())
								{
									QList<QPointF> funcPoints2;
									if (fun2 == 1)
									{
										funcPoints2 = aRule->getPoints();
									}

									else
									{
										funcPoints2 = aRule->getPoints2();
									}
									double d1 = 0, d2 = 0;
									for (int di = 0; di < funcPoints2.size() - 1; di++)
									{
										QPointF aPf1 = funcPoints2.at(di);
										QPointF bPf1 = funcPoints2.at(di + 1);
										if (mini >= aPf1.x() && mini <= bPf1.x())
										{
											if (aPf1.y() == bPf1.y())
											{
												d2 = bPf1.y();
											}

											else if (aPf1.y() < bPf1.y())
											{
												d2 = (((bPf1.y() - aPf1.y()) * ((mini - aPf1.x()))) / (bPf1.x() - aPf1.x())) + aPf1.y();
											}

											else if (aPf1.y() > bPf1.y())
											{
												d2 = (((aPf1.y() - bPf1.y()) * (bPf1.x() - mini)) / (bPf1.x() - aPf1.x())) + bPf1.y();
											}
										}
									}

									QList<QPointF> funcPoints;
									if (fun1 == 1)
									{
										funcPoints = aRule->getPoints();
									}

									else
									{
										funcPoints = aRule->getPoints2();
									}

									double myAngle = theSurfaces[items[i]]->theTriangles[f].theUnitNormal.Angle(Zo);
									double sArea = theSurfaces[items[i]]->theTriangles[f].theArea;
									double Ang_Pen = (myAngle * 180) / (pi);

									for (int di = 0; di < funcPoints.size() - 1; di++)
									{
										QPointF aPf = funcPoints.at(di);
										QPointF bPf = funcPoints.at(di + 1);

										if (Ang_Pen >= aPf.x() && Ang_Pen < bPf.x())
										{
											if (aPf.y() == bPf.y())
											{
												d1 = bPf.y();
											}

											else if (aPf.y() < bPf.y())
											{
												double myVal = (((bPf.y() - aPf.y()) * ((Ang_Pen - aPf.x()))) / (bPf.x() - aPf.x())) + aPf.y();
												d1 = myVal;
											}

											else if (aPf.y() > bPf.y())
											{
												double myVal = (((aPf.y() - bPf.y()) * (bPf.x() - (Ang_Pen))) / (bPf.x() - aPf.x())) + bPf.y();
												d1 = myVal;
											}
										}
									}

									maxVal = std::max(d1, d2);

									if (maxVal < 0.50)
									{
										//qDebug() << theSurfaces[items[i]]->theTriangles[f].minDistance;
										BRepBuilderAPI_MakePolygon aP;
										aP.Add(theSurfaces[items[i]]->theTriangles[f].Pn1);
										aP.Add(theSurfaces[items[i]]->theTriangles[f].Pn2);
										aP.Add(theSurfaces[items[i]]->theTriangles[f].Pn3);
										aP.Close();

										if (!aP.Wire().IsNull())
										{
											BRepBuilderAPI_MakeFace aM(aP.Wire());
											if (aM.IsDone())
												aB.Add(comp, aM.Face());
										}
									}
								}
							}
						}
					}
				}
			}

			Handle(AIS_Shape) aiss = new AIS_Shape(comp);
			VisuObject = aiss;
			VisuObject->SetDisplayMode(AIS_Shaded);
			Graphic3d_MaterialAspect myMat1(Graphic3d_NOM_BRASS);
			myMat1.SetColor(Quantity_Color(0.90, 0.05, 0.05, Quantity_TOC_RGB));
			VisuObject->SetMaterial(myMat1);
			theDoc->getContext()->Display(VisuObject, true);
			theDoc->getContext()->Deactivate(VisuObject, AIS_Shape::SelectionMode(TopAbs_SHAPE));
			theDoc->getContext()->Deactivate(VisuObject, AIS_Shape::SelectionMode(TopAbs_EDGE));
			theDoc->getContext()->Deactivate(VisuObject, AIS_Shape::SelectionMode(TopAbs_FACE));
			theDoc->getContext()->Deactivate(VisuObject, AIS_Shape::SelectionMode(TopAbs_VERTEX));

			IObject->SetTransparency(0.85);

			theDoc->myViewer->Redraw();
			theDoc->myViewer->Update();

			hideAllAttributedSurfaces();
		}
	}
}

void InteractiveShape::genRTS(double thresholdAngle, double space)
{
	RTSPath.clear();
	theDoc->getContext()->Remove(RTSObject, true);
	theDoc->getContext()->Remove(SupportObject, true);

	gp_Dir slicedir(0, 0, 1);
	
	TopoDS_Compound comp;
	BRep_Builder aB;
	aB.MakeCompound(comp);

	double pi = 3.141592654;
	gp_Pnt Origin(0, 0, 0);
	gp_Dir Zo(0, 0, -1);
	gp_Vec myZfix(Zo);

	Bnd_Box B;
	BRepBndLib::Add(theShape, B);
	double Bxmin, Bymin, Bzmin, Bxmax, Bymax, Bzmax;
	B.Get(Bxmin, Bymin, Bzmin, Bxmax, Bymax, Bzmax);

	for each (InteractiveSurface* var in theSurfaces)
	{
		double uMin, uMax, vMin, vMax;
		BRepTools::UVBounds(var->getFace(), uMin, uMax, vMin, vMax);
		Handle(Geom_Surface) mySurf = BRep_Tool::Surface(var->getFace());
		GeomAdaptor_Surface anAdaptor(mySurf);

		TopLoc_Location bLocation;
		Handle_Poly_Triangulation bTr = BRep_Tool::Triangulation(var->getFace(), bLocation);
		const Poly_Array1OfTriangle& triangles = bTr->Triangles();

		if (!bTr.IsNull())
		{
			double Area = var->getArea();
			gp_Vec Normy;
			double tri_Ang;

			Standard_Integer Nb_Tr = bTr->NbTriangles();
			Standard_Integer Nt = 0, n1, n2, n3;
			const TColgp_Array1OfPnt& bNodes = bTr->Nodes();

			for (Nt = 1; Nt < Nb_Tr + 1; Nt++)
			{
				if (var->getFace().Orientation() == TopAbs_FORWARD)
					triangles(Nt).Get(n1, n2, n3);
				else
					triangles(Nt).Get(n2, n1, n3);

				gp_Pnt Pnt1 = bNodes(n1).Transformed(bLocation);
				gp_Pnt Pnt2 = bNodes(n2).Transformed(bLocation);
				gp_Pnt Pnt3 = bNodes(n3).Transformed(bLocation);

				gp_Vec Vec_A(Pnt1, Pnt2);
				gp_Vec Vec_B(Pnt1, Pnt3);
				Normy = Vec_A.Crossed(Vec_B);
				double om = 0.5 * (Vec_A.CrossMagnitude(Vec_B));
				if (om > 0.0005)
				{
					tri_Ang = (myZfix.Angle(Normy) * 180) / (pi);
					if (tri_Ang <= thresholdAngle && tri_Ang >= 5)
					{
						QList<QList<gp_Pnt>> points = sliceTriangle(Pnt1, Pnt2, Pnt3, gp_Dir(Normy), gp_Dir(0, 0, 1), Bzmin, Bzmax, space);

						if (!points.isEmpty())
						{
							for each (QList<gp_Pnt> aSegment in points)
							{
								if (!aSegment.isEmpty())
								{
									BRepBuilderAPI_MakeEdge anEd(aSegment[0], aSegment[1]);
									aB.Add(comp, anEd.Edge());
									RTSPath.push_back(aSegment);
								}
							}
						}
					}
				}
			}
		}
	}

	Handle(AIS_Shape) aiss = new AIS_Shape(comp);
	RTSObject = aiss;
	Handle(Prs3d_LineAspect) LineAspect = new Prs3d_LineAspect(Quantity_NOC_PINK, Aspect_TOL_SOLID, 4);
	Handle(Prs3d_Drawer) rtsStyle = new Prs3d_Drawer();
	rtsStyle->SetLineAspect(LineAspect);
	rtsStyle->SetWireAspect(LineAspect);
	rtsStyle->SetWireDraw(true);
	RTSObject->SetAttributes(rtsStyle);
	theDoc->getContext()->Display(RTSObject, true);
}

void InteractiveShape::genRTS2(double thresholdAngle, double space, double space2)
{
	TopoDS_Compound comp;
	BRep_Builder aB;
	aB.MakeCompound(comp);
	gp_Vec zdir(0, 0, -1);
	double thre = cos(thresholdAngle * 3.14 / 180);
	gp_Pln aPlane(gp_Pnt(0, 0, thre), gp_Dir(0, 0, 1));
	BRepBuilderAPI_MakeFace af(aPlane);
	Handle(Geom_Surface) refFace = BRep_Tool::Surface(af.Face());

	for each (InteractiveSurface * var in theSurfaces)
	{
		QList<AngCoord> aList;
		QList<gp_Pnt> contourPoints;
		QList<CF_Pixel> levelPixels;

		Handle(Geom_Surface) mySurf = BRep_Tool::Surface(var->getFace());
		GeomAdaptor_Surface anAdaptor(mySurf);
		double uMin, uMax, vMin, vMax;
		BRepTools::UVBounds(var->getFace(), uMin, uMax, vMin, vMax);

		if (anAdaptor.GetType() == GeomAbs_Plane)
		{
			GeomLProp_SLProps props(mySurf, uMin, vMin, 1, 0.01);
			gp_Vec aNorm = props.Normal();

			if (var->getFace().Orientation() == TopAbs_REVERSED)
				aNorm.Reverse();
			if (aNorm.Magnitude() > 0.1)
			{
				double myAng = aNorm.Angle(zdir);
				if (myAng <= (thresholdAngle * 3.14) / 180)
				{
					TopExp_Explorer ExpFace;
					TopTools_IndexedMapOfShape myIndexMap;
					TopExp Mapper;
					Mapper.MapShapes(var->getFace(), TopAbs_WIRE, myIndexMap);

					for (int m = 1; m <= myIndexMap.Size(); m++)
					{
						TopoDS_Shape iShape = myIndexMap.FindKey(m);
						TopoDS_Wire iWire = TopoDS::Wire(myIndexMap.FindKey(m));
						aB.Add(comp, iWire);
					}
				}
			}
		}

		else
		{
			QList<double> uvMax;
			double pixSize = 0.5;
			//QList<CF_Pixel> finerMap= var->FinerPixelization(pixSize);

			//qDebug() << finerMap.size();
			//for (int i = 0; i < finerMap.size(); i++)
			//{
			//	double myAngle = finerMap[i].getDirection().Angle(zdir);
			//	
			//	if (cos(myAngle) >= thre)
			//	{
			//		//qDebug() << cos(myAngle);
			//		uvMax.push_back(finerMap[i].getCenter().X());
			//		uvMax.push_back(finerMap[i].getCenter().Y());
			//		CF_Pixel apix=finerMap[i];
			//		apix.setLevel(cos(myAngle));
			//		levelPixels.push_back(apix);
			//	}
			//}

			for (int i = 0; i < var->surfaceMap.size(); i++)
			{
				double myAngle = var->surfaceMap[i].getDirection().Angle(zdir);

				if (cos(myAngle) >= thre)
				{
					uvMax.push_back(var->surfaceMap[i].getCenter().X());
					uvMax.push_back(var->surfaceMap[i].getCenter().Y());
					CF_Pixel apix = var->surfaceMap[i];
					apix.setLevel(cos(myAngle));
					levelPixels.push_back(apix);
				}

			}

			if (!levelPixels.isEmpty())
			{
				QList<QList<CF_Pixel>> pixGroups;

				QList<CF_Pixel> aList2 = levelPixels;
				std::sort(uvMax.begin(), uvMax.end());
				double incmax = uvMax.last();
				int it = std::round(incmax / pixSize);
				it = it*2;
				double increm = pixSize;
				QList<CF_Pixel> stickGroup1;
				stickGroup1.push_back(levelPixels.first());
				CF_Pixel basePix = levelPixels.first();
				aList2.removeFirst();
				for (int i = 0; i < it; i++)
				{
					increm += /*increm + i**/ pixSize;

					QList<CF_Pixel> bufList = basePix.whoIsDomain(increm, aList2);
					//qDebug() << bufList.size();
					bool someStick = false;
					if (!bufList.isEmpty())
					{
						for (int n = 0; n < bufList.size(); n++)
						{
							for (int m = 0; m < stickGroup1.size(); m++)
							{
								if (stickGroup1[m].isStuckTo(bufList[n]))
								{
									stickGroup1.push_back(bufList[n]);
									someStick = true;
									for (int k = 0; k < aList2.size(); k++)
									{
										if (aList2[k].getCenter().IsEqual(bufList[n].getCenter(), 0.001))
										{
											aList2.removeAt(k);
											break;
										}
									}
									break;
								}
							}
						}

						if (!someStick && !aList2.isEmpty())
						{
							pixGroups.push_back(stickGroup1);
							//qDebug() << stickGroup1.size();
							basePix = aList2[0];
							i = 0;
							increm = pixSize;

							if (stickGroup1.isEmpty())
							{
								break;
							}

							else
							{
								stickGroup1.clear();
							}
							//break;
						}

						else if (aList2.isEmpty())
						{
							pixGroups.push_back(stickGroup1);
							stickGroup1.clear();
							break;
						}
					}

					else
					{
						if (!aList2.isEmpty())
						{
							pixGroups.push_back(stickGroup1);
							basePix = aList2[0];
							i = 0;
							increm = pixSize;
							stickGroup1.clear();
						}
						
						else
						{
							pixGroups.push_back(stickGroup1);
							stickGroup1.clear();
							break;
						}
					}

					
				}

				for (int i = 0; i < pixGroups.size(); i++)
				{
					QList<CF_Pixel> pixList = pixGroups[i];
					QList<CF_Pixel> pixList2 = pixGroups[i];
					QList<VarPix> sorterList;
					QList<double> sorterValues;
					
					QList<QList<CF_Pixel>> arrangedPix;
					for (int j = 0; j < pixList.size(); j++)
					{
						QList<CF_Pixel> vList;
						vList.push_back(pixList[j]);
						for (int k = 0; k < pixList2.size(); k++)
						{
							if (abs(vList[0].getCenter().X()-pixList2[k].getCenter().X()) <= 0.001)
							{
								vList.push_back(pixList2[k]);
								for (int n = 0; n < pixList.size(); n++)
								{
									if (pixList[n].getCenter().IsEqual(pixList2[k].getCenter(), 0.001))
									{
										pixList.removeAt(n);
									}
								}
							}
						}

						if (vList.size() > 0)
						{
							VarPix av;
							av.pixList = vList;
							av.xVal = vList[0].getCenter().X();
							sorterList.push_back(av);
							sorterValues.push_back(vList[0].getCenter().X());
						}
						j = 0;
					}

					if (!sorterValues.isEmpty())
					{
						std::sort(sorterValues.begin(), sorterValues.end());

						for (int s = 0; s < sorterValues.size(); s++)
						{
							for (int p = 0; p < sorterList.size(); p++)
							{
								if (abs(sorterList[p].xVal - sorterValues[s]) <= 0.001)
								{
									arrangedPix.push_back(sorterList[p].pixList);
									break;
								}
							}
						}
					}

					if (arrangedPix.size() > 1)
					{
						QList<double> maxSize;
						for (int j = 0; j < arrangedPix.size(); j++)
						{
							QList<CF_Pixel> dupliList;
							for (int k = 0; k < arrangedPix[j].size(); k++)
							{
								bool isInside = false;

								for (int p = 0; p < dupliList.size(); p++)
								{
									if (arrangedPix[j][k].getCenter().IsEqual(dupliList[p].getCenter(), 0.001))
									{
										isInside = true;
										break;
									}
								}

								if (!isInside)
								{
									dupliList.push_back(arrangedPix[j][k]);
								}
							}
							maxSize.push_back(dupliList.size());

							arrangedPix.replace(j, dupliList);	
						}

						if (!maxSize.isEmpty())
						{
							std::sort(maxSize.begin(), maxSize.end());
							TColgp_Array2OfPnt arrPnt(1, arrangedPix.size(), 1, maxSize.last());
							bool cantMake = false;
							for (int j = 0; j < arrangedPix.size(); j++)
							{
								if (arrangedPix[j].size() > 1)
								{
									for (int k = 0; k < arrangedPix[j].size(); k++)
									{
										arrPnt.SetValue(j + 1, k + 1, arrangedPix[j][k].getCenter());
									}
								}

								else 
								{
									cantMake = true;
								}
							}

							if (!cantMake)
							{
								GeomAPI_PointsToBSplineSurface aBspline(arrPnt, 3, 8, GeomAbs_C3, 0.001);
								//aBspline.Interpolate(arrPnt, false);
								Handle(Geom_Surface) aSurface = aBspline.Surface();

								BRepBuilderAPI_MakeFace aM(aSurface, 0.001);
								aB.Add(comp, aM.Face());
							}
						}
					}

					/*if (arrangedPix.size() > 1)
					{
						QList<Handle(Geom_Curve)> myCurves;
						for (int n = 0; n < arrangedPix.size(); n++)
						{
							if (arrangedPix[n].size() >= 2)
							{
								QList<gp_Pnt> dupliList;
								for (int j = 0; j < arrangedPix[n].size(); j++)
								{
									bool isInside = false;

									for (int p = 0; p < dupliList.size(); p++)
									{
										if (arrangedPix[n][j].getCenter().IsEqual(dupliList[p], 0.001))
										{
											isInside = true;
											break;
										}
									}

									if (!isInside)
									{
										dupliList.push_back(arrangedPix[n][j].getCenter());
									}
								}

								if (dupliList.size() > 1)
								{
									Handle(TColgp_HArray1OfPnt) arrPnt = new TColgp_HArray1OfPnt(1, dupliList.size());

									for (int j = 0; j < dupliList.size(); j++)
									{
										arrPnt->SetValue(j + 1, dupliList[j]);
									}
									GeomAPI_Interpolate interp(arrPnt, false, 0.001);
									interp.Perform();
									if (interp.IsDone())
										myCurves.push_back(interp.Curve());
								}
								
							}
						}

						for (int c = 0; c < myCurves.size() - 1; c++)
						{
							Handle(Geom_Surface) aSurf = GeomFill::Surface(myCurves[c], myCurves[c + 1]);

							BRepBuilderAPI_MakeFace aM(aSurf, 0.001);
							aB.Add(comp, aM.Face());
						}
					}*/
					//QList<gp_Pnt> aContourUV = outerBoundOfPixels(pixGroups[i]);
					//QList<gp_Pnt> aContour;

					//for (int n = 0; n < aContourUV.size(); n++)
					//{
					//	aContour.push_back(mySurf->Value(aContourUV[n].X(), aContourUV[n].Y()));
					//}
					////qDebug() << aContour.size();
					//if (aContour.size()>1)
					//{
					//	aContour = organizePointCCWiseUV(aContour);
					//	BRepBuilderAPI_MakePolygon myPolygon;

					//	for (int n = 0; n < aContour.size(); n++)
					//	{
					//		//myPolygon.Add(aContour[n]);
					//		BRepBuilderAPI_MakeVertex myVert(aContour[n]);
					//		aB.Add(comp, myVert.Vertex());
					//	}
					//	/*if (!myPolygon.Wire().IsNull())
					//		aB.Add(comp, myPolygon.Wire());*/
					//}
				}
			}
		}
	}

	Handle(AIS_Shape) aiss = new AIS_Shape(comp);
	RTSObject = aiss;
	Handle(Prs3d_LineAspect) LineAspect = new Prs3d_LineAspect(Quantity_NOC_RED, Aspect_TOL_SOLID, 3);
	Handle(Prs3d_Drawer) rtsStyle = new Prs3d_Drawer();
	rtsStyle->SetLineAspect(LineAspect);
	rtsStyle->SetWireAspect(LineAspect);
	rtsStyle->SetWireDraw(true);
	RTSObject->SetAttributes(rtsStyle);
	RTSObject->SetDisplayMode(AIS_Shaded);
	theDoc->getContext()->Display(RTSObject, true);
	theDoc->getContext()->Remove(IObject, true);
	//IObject->SetTransparency(0.5);

}

void InteractiveShape::genRTS3(double thresholdAngle, double space, double space2)
{
	TopoDS_Compound comp;
	BRep_Builder aB;
	aB.MakeCompound(comp);
	gp_Vec zdir(0, 0, -1);
	double thre = cos(thresholdAngle * 3.14 / 180);
	gp_Pln aPlane(gp_Pnt(0, 0, thre), gp_Dir(0, 0, 1));
	BRepBuilderAPI_MakeFace af(aPlane);
	Handle(Geom_Surface) refFace = BRep_Tool::Surface(af.Face());

	for each (InteractiveSurface * var in theSurfaces)
	{
		QList<AngCoord> aList;
		QList<gp_Pnt> contourPoints;
		QList<CF_Pixel> levelPixels;

		Handle(Geom_Surface) mySurf = BRep_Tool::Surface(var->getFace());
		GeomAdaptor_Surface anAdaptor(mySurf);
		double uMin, uMax, vMin, vMax;
		BRepTools::UVBounds(var->getFace(), uMin, uMax, vMin, vMax);

		if (anAdaptor.GetType() == GeomAbs_Plane)
		{
			GeomLProp_SLProps props(mySurf, uMin, vMin, 1, 0.01);
			gp_Vec aNorm = props.Normal();

			if (var->getFace().Orientation() == TopAbs_REVERSED)
				aNorm.Reverse();
			if (aNorm.Magnitude() > 0.1)
			{
				double myAng = aNorm.Angle(zdir);
				if (myAng <= (thresholdAngle * 3.14) / 180)
				{
					TopExp_Explorer ExpFace;
					TopTools_IndexedMapOfShape myIndexMap;
					TopExp Mapper;
					Mapper.MapShapes(var->getFace(), TopAbs_WIRE, myIndexMap);

					for (int m = 1; m <= myIndexMap.Size(); m++)
					{
						TopoDS_Shape iShape = myIndexMap.FindKey(m);
						TopoDS_Wire iWire = TopoDS::Wire(myIndexMap.FindKey(m));
						aB.Add(comp, iWire);
					}
				}
			}
		}

		else
		{
			QList<double> uvMax;
			double pixSize = 0.5;

			for (int i = 0; i < var->surfaceMap.size(); i++)
			{
				double myAngle = var->surfaceMap[i].getDirection().Angle(zdir);

				if (cos(myAngle) >= thre)
				{
					uvMax.push_back(var->surfaceMap[i].getCenter().X());
					uvMax.push_back(var->surfaceMap[i].getCenter().Y());
					CF_Pixel apix = var->surfaceMap[i];
					apix.setLevel(cos(myAngle));
					levelPixels.push_back(apix);
				}

			}

			if (!levelPixels.isEmpty())
			{
				QList<QList<CF_Pixel>> pixGroups;

				QList<CF_Pixel> aList2 = levelPixels;
				std::sort(uvMax.begin(), uvMax.end());
				double incmax = uvMax.last();
				int it = std::round(incmax / pixSize);
				it = it * 2;
				double increm = pixSize;
				QList<CF_Pixel> stickGroup1;
				stickGroup1.push_back(levelPixels.first());
				CF_Pixel basePix = levelPixels.first();
				aList2.removeFirst();
				for (int i = 0; i < it; i++)
				{
					increm += /*increm + i**/ pixSize;

					QList<CF_Pixel> bufList = basePix.whoIsDomain(increm, aList2);
					//qDebug() << bufList.size();
					bool someStick = false;
					if (!bufList.isEmpty())
					{
						for (int n = 0; n < bufList.size(); n++)
						{
							for (int m = 0; m < stickGroup1.size(); m++)
							{
								if (stickGroup1[m].isStuckTo(bufList[n]))
								{
									stickGroup1.push_back(bufList[n]);
									someStick = true;
									for (int k = 0; k < aList2.size(); k++)
									{
										if (aList2[k].getCenter().IsEqual(bufList[n].getCenter(), 0.001))
										{
											aList2.removeAt(k);
											break;
										}
									}
									break;
								}
							}
						}

						if (!someStick && !aList2.isEmpty())
						{
							pixGroups.push_back(stickGroup1);
							//qDebug() << stickGroup1.size();
							basePix = aList2[0];
							i = 0;
							increm = pixSize;

							if (stickGroup1.isEmpty())
							{
								break;
							}

							else
							{
								stickGroup1.clear();
							}
							//break;
						}

						else if (aList2.isEmpty())
						{
							pixGroups.push_back(stickGroup1);
							stickGroup1.clear();
							break;
						}
					}

					else
					{
						if (!aList2.isEmpty())
						{
							pixGroups.push_back(stickGroup1);
							basePix = aList2[0];
							i = 0;
							increm = pixSize;
							stickGroup1.clear();
						}

						else
						{
							pixGroups.push_back(stickGroup1);
							stickGroup1.clear();
							break;
						}
					}


				}

				for (int i = 0; i < pixGroups.size(); i++)
				{
					/*QList<CF_Pixel> pixList = pixGroups[i];
					QList<CF_Pixel> pixList2 = pixGroups[i];
					QList<VarPix> sorterList;
					QList<double> sorterValues;

					QList<QList<CF_Pixel>> arrangedPix;
					for (int j = 0; j < pixList.size(); j++)
					{
						QList<CF_Pixel> vList;
						vList.push_back(pixList[j]);
						for (int k = 0; k < pixList2.size(); k++)
						{
							if (abs(vList[0].getCenter().X() - pixList2[k].getCenter().X()) <= 0.001)
							{
								vList.push_back(pixList2[k]);
								for (int n = 0; n < pixList.size(); n++)
								{
									if (pixList[n].getCenter().IsEqual(pixList2[k].getCenter(), 0.001))
									{
										pixList.removeAt(n);
									}
								}
							}
						}

						if (vList.size() > 0)
						{
							VarPix av;
							av.pixList = vList;
							av.xVal = vList[0].getCenter().X();
							sorterList.push_back(av);
							sorterValues.push_back(vList[0].getCenter().X());
						}
						j = 0;
					}

					if (!sorterValues.isEmpty())
					{
						std::sort(sorterValues.begin(), sorterValues.end());

						for (int s = 0; s < sorterValues.size(); s++)
						{
							for (int p = 0; p < sorterList.size(); p++)
							{
								if (abs(sorterList[p].xVal - sorterValues[s]) <= 0.001)
								{
									arrangedPix.push_back(sorterList[p].pixList);
									break;
								}
							}
						}
					}*/


					QList<gp_Pnt> aContourUV = outerBoundOfPixels(pixGroups[i]);
					QList<gp_Pnt> aContour;

					/*for (int n = 0; n < aContourUV.size(); n++)
					{
						aContour.push_back(mySurf->Value(aContourUV[n].X(), aContourUV[n].Y()));
					}*/
					//qDebug() << aContour.size();
					if (aContourUV.size()>1)
					{
						//aContour = organizePointCCWiseUV(aContour);
						BRepBuilderAPI_MakePolygon myPolygon;

						for (int n = 0; n < aContourUV.size(); n++)
						{
							//myPolygon.Add(aContour[n]);
							BRepBuilderAPI_MakeVertex myVert(aContourUV[n]);
							aB.Add(comp, myVert.Vertex());
						}
						/*if (!myPolygon.Wire().IsNull())
							aB.Add(comp, myPolygon.Wire());*/
					}
				}
			}
		}
	}

	Handle(AIS_Shape) aiss = new AIS_Shape(comp);
	RTSObject = aiss;
	Handle(Prs3d_LineAspect) LineAspect = new Prs3d_LineAspect(Quantity_NOC_RED, Aspect_TOL_SOLID, 3);
	Handle(Prs3d_Drawer) rtsStyle = new Prs3d_Drawer();
	rtsStyle->SetLineAspect(LineAspect);
	rtsStyle->SetWireAspect(LineAspect);
	rtsStyle->SetWireDraw(true);
	RTSObject->SetAttributes(rtsStyle);
	RTSObject->SetDisplayMode(AIS_Shaded);
	theDoc->getContext()->Display(RTSObject, true);
	//theDoc->getContext()->Remove(IObject, true);
	IObject->SetTransparency(0.5);

}

void InteractiveShape::genRTS(double thresholdAngle, double space, double space2)
{
	TopoDS_Compound comp;
	BRep_Builder aB;
	aB.MakeCompound(comp);
	gp_Vec zdir(0, 0, -1);
	double thre = cos(thresholdAngle * 3.14 / 180);
	gp_Pln aPlane(gp_Pnt(0, 0, thre), gp_Dir(0, 0, 1));
	BRepBuilderAPI_MakeFace af(aPlane);
	Handle(Geom_Surface) refFace = BRep_Tool::Surface(af.Face());

	for each (InteractiveSurface * var in theSurfaces)
	{
		QList<AngCoord> aList;
		QList<gp_Pnt> contourPoints;
		Handle(Geom_Surface) mySurf = BRep_Tool::Surface(var->getFace());
		GeomAdaptor_Surface anAdaptor(mySurf);
		double uMin, uMax, vMin, vMax;
		BRepTools::UVBounds(var->getFace(), uMin, uMax, vMin, vMax);

		double stepElement = 0.05;

		if (anAdaptor.GetType() == GeomAbs_Plane)
		{
			GeomLProp_SLProps props(mySurf, uMin, vMin, 1, 0.01);
			gp_Vec aNorm = props.Normal();

			if (var->getFace().Orientation() == TopAbs_REVERSED)
				aNorm.Reverse();
			if (aNorm.Magnitude() > 0.1)
			{
				double myAng = aNorm.Angle(zdir);
				if (myAng <= (thresholdAngle * 3.14) / 180)
				{
					//allContours.push_back(mySurf);

					TopExp_Explorer ExpFace;
					TopTools_IndexedMapOfShape myIndexMap;
					TopExp Mapper;
					Mapper.MapShapes(var->getFace(), TopAbs_WIRE, myIndexMap);

					for (int m = 1; m <= myIndexMap.Size(); m++)
					{
						TopoDS_Shape iShape = myIndexMap.FindKey(m);
						TopoDS_Wire iWire = TopoDS::Wire(myIndexMap.FindKey(m));
						aB.Add(comp, iWire);
					}
				}
			}
		}

		else
		{
			Handle(Geom_Curve) aCurv1 = mySurf->VIso(vMin);
			if (!aCurv1.IsNull())
			{
				Standard_Real aLength = GCPnts_AbscissaPoint::Length(GeomAdaptor_Curve(aCurv1, uMin, uMax));
				if (aLength >= 1)
				{
					int spaceu = std::round(aLength / stepElement);

					QList<QList<gp_Pnt>> myPoints;
					QList<double> sCount;
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

									if (var->getFace().Orientation() == TopAbs_REVERSED)
										aNorm.Reverse();
									if (aNorm.Magnitude() > 0.5)
									{
										double myAng = aNorm.Angle(zdir);
										if (myAng <= 1.0466)
										{
											AngCoord ac;
											ac.theCos = cos(myAng);
											ac.uParam = udim;
											ac.vParam = vdim;
											pnts.push_back(gp_Pnt(udim, vdim, cos(myAng)));
											aList.push_back(ac);
										}
									}
								}
								myPoints.push_back(pnts);
								sCount.push_back(pnts.size());
							}
						}
					}

					if (aList.size() > 2)
					{
						QList<Handle(Geom_Curve)> myCurves;
						for (int i = 0; i < myPoints.size(); i++)
						{
							if (myPoints[i].size() >= 2)
							{
								Handle(TColgp_HArray1OfPnt) arrPnt = new TColgp_HArray1OfPnt(1, myPoints[i].size());
								for (int j = 0; j < myPoints[i].size(); j++)
								{
									arrPnt->SetValue(j + 1, myPoints[i][j]);
								}
								GeomAPI_Interpolate interp(arrPnt, false, 0.0001);
								interp.Perform();
								if (interp.IsDone())
									myCurves.push_back(interp.Curve());
							}
						}

						//Intersection Curves
						QList<Handle(Geom_Curve)> listofInter;
						for (int c = 0; c < myCurves.size() - 1; c++)
						{
							Handle(Geom_Surface) aSurf = GeomFill::Surface(myCurves[c], myCurves[c + 1]);

							GeomAPI_IntSS inters(aSurf, refFace, 0.001);

							if (inters.NbLines() > 0)
							{
								for (int it = 1; it <= inters.NbLines(); it++)
								{
									listofInter.push_back(inters.Line(it));
								}
							}
							/*BRepBuilderAPI_MakeFace aMakeFace(aSurf, 0.01);
							aB.Add(comp, aMakeFace.Face());*/
						}

						if (listofInter.size() == 0)
						{
							if (aList.size() > 1)
							{
								if (aList[0].theCos >= thre && aList.last().theCos >= thre)
								{
									TopExp_Explorer ExpFace;
									TopTools_IndexedMapOfShape myIndexMap;
									TopExp Mapper;
									Mapper.MapShapes(var->getFace(), TopAbs_WIRE, myIndexMap);

									for (int m = 1; m <= myIndexMap.Size(); m++)
									{
										TopoDS_Shape iShape = myIndexMap.FindKey(m);
										TopoDS_Wire iWire = TopoDS::Wire(myIndexMap.FindKey(m));
										aB.Add(comp, iWire);
									}
								}
							}
						}

						else if (listofInter.size() > 0)
						{

							//Additionnal Points to Close the contours
							gp_Pln aPlane1(gp_Pnt(uMin, vMin, thre), gp_Dir(0, 1, 0));
							BRepBuilderAPI_MakeFace af1(aPlane1);
							Handle(Geom_Surface) refFace1 = BRep_Tool::Surface(af1.Face());

							gp_Pln aPlane2(gp_Pnt(uMin, vMax, thre), gp_Dir(0, 1, 0));
							BRepBuilderAPI_MakeFace af2(aPlane2);
							Handle(Geom_Surface) refFace2 = BRep_Tool::Surface(af2.Face());

							QList<gp_Pnt> additionalPoints;
							QList<QList<gp_Pnt>> additionalPoints2;

							for (int k = 0; k < listofInter.size(); k++)
							{
								QList<gp_Pnt> pp;
								GeomAPI_IntCS inters(listofInter[k], refFace1);
								if (inters.NbPoints() > 0)
								{
									for (int it = 1; it <= inters.NbPoints(); it++)
									{
										pp.push_back(inters.Point(it));
									}
								}

								GeomAPI_IntCS inters1(listofInter[k], refFace2);
								if (inters1.NbPoints() > 0)
								{
									for (int it = 1; it <= inters1.NbPoints(); it++)
									{
										pp.push_back(inters1.Point(it));
									}
								}

								if (pp.size() == 2)
								{
									additionalPoints.push_back(pp[0]);
									additionalPoints.push_back(pp[1]);
									additionalPoints2.push_back(pp);
								}
							}

							QList<QList<Handle(Geom_Curve)>> listofContours;

							if (additionalPoints2.size() == 0)
							{
								for (int k = 0; k < listofInter.size(); k++)
								{

								}
							}

							if (additionalPoints2.size() >= 2)
							{
								QList<AngPoint> ord;
								QList<double> vals;
								for (int a = 0; a < additionalPoints2.size() - 1; a++)
								{
									AngPoint aP;
									aP.theAngle = additionalPoints2[a][0].X();
									aP.pointList = additionalPoints2[a];
									ord.push_back(aP);
									vals.push_back(additionalPoints2[a][0].X());
								}

								std::sort(vals.begin(), vals.end());
								QList<QList<gp_Pnt>> orderedPoints;
								QList<double> valList;
								for (int n = 0; n < vals.size(); n++)
								{
									for (int m = 0; m < ord.size(); m++)
									{
										if (vals[n] == ord[m].theAngle)
										{
											bool dupli = false;
											for (int o = 0; o < valList.size(); o++)
											{
												if (valList[o] == vals[n])
												{
													dupli = true;
													break;
												}
											}
											if (!dupli)
											{
												orderedPoints.push_back(ord[m].pointList);
												valList.push_back(vals[n]);
												break;
											}
										}
									}
								}
								additionalPoints2 = orderedPoints;

								//Verify and Add Umin and Umax Curves
								/*if (additionalPoints2.first()[0].X() > uMin || additionalPoints2.first()[1].X() > uMin)
								{
									gp_Pnt p1(uMin, vMin, thre);
									gp_Pnt p2(uMin, vMax, thre);
									QList<gp_Pnt> p1p2;

									p1p2.push_back(p1);
									p1p2.push_back(p2);

									additionalPoints2.insert(0, p1p2);

								}*/

								/*if (additionalPoints2.last()[0].X() < uMax || additionalPoints2.last()[1].X() < uMax)
								{
									gp_Pnt p1(uMax, vMin, thre);
									gp_Pnt p2(uMax, vMax, thre);
									QList<gp_Pnt> p1p2;

									p1p2.push_back(p1);
									p1p2.push_back(p2);

									additionalPoints2.push_back(p1p2);
								}*/

								for (int a = 0; a < additionalPoints2.size() - 1; a++)
								{
									gp_Pnt p1 = additionalPoints2[a][0];
									gp_Pnt p2 = additionalPoints2[a][1];
									gp_Pnt p3 = additionalPoints2[a + 1][0];
									gp_Pnt p4 = additionalPoints2[a + 1][1];

									if (!p1.IsEqual(p3, 0.005) && !p2.IsEqual(p4, 0.005))
									{
										bool croiss = false;
										for (int d = 0; d < aList.size() - 1; d++)
										{
											if (p1.X() >= aList[d].uParam && p1.X() <= aList[d + 1].uParam)
											{
												if (aList[d].theCos < aList[d + 1].theCos)
												{
													croiss = true;
													break;
												}
											}
										}

										if (croiss)
										{
											QList<Handle(Geom_Curve)> myContour;

											Handle(TColgp_HArray1OfPnt) arrPnt1 = new TColgp_HArray1OfPnt(1, 2);
											arrPnt1->SetValue(1, p2);
											arrPnt1->SetValue(2, p1);

											GeomAPI_Interpolate interp1(arrPnt1, false, 0.0001);
											interp1.Perform();
											if (interp1.IsDone())
												myContour.push_back(interp1.Curve());

											//
											Handle(TColgp_HArray1OfPnt) arrPnt2 = new TColgp_HArray1OfPnt(1, 2);
											arrPnt2->SetValue(1, p1);
											arrPnt2->SetValue(2, p3);

											GeomAPI_Interpolate interp2(arrPnt2, false, 0.0001);
											interp2.Perform();
											if (interp2.IsDone())
												myContour.push_back(interp2.Curve());

											//
											Handle(TColgp_HArray1OfPnt) arrPnt3 = new TColgp_HArray1OfPnt(1, 2);
											arrPnt3->SetValue(1, p3);
											arrPnt3->SetValue(2, p4);

											GeomAPI_Interpolate interp3(arrPnt3, false, 0.0001);
											interp3.Perform();
											if (interp3.IsDone())
												myContour.push_back(interp3.Curve());

											//
											Handle(TColgp_HArray1OfPnt) arrPnt4 = new TColgp_HArray1OfPnt(1, 2);
											arrPnt4->SetValue(1, p4);
											arrPnt4->SetValue(2, p2);

											GeomAPI_Interpolate interp4(arrPnt4, false, 0.0001);
											interp4.Perform();
											if (interp4.IsDone())
												myContour.push_back(interp4.Curve());

											listofContours.push_back(myContour);
										}
									}
								}
							}

							else if (additionalPoints2.size() == 1)
							{
								//Check Evolution of the COsine, find the increasing side
								gp_Pnt p1, p2;
								QList<Handle(Geom_Curve)> myContour;
								if (additionalPoints[0].X() == uMin || additionalPoints[1].X() == uMin)
								{
									p1 = gp_Pnt(uMax, additionalPoints[0].Y(), thre);
									p2 = gp_Pnt(uMax, additionalPoints[1].Y(), thre);
								}

								else if (additionalPoints[0].X() == uMax || additionalPoints[1].X() == uMax)
								{
									p1 = gp_Pnt(uMin, additionalPoints[0].Y(), thre);
									p2 = gp_Pnt(uMin, additionalPoints[1].Y(), thre);
								}

								else
								{
									for (int d = 0; d < aList.size() - 1; d++)
									{
										if (additionalPoints[0].X() >= aList[d].uParam
											&& additionalPoints[0].X() <= aList[d + 1].uParam)
										{
											if (aList[d].theCos > aList[d + 1].theCos)
											{
												p1 = gp_Pnt(uMin, additionalPoints[0].Y(), thre);
												p2 = gp_Pnt(uMin, additionalPoints[1].Y(), thre);
												break;
											}

											else if (aList[d].theCos < aList[d + 1].theCos)
											{
												p1 = gp_Pnt(uMax, additionalPoints[0].Y(), thre);
												p2 = gp_Pnt(uMax, additionalPoints[1].Y(), thre);
												break;
											}
										}
									}
								}

								Handle(TColgp_HArray1OfPnt) arrPnt1 = new TColgp_HArray1OfPnt(1, 2);
								arrPnt1->SetValue(1, additionalPoints[0]);
								arrPnt1->SetValue(2, p1);

								GeomAPI_Interpolate interp1(arrPnt1, false, 0.0001);
								interp1.Perform();
								if (interp1.IsDone())
									listofInter.push_back(interp1.Curve());

								Handle(TColgp_HArray1OfPnt) arrPnt2 = new TColgp_HArray1OfPnt(1, 2);
								arrPnt2->SetValue(1, additionalPoints[1]);
								arrPnt2->SetValue(2, p2);

								GeomAPI_Interpolate interp2(arrPnt2, false, 0.0001);
								interp2.Perform();
								if (interp2.IsDone())
									myContour.push_back(interp2.Curve());

								if (!p1.IsEqual(p2, 0.005))
								{
									Handle(TColgp_HArray1OfPnt) arrPnt3 = new TColgp_HArray1OfPnt(1, 2);
									arrPnt3->SetValue(1, p1);
									arrPnt3->SetValue(2, p2);

									GeomAPI_Interpolate interp3(arrPnt3, false, 0.0001);
									interp3.Perform();
									if (interp3.IsDone())
										myContour.push_back(interp3.Curve());
								}

								for (int l = 0; l < listofInter.size(); l++)
								{
									myContour.push_back(listofInter[l]);
								}
								listofContours.push_back(myContour);

							}

							else if (additionalPoints2.size() == 0)
							{
								if (listofInter.size() != 0)
									listofContours.push_back(listofInter);
							}

							QList<QList<gp_Pnt>> correspondingUV;
							for (int c = 0; c < listofContours.size(); c++)
							{
								QList<gp_Pnt> contPoints;
								for (int k = 0; k < listofContours[c].size(); k++)
								{
									Standard_Real totalLength = GCPnts_AbscissaPoint::
										Length(GeomAdaptor_Curve(listofContours[c][k], listofContours[c][k]->FirstParameter(), listofContours[c][k]->LastParameter()));

									int spacev = totalLength / stepElement;
									for (int v = 0; v <= spacev; v++)
									{
										double vdim = listofContours[c][k]->FirstParameter() +
											((listofContours[c][k]->LastParameter() - listofContours[c][k]->FirstParameter()) * v * stepElement) / totalLength;

										contPoints.push_back(listofContours[c][k]->Value(vdim));
									}
								}
								correspondingUV.push_back(contPoints);
							}

							if (correspondingUV.size() >= 1)
							{
								for (int c = 0; c < correspondingUV.size(); c++)
								{
									//Sort the Points in Counter Clock Wise
									QList<gp_Pnt> cUV = organizePointCCWiseUV(correspondingUV[c]);

									//Handle(TColgp_HArray1OfPnt) arrPnt = new TColgp_HArray1OfPnt(1, correspondingUV.size());
									QList<gp_Pnt> arrPnt;
									BRepBuilderAPI_MakePolygon myPolygon;
									for (int k = 0; k < cUV.size(); k++)
									{
										gp_Pnt p = mySurf->Value(cUV[k].X(), cUV[k].Y());
										arrPnt.push_back(p);
										myPolygon.Add(arrPnt[k]);
									}
									if (!myPolygon.Wire().IsNull())
										aB.Add(comp, myPolygon.Wire());

									/*for (int k = 0; k < arrPnt.size() - 1; k++)
									{
										if (!arrPnt[k].IsEqual(arrPnt[k - 1], 0.02))
										{
											myPolygon.Add(arrPnt[k]);
											/*BRepBuilderAPI_MakeEdge aV(arrPnt[k], arrPnt[k + 1]);
											aB.Add(comp, aV.Edge());
										}
									}*/
								}
							}
						}
					}
				}
			}
		}
	}

	Handle(AIS_Shape) aiss = new AIS_Shape(comp);
	RTSObject = aiss;
	Handle(Prs3d_LineAspect) LineAspect = new Prs3d_LineAspect(Quantity_NOC_RED, Aspect_TOL_SOLID, 4);
	Handle(Prs3d_Drawer) rtsStyle = new Prs3d_Drawer();
	rtsStyle->SetLineAspect(LineAspect);
	rtsStyle->SetWireAspect(LineAspect);
	rtsStyle->SetWireDraw(true);
	RTSObject->SetAttributes(rtsStyle);
	RTSObject->SetDisplayMode(AIS_Shaded);
	theDoc->getContext()->Display(RTSObject, true);
	IObject->SetTransparency(0.5);
}

void InteractiveShape::generateSupport(double h, double w, SupportStructure::ConnectionType  ctype,
										SupportStructure::PerforationType  ptype)
{
	theDoc->getContext()->Remove(SupportObject, true);
	TopoDS_Compound comp;
	BRep_Builder aB;
	aB.MakeCompound(comp);

	for each (QList<gp_Pnt> aSegment in RTSPath)
	{
		/*gp_Vec aVec(aSegment[0], aSegment[1]);
		if (aVec.Magnitude() > 1)
		{*/
			SupportStructure aSupp(this);
			aSupp.setPoints(aSegment);
			aSupp.setHeight(h);
			aSupp.setWidth(w);
			aSupp.Build();

			aB.Add(comp, aSupp.getShape());
		//}
		
	}

	Handle(AIS_Shape) aiss = new AIS_Shape(comp);
	SupportObject = aiss;
	Handle(Prs3d_LineAspect) LineAspect = new Prs3d_LineAspect(Quantity_NOC_PINK, Aspect_TOL_SOLID, 4);
	Handle(Prs3d_Drawer) rtsStyle = new Prs3d_Drawer();
	rtsStyle->SetLineAspect(LineAspect);
	rtsStyle->SetWireAspect(LineAspect);
	rtsStyle->SetWireDraw(true);
	SupportObject->SetAttributes(rtsStyle);
	SupportObject->SetDisplayMode(AIS_Shaded);
	Graphic3d_MaterialAspect myMat1(Graphic3d_NOM_BRASS);
	myMat1.SetColor(Quantity_Color(1, 0.3, 0.3, Quantity_TOC_RGB));
	SupportObject->SetMaterial(myMat1);
	theDoc->getContext()->Display(SupportObject, true);
	IObject->SetTransparency(0.5);
}


void InteractiveShape::simulateSSDR(double d, double ang)
{
	theDoc->getContext()->Remove(SSDRObject, true);
	QList<CF_Pixel> allPixels;
	QList<double> allDistances;
	for (int i = 0; i < theSurfaces.size(); i++)
	{
		QList<CF_Pixel> thisResult=theSurfaces[i]->simulateMinDistanceAround(theSurfaces, ang, d);

		for (int j = 0; j < thisResult.size(); j++)
		{
			allPixels.push_back(thisResult[j]);
			allDistances.push_back(thisResult[j].getMinDistanceAround());
		}
	}

	double mind = 0;
	double maxd = d;

	if (!allDistances.isEmpty())
	{
		std::sort(allDistances.begin(), allDistances.end());
	
		TopoDS_Compound comp;
		BRep_Builder aB;
		aB.MakeCompound(comp);

		for (int i = 0; i < allPixels.size(); i++)
		{
			gp_Ax2 ax(allPixels[i].getCenter(), allPixels[i].getDirection());
			gp_Circ aCircle(ax, 1);

			BRepBuilderAPI_MakeEdge aW(aCircle);
			aB.Add(comp, aW.Edge());
		}

		Handle(AIS_Shape) aiss = new AIS_Shape(comp);
		SSDRObject = aiss;
		Handle(Prs3d_LineAspect) LineAspect = new Prs3d_LineAspect(Quantity_NOC_RED, Aspect_TOL_SOLID, 2);
		Handle(Prs3d_Drawer) rtsStyle = new Prs3d_Drawer();
		rtsStyle->SetLineAspect(LineAspect);
		rtsStyle->SetWireAspect(LineAspect);
		rtsStyle->SetWireDraw(true);
		SSDRObject->SetAttributes(rtsStyle);
		theDoc->getContext()->Display(SSDRObject, true);
	}
}

//////////////////////////////////////
///////  Useful Algorithms  //////////
/////////////////////////////////////

QList<gp_Pnt> InteractiveShape::organizePointCCWiseUV(QList<gp_Pnt> aList)
{
	double xMean = 0;
	double yMean = 0;
	double zMean = 0;

	for (int i = 0; i < aList.size(); i++)
	{
		xMean += aList[i].X();
		yMean += aList[i].Y();
		zMean += aList[i].Z();
	}

	xMean = xMean / aList.size();
	yMean = yMean / aList.size();
	zMean = zMean / aList.size();
	gp_Pnt meanPoint(xMean, yMean, zMean);
	gp_Vec xDir(10, 0, 0);
	QList<AngPoint> listAngPoint;
	QList<double> listAng;
	for (int i = 0; i < aList.size(); i++)
	{
		gp_Vec aVec(meanPoint, aList[i]);
		double ang = aVec.Angle(xDir);
		if (aList[i].Y() < yMean)
		{
			ang = 6.28 - ang;
		}

		AngPoint a;
		a.theAngle = ang;
		a.Point = aList[i];

		listAngPoint.push_back(a);
		listAng.push_back(ang);
	}

	std::sort(listAng.begin(), listAng.end());

	QList<gp_Pnt> newList;
	QList<double> duplicatePoints;
	for (int i = 0; i < listAng.size(); i++)
	{
		bool alreadyPicked = false;
		for (int d = 0; d < duplicatePoints.size(); d++)
		{
			if (duplicatePoints[d] == listAng[i])
			{
				alreadyPicked = true;
				break;
			}
		}

		if (!alreadyPicked)
		{
			for (int j = 0; j < listAngPoint.size(); j++)
			{
				if (listAng[i] == listAngPoint[j].theAngle)
				{
					newList.push_back(listAngPoint[j].Point);
					break;
				}
			}
		}
	}

	return newList;
}

QList<gp_Pnt> InteractiveShape::outerBoundOfPixels(QList<CF_Pixel> aList)
{
	QList<gp_Pnt> outerB;

	for (int i = 0; i < aList.size(); i++)
	{
		QList<CF_Pixel> compareList = aList;
		compareList.removeAt(i);
		/*if (!aList[i].hasFourSideNeighbours(compareList))
		{*/
			outerB.push_back(aList[i].getCenter());
		//}
	}

	/*if (!outerB.isEmpty())
	{
		outerB = organizePointCCWiseUV(outerB);
	}*/

	return outerB;
}

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

double InteractiveShape::getLimitedTotalArea(ActionRule* aRule)
{
	VarAttribute aVarAttrib;
	QList<int> items;
	for each (VarAttribute var in theAttributes)
	{
		if (var.id == aRule->getID())
		{
			items = var.items;
			break;
		}
	}

	double tot = 0;
	for (int i = 0; i < items.size(); i++)
	{
		tot = tot + theSurfaces[items[i]]->theArea;
	}

	return tot;
}

void InteractiveShape::rotateShape2(gp_Quaternion aQuaternion, double angx, double angy, double angz)
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
	//gp_Pnt PrinterCenter(theDoc->PrinterX / 2, theDoc->PrinterY / 2, 0);

	trlVector = gp_Vec(Pcenter, initcenter);
	gp_Trsf transTrsf;
	transTrsf.SetTranslation(trlVector);
	BRepBuilderAPI_Transform myPartTranslation(theShape, transTrsf, Standard_False);
	theShape = myPartTranslation.Shape();

	//TransformInteractives
	gp_Trsf myTransform;
	myTransform.SetTransformation(rotQuaternion, trlVector);
	//IObject->SetLocalTransformation(myTransform);

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
	//gp_Pnt PrinterCenter(theDoc->PrinterX / 2, theDoc->PrinterY / 2, 0);

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

	/*TopoDS_Compound comp;
	BRep_Builder aB;
	aB.MakeCompound(comp);
	aB.Add(comp, theShape);*/

	for (int m = 1; m <= myIndexMap.Size(); m++)
	{
		if (m - 1 < theSurfaces.size())
		{
			theSurfaces[m - 1]->transformTriangles(myTransform);
			//theSurfaces[m - 1]->transformPixels(myTransform);
			/*for (int i = 0; i < theSurfaces[m - 1]->theTriangles.size(); i++)
			{
				BRepBuilderAPI_MakeVertex v1(theSurfaces[m - 1]->theTriangles[i].Pn1);
				BRepBuilderAPI_MakeVertex v2(theSurfaces[m - 1]->theTriangles[i].Pn2);
				BRepBuilderAPI_MakeVertex v3(theSurfaces[m - 1]->theTriangles[i].Pn3);

				aB.Add(comp, v1.Vertex());
				aB.Add(comp, v2.Vertex());
				aB.Add(comp, v3.Vertex());
			}*/
			
			if (theSurfaces[m - 1]->getFace().IsSame(TopoDS::Face(myIndexMap.FindKey(m))));
			theSurfaces[m - 1]->setFace(TopoDS::Face(myIndexMap.FindKey(m)));
		}
	}
	//Handle(AIS_Shape) aiss = new AIS_Shape(comp);
	//theDoc->getContext()->Display(aiss, true);
	IObject->SetTransparency(0.0);
	theDoc->getContext()->Remove(RTSObject, true);
	theDoc->getContext()->Remove(SSDRObject, true);
	theDoc->getContext()->Remove(SupportObject, true);
	theDoc->getContext()->Remove(VisuObject, true);

	for (int i = 0; i < theAttributes.size(); i++)
	{
		theDoc->getContext()->Remove(theAttributes[i].theDisplayable, true);

		if (theAttributes[i].isDisplayed)
		{
			showAttributedSurfaces(theAttributes[i].id);
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

	rotateShape(rotQuaternion, Rx, Ry, Rz);

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

double InteractiveShape::sliceIntersectMeasure()
{
	double weiMaxLength=0;

	omp_set_dynamic(1);
	omp_set_num_threads(ThreadsToUse);

	gp_Dir SliceDirection(0, 0, 1);
	double space = 1;
	concurrent_vector<CF_Triangle> allTri;

#pragma omp parallel default (none)
	{
#pragma omp for
		for (int i = 0; i < theSurfaces.size(); i++)
		{
			for (int j = 0; j < theSurfaces[i]->theTriangles.size(); j++)
			{
				allTri.push_back(theSurfaces[i]->theTriangles[j]);
			}
		}
	}

	Bnd_Box B;
	BRepBndLib::Add(theShape, B);
	double Bxmin, Bymin, Bzmin, Bxmax, Bymax, Bzmax;
	B.Get(Bxmin, Bymin, Bzmin, Bxmax, Bymax, Bzmax);

	concurrent_vector<concurrent_vector<CF_Edge>> allSegments;
	QList<QList<CF_Edge>> allSegments2;
	QList<QList<gp_Pnt>> ListOfPoints;
	int range = abs(Bzmax - Bzmin);
	for (int i = 0; i < range; i++)
	{
		concurrent_vector<CF_Edge> aList;
		QList<CF_Edge> bList;
		allSegments.push_back(aList);
		allSegments2.push_back(bList);

		QList<gp_Pnt> cList;
		ListOfPoints.push_back(cList);
	}

#pragma omp parallel default (none)
	{
#pragma omp for
		for (int i = 0; i < allTri.size(); i++)
		{
			for (int it = 0; it < range; it++)
			{
				double H = Bzmin + it;
				if (H >= allTri[i].getZmin() && H <= allTri[i].getZmax() && allTri[i].getArea() > 0.05)
				{
					if (!allTri[i].getUnitNormal().IsParallel(SliceDirection, 0.0348))
					{
						gp_Pnt Pnt1 = allTri[i].getNodes()[0];
						gp_Pnt Pnt2 = allTri[i].getNodes()[1];
						gp_Pnt Pnt3 = allTri[i].getNodes()[2];
						if (!Pnt1.IsEqual(Pnt2, 0.01) && !Pnt2.IsEqual(Pnt3, 0.01) && !Pnt1.IsEqual(Pnt3, 0.01))
						{
							gp_Pnt P1;
							gp_Pnt P2;

							if (Pnt1.Z() == H && Pnt2.Z() == H)
							{
								P1 = Pnt1;
								P2 = Pnt2;

								CF_Edge anEd(P1, P2);
								(allSegments)[it].push_back(anEd);
							}

							else if (Pnt2.Z() == H && Pnt3.Z() == H)
							{
								P1 = Pnt2;
								P2 = Pnt3;

								CF_Edge anEd(P1, P2);
								(allSegments)[it].push_back(anEd);
							}

							else if (Pnt1.Z() == H && Pnt3.Z() == H)
							{
								P1 = Pnt1;
								P2 = Pnt3;

								CF_Edge anEd(P1, P2);
								(allSegments)[it].push_back(anEd);
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

								CF_Edge anEd(P1, P2);
								(allSegments)[it].push_back(anEd);
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

								CF_Edge anEd(P1, P2);
								(allSegments)[it].push_back(anEd);
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

								CF_Edge anEd(P1, P2);
								(allSegments)[it].push_back(anEd);
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

								CF_Edge anEd(P1, P2);
								(allSegments)[it].push_back(anEd);
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

								CF_Edge anEd(P1, P2);
								(allSegments)[it].push_back(anEd);
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

								CF_Edge anEd(P1, P2);
								(allSegments)[it].push_back(anEd);
							}
						}
					}
				}
			}
		}
	}



	if (allSegments.size() != 0)
	{
		for (int i = 0; i < allSegments.size(); i++)
		{
			if (allSegments[i].size() != 0)
			{
				(ListOfPoints)[i] = EdgeSorting(allSegments[i], i);
			}
		}
	}

	QList<double> weightedLength;
	//Intersection + Length Measurement
	for (int i = 0; i < ListOfPoints.size() - 1; i++)
	{
		double l1 = 0, l2 = 0, l3 = 0, l4 = 0;
		if (i < ListOfPoints.size() - 4)
		{
			QList<gp_Pnt> aList1 = contourIntersection(ListOfPoints[i], ListOfPoints[i + 1]);
			QList<double> vals1 = hatchMeasurement(aList1);
			if (vals1.size() != 0)
			{
				std::sort(vals1.begin(), vals1.end());
				l1 = vals1.last();

				QList<gp_Pnt> aList2 = contourIntersection(aList1, ListOfPoints[i + 2]);
				QList<double> vals2 = hatchMeasurement(aList2);
				if (vals2.size() != 0)
				{
					std::sort(vals2.begin(), vals2.end());
					l2 = vals2.last();

					QList<gp_Pnt> aList3 = contourIntersection(aList2, ListOfPoints[i + 3]);
					QList<double> vals3 = hatchMeasurement(aList3);
					if (vals3.size() != 0)
					{
						std::sort(vals3.begin(), vals3.end());
						l3 = vals3.last();

						QList<gp_Pnt> aList4 = contourIntersection(aList3, ListOfPoints[i + 4]);
						QList<double> vals4 = hatchMeasurement(aList4);

						if (vals4.size() != 0)
						{
							std::sort(vals4.begin(), vals4.end());
							l4 = vals4.last();
						}
					}
				}
			}
		}

		if (i == ListOfPoints.size() - 4)
		{
			QList<gp_Pnt> aList1 = contourIntersection(ListOfPoints[i], ListOfPoints[i + 1]);
			QList<double> vals1 = hatchMeasurement(aList1);
			if (vals1.size() != 0)
			{
				std::sort(vals1.begin(), vals1.end());
				l1 = vals1.last();

				QList<gp_Pnt> aList2 = contourIntersection(aList1, ListOfPoints[i + 2]);
				QList<double> vals2 = hatchMeasurement(aList2);
				if (vals2.size() != 0)
				{
					std::sort(vals2.begin(), vals2.end());
					l2 = vals2.last();

					QList<gp_Pnt> aList3 = contourIntersection(aList2, ListOfPoints[i + 3]);
					QList<double> vals3 = hatchMeasurement(aList3);
					if (vals3.size() != 0)
					{
						std::sort(vals3.begin(), vals3.end());
						l3 = vals3.last();
					}
				}
			}
		}
		
		if (i == ListOfPoints.size() - 3)
		{
			QList<gp_Pnt> aList1 = contourIntersection(ListOfPoints[i], ListOfPoints[i + 1]);
			QList<double> vals1 = hatchMeasurement(aList1);
			if (vals1.size() != 0)
			{
				std::sort(vals1.begin(), vals1.end());
				l1 = vals1.last();

				QList<gp_Pnt> aList2 = contourIntersection(aList1, ListOfPoints[i + 2]);
				QList<double> vals2 = hatchMeasurement(aList2);
				if (vals2.size() != 0)
				{
					std::sort(vals2.begin(), vals2.end());
					l2 = vals2.last();
				}
			}
		}
		
		if (i == ListOfPoints.size() - 2)
		{
			QList<gp_Pnt> aList1 = contourIntersection(ListOfPoints[i], ListOfPoints[i + 1]);
			QList<double> vals1 = hatchMeasurement(aList1);
			if (vals1.size() != 0)
			{
				std::sort(vals1.begin(), vals1.end());
				l1 = vals1.last();
			}
		}

		double weiLength = 0.45 * l1 + 0.3 * l2 + 0.15 * l3 + 0.1 * l4;
		weightedLength.push_back(weiLength);
	}

	if (weightedLength.size() != 0)
	{
		std::sort(weightedLength.begin(), weightedLength.end());
		weiMaxLength = weightedLength.last();
	}

	return weiMaxLength;
}

QList<gp_Pnt> InteractiveShape::EdgeSorting(concurrent_vector<CF_Edge> thelistIn, int LayerId)
{
	QList<gp_Pnt> listOfPoints;
	QList<CF_Edge> thelist2;
	QList<CF_Edge> thelist(thelistIn.begin(), thelistIn.end());

	QList<QList<CF_Edge>> listofSepPolygon;
	QList<CF_Edge> newList;
	newList.push_back(thelist.at(0));
	thelist.erase(thelist.begin());


	while (thelist.size() > 0)
	{
		for (int i = 0; i < thelist.size(); i++)
		{
			if (newList[newList.size() - 1].P1().IsEqual(thelist[i].P1(), 0.05) || newList[newList.size() - 1].P1().IsEqual(thelist[i].P2(), 0.05))
			{
				newList.push_back(thelist[i]);
				thelist.removeAt(i);
				i = 0;
			}

			else if (newList[newList.size() - 1].P2().IsEqual(thelist[i].P1(), 0.05) || newList[newList.size() - 1].P2().IsEqual(thelist[i].P2(), 0.05))
			{
				newList.push_back(thelist[i]);
				thelist.removeAt(i);
				i = 0;
			}
		}

		if (thelist.size() == 0)
		{
			listofSepPolygon.push_back(newList);
		}

		else if (thelist.size() != 0)
		{
			listofSepPolygon.push_back(newList);
			newList.clear();
			newList.push_back(thelist[0]);
			thelist.removeFirst();
		}
	}

	//theListofSepPolygonPerLayer.push_back(listofSepPolygon);

	for (int p = 0; p < listofSepPolygon.size(); p++)
	{
		if (listofSepPolygon.at(p).size() != 0)
		{
			for (int q = 0; q < listofSepPolygon.at(p).size(); q++)
			{
				//thelist2.push_back(listofSepPolygon[p][q]);
				listOfPoints.push_back(listofSepPolygon[p][q].P1());
				listOfPoints.push_back(listofSepPolygon[p][q].P2());
				//(*ListOfPoints)[LayerId].push_back(listofSepPolygon[p][q].V1.CartesianPoint);
				//(*ListOfPoints)[LayerId].push_back(listofSepPolygon[p][q].V2.CartesianPoint);
			}

		}
	}
	//}
	return listOfPoints;
}

QList<gp_Pnt> InteractiveShape::contourIntersection(QList<gp_Pnt> PointList1, QList<gp_Pnt> PointList2)
{
	concurrent_vector<gp_Pnt> ListOfResultingPoints;

	omp_set_dynamic(1);
	omp_set_num_threads(ThreadsToUse);

	if (PointList1.size() != 0 && PointList2.size() != 0)
	{
#pragma omp parallel default (none)
		{
#pragma omp for
			for (int i = 0; i < PointList1.size() - 2; i++)
			{
				int n;
				if (i == 0)
					n = i;
				else
					n = i + 1;
				double a1x = PointList1.at(n).X(); double a1y = PointList1.at(n).Y(); double a1z = PointList1.at(n).Z();
				double a2x = PointList1.at(n + 1).X(); double a2y = PointList1.at(n + 1).Y(); double a2z = PointList1.at(n + 1).Z();
				double d1x = a2x - a1x; double d1y = a2y - a1y; double d1z = a2z - a1z;
				//im_DirVec v1(PointList1.at(n), PointList1.at(n + 1));
				for (int j = 0; j < PointList2.size() - 1; j++)
				{
					//im_DirVec v2(PointList2.at(j), PointList2.at(j + 1));
					double a3x = PointList2.at(j).X(); double a3y = PointList2.at(j).Y(); double a3z = PointList2.at(j).Z();
					double a4x = PointList2.at(j + 1).X(); double a4y = PointList2.at(j + 1).Y(); double a4z = PointList2.at(j + 1).Z();
					double d2x = a4x - a3x; double d2y = a4y - a3y; double d2z = a4z - a3z;

					if ((d1x / d2x) == (d1y / d2y) && (d1x / d2x) == (d1z / d2z) && (d1y / d2y) == (d1z / d2z))
					{

					}
					else
					{
						double lineSlope1 = (a2y - a1y) / (a2x - a1x);

						double lineSlope2 = (a4y - a3y) / (a4x - a3x);

						double lineConstant1 = a1y - (lineSlope1 * a1x);
						double lineConstant2 = a3y - (lineSlope2 * a3x);

						double x = (lineConstant1 - lineConstant2) / (lineSlope2 - lineSlope1);
						double y = (lineSlope1 * x) + (lineConstant1);

						bool onLine1 = false;
						bool onLine2 = false;
						if (x >= min(a1x, a2x) && x <= max(a1x, a2x) && y >= min(a1y, a2y) && y <= max(a1y, a2y))
						{
							onLine1 = true;
						}

						if (x >= min(a3x, a4x) && x <= max(a3x, a4x) && y >= min(a3y, a4y) && y <= max(a3y, a4y))
						{
							onLine2 = true;
						}


						if (onLine1 == true && onLine2 == true)
						{
							ListOfResultingPoints.push_back(gp_Pnt(x, y, 0));
						}

					}
					j++;
				}
			}
		}

		int PointOnLineCount = 0;
#pragma omp parallel default (none)
		{
#pragma omp for /*schedule(dynamic,1) */
			for (int i = 0; i < PointList1.size(); i++)
			{
				double aix = PointList1.at(i).X(); double aiy = PointList1.at(i).Y();
				int nInter = 0;
				bool PointOnLine = false;

				for (int j = 0; j < PointList2.size() - 1; j++)
				{
					double a1x = PointList2.at(j).X(); double a1y = PointList2.at(j).Y();
					int o = j + 1;
					double a2x = PointList2.at(o).X(); double a2y = PointList2.at(o).Y();

					double lineSlope = (a2y - a1y) / (a2x - a1x);
					double lineConstant = a1y - (lineSlope * a1x);
					double verifPi = aiy - (lineSlope * aix);

					if (verifPi >= lineConstant - 0.08 && verifPi <= lineConstant + 0.08)
					{
						if (aiy >= min(a1y - 0.05, a2y - 0.05) && aiy <= max(a1y + 0.05, a2y + 0.05) && aix >= min(a1x - 0.05, a2x - 0.05) && aix <= max(a1x + 0.05, a2x + 0.05))
						{
							PointOnLine = true;
							PointOnLineCount++;
							break;
						}
					}

					else
					{
						if (aiy >= min(a1y, a2y) && aiy <= max(a1y, a2y))
						{
							if (a1x > aix || a2x > aix)
							{
								nInter++;
							}
						}

					}

					j++;
				}

				if (PointOnLine == true)
				{
					ListOfResultingPoints.push_back(gp_Pnt(aix, aiy, 0));
				}

				if (nInter % 2 != 0 && nInter != 0 && PointOnLine == false)
				{
					ListOfResultingPoints.push_back(gp_Pnt(aix, aiy, 0));
				}


			}
		}

		if (!ListOfResultingPoints.empty())
		{
#pragma omp parallel default (none)
			{
#pragma omp for /*schedule(dynamic,1) */
				for (int i = 0; i < PointList2.size(); i++)
				{
					//gp_Pnt Pi = theList2.at(i);
					double aix = PointList2.at(i).X(); double aiy = PointList2.at(i).Y();

					int nInter = 0;
					bool PointOnLine = false;

					for (int j = 0; j < PointList1.size() - 1; j++)
					{

						double a1x = PointList1.at(j).X(); double a1y = PointList1.at(j).Y();
						int o = j + 1;
						//gp_Pnt P2 = theList2.at(o);
						double a2x = PointList1.at(o).X(); double a2y = PointList1.at(o).Y();

						double lineSlope = (a2y - a1y) / (a2x - a1x);
						double lineConstant = a1y - (lineSlope * a1x);
						double verifPi = aiy - (lineSlope * aix);

						if (verifPi >= lineConstant - 0.08 && verifPi <= lineConstant + 0.08)
						{
							if (aiy >= min(a1y - 0.05, a2y - 0.05) && aiy <= max(a1y + 0.05, a2y + 0.05) && aix >= min(a1x - 0.05, a2x - 0.05) && aix <= max(a1x + 0.05, a2x + 0.05))
							{
								PointOnLine = true;
								PointOnLineCount++;
								break;
							}
						}

						if (aiy >= min(a1y, a2y) && aiy <= max(a1y, a2y))
						{
							if (a1x > aix || a2x > aix)
							{
								nInter++;
							}
						}

						j++;
					}

					if (PointOnLine == true)
					{
						ListOfResultingPoints.push_back(gp_Pnt(aix, aiy, 0));
					}
					if (nInter % 2 != 0 && nInter != 0 && PointOnLine == false)
					{
						ListOfResultingPoints.push_back(gp_Pnt(aix, aiy, 0));
					}
				}
			}
		}
	}
	QList<gp_Pnt> myList(ListOfResultingPoints.begin(), ListOfResultingPoints.end());

	return myList;;
}

QList<double> InteractiveShape::hatchMeasurement(QList<gp_Pnt> thePoints)
{
	vector<vector<double>> AllIncrements;

	double ux = 0;
	double uy = 0;
	double uz = 1;

	concurrent_vector<double> LengthList;
	concurrent_vector<double> LengthList2;

	if (thePoints.size() >= 3)
	{
		omp_set_dynamic(1);
		omp_set_num_threads(ThreadsToUse);
#pragma omp parallel default (none)
		{
#pragma omp for
			for (int u = 0; u < 45; u++)
			{
				double Ang = u * 0.0698;
				vector <gp_Pnt> RotList;
				vector<double> theY;
				for (int n = 0; n < thePoints.size(); n++)
				{
					double xx = (thePoints.at(n).X() * (cos(Ang) + (ux * ux * (1 - cos(Ang)))) + thePoints.at(n).Y() * ((uy * ux * (1 - cos(Ang))) - (uz * sin(Ang))) + thePoints.at(n).Z() * ((ux * uz * (1 - cos(Ang))) + (uy * sin(Ang))));
					double yy = (thePoints.at(n).X() * ((ux * uy * (1 - cos(Ang))) + (uz * sin(Ang))) + thePoints.at(n).Y() * (cos(Ang) + (uy * uy * (1 - cos(Ang)))) + thePoints.at(n).Z() * ((uy * uz * (1 - cos(Ang))) - (ux * sin(Ang))));
					double zz = (thePoints.at(n).X() * ((uz * ux * (1 - cos(Ang))) - (uy * sin(Ang))) + thePoints.at(n).Y() * ((uz * uy * (1 - cos(Ang))) + (ux * sin(Ang))) + thePoints.at(n).Z() * (cos(Ang) + (uz * uz * (1 - cos(Ang)))));

					RotList.push_back(gp_Pnt(xx, yy, zz));
					theY.push_back(yy);
				}

				if (theY.size() > 1)
				{
					std::sort(theY.begin(), theY.end());
					int space = abs(theY.at(theY.size() - 1) - theY.at(0));
					double increm = theY.at(0);
					for (int p = 0; p < space; p++)
					{
						vector<double> mySorter;
						for (int i = 0; i < RotList.size() - 1; i = i + 2)
						{
							double x1 = RotList.at(i).X(), x2 = RotList.at(i + 1).X();
							double y1 = RotList.at(i).Y(), y2 = RotList.at(i + 1).Y();

							//Testing position of line
							if (increm >= min(y1, y2) && increm <= max(y1, y2))
							{
								double InterX = 0;
								if (max(x1, x2) - min(x1, x2) == 0)
								{
									InterX = x2;
									mySorter.push_back(InterX);
								}

								else if (max(y1, y2) - min(y1, y2) == 0)
								{

								}
								else
								{
									double lineSlope = (y2 - y1) / (x2 - x1);
									double lineConstant = y1 - (lineSlope * x1);
									InterX = (increm - lineConstant) / lineSlope;
									mySorter.push_back(InterX);
								}
							}
						}

						if (mySorter.size() % 2 == 0 && mySorter.size() != 0)
						{
							std::sort(mySorter.begin(), mySorter.end());

							for (int w = 0; w < mySorter.size() - 1; w++)
							{
								double l = mySorter.at(w + 1) - mySorter.at(w);
								LengthList.push_back(l);
								w = w + 1;
							}
						}

						increm = increm + 1;
					}
				}
			}
		}
	}
	QList<double> ListOfLength(LengthList.begin(), LengthList.end());

	return ListOfLength;
}


/*
for each (InteractiveSurface* var in theSurfaces)
	{
		Handle(Geom_Surface) mySurf = BRep_Tool::Surface(var->getFace());
		//GeomAdaptor_Surface anAdaptor(mySurf);
		double uMin, uMax, vMin, vMax;
		BRepTools::UVBounds(var->getFace(), uMin, uMax, vMin, vMax);

		BRepAdaptor_Surface surf(var->getFace(), true);

		if (surf.GetType()== GeomAbs_SurfaceType::GeomAbs_Plane)
		{
			uMin = surf.FirstUParameter();
			uMax = surf.LastUParameter();

			vMin = surf.FirstVParameter();
			vMax = surf.LastVParameter();
		}

		else
		{
			QList<gp_Pnt2d> list;
			QList<double> Ulist;
			QList<double> Vlist;
			QList<double> length;
			TopTools_IndexedMapOfShape myIndexMap;
			TopExp Mapper;
			Mapper.MapShapes(var->getFace(), TopAbs_WIRE, myIndexMap);

			for (int m = 1; m <= myIndexMap.Size(); m++)
			{
				double leng = 0;
				TopoDS_Shape iShape = myIndexMap.FindKey(m);
				TopoDS_Wire iWire = TopoDS::Wire(iShape);

				TopTools_IndexedMapOfShape myIndexMap2;
				TopExp Mapper2;
				Mapper2.MapShapes(iWire, TopAbs_EDGE, myIndexMap2);

				for (int n = 1; n <= myIndexMap2.Size(); n++)
				{
					TopoDS_Shape iShape2 = myIndexMap2.FindKey(n);
					TopoDS_Edge iEdge = TopoDS::Edge(myIndexMap2.FindKey(n));

					TopTools_IndexedMapOfShape myIndexMap3;
					TopExp Mapper3;
					Mapper3.MapShapes(iEdge, TopAbs_VERTEX, myIndexMap3);


			std::sort(Ulist.begin(), Ulist.end());
			std::sort(Vlist.begin(), Vlist.end());

			uMin = Ulist.first();
			uMax = Ulist.last();

			vMin = Vlist.first();
			vMax = Vlist.last();
		}

		int spaceu = abs(uMax - uMin);
		int spacev = abs(vMax - vMin);

		for (int u = 0; u < spaceu; u++)
		{
			for (int v = 0; v < spacev; v++)
			{
				gp_Pnt p = mySurf->Value(uMin + u, vMin + v);
				BRepBuilderAPI_MakeVertex aV(p);
				aB.Add(comp, aV.Vertex());
			}
		}*/


/*double udim = uMin + u * stepElement;

			for (int v = 0; v < spacev; v++)
			{
				double vdim = vMin+ v * stepElement;
				gp_Pnt p;
				gp_Vec uVec;
				gp_Vec vVec;
				mySurf->D1(udim, vdim, p, uVec, vVec);
				gp_Vec aNormal = uVec.Crossed(vVec);
				if (aNormal.Magnitude() >= 0.15)
				{
					double ang = aNormal.Angle(zdir);

					if (ang <= 0.785)
					{
						AngCoord acoord;
						acoord.p = tan(ang);
						acoord.Pnt = p;
						aList.push_back(acoord);

					}
				}
			}*/