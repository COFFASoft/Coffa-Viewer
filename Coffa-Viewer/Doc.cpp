
#include "Doc.h"
#include "View.h"
#include "CoffaViewer.h"
#include "InteractiveShape.h"

//Std C++
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <concurrent_vector.h>
#include <omp.h>

//QT
#include <QtWidgets/qmessagebox.h>

//OCCT 
#include <cmath>
#include <OpenGl_GraphicDriver.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_ListOfInteractive.hxx>
#include <AIS_Trihedron.hxx>
#include <AIS_Plane.hxx>
#include <AIS_ColorScale.hxx>
#include <AIS_ColoredShape.hxx>

#include <STEPControl_Reader.hxx>
#include <STEPControl_Writer.hxx>
#include <StlAPI.hxx>
#include <StlAPI_Reader.hxx>
#include <StlAPI_Writer.hxx>
#include <RWStl.hxx>

#include<BRepMesh_IncrementalMesh.hxx>
#include <BRepFeat_MakePrism.hxx>

#include <QtWidgets/QColorDialog>
#include <V3d_Viewer.hxx>
#include <AIS_Shape.hxx>
#include <Prs3d_Drawer.hxx>
#include <Prs3d_PointAspect.hxx>
#include <Graphic3d_MaterialAspect.hxx>
#include <Quantity_Color.hxx>

#include <Aspect_GridType.hxx>
#include <Aspect_GridDrawMode.hxx>


using namespace concurrency;
using namespace std;

// THE VIEWER
Handle(V3d_Viewer) Doc::Viewer(const Standard_ExtString theName,
	const Standard_CString theDomain,
	const Standard_Real theViewSize, const V3d_TypeOfOrientation theViewProj)
{
	Handle(OpenGl_GraphicDriver) aGraphicDriver;
	if (aGraphicDriver.IsNull())
	{
		Handle(Aspect_DisplayConnection) aDisplayConnection;
#if !defined(_WIN32) && !defined(__WIN32__) && (!defined(__APPLE__) || defined(MACOSX_USE_GLX))
		aDisplayConnection = new Aspect_DisplayConnection(qgetenv("DISPLAY").constData());
#endif
		aGraphicDriver = new OpenGl_GraphicDriver(aDisplayConnection);
	}
	Handle(V3d_Viewer) aViewer = new V3d_Viewer(aGraphicDriver);
	aViewer->SetDefaultViewSize(theViewSize);
	aViewer->SetDefaultViewProj(theViewProj);

	aViewer->SetComputedMode(Standard_True);
	aViewer->SetDefaultComputedMode(Standard_True);
	return aViewer;
}

Doc::Doc(CoffaViewer* app)
	: QObject(),
	myApp(app),
	IOid(0),
	PlatformVisible(0),
	PrinterX(200),
	PrinterY(200),
	PrinterZ(180),
	PrinterThreshold(0)
{
	//setPriority(QThread::NormalPriority);
	Aspect_GridType aGridtype = Aspect_GT_Rectangular;
	Aspect_GridDrawMode aGridDrawmode= Aspect_GDM_Lines;
	Quantity_Color Col1, Col2;
	
	Col1.SetValues(Quantity_NOC_WHITE);
	Col2.SetValues(Quantity_NOC_WHITE);
	TCollection_ExtendedString a3DName("Visu3D");
	myViewer = Viewer(a3DName.ToExtString(), "", 1000.0, V3d_XposYposZpos);
	myViewer->SetDefaultBgGradientColors(Col1, Col2, Aspect_GFM_VER);
	myViewer->SetDefaultLights();
	myViewer->SetLightOn();
	myContext = new AIS_InteractiveContext(myViewer);
	myContext->ShiftSelect(true);
	Platform();
	PlatformOn();
}

Doc::~Doc()
{

}


CoffaViewer* Doc::getApplication()
{
	return myApp;
}

Handle(AIS_InteractiveContext) Doc::getContext()
{
	return myContext;
}

void Doc::fitAll()
{
	myApp->getViewer()->fitAll();
}

void Doc::SetupSelectionModes()
{
	getContext()->Activate(listOfShapes[Shapeid]->getInteractiveObject(), AIS_Shape::SelectionMode(TopAbs_FACE));
}

void Doc::RestoreSelectionModes()
{
	for (int i = 0; i < listOfShapes.size(); i++)
	{
		getContext()->Deactivate(listOfShapes.at(i)->getInteractiveObject(), AIS_Shape::SelectionMode(TopAbs_FACE));
		getContext()->Activate(listOfShapes.at(i)->getInteractiveObject(), AIS_Shape::SelectionMode(TopAbs_SHAPE));
	}

}

void Doc::AddPart(QString fileName)
{
	QFileInfo myFileInfo(fileName);
	QString mySuffix = myFileInfo.suffix();
	QString BaseName = myFileInfo.baseName();
	QByteArray ba = fileName.toLatin1();
	const char* myFileStl = ba.data();
	TCollection_AsciiString myFile = fileName.toUtf8().constData();

	if (mySuffix == "STEP" || mySuffix == "STP" || mySuffix == "stp" || mySuffix == "step")
	{
		STEPControl_Reader myReader;
		IFSelect_ReturnStatus stat = myReader.ReadFile(myFile.ToCString());
		Standard_Integer NbRoots = myReader.NbRootsForTransfer();
		Standard_Integer NbTrans = myReader.TransferRoots();
		TopoDS_Shape aShape = myReader.OneShape();
		BRepMesh_IncrementalMesh aMesh(aShape, 1.0, false, 0.035, true);
		aShape = aMesh.Shape();

		Bnd_Box B;
		BRepBndLib::Add(aShape, B);
		double Bxmin, Bymin, Bzmin, Bxmax, Bymax, Bzmax;
		B.Get(Bxmin, Bymin, Bzmin, Bxmax, Bymax, Bzmax);

		double Px = ((Bxmax)+(Bxmin)) / 2;
		double Py = ((Bymax)+(Bymin)) / 2;
		double Pz = ((Bzmax)+(Bzmin)) / 2;

		gp_Pnt Pcenter(Px, Py, Bzmin);
		gp_Trsf myTransform;
		myTransform.SetTranslation(Pcenter, gp_Pnt(PrinterX / 2, PrinterY / 2, 0));
		BRepBuilderAPI_Transform myPartCentered(aShape, myTransform, Standard_False);
		aShape = myPartCentered.Shape();

		InteractiveShape* intShape = new InteractiveShape(aShape, this);
		intShape->setName(BaseName);
		displayCAD(intShape);
		intShape->parseFaces();
		listOfShapes.push_back(intShape);

	}

	else if (mySuffix == "STL" || mySuffix == "stl")
	{
		StlAPI_Reader myAPI1;
		TopoDS_Shape aShape;
		myAPI1.Read(aShape, myFileStl);

		Bnd_Box B;
		BRepBndLib::Add(aShape, B);
		double Bxmin, Bymin, Bzmin, Bxmax, Bymax, Bzmax;
		B.Get(Bxmin, Bymin, Bzmin, Bxmax, Bymax, Bzmax);

		double Px = ((Bxmax)+(Bxmin)) / 2;
		double Py = ((Bymax)+(Bymin)) / 2;
		double Pz = ((Bzmax)+(Bzmin)) / 2;

		gp_Pnt Pcenter(Px, Py, Bzmin);
		gp_Trsf myTransform;
		myTransform.SetTranslation(Pcenter, gp_Pnt(PrinterX / 2, PrinterY / 2, 0));
		BRepBuilderAPI_Transform myPartCentered(aShape, myTransform, Standard_False);
		aShape = myPartCentered.Shape();

		BRepMesh_IncrementalMesh aMesh(aShape, 1.0, false, 0.035, true);
		aShape = aMesh.Shape();

		InteractiveShape* intShape = new InteractiveShape(aShape, this);
		intShape->setName(BaseName);
		displayCAD(intShape);
		intShape->parseFaces();
		listOfShapes.push_back(intShape);
	}

	else if (mySuffix == "BREP" || mySuffix == "brep")
	{
		BRep_Builder b;
		std::ifstream is;
		is.open(myFile.ToCString());
		TopoDS_Shape aShape;
		BRepTools::Read(aShape, is, b);
		is.close();

		Bnd_Box B;
		BRepBndLib::Add(aShape, B);
		double Bxmin, Bymin, Bzmin, Bxmax, Bymax, Bzmax;
		B.Get(Bxmin, Bymin, Bzmin, Bxmax, Bymax, Bzmax);

		double Px = ((Bxmax)+(Bxmin)) / 2;
		double Py = ((Bymax)+(Bymin)) / 2;
		double Pz = ((Bzmax)+(Bzmin)) / 2;

		gp_Pnt Pcenter(Px, Py, Bzmin);
		gp_Trsf myTransform;
		myTransform.SetTranslation(Pcenter, gp_Pnt(PrinterX / 2, PrinterY / 2, 0));
		BRepBuilderAPI_Transform myPartCentered(aShape, myTransform, Standard_False);
		aShape = myPartCentered.Shape();

		BRepMesh_IncrementalMesh aMesh(aShape, 1.0, false, 0.035, true);
		aShape = aMesh.Shape();

		InteractiveShape* intShape = new InteractiveShape(aShape, this);
		intShape->setName(BaseName);
		displayCAD(intShape);
		intShape->parseFaces();
		listOfShapes.push_back(intShape);
	}

	else
	{
		QMessageBox::warning(myApp, QObject::tr("Incompatible File"),
			QObject::tr("Please load BRep, STEP, OBJ or STL files only\nTip: you can use other software to export your model into STEP or STL"),
			QObject::tr("OK"), QString::null, QString::null, 0, 0);
		return;
	}
}

void Doc::RemovePart(int theShapeID)
{
	listOfShapes[Shapeid]->removeAllInteractives();
	listOfShapes.removeAt(Shapeid);
	myViewer->Redraw();
	myViewer->Update();
}

QList<InteractiveShape*> Doc::getListOfShapes()
{
	return listOfShapes;
}

void  Doc::displayCAD(InteractiveShape* pShape)
{
	Handle(Prs3d_LineAspect) LineAspect = new Prs3d_LineAspect(Quantity_NOC_BLACK, Aspect_TOL_SOLID, 2);
	Handle(Prs3d_PointAspect) PointAspect = new Prs3d_PointAspect(Aspect_TOM_BALL, Quantity_NOC_BLACK, 2);
	Handle(Prs3d_Drawer) selectionStyle = new Prs3d_Drawer();
	selectionStyle->SetLineAspect(LineAspect);
	selectionStyle->SetColor(Quantity_Color(0.376 / 2, 0.0470 / 2, 0.988, Quantity_TOC_RGB));
	selectionStyle->SetFaceBoundaryDraw(true);
	selectionStyle->SetFaceBoundaryAspect(LineAspect);
	selectionStyle->SetWireAspect(LineAspect);
	selectionStyle->SetWireDraw(true);
	selectionStyle->SetPointAspect(PointAspect);

	pShape->getInteractiveObject()->SetAttributes(selectionStyle);
	pShape->getInteractiveObject()->SetDynamicHilightAttributes(selectionStyle);
	pShape->getInteractiveObject()->SetHilightMode(1);
	Graphic3d_MaterialAspect myMat1(Graphic3d_NOM_BRASS);
	myMat1.SetColor(Quantity_Color(0.458, 0.466, 0.495, Quantity_TOC_RGB));
	pShape->getInteractiveObject()->SetMaterial(myMat1);
	myContext->Display(pShape->getInteractiveObject(), Standard_True);

	myContext->Activate(pShape->getInteractiveObject(), AIS_Shape::SelectionMode(TopAbs_SHAPE));
	//myContext->Activate(pShape->getInteractiveObject(), AIS_Shape::SelectionMode(TopAbs_EDGE));
	//listOfObjects.push_back(pShape->getInteractiveObject());
}

void Doc::rotateCurrentShape(int aSequence, double angX, double angY, double angZ)
{
	InteractiveShape::AxisSequence seq;
	gp_EulerSequence mySequence;

	double AngX = angX * 3.14 / 180;
	double AngY = angY * 3.14 / 180;
	double AngZ = angZ * 3.14 / 180;

	gp_Quaternion oQuaternion;
	if (aSequence == 1)
	{
		gp_XYZ Euler1((cos(AngY) * cos(AngZ)), (cos(AngX) * sin(AngZ) + cos(AngZ) * sin(AngX) * sin(AngY)), (sin(AngX) * sin(AngZ) - cos(AngX) * cos(AngZ) * sin(AngY)));
		gp_XYZ Euler2((-cos(AngY) * sin(AngZ)), (cos(AngX) * cos(AngZ) - sin(AngX) * sin(AngY) * sin(AngZ)), (cos(AngZ) * sin(AngY) + cos(AngX) * sin(AngY) * sin(AngZ)));
		gp_XYZ Euler3((sin(AngY)), (-cos(AngY) * sin(AngX)), (cos(AngX) * cos(AngY)));
		gp_Mat oEuler(Euler1, Euler2, Euler3);

		oQuaternion = gp_Quaternion(oEuler);
		mySequence = gp_Extrinsic_XYZ;
		oQuaternion.SetEulerAngles(mySequence, AngX, AngY, AngZ);
		seq = InteractiveShape::XYZ;
	}

	else if (aSequence == 2)
	{
		gp_XYZ Euler1((cos(AngX) * cos(AngY)), (sin(AngX) * sin(AngY) + cos(AngX) * cos(AngY) * sin(AngZ)), (cos(AngY) * sin(AngX) * sin(AngZ) - cos(AngX) * sin(AngY)));
		gp_XYZ Euler2((-sin(AngZ)), (cos(AngX) * cos(AngZ)), (cos(AngZ) * sin(AngX)));
		gp_XYZ Euler3((cos(AngZ) * sin(AngY)), (cos(AngX) * sin(AngZ) * sin(AngY) - cos(AngY) * sin(AngX)), (cos(AngX) * cos(AngY) + sin(AngX) * sin(AngZ) * sin(AngY)));

		gp_Mat oEuler(Euler1, Euler2, Euler3);
		oQuaternion = gp_Quaternion(oEuler);
		oQuaternion.SetEulerAngles(mySequence, AngX, AngY, AngZ);
		mySequence = gp_Extrinsic_XZY;
		seq = InteractiveShape::XZY;
	}

	else if (aSequence == 3)
	{
		gp_XYZ Euler1((cos(AngY) * cos(AngZ) + sin(AngY) * sin(AngX) * sin(AngZ)), (cos(AngX) * sin(AngZ)), (cos(AngY) * sin(AngX) * sin(AngZ) - cos(AngZ) * sin(AngY)));
		gp_XYZ Euler2((-cos(AngZ) * sin(AngY) * sin(AngX) - cos(AngY) * sin(AngZ)), (cos(AngX) * cos(AngZ)), (cos(AngY) * cos(AngZ) * sin(AngX) + sin(AngY) * sin(AngZ)));
		gp_XYZ Euler3((cos(AngX) * sin(AngY)), (-sin(AngX)), (cos(AngX) * cos(AngY)));
		gp_Mat oEuler(Euler1, Euler2, Euler3);
		oQuaternion = gp_Quaternion(oEuler);
		mySequence = gp_Extrinsic_YXZ;
		oQuaternion.SetEulerAngles(mySequence, AngX, AngY, AngZ);
		seq = InteractiveShape::YXZ;
	}

	else if (aSequence == 4)
	{
		gp_XYZ Euler1((cos(AngY) * cos(AngZ)), (sin(AngZ)), (-cos(AngZ) * sin(AngY)));
		gp_XYZ Euler2((sin(AngY) * sin(AngX) - cos(AngY) * cos(AngX) * sin(AngZ)), (cos(AngX) * cos(AngZ)), (cos(AngY) * sin(AngX) + cos(AngX) * sin(AngY) * sin(AngZ)));
		gp_XYZ Euler3((cos(AngX) * sin(AngY) + cos(AngY) * sin(AngZ) * sin(AngX)), (-cos(AngZ) * sin(AngX)), (cos(AngY) * cos(AngX) - sin(AngY) * sin(AngZ) * sin(AngX)));
		gp_Mat oEuler(Euler1, Euler2, Euler3);
		oQuaternion = gp_Quaternion(oEuler);
		mySequence = gp_Extrinsic_YZX;
		oQuaternion.SetEulerAngles(mySequence, AngX, AngY, AngZ);
		seq = InteractiveShape::YZX;
	}

	else if (aSequence == 5)
	{
		gp_XYZ Euler1((cos(AngY) * cos(AngZ) - sin(AngZ) * sin(AngX) * sin(AngY)), (cos(AngY) * sin(AngZ) + cos(AngZ) * sin(AngX) * sin(AngY)), (-cos(AngX) * sin(AngY)));
		gp_XYZ Euler2((-cos(AngX) * sin(AngZ)), (cos(AngZ) * cos(AngX)), (sin(AngX)));
		gp_XYZ Euler3((cos(AngZ) * sin(AngY) + cos(AngY) * sin(AngZ) * sin(AngX)), (sin(AngZ) * sin(AngY) - cos(AngZ) * cos(AngY) * sin(AngX)), (cos(AngX) * cos(AngY)));
		gp_Mat oEuler(Euler1, Euler2, Euler3);
		oQuaternion = gp_Quaternion(oEuler);
		mySequence = gp_Extrinsic_ZXY;
		oQuaternion.SetEulerAngles(mySequence, AngX, AngY, AngZ);
		seq = InteractiveShape::ZXY;
	}

	else if (aSequence == 6)
	{
		gp_XYZ Euler1((cos(AngY) * cos(AngZ)), (cos(AngY) * sin(AngZ)), (sin(AngY)));
		gp_XYZ Euler2((cos(AngZ) * sin(AngY) * sin(AngX) - cos(AngX) * sin(AngZ)), (cos(AngZ) * cos(AngX) + sin(AngZ) * sin(AngY) * sin(AngX)), (cos(AngY) * sin(AngX)));
		gp_XYZ Euler3((sin(AngZ) * sin(AngX) + cos(AngZ) * cos(AngX) * sin(AngY)), (cos(AngX) * sin(AngZ) * sin(AngY) - cos(AngZ) * sin(AngX)), (cos(AngX) * cos(AngY)));
		gp_Mat oEuler(Euler1, Euler2, Euler3);
		oQuaternion = gp_Quaternion(oEuler);
		mySequence = gp_Extrinsic_ZYX;
		oQuaternion.SetEulerAngles(mySequence, AngX, AngY, AngZ);
		seq = InteractiveShape::ZYX;
	}

	if (!listOfShapes.isEmpty() && Shapeid < listOfShapes.size())
	{
		listOfShapes[Shapeid]->rotateShape(oQuaternion, angX, angY, angZ);
	}
	myViewer->Update();
}

void Doc::translateCurrentShape(double mx, double my, double mz)
{
	if (!listOfShapes.isEmpty() && Shapeid < listOfShapes.size())
	{
		listOfShapes[Shapeid]->translatePart(mx, my, mz);
		myViewer->Update();
	}
}

void Doc::ExportFileSTEP(Standard_CString aFile3, int theShapeID)
{
	STEPControl_Writer aWriter;
	IFSelect_ReturnStatus aStat = aWriter.Transfer(listOfShapes[theShapeID]->getShape(), STEPControl_AsIs);
	aStat = aWriter.Write(aFile3);
}

void Doc::ExportFileBRep(Standard_CString aFile3, int theShapeID)
{
	BRepTools::Write(listOfShapes[theShapeID]->getShape(), aFile3);
}

//void Doc::ExportFileIGS(Standard_CString aFile3, int theShapeID)
//{
//	IGESControl_Reader reader;
//	IFSelect_ReturnStatus stat = reader.ReadFile(“filename.igs”);
//}

void Doc::ExportFileSTL(Standard_CString aFile3, int theShapeID)
{
	StlAPI_Writer myAPI1;
	myAPI1.Write(listOfShapes[theShapeID]->getShape(), aFile3);
}

void Doc::Platform()
{	
	//the Bounding Box of Printer
	gp_Pnt P1(0, 0, 0);
	gp_Pnt P2(PrinterX, 0, 0);
	gp_Pnt P3(0, PrinterY, 0);
	gp_Pnt P4(PrinterX, PrinterY, 0);
	gp_Pnt P5(0, 0, PrinterZ);
	gp_Pnt P6(PrinterX, 0, PrinterZ);
	gp_Pnt P7(0, PrinterY, PrinterZ);
	gp_Pnt P8(PrinterX, PrinterY, PrinterZ);

	BRepBuilderAPI_MakePolygon polygon1, polygon2;

	polygon1.Add(P1);
	polygon1.Add(P2);
	polygon1.Add(P4);
	polygon1.Add(P3);
	polygon1.Close();
	TopoDS_Wire wire1 = polygon1.Wire();

	polygon2.Add(P5);
	polygon2.Add(P6);
	polygon2.Add(P8);
	polygon2.Add(P7);
	polygon2.Close();
	TopoDS_Wire wire2 = polygon2.Wire();
	PAISWire2 = new AIS_Shape(wire2);

	BRepBuilderAPI_MakeEdge edge1(P1, P5);
	BRepBuilderAPI_MakeEdge edge2(P2, P6);
	BRepBuilderAPI_MakeEdge edge3(P3, P7);
	BRepBuilderAPI_MakeEdge edge4(P4, P8);
	PAISEdge1 = new AIS_Shape(edge1.Edge());
	PAISEdge2 = new AIS_Shape(edge2.Edge());
	PAISEdge3 = new AIS_Shape(edge3.Edge());
	PAISEdge4 = new AIS_Shape(edge4.Edge());

	Handle(Prs3d_LineAspect) LineAspect = new Prs3d_LineAspect(Quantity_NOC_GRAY, Aspect_TOL_SOLID, 1);
	Handle(Prs3d_Drawer) selectionStyle1 = new Prs3d_Drawer();
	selectionStyle1->SetLineAspect(LineAspect);
	selectionStyle1->SetWireAspect(LineAspect);
	selectionStyle1->SetWireDraw(true);
	
	PAISWire2->SetAttributes(selectionStyle1);
	PAISEdge1->SetAttributes(selectionStyle1);
	PAISEdge2->SetAttributes(selectionStyle1);
	PAISEdge3->SetAttributes(selectionStyle1);
	PAISEdge4->SetAttributes(selectionStyle1);
	
	//getContext()->Deactivate(AIS_Shape::SelectionMode(TopAbs_EDGE));
	//getContext()->Deactivate(AIS_Shape::SelectionMode(TopAbs_VERTEX));
	
	
	TopoDS_Wire wire = polygon1.Wire();
	TopoDS_Face Plateface = BRepBuilderAPI_MakeFace(wire);
	TopoDS_Shape PlateShape = BRepPrimAPI_MakePrism(Plateface, gp_Vec(0, 0, -2));
	Handle(AIS_Shape) PlateAIS = new AIS_Shape(PlateShape);
	Handle(Prs3d_Drawer) selectionStyle = new Prs3d_Drawer();
	selectionStyle->SetLineAspect(LineAspect);
	selectionStyle->SetFaceBoundaryDraw(true);
	selectionStyle->SetFaceBoundaryAspect(LineAspect);
	selectionStyle->SetWireAspect(LineAspect);
	selectionStyle->SetWireDraw(true);
	PlateAIS->SetAttributes(selectionStyle);
	PlateAIS->SetDisplayMode(AIS_Shaded);
	PlateAIS->SetTransparency(0.5);
	PlateAIS->SetColor(Quantity_NOC_GRAY);
	PlatformIO = PlateAIS;
	
	////aTrihedron;
	gp_Pnt OO(0, 0, 0);
	gp_Pnt PA(50, 0, 0);
	gp_Pnt PB(0, 50, 0);
	gp_Pnt PC(0, 0, 50);

	BRepBuilderAPI_MakeEdge edgeA(OO, PA);
	BRepBuilderAPI_MakeEdge edgeB(OO, PB);
	BRepBuilderAPI_MakeEdge edgeC(OO, PC);

	PAISEdgeA = new AIS_Shape(edgeA.Edge());
	PAISEdgeB = new AIS_Shape(edgeB.Edge());
	PAISEdgeC = new AIS_Shape(edgeC.Edge());

	Handle(Prs3d_LineAspect) LineAspect2 = new Prs3d_LineAspect(Quantity_NOC_BLACK, Aspect_TOL_SOLID, 4);
	Handle(Prs3d_Drawer) selectionStyle2 = new Prs3d_Drawer();
	selectionStyle2->SetLineAspect(LineAspect2);
	selectionStyle2->SetWireAspect(LineAspect2);
	selectionStyle2->SetWireDraw(true);

	Handle(Prs3d_LineAspect) LineAspect3 = new Prs3d_LineAspect(Quantity_NOC_BLACK, Aspect_TOL_SOLID, 4);
	Handle(Prs3d_Drawer) selectionStyle3 = new Prs3d_Drawer();
	selectionStyle3->SetLineAspect(LineAspect3);
	selectionStyle3->SetWireAspect(LineAspect3);
	selectionStyle3->SetWireDraw(true);

	Handle(Prs3d_LineAspect) LineAspect4 = new Prs3d_LineAspect(Quantity_NOC_BLACK, Aspect_TOL_SOLID, 4);
	Handle(Prs3d_Drawer) selectionStyle4 = new Prs3d_Drawer();
	selectionStyle4->SetLineAspect(LineAspect4);
	selectionStyle4->SetWireAspect(LineAspect4);
	selectionStyle4->SetWireDraw(true);

	PAISEdgeA->SetAttributes(selectionStyle2);
	PAISEdgeB->SetAttributes(selectionStyle3);
	PAISEdgeC->SetAttributes(selectionStyle4);
	PAISEdgeA->SetColor(Quantity_NOC_RED);
	PAISEdgeB->SetColor(Quantity_NOC_GREEN);
	PAISEdgeC->SetColor(Quantity_NOC_BLUE1);
	

	//getContext()->Display(PlatformIO, Standard_True);
}

void Doc::PlatformOn()
{
	getContext()->Display(PlatformIO, Standard_True);
	getContext()->Deactivate(PlatformIO, AIS_Shape::SelectionMode(TopAbs_SHAPE));
	getContext()->Deactivate(PlatformIO, AIS_Shape::SelectionMode(TopAbs_FACE));
	getContext()->Deactivate(PlatformIO, AIS_Shape::SelectionMode(TopAbs_EDGE));
	getContext()->Deactivate(PlatformIO, AIS_Shape::SelectionMode(TopAbs_VERTEX));
	getContext()->Deactivate(PlatformIO, AIS_Shape::SelectionMode(TopAbs_WIRE));

	getContext()->Display(PAISWire2, true);
	getContext()->Display(PAISEdge1, true);
	getContext()->Display(PAISEdge2, true);
	getContext()->Display(PAISEdge3, true);
	getContext()->Display(PAISEdge4, true);
	getContext()->Display(PAISEdgeA, true);
	getContext()->Display(PAISEdgeB, true);
	getContext()->Display(PAISEdgeC, true);
	//getContext()->Deactivate(AIS_Shape::SelectionMode(TopAbs_SHAPE));
	getContext()->Deactivate(PAISWire2, AIS_Shape::SelectionMode(TopAbs_WIRE));
	getContext()->Deactivate(PAISWire2, AIS_Shape::SelectionMode(TopAbs_SHAPE));
	getContext()->Deactivate(PAISEdge1, AIS_Shape::SelectionMode(TopAbs_EDGE));
	getContext()->Deactivate(PAISEdge1, AIS_Shape::SelectionMode(TopAbs_SHAPE));
	getContext()->Deactivate(PAISEdge2, AIS_Shape::SelectionMode(TopAbs_EDGE));
	getContext()->Deactivate(PAISEdge2, AIS_Shape::SelectionMode(TopAbs_SHAPE));
	getContext()->Deactivate(PAISEdge3, AIS_Shape::SelectionMode(TopAbs_EDGE));
	getContext()->Deactivate(PAISEdge3, AIS_Shape::SelectionMode(TopAbs_SHAPE));
	getContext()->Deactivate(PAISEdge4, AIS_Shape::SelectionMode(TopAbs_EDGE));
	getContext()->Deactivate(PAISEdge4, AIS_Shape::SelectionMode(TopAbs_SHAPE));
	getContext()->Deactivate(PAISEdgeA, AIS_Shape::SelectionMode(TopAbs_EDGE));
	getContext()->Deactivate(PAISEdgeA, AIS_Shape::SelectionMode(TopAbs_SHAPE));
	getContext()->Deactivate(PAISEdgeB, AIS_Shape::SelectionMode(TopAbs_EDGE));
	getContext()->Deactivate(PAISEdgeB, AIS_Shape::SelectionMode(TopAbs_SHAPE));
	getContext()->Deactivate(PAISEdgeC, AIS_Shape::SelectionMode(TopAbs_EDGE));
	getContext()->Deactivate(PAISEdgeC, AIS_Shape::SelectionMode(TopAbs_SHAPE));
	PlatformVisible = true;
}

void Doc::PlatformOff()
{
	getContext()->Remove(PlatformIO, true);
	getContext()->Remove(PAISWire2, true);
	getContext()->Remove(PAISEdge1, true);
	getContext()->Remove(PAISEdge2, true);
	getContext()->Remove(PAISEdge3, true);
	getContext()->Remove(PAISEdge4, true);
	getContext()->Remove(PAISEdgeA, true);
	getContext()->Remove(PAISEdgeB, true);
	getContext()->Remove(PAISEdgeC, true);
	//PlatformVisible = false;
}

void Doc::checkPartFitnessTool()
{
	fitnessDialog = new QDialog(myApp);
	fitnessDialog->setFixedSize(500, 300);

	fitnessLayout = new QFormLayout;
	fitnessDialog->setLayout(fitnessLayout);

	QDoubleSpinBox* xspin = new QDoubleSpinBox;
	xspin->setRange(-10000, 10000);
	xspin->setValue(PrinterX);
	xspin->setSuffix("mm");
	connect(xspin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
		PrinterX = xspin->value();
		/*PlatformOff();
		Platform();*/
	});

	QDoubleSpinBox* yspin = new QDoubleSpinBox;
	yspin->setRange(-10000, 10000);
	yspin->setValue(PrinterY);
	yspin->setSuffix("mm");
	connect(yspin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
		PrinterY = yspin->value();
		/*PlatformOff();
		Platform();*/
	});

	QDoubleSpinBox* zspin = new QDoubleSpinBox;
	zspin->setRange(-10000, 10000);
	zspin->setValue(PrinterZ);
	zspin->setSuffix("mm");
	connect(zspin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
		PrinterZ = zspin->value();
		/*PlatformOff();
		Platform();*/
	});

	QDoubleSpinBox* threspin = new QDoubleSpinBox;
	threspin->setRange(-10000, 10000);
	threspin->setValue(PrinterThreshold);
	threspin->setSuffix("mm");
	connect(threspin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
		PrinterThreshold = threspin->value();
	});

	PrinterX = xspin->value();
	PrinterY = yspin->value();
	PrinterZ = zspin->value();
	PrinterThreshold = 15;
	
	QPushButton* okButton = new QPushButton("Check Fitness");
	okButton->setFixedSize(150, 30);
	QObject::connect(okButton, SIGNAL(released()), this, SLOT(onCheckFitness()));

	fitnessLayout->addRow("Width (X)", xspin);
	fitnessLayout->addRow("Depth (Y)", yspin);
	fitnessLayout->addRow("Height (Z)", zspin);
	fitnessLayout->addRow("Theshold", threspin);
	fitnessLayout->addRow("", okButton);

	dimLabel = new QLabel();
	dimLabel->setFixedWidth(400);

	msgLabelX = new QLabel();
	msgLabelX->setFixedWidth(400);

	msgLabelY = new QLabel();
	msgLabelY->setFixedWidth(400);

	msgLabelZ = new QLabel();
	msgLabelZ->setFixedWidth(400);

	fitnessLayout->addWidget(dimLabel);
	fitnessLayout->addWidget(msgLabelX);
	fitnessLayout->addWidget(msgLabelY);
	fitnessLayout->addWidget(msgLabelZ);

	fitnessDialog->show();
}

void Doc::onCheckFitness()
{
	PlatformOff();
	Platform();
	PlatformOn();
	Bnd_Box B;

	double Bxmin = 0, Bymin = 0, Bzmin = 0, Bxmax = 0, Bymax = 0, Bzmax = 0;

	if (listOfShapes.size() != 0)
	{
		BRepBndLib::Add(listOfShapes.at(Shapeid)->getShape(), B);
		B.Get(Bxmin, Bymin, Bzmin, Bxmax, Bymax, Bzmax);


		double dimX = Bxmax - Bxmin;
		double dimY = Bymax - Bymin;
		double dimZ = Bzmax - Bzmin;

		dimLabel->setText("Dimensions: X = " + QString::number(dimX) + "   Y = " + QString::number(dimY) + "   Z = " + QString::number(dimZ));

		if (dimX > PrinterX - (2 * PrinterThreshold))
		{
			msgLabelX->setText("Too Big along X");
		}

		else
		{
			msgLabelX->setText("Ok along X");
		}

		if (dimY > PrinterY - (2 * PrinterThreshold))
		{
			msgLabelY->setText("Too Big along Y");
		}

		else
		{
			msgLabelY->setText("Ok along Y");
		}

		if (dimZ > PrinterZ - (PrinterThreshold))
		{
			msgLabelZ->setText("Too Big along Z");
		}

		else
		{
			msgLabelZ->setText("Ok along Z");
		}

	}

}