#include "OrientationProject.h"

#include "Doc.h"
#include "View.h"
#include "ctkrangeslider.h"
#include "MakeSurfaceResponse.h"

#include <qdebug.h>
#include <OpenGl_GraphicDriver.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_Plane.hxx>
#include <Geom_Plane.hxx>
#include <V3d_Viewer.hxx>
#include <stdlib.h>
#include <stdio.h>

#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_GTransform.hxx>

#include <StlAPI.hxx>
#include <StlAPI_Reader.hxx>
#include <RWStl.hxx>

#include <QtCore/qobject.h>
#include<QtWidgets/QWidget>
#include<QtWidgets/QVBoxLayout>
#include <QtWidgets/qpushbutton.h>
#include <QtCore/QFile>
#include <QtCore/qsignalmapper.h>
#include <QtWidgets/QMdiSubWindow>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/qbuttongroup.h>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/QToolBar>
#include <QtGui/QIcon>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qslider.h>
#include <QtWidgets/qstackedwidget.h>
#include <QtWidgets/qspinbox.h>
#include <QtGui/qfont.h>
#include <QtWidgets/qtreeview.h>
#include <QtCore/QSize>
#include <QtCore/QTimer>
#include <QtWidgets/QProgressDialog>
#include <QtWidgets/qtoolbox.h>
#include <QtWidgets/qtabwidget.h>
#include <QtGui/QWindow>
#include <QtWidgets/qdockwidget.h>
#include <QtWidgets/qscrollbar.h>
#include <QtWidgets/qscrollarea.h>
#include <QtWidgets\qcombobox.h>
#include <QtGui/qcursor.h>
#include <QtGui\QStandardItem>
#include <QtGui\QStandardItemModel>
#include <QtWidgets\QTreeView>
#include <qscreen.h>
#include <qmenubar.h>
#include <qapplication.h>

#include <QtDataVisualization/q3dsurface.h>

#include <QtDataVisualization/QSurfaceDataProxy>
#include <QtCore/QVector>
#include <QtWidgets/qgraphicsscene.h>
#include <QtWidgets/qgraphicsview.h>
#include <QtGui/qpolygon.h>
#include <QtGui/qpainterpath.h>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/qtoolbutton.h>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/qlayout.h>
#include <QtCore/qregexp.h>
#include <QtCore/QAbstractItemModel>
#include <QtDataVisualization/QCustom3DItem>
#include <QtDataVisualization/q3dscatter.h>
#include <QtDataVisualization/QValue3DAxis>

#include<QtWidgets/qwidgetaction.h>
#include <qstatusbar.h>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DCore/QEntity>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DExtras/QTextureMaterial>
#include <Qt3DExtras/QOrbitCameraController>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QCamera>
#include <Qt3DExtras/QPerVertexColorMaterial>
#include <QtCore/qmath.h>
#include <QtWidgets/QDialog>
#include <QtWidgets/QProgressBar>
#include <QtQml\qqmlengine.h>
#include <QtQml\qqmlcomponent.h>
#include <QtQuick\qquickview.h>
#include <QtQml\qqmlproperty.h>



static OrientationProject* stApp = 0;
Coffa_MainWindow* MainApp = 0;
static QMdiArea* stWs = 0;

OrientationProject::OrientationProject(QWidget* parent, Coffa_MainWindow* ParentApp, QString aProjectName, QString aFileName, QString aFilePath, Qt::WindowFlags wflags)
	: QMainWindow(parent, wflags),
	myNbDocuments(0),
	PosOnWid(1),
	ExpertFileExist(false),
	nExpertFile(0),
	EntitySelectionMode(false)
{
	MainApp = ParentApp;
	ProjectName = aProjectName;
	FilePath = aFileName;
	FileDirectory = aFilePath;
	stApp = this;
	stApp->setWindowTitle(ProjectName);
	stApp->setMouseTracking(1);
	setDockNestingEnabled(true);
	QIcon iconApp;
	iconApp.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/AppCof.png")), QIcon::Normal, QIcon::Off);
	stApp->setWindowIcon(iconApp);
	mainFrame = new QFrame(this);
	QVBoxLayout* layout = new QVBoxLayout(mainFrame);
	layout->setMargin(0);
	mainFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	setCentralWidget(mainFrame);
	this->setPalette(ParentApp->getPalette());

	setStyleSheet("QToolTip{background-color: white;border: 1px solid rgba(73, 84, 100,1); border-radius:4px}");

	aDoc = createNewDocument();
	//connect(aDoc, SIGNAL(sendCloseDocument(Doc*)), this, SLOT(onCloseDocument(Doc*)));
	//connect(aDoc, SIGNAL(progressChanged(int)), this, SLOT(onComputeProgress(int)));
	//connect(aDoc, SIGNAL(progressChanged(int)), myProgressBar, SLOT(setValue(int)));
	//connect(aDoc, SIGNAL(progressFinished(int)), this, SLOT(onComputeFinished()));

	myView = new View(aDoc->getContext(), mainFrame);
	layout->addWidget(myView);

	connect(myView, SIGNAL(PartClicked(int)), this, SLOT(getViewClicked()));
	connect(myView, SIGNAL(GridClick(int)), this, SLOT(ActivatetheGrid()));
	connect(myView, SIGNAL(GridClick2(int)), this, SLOT(DeActivatetheGrid()));

	createTabsOfTools();
	fitAll();
	onInit();

	ReadProject(aFileName);
}

OrientationProject::OrientationProject(QWidget* parent, Coffa_MainWindow* ParentApp, QString aProjectName, QString aFileName, Qt::WindowFlags wflags, int aMode)
	: QMainWindow(parent, wflags),
	myNbDocuments(0),
	PosOnWid(1),
	ExpertFileExist(false),
	nExpertFile(0),
	EntitySelectionMode(false)
{
	MainApp = ParentApp;
	ProjectName = aProjectName;
	FilePath = aFileName;
	stApp = this;
	stApp->setWindowTitle(ProjectName);
	stApp->setMouseTracking(1);
	setDockNestingEnabled(true);
	QIcon iconApp;
	iconApp.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/AppCof.png")), QIcon::Normal, QIcon::Off);
	stApp->setWindowIcon(iconApp);
	mainFrame = new QFrame(this);
	QVBoxLayout* layout = new QVBoxLayout(mainFrame);
	layout->setMargin(0);
	mainFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	setCentralWidget(mainFrame);
	this->setPalette(ParentApp->getPalette());

	setStyleSheet("QToolTip{background-color: white;border: 1px solid rgba(73, 84, 100,1); border-radius:4px}");

	aDoc = createNewDocument();
	//connect(aDoc, SIGNAL(sendCloseDocument(Doc*)), this, SLOT(onCloseDocument(Doc*)));
	//connect(aDoc, SIGNAL(progressChanged(int)), this, SLOT(onComputeProgress(int)));
	//connect(aDoc, SIGNAL(progressChanged(int)), myProgressBar, SLOT(setValue(int)));
	//connect(aDoc, SIGNAL(progressFinished(int)), this, SLOT(onComputeFinished()));

	myView = new View(aDoc->getContext(), mainFrame);
	layout->addWidget(myView);

	connect(myView, SIGNAL(PartClicked(int)), this, SLOT(getViewClicked()));
	connect(myView, SIGNAL(GridClick(int)), this, SLOT(ActivatetheGrid()));
	connect(myView, SIGNAL(GridClick2(int)), this, SLOT(DeActivatetheGrid()));

	createTabsOfTools();
	fitAll();
	onInit();

	QFile thisFile(":/Coffa_Project/Resources/GSCOP_ZTExpertise.json");
	thisFile.open(QIODevice::ReadOnly | QIODevice::Text);
	QString fileContent = thisFile.readAll();
	thisFile.close();

	QJsonDocument doc = QJsonDocument::fromJson(fileContent.toUtf8());
	QJsonObject Project_obj = doc.object();
	QJsonArray listRules = Project_obj.value(QString("ActionRules")).toArray();
	onLoadExpertFileJSON(listRules);

	//onSaveProject();

}

void OrientationProject::createTabsOfTools()
{
	//Tab of Tools
	fileDock = new QDockWidget(this);
	addDockWidget(Qt::TopDockWidgetArea, fileDock);
	fileDock->setFixedHeight(90);
	fileDock->resize(this->width(), 90);
	fileDock->setWindowTitle("  File  ");
	fileDock->setStyleSheet("QDockWidget::pane { border: 0; }");
	QWidget* titleWidget0 = new QWidget(this);
	fileDock->setTitleBarWidget(titleWidget0);

	printDock = new QDockWidget(this);
	addDockWidget(Qt::TopDockWidgetArea, printDock);
	printDock->setFixedHeight(90);
	printDock->resize(this->width(), 90);
	printDock->setWindowTitle("  Printer  ");
	printDock->setStyleSheet("QDockWidget::pane { border: 0; }");
	QWidget* titleWidget01 = new QWidget(this);
	printDock->setTitleBarWidget(titleWidget01);

	orientDock = new QDockWidget(this);
	addDockWidget(Qt::TopDockWidgetArea, orientDock);
	orientDock->setFixedHeight(90);
	orientDock->resize(this->width(), 90);
	orientDock->setWindowTitle("  Orientation  ");
	orientDock->setStyleSheet("QDockWidget::pane { border: 0; }");
	QWidget* titleWidget1 = new QWidget(this);
	orientDock->setTitleBarWidget(titleWidget1);

	supportDock = new QDockWidget(this);
	addDockWidget(Qt::TopDockWidgetArea, supportDock);
	supportDock->setFixedHeight(90);
	supportDock->resize(this->width(), 90);
	supportDock->setWindowTitle("  Support  ");
	QWidget* titleWidget2 = new QWidget(this);
	supportDock->setTitleBarWidget(titleWidget2);

	viewDock = new QDockWidget(this);
	addDockWidget(Qt::TopDockWidgetArea, viewDock);
	viewDock->setFixedHeight(90);
	viewDock->resize(this->width(), 90);
	viewDock->setWindowTitle("  View  ");
	viewDock->setStyleSheet("QDockWidget::pane { border: 0; }");
	QWidget* titleWidget3 = new QWidget(this);
	viewDock->setTitleBarWidget(titleWidget3);

	tabifyDockWidget(fileDock, viewDock);
	tabifyDockWidget(viewDock, printDock);
	tabifyDockWidget(printDock, orientDock);
	tabifyDockWidget(orientDock, supportDock);
	setTabPosition(Qt::TopDockWidgetArea, QTabWidget::North);

	QTabBar* northBar = this->findChildren<QTabBar*>().last();
	northBar->setStyleSheet("QTabBar{background-color : rgba(232, 232, 232,1);font-size: 13px;border-width: 0px 0px 0px 0px; height:28px}"
		"QTabBar::tab { width:125px;  height:28px} "
		"QTabBar::tab:selected { font: bold; background: rgba(244, 244, 242,1); color: rgba(73, 84, 100, 1); border-width:0px 0px 5px 0px; border-style: solid; border-color: white white rgba(73, 84, 100, 1) white;} "
		"QTabBar::tab:!selected { background: rgba(255,255,255,1); color: rgba(73, 84, 100, 1);  border-width: 0px 0px 0px 0px; border-style: solid; border-color: white white white white;} "
		"QTabBar::tab:hover { background: rgba(73, 84, 100, 1); color: white; border-width: 0px 0px 5px 0px; border-style: solid; border-color: white white #827878 white;} "
		"QWidget { background: rgba(255, 255, 255,1); border-width: 0px 0px 0px 0px;} ");
	northBar->setCurrentIndex(0);
}

void OrientationProject::fitAll()
{
	myView->fitAll();
}

void OrientationProject::onInit()
{
	ProjectToolBar();
	LeftTree();
	onSingleResponseTools();
	onCreateVisualizationDialog();

	stApp->setDockNestingEnabled(true);

	ParttoBuildProp = new QMenu(this);

	QIcon iconrem;
	iconrem.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/delete.png")), QIcon::Normal, QIcon::Off);
	QAction* action1 = new QAction("Remove", this);
	action1->setIcon(iconrem);
	action1->setToolTip("Remove Part from Project");
	action1->setShortcut(QObject::tr("DELETE"));
	connect(action1, SIGNAL(triggered()), this, SLOT(onRemovePartFromPlate()));

	QIcon iconr;
	iconr.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/CustomOr.png")), QIcon::Normal, QIcon::Off);
	QAction* action2 = new QAction("Rotate", this);
	action2->setIcon(iconr);
	action2->setToolTip("Rotate Part");
	connect(action2, SIGNAL(triggered()), this, SLOT(onShowRotateDialog()));

	QIcon icont;
	icont.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/Translate.png")), QIcon::Normal, QIcon::Off);
	QAction* action3 = new QAction("Move", this);
	action3->setIcon(icont);
	action3->setToolTip("Move Part");
	connect(action3, SIGNAL(triggered()), this, SLOT(onShowTranslateDialog()));

	QIcon iconex;
	iconex.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/Export.png")), QIcon::Normal, QIcon::Off);
	QAction* actionexp = new QAction("Export Geometry", this);
	actionexp->setIcon(iconex);
	actionexp->setToolTip("Export Geometry into STEP or STL");
	connect(actionexp, SIGNAL(triggered()), this, SLOT(onExportDialog()));

	QToolBar* toolbarl = new QToolBar;
	toolbarl->addAction(action1);
	toolbarl->addSeparator();
	toolbarl->addAction(action2);
	toolbarl->addAction(action3);
	toolbarl->addSeparator();
	toolbarl->addAction(actionexp);
	
	QWidgetAction* act = new QWidgetAction(this);
	act->setDefaultWidget(toolbarl);
	ParttoBuildProp->addAction(act);
	ParttoBuildProp->hide();

	//Part Click from the Tree
	ParttoTreeProp = new QMenu(this);
	QAction* action11 = new QAction("Remove", this);
	action11->setIcon(iconrem);
	action11->setToolTip("Remove Part from Project");
	connect(action11, SIGNAL(triggered()), this, SLOT(onRemovePartFromPlate()));

	QAction* action22 = new QAction("Rotate", this);
	action22->setIcon(iconr);
	action22->setToolTip("Rotate Part");
	connect(action22, SIGNAL(triggered()), this, SLOT(onShowRotateDialog()));

	QAction* action33 = new QAction("Move", this);
	action33->setIcon(icont);
	action33->setToolTip("Move Part");
	connect(action33, SIGNAL(triggered()), this, SLOT(onShowTranslateDialog()));

	QAction* actionexp2 = new QAction("Export Geometry", this);
	actionexp2->setIcon(iconex);
	actionexp2->setToolTip("Export Geometry into STEP or STL");
	connect(actionexp2, SIGNAL(triggered()), this, SLOT(onExportDialog()));
	
	QToolBar* toolbarl1 = new QToolBar;
	toolbarl1->addAction(action11);
	toolbarl1->addSeparator();
	toolbarl1->addAction(action22);
	toolbarl1->addAction(action33);
	toolbarl1->addSeparator();
	toolbarl1->addAction(actionexp2);
	QWidgetAction* act1 = new QWidgetAction(this);
	act1->setDefaultWidget(toolbarl1);
	ParttoTreeProp->addAction(act1);

	ParttoTreeProp->hide();

	//XPRT File Click Properties

	//Transformation Dialog
	createRotationDialog();
	createTranslationDialog();

	//Context Menu for Adding Surfaces
	SurfClickProperties = new QMenu(this);
	QIcon iconadd;
	iconadd.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/GenFunc.png")), QIcon::Normal, QIcon::Off);
	QAction* actionAdd = new QAction("Add To List", this);
	actionAdd->setIcon(iconadd);
	connect(actionAdd, SIGNAL(triggered()), this, SLOT(addFaceTo()));

	QAction* actionRemove = new QAction("Remove From List", this);
	actionRemove->setIcon(iconrem);
	connect(actionRemove, SIGNAL(triggered()), this, SLOT(RemoveFaceFrom2()));

	QToolBar* toolbars = new QToolBar;
	toolbars->addAction(actionAdd);
	toolbars->addAction(actionRemove);
	QWidgetAction* acts = new QWidgetAction(this);
	act->setDefaultWidget(toolbars);
	SurfClickProperties->addAction(acts);

	/*SurfListProperties = new QMenu(this);
	QAction* action3 = new QAction("Remove From List", this);
	action3->setIcon(iconr);
	connect(action3, SIGNAL(triggered()), this, SLOT(RemoveFaceFrom2()));

	QToolBar* toolbarl2 = new QToolBar;
	toolbarl2->addAction(action3);
	QWidgetAction* act2 = new QWidgetAction(this);
	act2->setDefaultWidget(toolbarl2);
	SurfListProperties->addAction(act2);*/

}

void OrientationProject::closeEvent(QCloseEvent* event)
{
	if (myXRDF.exists())
		myXRDF.remove();
	/*QSettings settings("MyCompany", "MyApp");
	settings.setValue("geometry", saveGeometry());
	settings.setValue("windowState", saveState());*/
	QMainWindow::closeEvent(event);
}

void OrientationProject::createViewActions()
{
	WinCreated = true;

	aToolBar = new QToolBar(this);
	aToolBar->setOrientation(Qt::Horizontal);
	aToolBar->setFixedSize(500, 45);
	aToolBar->setMovable(0);
	QSize asi;
	asi.setHeight(25);
	asi.setWidth(25);
	aToolBar->setIconSize(asi);

	QList<QAction*>* aList = myView->getViewActions();

	aToolBar->addActions(*aList);
	aToolBar->setStyleSheet("QToolBar{background: #4b4b4b;}");
	aToolBar->toggleViewAction()->setVisible(true);
}

void OrientationProject::resizeEvent(QResizeEvent* e)
{
	QMainWindow::resizeEvent(e);

	if (leftTreeCreated)
	{
		onUpdateLeftTree();
	}

	if (respWidgetCreated)
	{
		SingleRespWidget->move(mainFrame->width()- 520, 95);
		SingleRespWidget->setFixedSize(500, mainFrame->height()-110);
		//respTitleWidget->setFixedSize(mainFrame->width() - 250, 25);
		SingleRespScroll->setGeometry(0, 30, SingleRespWidget->width(), SingleRespWidget->height()-25);

	}
}

void OrientationProject::ProjectToolBar()
{
	QIcon iconsv;
	iconsv.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/SaveProj.png")), QIcon::Normal, QIcon::Off);
	QToolButton* SaveProjButton = new QToolButton();
	SaveProjButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	SaveProjButton->setIcon(iconsv);
	SaveProjButton->setText("Save\nProject");
	SaveProjButton->setToolTip("Save current project\nCTRL+S");
	SaveProjButton->setStatusTip(QObject::tr("Save Project - CTRL+S"));
	SaveProjButton->setShortcut(QObject::tr("CTRL+S"));
	connect(SaveProjButton, SIGNAL(released()), this, SLOT(onSaveProject()));
	ListofToolBarButton.push_back(SaveProjButton);

	QIcon icon1;
	icon1.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/Load.png")), QIcon::Normal, QIcon::Off);
	QToolButton* LoadButton = new QToolButton(this);
	LoadButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	LoadButton->setIcon(icon1);
	LoadButton->setText("Load\nPart");
	LoadButton->setStatusTip(QObject::tr("Load Part"));
	LoadButton->setStatusTip(QObject::tr("Load new Part to Plate"));
	connect(LoadButton, SIGNAL(released()), this, SLOT(onAddPartToPlate()));
	ListofToolBarButton.push_back(LoadButton);

	QIcon iconExport;
	iconExport.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/Export.png")), QIcon::Normal, QIcon::Off);
	QToolButton* SaveToSTLButton = new QToolButton();
	SaveToSTLButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	SaveToSTLButton->setIcon(iconExport);
	SaveToSTLButton->setText("Export\nPart");
	SaveToSTLButton->setToolTip("Export the processed part");
	SaveToSTLButton->setStatusTip(QObject::tr("Export Part"));
	connect(SaveToSTLButton, SIGNAL(released()), this, SLOT(onExportDialog()));
	ListofToolBarButton.push_back(SaveToSTLButton);

	QIcon imp;
	imp.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/ImportXPRT.png")), QIcon::Normal, QIcon::Off);
	QToolButton* ImportXPT = new QToolButton();
	ImportXPT->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	ImportXPT->setIcon(imp);
	ImportXPT->setText("Import\nExpertise");
	ImportXPT->setStatusTip(QObject::tr("Import Expertise File"));
	ImportXPT->setShortcut(QObject::tr("CTRL+E"));
	ImportXPT->setToolTip("Import Expertise File: .json, .txt\nCtrl+E");
	connect(ImportXPT, SIGNAL(released()), this, SLOT(onLoadExpertFileJSON()));
	ListofToolBarButton.push_back(ImportXPT);

	QIcon sho;
	sho.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/OpenXPRT.png")), QIcon::Normal, QIcon::Off);
	QToolButton* showXPT = new QToolButton();
	showXPT->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	showXPT->setIcon(sho);
	showXPT->setText("Visualize\nExpertise");
	showXPT->setStatusTip(QObject::tr("Visualize Expertise File"));
	showXPT->setToolTip("Visualize Expertise File");
	connect(showXPT, SIGNAL(released()), this, SLOT(onARVizualizer()));
	ListofToolBarButton.push_back(showXPT);

	QIcon icon3;
	icon3.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/About.png")), QIcon::Normal, QIcon::Off);
	QToolButton* AboutButton = new QToolButton();
	AboutButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	AboutButton->setIcon(icon3);
	AboutButton->setText("About\nCOFFA");
	AboutButton->setToolTip("About COFFA");
	AboutButton->setStatusTip(QObject::tr("Learn More About COFFA"));
	connect(AboutButton, SIGNAL(released()), this, SLOT(onAbout()));

	//Printer
	QIcon iconsetpr;
	iconsetpr.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/prSettings.png")), QIcon::Normal, QIcon::Off);
	QToolButton* PrSetButton = new QToolButton();
	PrSetButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	PrSetButton->setIcon(iconsetpr);
	PrSetButton->setText("Printer\nSettings");
	PrSetButton->setToolTip("Printer Settings");
	connect(PrSetButton, SIGNAL(released()), this, SLOT(onShowPrinterSettings()));
	ListofToolBarButton.push_back(PrSetButton);

	//Orientation
	QIcon iconsetor;
	iconsetor.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/orSettings.png")), QIcon::Normal, QIcon::Off);
	QToolButton* OrSetButton = new QToolButton();
	OrSetButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	OrSetButton->setIcon(iconsetor);
	OrSetButton->setText("Settings");
	OrSetButton->setToolTip("Orientation Settings");
	connect(OrSetButton, SIGNAL(released()), this, SLOT(onShowOrientSettings()));
	ListofToolBarButton.push_back(OrSetButton);

	QIcon iconComp;
	iconComp.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/Compute.png")), QIcon::Normal, QIcon::Off);
	QToolButton* ComputeButton = new QToolButton();
	ComputeButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	ComputeButton->setIcon(iconComp);
	ComputeButton->setText("Compute\nOrientation");
	ComputeButton->setToolTip("Compute All Orientations Desirability");
	ComputeButton->setStatusTip(QObject::tr("Compute All Orientations Desirability"));
	connect(ComputeButton, SIGNAL(released()), this, SLOT(onStartComputation()));
	ListofToolBarButton.push_back(ComputeButton);

	QIcon iconCustComp;
	iconCustComp.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/CustCompute.png")), QIcon::Normal, QIcon::Off);
	QToolButton* CustCompButton = new QToolButton();
	CustCompButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	CustCompButton->setIcon(iconCustComp);
	CustCompButton->setText("Evaluate\nDesirability");
	CustCompButton->setToolTip("Evaluate Desirability for a given orientation");
	CustCompButton->setStatusTip(QObject::tr("Evaluate Desirability for a given orientation"));
	connect(CustCompButton, SIGNAL(released()), this, SLOT(onShowComputeForOnePos()));
	ListofToolBarButton.push_back(CustCompButton);

	QIcon iconCust;
	iconCust.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/CustomOr.png")), QIcon::Normal, QIcon::Off);
	QToolButton* CustomOrient = new QToolButton();
	CustomOrient->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	CustomOrient->setIcon(iconCust);
	CustomOrient->setText("Rotate\nPart");
	CustomOrient->setToolTip("Set Custom Orientation by Defining Angles");
	CustomOrient->setStatusTip(QObject::tr("Set Custom Orientation by Defining Angles"));
	connect(CustomOrient, SIGNAL(released()), this, SLOT(onShowRotateDialog()));
	ListofToolBarButton.push_back(CustomOrient);

	QIcon iconCust1;
	iconCust1.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/Translate.png")), QIcon::Normal, QIcon::Off);
	QToolButton* CustomTranslate = new QToolButton();
	CustomTranslate->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	CustomTranslate->setIcon(iconCust1);
	CustomTranslate->setText("Move\nPart");
	CustomTranslate->setToolTip("Set Custom Position by Defining Coordinates");
	CustomTranslate->setStatusTip(QObject::tr("Set Custom Position by Defining Coordinates"));
	connect(CustomTranslate, SIGNAL(released()), this, SLOT(onShowTranslateDialog()));
	ListofToolBarButton.push_back(CustomTranslate);

	QIcon iconen;
	iconen.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/enlargeWin.png")), QIcon::Normal, QIcon::Off);
	QToolButton* WinButton = new QToolButton();
	WinButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	WinButton->setIcon(iconen);
	WinButton->setText("Display\nResults");
	WinButton->setToolTip("Display all individual results\nResponse Surfaces");
	WinButton->setStatusTip("Display all individual results\nResponse Surfaces");
	connect(WinButton, SIGNAL(released()), this, SLOT(onDisplayResponseWidget()));
	ListofToolBarButton.push_back(WinButton);

	QIcon iconanalyz;
	iconanalyz.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/orAnalysis.png")), QIcon::Normal, QIcon::Off);
	QToolButton* AnalysisButton = new QToolButton();
	AnalysisButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	AnalysisButton->setIcon(iconanalyz);
	AnalysisButton->setText("Analyze\nResults");
	AnalysisButton->setToolTip("Analyze Results\nwith Decision-Making Tools");
	//AnalysisButton->setStatusTip("Display all individual results\nResponse Surfaces");
	connect(AnalysisButton, SIGNAL(released()), this, SLOT(onShowAnalysisTools()));
	ListofToolBarButton.push_back(AnalysisButton);

	QIcon icondistrib;
	icondistrib.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/shapeVisu.png")), QIcon::Normal, QIcon::Off);
	QToolButton* VisuDistribButton = new QToolButton();
	VisuDistribButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	VisuDistribButton->setIcon(icondistrib);
	VisuDistribButton->setText("Desirability\nDistribution");
	VisuDistribButton->setToolTip("Visualize desirability distribution on\ncurrent shape");
	//AnalysisButton->setStatusTip("Display all individual results\nResponse Surfaces");
	connect(VisuDistribButton, SIGNAL(released()), this, SLOT(onShowVisualizationDialog()));
	ListofToolBarButton.push_back(VisuDistribButton);


	//Support
	QIcon iconsetsup;
	iconsetsup.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/supSettings.png")), QIcon::Normal, QIcon::Off);
	QToolButton* SupSetButton = new QToolButton();
	SupSetButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	SupSetButton->setIcon(iconsetsup);
	SupSetButton->setText("Settings");
	SupSetButton->setToolTip("Support Settings");
	SupSetButton->setDisabled(true);
	connect(SupSetButton, SIGNAL(released()), this, SLOT(onShowSupportSettings()));
	//ListofToolBarButton.push_back(SupSetButton);

	QIcon iconFit;
	iconFit.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/Fitness.png")), QIcon::Normal, QIcon::Off);
	QToolButton* FitButton = new QToolButton();
	FitButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	FitButton->setIcon(iconFit);
	FitButton->setText("Part\nFitness");
	FitButton->setToolTip("Check Part Fitness in Machine\n");
	FitButton->setStatusTip(QObject::tr("Check Part Fitness in Machine"));
	FitButton->setDisabled(true);
	connect(FitButton, SIGNAL(released()), this, SLOT(onFitnessChecker()));
	ListofToolBarButton.push_back(FitButton);

	QIcon iconRTS;
	iconRTS.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/rts.png")), QIcon::Normal, QIcon::Off);
	QToolButton* RTSButton = new QToolButton();
	RTSButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	RTSButton->setIcon(iconRTS);
	RTSButton->setText("Regions\nto Support");
	RTSButton->setToolTip("Generate Regions to Support\n");
	RTSButton->setStatusTip(QObject::tr("Generate Regions to Support"));
	RTSButton->setDisabled(true);
	connect(RTSButton, SIGNAL(released()), this, SLOT(onGenRTS()));
	//ListofToolBarButton.push_back(RTSButton);

	QIcon iconSupport;
	iconSupport.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/suppGen.png")), QIcon::Normal, QIcon::Off);
	QToolButton* SuppButton = new QToolButton();
	SuppButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	SuppButton->setIcon(iconSupport);
	SuppButton->setText("Build\nSupport");
	SuppButton->setToolTip("Build Support Structures");
	SuppButton->setStatusTip(QObject::tr("Build Support Structures"));
	SuppButton->setDisabled(true);
	connect(SuppButton, SIGNAL(released()), this, SLOT(onGenSupport()));
	//ListofToolBarButton.push_back(SuppButton);

	QIcon iconSSDR;
	iconSSDR.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/SSDR.png")), QIcon::Normal, QIcon::Off);
	QToolButton* SSDRButton = new QToolButton();
	SSDRButton->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	SSDRButton->setIcon(iconSSDR);
	SSDRButton->setText("Support\nRemoval");
	SSDRButton->setToolTip("Simulate Zones with Potential\n Support Difficult to Remove");
	SSDRButton->setStatusTip(QObject::tr("Compute & Display Zones with Potential\n Support Difficult to Remove"));
	SSDRButton->setDisabled(true);
	connect(SSDRButton, SIGNAL(released()), this, SLOT(onSSDRTools()));
	//ListofToolBarButton.push_back(SSDRButton);

	QIcon iconSSDR2;
	iconSSDR2.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/SSDR2.png")), QIcon::Normal, QIcon::Off);
	QToolButton* SSDRButton2 = new QToolButton();
	SSDRButton2->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	SSDRButton2->setIcon(iconSSDR2);
	SSDRButton2->setText("Analyze\nSupport");
	SSDRButton2->setToolTip("Find the Blocs of\n Support Difficult to Remove");
	SSDRButton2->setStatusTip(QObject::tr("Find the Blocs of Support Difficult to Remove"));
	SSDRButton2->setDisabled(true);
	connect(SSDRButton2, SIGNAL(released()), this, SLOT(onSSDRTools2()));
	//ListofToolBarButton.push_back(SSDRButton2);


	QSize asi;
	asi.setHeight(34);
	asi.setWidth(34);

	//File
	myStdToolBar = addToolBar(tr("Main Tool-Bar"));
	myStdToolBar->setFixedHeight(90);
	myStdToolBar->setParent(fileDock);
	myStdToolBar->setOrientation(Qt::Horizontal);
	myStdToolBar->setMinimumWidth(1000);
	myStdToolBar->setMovable(0);
	myStdToolBar->setIconSize(asi);
	myStdToolBar->setStyleSheet("QToolBar::separator{background-color: rgba(187, 191, 202, 1);width: 0.5px}"
		"QToolButton{height:80px; width:55px;}");

	myStdToolBar->addWidget(SaveProjButton);
	myStdToolBar->addSeparator();
	myStdToolBar->addWidget(LoadButton);
	myStdToolBar->addWidget(SaveToSTLButton);
	myStdToolBar->addSeparator();
	myStdToolBar->addWidget(ImportXPT);
	myStdToolBar->addWidget(showXPT);
	myStdToolBar->addSeparator();
	myStdToolBar->addWidget(AboutButton);

	//View
	viewToolBar = addToolBar(tr("View Tool-Bar"));
	viewToolBar->setFixedHeight(90);
	viewToolBar->setParent(viewDock);
	viewToolBar->setOrientation(Qt::Horizontal);
	viewToolBar->setMinimumWidth(1000);
	viewToolBar->setMovable(0);
	viewToolBar->setIconSize(asi);
	viewToolBar->setStyleSheet("QToolBar::separator{background-color: rgba(187, 191, 202, 1);width: 0.5px}"
		"QToolButton{height:80px; width:50px;}");
	viewToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

	QList<QAction*>* aList = myView->getViewActions();
	viewToolBar->addActions(*aList);

	//Printer
	printToolBar = addToolBar(tr("Printer Tool-Bar"));
	printToolBar->setFixedHeight(90);
	printToolBar->setParent(printDock);
	printToolBar->setOrientation(Qt::Horizontal);
	printToolBar->setMinimumWidth(1000);
	printToolBar->setMovable(0);
	printToolBar->setIconSize(asi);
	printToolBar->setStyleSheet("QToolBar::separator{background-color: rgba(187, 191, 202, 1);width: 0.5px}"
		"QToolButton{height:80px; width:60px;}");
	printToolBar->addWidget(PrSetButton);
	printToolBar->addSeparator();

	//Orientation
	orientToolbar = addToolBar(tr("Orientation Tool-Bar"));
	orientToolbar->setFixedHeight(90);
	orientToolbar->setParent(orientDock);
	orientToolbar->setOrientation(Qt::Horizontal);
	orientToolbar->setMinimumWidth(1000);
	orientToolbar->setMovable(0);
	orientToolbar->setIconSize(asi);
	orientToolbar->setStyleSheet("QToolBar::separator{background-color: rgba(187, 191, 202, 1);width: 0.5px}"
		"QToolButton{height:80px; width:60px;}");

	orientToolbar->addWidget(OrSetButton);
	orientToolbar->addSeparator();
	orientToolbar->addWidget(CustomOrient);
	orientToolbar->addWidget(CustomTranslate);
	orientToolbar->addSeparator();
	orientToolbar->addWidget(ComputeButton);
	orientToolbar->addWidget(CustCompButton);
	orientToolbar->addSeparator();
	orientToolbar->addWidget(WinButton);
	orientToolbar->addWidget(AnalysisButton);
	orientToolbar->addWidget(VisuDistribButton);
	orientToolbar->addSeparator();
	orientToolbar->addWidget(FitButton);

	//Support
	suppToolBar = addToolBar(tr("Orientation Tool-Bar"));
	suppToolBar->setFixedHeight(90);
	suppToolBar->setParent(supportDock);
	suppToolBar->setOrientation(Qt::Horizontal);
	suppToolBar->setMinimumWidth(1000);
	suppToolBar->setMovable(0);
	suppToolBar->setIconSize(asi);
	suppToolBar->setStyleSheet("QToolBar::separator{background-color: rgba(187, 191, 202, 1);width: 0.5px}"
		"QToolButton{height:80px; width:60px;}");

	suppToolBar->addWidget(SupSetButton);
	suppToolBar->addSeparator();
	suppToolBar->addWidget(RTSButton);
	suppToolBar->addWidget(SuppButton);
	suppToolBar->addSeparator();
	suppToolBar->addWidget(SSDRButton);
	suppToolBar->addWidget(SSDRButton2);
}

void OrientationProject::ProjectMenuBar()
{
	QAction* PartImportAction, * PartExportAction, * fileCloseAction, * XCADExportAction,
		* fileQuitAction, * viewToolAction, * helpAboutAction, * ImportXPTAction, * ComputeAction, * ShowRespAction,
		* CustomOrtAction, * CustomTrlAction/*fileOpenAction*/;

	QIcon icon1;
	icon1.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/Load.png")), QIcon::Normal, QIcon::Off);
	PartImportAction = new QAction(tr("Load Geometry"), this);
	PartImportAction->setToolTip(QObject::tr("Load a Geometry to Start Analysis"));
	PartImportAction->setIcon(icon1);
	PartImportAction->setIconText("Load Geometry");
	PartImportAction->setStatusTip(QObject::tr("Load Geometry"));
	PartImportAction->setShortcut(QObject::tr("CTRL+O"));
	connect(PartImportAction, SIGNAL(triggered()), this, SLOT(onAddPartToPlate()));
	myStdActions.insert(FileNewId, PartImportAction);

	QIcon iconExport;
	iconExport.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/Export.png")), QIcon::Normal, QIcon::Off);
	PartExportAction = new QAction(tr("Export Geometry"), this);
	PartExportAction->setToolTip(QObject::tr("Export Geometry into STEP or STL\nCTRL+E"));
	PartExportAction->setIcon(iconExport);
	PartExportAction->setIconText("Export Geometry into STEP or STL");
	PartExportAction->setStatusTip(QObject::tr("Export Geometry into STEP or STL"));
	PartExportAction->setShortcut(QObject::tr("CTRL+E"));
	connect(PartExportAction, SIGNAL(triggered()), this, SLOT(onExportDialog()));
	myStdActions.insert(FileExportId, PartExportAction);

	QIcon imp;
	imp.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/ImportXPRT.png")), QIcon::Normal, QIcon::Off);
	ImportXPTAction = new QAction(tr("Import\nXPT File"), this);
	ImportXPTAction->setToolTip(QObject::tr("Import Expert File: .xprt, .txt\nCtrl+P"));
	ImportXPTAction->setIcon(imp);
	ImportXPTAction->setIconText("Import\nXPT File");
	ImportXPTAction->setStatusTip(QObject::tr("Import XPT File"));
	ImportXPTAction->setShortcut(QObject::tr("CTRL+P"));
	connect(ImportXPTAction, SIGNAL(triggered()), this, SLOT(onLoadExpertFile()));
	myStdActions.insert(ImportXPTId, ImportXPTAction);

	QIcon objic;
	objic.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/XCAD.png")), QIcon::Normal, QIcon::Off);
	XCADExportAction = new QAction(tr("Export Extended CAD File"), this);
	XCADExportAction->setToolTip(QObject::tr("Export Extended CAD File: .xprt, .txt\nCtrl+P"));
	XCADExportAction->setIcon(objic);
	XCADExportAction->setStatusTip(QObject::tr("Export Extended CAD File"));
	connect(XCADExportAction, SIGNAL(triggered()), this, SLOT(onMakeExtendedCAD()));
	XCADExportAction->setDisabled(1);
	myStdActions.insert(FileXCADId, XCADExportAction);

	fileCloseAction = new QAction(QObject::tr("Close"), this);
	fileCloseAction->setToolTip(QObject::tr("Close"));
	fileCloseAction->setStatusTip(QObject::tr("Close document"));
	fileCloseAction->setShortcut(QObject::tr("CTRL+W"));
	connect(fileCloseAction, SIGNAL(triggered()), this, SLOT(onCloseWindow()));
	myStdActions.insert(FileCloseId, fileCloseAction);

	QIcon quitic;
	quitic.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/CloseTab.png")), QIcon::Normal, QIcon::Off);
	fileQuitAction = new QAction(QObject::tr("Quit"), this);
	fileQuitAction->setToolTip(QObject::tr("Quit COFFA Analyzer"));
	fileQuitAction->setStatusTip(QObject::tr("Quit"));
	fileQuitAction->setIcon(quitic);
	fileQuitAction->setShortcut(QObject::tr("CTRL+Q"));
	connect(fileQuitAction, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
	myStdActions.insert(FileQuitId, fileQuitAction);

	viewToolAction = new QAction(QObject::tr("Show/Hide ToolBar"), this);
	viewToolAction->setToolTip(QObject::tr("Show/Hide ToolBar"));
	viewToolAction->setStatusTip(QObject::tr("Show/Hide ToolBar"));
	connect(viewToolAction, SIGNAL(triggered()), this, SLOT(onViewToolBar()));
	viewToolAction->setCheckable(true);
	viewToolAction->setChecked(true);
	myStdActions.insert(ViewToolId, viewToolAction);

	QIcon iconCust;
	iconCust.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/CustomOr.png")), QIcon::Normal, QIcon::Off);
	CustomOrtAction = new QAction(QObject::tr("Rotate Part"), this);
	CustomOrtAction->setIcon(iconCust);
	CustomOrtAction->setText("Rotate Part");
	CustomOrtAction->setToolTip("Set Custom Orientation by Defining Angles");
	CustomOrtAction->setStatusTip(QObject::tr("Set Custom Orientation by Defining Angles"));
	connect(CustomOrtAction, SIGNAL(released()), this, SLOT(onisRotatePressed()));
	myStdActions.insert(CustOrId, CustomOrtAction);

	QIcon iconCust1;
	iconCust1.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/Translate.png")), QIcon::Normal, QIcon::Off);
	CustomTrlAction = new QAction(QObject::tr("Move Part"), this);
	CustomTrlAction->setToolTip(QObject::tr("Move Part"));
	CustomTrlAction->setStatusTip(QObject::tr("Move Along X, Y or Z"));
	CustomTrlAction->setIcon(iconCust1);
	connect(CustomTrlAction, SIGNAL(triggered()), this, SLOT(onisTranslatePressed()));
	myStdActions.insert(CustTrId, CustomTrlAction);


	QIcon icon3;
	icon3.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/About.png")), QIcon::Normal, QIcon::Off);
	helpAboutAction = new QAction(QObject::tr("About"), this);
	helpAboutAction->setToolTip(QObject::tr("About COFFA"));
	helpAboutAction->setStatusTip(QObject::tr("About COFFA"));
	helpAboutAction->setShortcut(QObject::tr("F1"));
	helpAboutAction->setIcon(icon3);
	connect(helpAboutAction, SIGNAL(triggered()), this, SLOT(onAbout()));
	myStdActions.insert(HelpAboutId, helpAboutAction);

	QIcon iconComp;
	iconComp.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/Compute.png")), QIcon::Normal, QIcon::Off);
	ComputeAction = new QAction(QObject::tr("Compute"), this);
	ComputeAction->setStatusTip("Compute All Orientations");
	ComputeAction->setToolTip("Compute All Orientations");
	ComputeAction->setStatusTip(QObject::tr("Compute All Orientations"));
	ComputeAction->setIcon(iconComp);
	connect(ComputeAction, SIGNAL(triggered()), this, SLOT(onStartComputation()));
	myStdActions.insert(CompId, ComputeAction);

	QIcon iconResponse;
	iconResponse.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/enlargeWin.png")), QIcon::Normal, QIcon::Off);
	ShowRespAction = new QAction(QObject::tr("Display Results"), this);
	ShowRespAction->setStatusTip("Display all results");
	ShowRespAction->setIcon(iconResponse);
	ShowRespAction->setToolTip("Display all results");
	connect(ShowRespAction, SIGNAL(triggered()), this, SLOT(onDisplayResponseWidget()));
	myStdActions.insert(ShowRespId, ShowRespAction);



	menuBar()->setStyleSheet("QMenuBar{ background: #696969; color: white}"
		"QMenuBar::selected{ background: #ff9f00; color: black}");

	//Files Menu
	QMenu* filem = new QMenu(this);
	filem = menuBar()->addMenu(QObject::tr("File"));
	filem->addAction(PartImportAction);
	filem->addAction(PartExportAction);
	filem->addAction(ImportXPTAction);
	filem->addAction(XCADExportAction);
	//filem->addAction(fileCloseAction);
	filem->addAction(fileQuitAction);

	//View Menu
	QMenu* view = new QMenu(this);
	view = menuBar()->addMenu(QObject::tr("View"));
	view->addAction(viewToolAction);

	//Tools Menu
	QMenu* tools = new QMenu(this);
	tools = menuBar()->addMenu(QObject::tr("Tools"));

	tools->addAction(CustomOrtAction);
	tools->addAction(CustomTrlAction);
	tools->addSeparator();
	tools->addAction(ComputeAction);
	tools->addSeparator();
	tools->addAction(ShowRespAction);

	//Help menu
	QMenu* help = new QMenu(this);
	menuBar()->addSeparator();
	help = menuBar()->addMenu(QObject::tr("Help"));
	help->addAction(helpAboutAction);

}

QMdiArea* OrientationProject::getWorkspace()
{
	return stWs;
}

OrientationProject* OrientationProject::getApplication()
{
	return stApp;
}

Doc* OrientationProject::createNewDocument()
{
	return new Doc(++myNbDocuments, this);
}

void OrientationProject::onCloseWindow()
{
	stWs->activeSubWindow()->close();

}

void OrientationProject::onViewStatusBar()
{
	bool show = myStdActions.at(ViewStatusId)->isChecked();
	if (show == statusBar()->isVisible())
		return;
	if (show)
		statusBar()->show();
	else
		statusBar()->hide();
}

void OrientationProject::onAbout()
{
	QMessageBox::information(this, QObject::tr("ABOUT"), QObject::tr("COFFA Project: Assistance in CAM Decision Making in Additive Manufacturing.\nThis ANR project is carried by G-SCOP Grenoble and I2M Bordeaux.\n M. M. MBOW,\nF. VIGNAT,\nP. R. MARIN,\nN. PERRY,\nF. POURROY,\nJ. PAILHES,\nM. MONTEMURRO, \nC. GRANDVALLET"),
		QObject::tr("OK"), QString::null, QString::null, 0, 0);
}


//Left Dock Widget
void OrientationProject::LeftTree()
{
	LeftWidget = new QWidget(mainFrame);
	LeftWidget->move(20, 45);
	LeftWidget->setFixedSize(400, 700);
	LeftWidget->setStyleSheet("QWidget{background: rgba(232, 232, 232, 1); }");

	QWidget* titWidget = new QWidget(mainFrame);
	titWidget->move(20, 15);
	titWidget->setFixedSize(400, 30);
	titWidget->setStyleSheet("QWidget{background: rgba(187, 191, 202, 1); color: rgba(73, 54, 100, 1); font-size: 16px;}");

	QLabel* iconLabel = new QLabel(titWidget);
	iconLabel->setPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/ProjIcon.png")));
	iconLabel->move(10, 2.5);

	QLabel* titleLabel = new QLabel(titWidget);
	titleLabel->move(55, 0);
	titleLabel->setText(" Project Manager");
	titleLabel->setFixedSize(150, 30);
	//titleLabel->setStyleSheet("QLabel{background: rgba(187, 191, 202, 1); color: rgba(73, 54, 100, 1); font-size: 16px;}"); //border-top-left-radius: 15px; border-top-right-radius: 15px;
	titleLabel->show();

	QIcon aHideicon;
	aHideicon.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/hider.png")), QIcon::Normal, QIcon::Off);
	hideButton = new QPushButton(titWidget);
	hideButton->setFixedSize(25, 25);
	hideButton->move(360, 2.5);
	hideButton->setIcon(aHideicon);
	hideButton->setToolTip("Hide Manager");
	hideButton->show();
	connect(hideButton, SIGNAL(released()), this, SLOT(onHideLeftWidget()));

	QIcon aShowicon;
	aShowicon.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/shower.png")), QIcon::Normal, QIcon::Off);
	showButton = new QPushButton(titWidget);
	showButton->setFixedSize(25, 25);
	showButton->move(360, 2.5);
	showButton->setIcon(aShowicon);
	showButton->setToolTip("Show Manager");
	showButton->hide();
	connect(showButton, SIGNAL(released()), this, SLOT(onShowLeftWidget()));


	LeftFrame = new QFrame(LeftWidget);
	LeftFrame->move(0, 0);
	LeftFrame->setFixedSize(400, 670);

	projectStack = new QStackedLayout;
	LeftFrame->setLayout(projectStack);

	TreeWidget = new QWidget(this);
	TreeWidget->setMinimumWidth(399);
	TreeWidget->setStyleSheet("QWidget { background: rgba(232, 232, 232, 1);}");
	TreeWidget->show();

	TreeScroll = new QScrollArea(TreeWidget);
	TreeScroll->setGeometry(0, 10, 379, TreeWidget->height());
	TreeScroll->setAlignment(Qt::AlignRight);
	TreeScroll->setWidgetResizable(true);
	TreeScroll->setFrameShape(QFrame::NoFrame);

	myTree = new QTreeView();
	myTree->setHeaderHidden(true);
	myTree->setFrameShape(QFrame::NoFrame);
	TreeScroll->setWidget(myTree);

	model = new QStandardItemModel();
	model->setColumnCount(1);

	itemProject = new QStandardItem(ProjectName);
	itemProject->setEditable(false);

	itemExpertise = new QStandardItem("Expertise");
	itemExpertise->setEditable(false);

	model->appendRow(itemProject);
	model->appendRow(itemExpertise);
	myTree->setModel(model);
	myTree->show();

	generalSettings = new QWidget(this);
	printerSettingTools();
	orientSettings = new QWidget(this);
	OrientationSettingTools();
	supportSettings = new QWidget(this);
	SupportSettingTools();
	attributeWidget = new QWidget(this);
	onMakeAttributeWidget();
	
	AnalysisWidget = new QWidget(/*"Settings",*/ this);
	onAnalysisTools();

	projectStack->addWidget(TreeWidget);
	projectStack->addWidget(generalSettings);
	projectStack->addWidget(orientSettings);
	projectStack->addWidget(supportSettings);
	projectStack->addWidget(AnalysisWidget);
	projectStack->addWidget(attributeWidget);

	LeftWidget->show();
	connect(myTree, SIGNAL(clicked(QModelIndex)), this, SLOT(myTreeClicked(QModelIndex)));
	connect(myTree, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(myTreeDoubleClicked(QModelIndex)));

	leftTreeCreated = true;
}

void OrientationProject::onHideLeftWidget()
{
	LeftWidget->hide();
	hideButton->hide();
	showButton->show();
}

void OrientationProject::onShowLeftWidget()
{
	LeftWidget->show();
	hideButton->show();
	showButton->hide();
}

void OrientationProject::printerSettingTools()
{
	/*Tabb2 = new QTabWidget(SetgWidget);
	Tabb2->setTabPosition(QTabWidget::North);
	Tabb2->setMinimumWidth(380);
	Tabb2->setMinimumHeight(LeftWidget->height());
	Tabb2->move(0, 0);
	Tabb2->setStyleSheet("QTabBar { height: 40px; width: 40px;  background: rgba(232, 232, 232, 1);}"
		"QTabBar::tab { height: 30px; width: 100px;}"
		"QTabBar::tab:selected { background: rgba(255,255,255,1); color: rgba(73, 84, 100, 1); }"
		"QTabBar::tab:hover { background: rgba(73, 84, 100, 1); color: white; }");*/

		//General

	QIcon aValicon;
	aValicon.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/Validate.png")), QIcon::Normal, QIcon::Off);
	QPushButton* ValidButton = new QPushButton(generalSettings);
	ValidButton->setFixedSize(40, 40);
	ValidButton->move(10, 10);
	ValidButton->setIcon(aValicon);
	ValidButton->setToolTip("Ok/Go Back");
	ValidButton->show();
	connect(ValidButton, SIGNAL(released()), this, SLOT(onBackHome()));

	SettingScroll = new QScrollArea(generalSettings);
	SettingScroll->setGeometry(0, 50, 389, generalSettings->height());
	SettingScroll->setAlignment(Qt::AlignRight);
	SettingScroll->setWidgetResizable(true);
	SettingScroll->setFrameShape(QFrame::NoFrame);

	QGroupBox* printerWidgets = new QGroupBox(generalSettings);
	printerWidgets->setTitle("Printer Dimensions Setting");
	printerWidgets->move(10, 60);
	printerWidgets->setFixedSize(380, 150);
	QFormLayout* printerLayout = new QFormLayout();
	printerWidgets->setLayout(printerLayout);

	widSpin = new QDoubleSpinBox;
	widSpin->setRange(0, 5000);
	widSpin->setValue(aDoc->PrinterX);
	widSpin->setSuffix(" mm");
	widSpin->setSingleStep(1);
	widSpin->setAlignment(Qt::AlignRight);
	widSpin->setFixedHeight(25);
	connect(widSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
		aDoc->PrinterX = widSpin->value();
	});

	depSpin = new QDoubleSpinBox;
	depSpin->setRange(0, 5000);
	depSpin->setValue(aDoc->PrinterY);
	depSpin->setSuffix(" mm");
	depSpin->setSingleStep(1);
	depSpin->setAlignment(Qt::AlignRight);
	depSpin->setFixedHeight(25);
	connect(depSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
		aDoc->PrinterY = depSpin->value();
	});

	heiSpin = new QDoubleSpinBox;
	heiSpin->setRange(0, 5000);
	heiSpin->setValue(aDoc->PrinterZ);
	heiSpin->setSuffix(" mm");
	heiSpin->setSingleStep(1);
	heiSpin->setAlignment(Qt::AlignRight);
	heiSpin->setFixedHeight(25);
	connect(heiSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
		aDoc->PrinterZ = heiSpin->value();
	});

	QPushButton* setPlatform = new QPushButton("Update Dimensions", generalSettings);
	setPlatform->setFixedSize(200, 30);
	setPlatform->move(100, 210);
	setPlatform->show();
	connect(setPlatform, SIGNAL(released()), this, SLOT(onPlatformUpdate()));

	printerLayout->addRow("Width", widSpin);
	printerLayout->addRow("Depth", depSpin);
	printerLayout->addRow("Height", heiSpin);


	/*Tabb2->addTab(generalSettings, "General");
	Tabb2->addTab(orientSettings, "Orientation");
	Tabb2->addTab(supportSettings, "Support");*/
}

void OrientationProject::onShowPrinterSettings()
{
	projectStack->setCurrentWidget(generalSettings);
}

void OrientationProject::OrientationSettingTools()
{
	QIcon aValicon;
	aValicon.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/Validate.png")), QIcon::Normal, QIcon::Off);
	QPushButton* ValidButton = new QPushButton(orientSettings);
	ValidButton->setFixedSize(40, 40);
	ValidButton->move(10, 10);
	ValidButton->setIcon(aValicon);
	ValidButton->setToolTip("Ok/Go Back");
	ValidButton->show();
	connect(ValidButton, SIGNAL(released()), this, SLOT(onBackHome()));

	QGroupBox* firtsAxWidgets = new QGroupBox(orientSettings);
	firtsAxWidgets->setTitle("First Axis");
	firtsAxWidgets->move(10, 60);
	firtsAxWidgets->setFixedSize(380, 110);
	QFormLayout* firstAxLayout = new QFormLayout;
	firtsAxWidgets->setLayout(firstAxLayout);

	QWidget* firtsCheckWidgets = new QWidget(orientSettings);
	firtsCheckWidgets->setFixedSize(280, 40);
	QHBoxLayout* firstCheckLayout = new QHBoxLayout;
	firtsCheckWidgets->setLayout(firstCheckLayout);

	QGroupBox* secAxWidgets = new QGroupBox(orientSettings);
	secAxWidgets->setTitle("Second Axis");
	secAxWidgets->move(10, 210);
	secAxWidgets->setFixedSize(380, 110);
	QFormLayout* secAxLayout = new QFormLayout;
	secAxWidgets->setLayout(secAxLayout);

	QWidget* secCheckWidgets = new QWidget(orientSettings);
	secCheckWidgets->setFixedSize(280, 40);
	QHBoxLayout* secCheckLayout = new QHBoxLayout;
	secCheckWidgets->setLayout(secCheckLayout);

	xax1 = new QRadioButton("X");
	xax1->setChecked(1);
	connect(xax1, &QRadioButton::clicked, [=]() {
		theAxis1 = 1;
		onAxisChanged();
	});
	firstCheckLayout->addWidget(xax1);

	yax1 = new QRadioButton("Y");
	connect(yax1, &QRadioButton::clicked, [=]() {
		theAxis1 = 2;
		onAxisChanged();
	});
	firstCheckLayout->addWidget(yax1);

	zax1 = new QRadioButton("Z");
	connect(zax1, &QRadioButton::clicked, [=]() {
		theAxis1 = 3;
		onAxisChanged();
	});
	firstCheckLayout->addWidget(zax1);

	QDoubleSpinBox* step1Spin = new QDoubleSpinBox;
	step1Spin->setRange(0, 180);
	step1Spin->setValue(theStep1);
	step1Spin->setSuffix(" deg");
	step1Spin->setSingleStep(1);
	step1Spin->setAlignment(Qt::AlignRight);
	step1Spin->setFixedSize(150, 25);
	connect(step1Spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
		theStep1 = step1Spin->value();
	});

	firstAxLayout->addRow("Axis", firtsCheckWidgets);
	firstAxLayout->addRow("Angle Step", step1Spin);

	xax2 = new QRadioButton("X");
	connect(xax2, &QRadioButton::clicked, [=]() {
		theAxis2 = 1;
		onAxisChanged();
	});
	secCheckLayout->addWidget(xax2);

	yax2 = new QRadioButton("Y");
	yax2->setChecked(1);
	connect(yax2, &QRadioButton::toggled, [=]() {
		theAxis2 = 2;
		onAxisChanged();
	});
	secCheckLayout->addWidget(yax2);

	zax2 = new QRadioButton("Z");
	connect(zax2, &QRadioButton::clicked, [=]() {
		theAxis2 = 3;
		onAxisChanged();
	});
	secCheckLayout->addWidget(zax2);

	QDoubleSpinBox* step2Spin = new QDoubleSpinBox;
	step2Spin->setRange(0, 180);
	step2Spin->setValue(theStep2);
	step2Spin->setSuffix(" deg");
	step2Spin->setSingleStep(1);
	step2Spin->setAlignment(Qt::AlignRight);
	step2Spin->setFixedSize(150, 25);
	connect(step2Spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
		theStep2 = step2Spin->value();
	});

	secAxLayout->addRow("Axis", secCheckWidgets);
	secAxLayout->addRow("Angle Step", step2Spin);
}

void OrientationProject::onShowOrientSettings()
{
	projectStack->setCurrentWidget(orientSettings);
}

void OrientationProject::onBackHome()
{
	projectStack->setCurrentWidget(TreeWidget);
}

void OrientationProject::onAnalysisTools()
{
	AnalysisWidget->setFixedWidth(400);
	AnalysisWidget->setStyleSheet("QWidget { background:rgba(232, 232, 232, 1);}");

	QIcon aValicon;
	aValicon.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/GoBack.png")), QIcon::Normal, QIcon::Off);
	QPushButton* ValidButton = new QPushButton(AnalysisWidget);
	ValidButton->setFixedSize(40, 40);
	ValidButton->move(10, 10);
	ValidButton->setIcon(aValicon);
	ValidButton->setToolTip("Go Back");
	ValidButton->show();
	connect(ValidButton, SIGNAL(released()), this, SLOT(onBackHome()));

	QGroupBox* selGroup = new QGroupBox("Selection", AnalysisWidget);
	selGroup->setFixedSize(380, 110);
	selGroup->move(10, 60);

	QFormLayout* aform = new QFormLayout();
	selGroup->setLayout(aform);

	AnalysisModeCombo = new QComboBox(AnalysisWidget);
	AnalysisModeCombo->setFixedHeight(25);
	connect(AnalysisModeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onUpdateSelectedAnalysisMode()));

	workingPartCombo = new QComboBox(AnalysisWidget);
	workingPartCombo->setFixedHeight(25);
	connect(workingPartCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onUpdateSelectedPart()));

	aform->addRow("Display Mode", AnalysisModeCombo);
	aform->addRow("Select Part", workingPartCombo);

	analysisFrame = new QFrame(AnalysisWidget);
	analysisFrame->move(0, 180);
	analysisFrame->setFixedWidth(400);
	analysisFrame->setFixedHeight(800);
	analysisStack = new QStackedLayout(analysisFrame);

	AnalysisScroll = new QScrollArea(AnalysisWidget);
	AnalysisScroll->setGeometry(0, 180, 400, 800);
	AnalysisScroll->setAlignment(Qt::AlignRight);
	AnalysisScroll->setWidgetResizable(true);
	AnalysisScroll->setFrameShape(QFrame::NoFrame);
	AnalysisScroll->setWidget(analysisFrame);

	barsWidget = new QWidget();
	barsWidget->setFixedWidth(400);
	createBarsWidget();
	
	radarWidget = new QWidget();
	radarWidget->setFixedWidth(400);
	createRadarWidget();

	/*thresholdingWidget = new QWidget();
	thresholdingWidget->setFixedWidth(400);
	createThresholdingWidget();*/
	
	weightingWidget = new QWidget();
	weightingWidget->setFixedWidth(400);
	weightingWidget->setFixedHeight(800);
	createWeightingWidget();

	analysisStack->addWidget(radarWidget);
	analysisStack->addWidget(barsWidget);
	analysisStack->addWidget(weightingWidget);
	//analysisStack->addWidget(thresholdingWidget);

	
	AnalysisModeCombo->addItem("Radar");
	AnalysisModeCombo->addItem("Bars");
	AnalysisModeCombo->addItem("Weighting");
	//AnalysisModeCombo->addItem("Thresholding");

	
}

void OrientationProject::onShowAnalysisTools()
{
	projectStack->setCurrentWidget(AnalysisWidget);
}

void OrientationProject::onUpdateSelectedAnalysisMode()
{
	int cur = AnalysisModeCombo->currentIndex();

	if (cur == 0)
	{
		onShowRadarWidget();
	}

	else if (cur == 1)
	{
		onShowBarsWidget();
	}

	else if (cur == 2)
	{
		onShowWeigthingWidget();
	}

	/*else if (cur == 3)
	{
		onShowThresholdingWidget();
	}*/
}

void OrientationProject::createRadarWidget()
{
	QGroupBox *RotationCtrlWid = new QGroupBox(radarWidget);
	RotationCtrlWid->setTitle("Orientation Control");
	RotationCtrlWid->move(10, 0);
	RotationCtrlWid->setFixedSize(380, 150);
	QHBoxLayout *rotCtrlLayout = new QHBoxLayout;
	RotationCtrlWid->setLayout(rotCtrlLayout);

	QFont aFont;
	aFont.setPointSize(8);
	aFont.setBold(1);

	QWidget* widX = new QWidget;
	widX->setFixedSize(100, 100);
	QVBoxLayout* vlayX = new QVBoxLayout;
	widX->setLayout(vlayX);

	QLabel* label1 = new QLabel("First Axis", widX);
	label1->setAlignment(Qt::AlignCenter);
	label1->setFixedSize(100, 20);
	label1->move(0, 0);
	Rot1Dial = new QDial(widX);
	Rot1Dial->setFixedSize(80, 80);
	Rot1Dial->move(0, 20);
	Rot1Dial->setMaximum(180);
	Rot1Dial->setMinimum(0);
	Rot1Dial->setSingleStep(theStep1);
	Rot1Dial->setPageStep(theStep1 * 2);
	Rot1Dial->setNotchesVisible(1);
	Rot1Dial->setNotchTarget(theStep1);
	Rot1Dial->setTracking(1);

	QLabel* XDLabel = new QLabel("0", Rot1Dial);
	XDLabel->setFixedSize(40, 25);
	XDLabel->move(20, 27.5);
	XDLabel->setAlignment(Qt::AlignCenter);
	XDLabel->setFont(aFont);
	XDLabel->setStyleSheet("QLabel {background : transparent; }");
	XDLabel->show();
	connect(Rot1Dial, QOverload<int>::of(&QDial::valueChanged), [=](double value) {
		Rot1 = Rot1Dial->value();
		XDLabel->setText(QString::number(Rot1Dial->value()));
	});
	connect(Rot1Dial, SIGNAL(sliderReleased()), this, SLOT(onRadarDisplay()));
	rotCtrlLayout->addWidget(widX, Qt::AlignCenter);

	QWidget* widY = new QWidget;
	widY->setFixedSize(100, 100);
	QVBoxLayout* vlayY = new QVBoxLayout;
	widY->setLayout(vlayY);

	QLabel* label2 = new QLabel("Second Axis", widY);
	label2->setAlignment(Qt::AlignCenter);
	label2->move(0, 0);
	Rot2Dial = new QDial(widY);
	Rot2Dial->setFixedSize(80, 80);
	Rot2Dial->move(0, 20);
	Rot2Dial->setMaximum(360);
	Rot2Dial->setMinimum(0);
	Rot2Dial->setSingleStep(theStep2);
	Rot2Dial->setPageStep(theStep2 * 2);
	Rot2Dial->setNotchesVisible(1);
	Rot2Dial->setNotchTarget(theStep2);

	QLabel* YDLabel = new QLabel("0", Rot2Dial);
	YDLabel->setFixedSize(40, 25);
	YDLabel->move(20, 27.5);
	YDLabel->setAlignment(Qt::AlignCenter);
	YDLabel->setFont(aFont);
	YDLabel->setStyleSheet("QLabel {background : transparent; border-radius:5px;}");
	YDLabel->show();
	connect(Rot2Dial, QOverload<int>::of(&QDial::valueChanged), [=](double value) {
		Rot2 = Rot2Dial->value();
		YDLabel->setText(QString::number(Rot2Dial->value()));
	});
	connect(Rot2Dial, SIGNAL(sliderReleased()), this, SLOT(onRadarDisplay()));
	rotCtrlLayout->addWidget(widY, Qt::AlignCenter);


	//Chart Making
	ResultPolarChart = new QPolarChart();
	ResultPolarChart->legend()->hide();
	QChartView* chartView = new QChartView(ResultPolarChart, radarWidget);
	chartView->setRenderHint(QPainter::Antialiasing);
	chartView->move(10, 160);
	chartView->setFixedSize(380, 320);

	QLineSeries* aSerie = new QLineSeries;

	PolarAxisX = new QCategoryAxis();
	PolarAxisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
	PolarAxisX->setRange(0, 360);
	//PolarAxisX->append("Cat 1", 20);
	//PolarAxisX->setLabelsAngle(270);

	if (theRules.size() != 0)
	{
		double pos = 360 / theRules.size();
		for (int i = 0; i < theRules.size(); i++)
		{
			PolarAxisX->append(theRules[i]->getName(), pos*i);
			*aSerie << QPointF(pos*i, 1);
		}
	}

	ResultPolarChart->addAxis(PolarAxisX, QPolarChart::PolarOrientationAngular);
	ResultPolarChart->addSeries(aSerie);

	aSerie->attachAxis(PolarAxisX);

	PolarAxisY = new QValueAxis();
	PolarAxisY->setRange(0, 1.0);
	ResultPolarChart->addAxis(PolarAxisY, QPolarChart::PolarOrientationRadial);
	aSerie->attachAxis(PolarAxisY);
}

void OrientationProject::onShowRadarWidget()
{
	//WeightingCtrlWid->hide();
	//RotationCtrlWid->show();
	analysisStack->setCurrentWidget(radarWidget);
}

void OrientationProject::createBarsWidget()
{
	//Rotation Control
	QFont aFont;
	aFont.setPointSize(8);
	aFont.setBold(1);

	QGroupBox* RotationCtrlWid = new QGroupBox(barsWidget);
	RotationCtrlWid->setTitle("Orientation Control");
	RotationCtrlWid->move(10, 0);
	RotationCtrlWid->setFixedSize(380, 150);
	QHBoxLayout* rotCtrlLayout = new QHBoxLayout;
	RotationCtrlWid->setLayout(rotCtrlLayout);

	QWidget* widX = new QWidget;
	widX->setFixedSize(100, 100);
	QVBoxLayout* vlayX = new QVBoxLayout;
	widX->setLayout(vlayX);

	QLabel* label1 = new QLabel("First Axis", widX);
	label1->setAlignment(Qt::AlignCenter);
	label1->setFixedSize(100, 20);
	label1->move(0, 0);
	Rot11Dial = new QDial(widX);
	Rot11Dial->setFixedSize(80, 80);
	Rot11Dial->move(0, 20);
	Rot11Dial->setMaximum(180);
	Rot11Dial->setMinimum(0);
	Rot11Dial->setSingleStep(theStep1);
	Rot11Dial->setPageStep(theStep1 * 2);
	Rot11Dial->setNotchesVisible(1);
	Rot11Dial->setNotchTarget(theStep1);
	Rot11Dial->setTracking(1);

	QLabel* XDLabel = new QLabel("0", Rot11Dial);
	XDLabel->setFixedSize(40, 25);
	XDLabel->move(20, 27.5);
	XDLabel->setAlignment(Qt::AlignCenter);
	XDLabel->setFont(aFont);
	XDLabel->setStyleSheet("QLabel {background : transparent; }");
	XDLabel->show();
	connect(Rot11Dial, QOverload<int>::of(&QDial::valueChanged), [=](double value) {
		Rot1 = Rot11Dial->value();
		XDLabel->setText(QString::number(Rot11Dial->value()));
	});
	connect(Rot11Dial, SIGNAL(sliderReleased()), this, SLOT(onBarsDisplay()));
	rotCtrlLayout->addWidget(widX, Qt::AlignCenter);

	QWidget* widY = new QWidget;
	widY->setFixedSize(100, 100);
	QVBoxLayout* vlayY = new QVBoxLayout;
	widY->setLayout(vlayY);

	QLabel* label2 = new QLabel("Second Axis", widY);
	label2->setAlignment(Qt::AlignCenter);
	label2->move(0, 0);
	Rot22Dial = new QDial(widY);
	Rot22Dial->setFixedSize(80, 80);
	Rot22Dial->move(0, 20);
	Rot22Dial->setMaximum(360);
	Rot22Dial->setMinimum(0);
	Rot22Dial->setSingleStep(theStep2);
	Rot22Dial->setPageStep(theStep2 * 2);
	Rot22Dial->setNotchesVisible(1);
	Rot22Dial->setNotchTarget(theStep2);

	QLabel* YDLabel = new QLabel("0", Rot22Dial);
	YDLabel->setFixedSize(40, 25);
	YDLabel->move(20, 27.5);
	YDLabel->setAlignment(Qt::AlignCenter);
	YDLabel->setFont(aFont);
	YDLabel->setStyleSheet("QLabel {background : transparent; border-radius:5px;}");
	YDLabel->show();
	connect(Rot22Dial, QOverload<int>::of(&QDial::valueChanged), [=](double value) {
		Rot2 = Rot22Dial->value();
		YDLabel->setText(QString::number(Rot22Dial->value()));
	});
	connect(Rot22Dial, SIGNAL(sliderReleased()), this, SLOT(onBarsDisplay()));
	rotCtrlLayout->addWidget(widY, Qt::AlignCenter);

	//Chart Making
	ResultBarChart = new QChart();
	ResultBarChart->legend()->hide();
	QChartView* chartView = new QChartView(ResultBarChart, barsWidget);
	chartView->setRenderHint(QPainter::Antialiasing);
	chartView->move(10, 160);
	chartView->setFixedSize(380, 320);

	QBarSeries* aSerie = new QBarSeries;
	QBarSet* seti = new QBarSet("AR");

	QStringList rulesLabel;
	if (theRules.size() != 0)
	{
		for (int i = 0; i < theRules.size(); i++)
		{
			rulesLabel.push_back(theRules[i]->getName());
			*seti << 0.01;
		}
	}
	ResultBarChart->addSeries(aSerie);

	aSerie->append(seti);
	BarAxisX = new QBarCategoryAxis();
	BarAxisX->append(rulesLabel);
	BarAxisX->setLabelsAngle(270);
	ResultBarChart->addAxis(BarAxisX, Qt::AlignBottom);
	aSerie->attachAxis(BarAxisX);

	BarAxisY = new QValueAxis();
	BarAxisY->setRange(0, 1.0);
	ResultBarChart->addAxis(BarAxisY, Qt::AlignLeft);
	aSerie->attachAxis(BarAxisY);
}

void OrientationProject::onShowBarsWidget()
{
	//WeightingCtrlWid->hide();
	//RotationCtrlWid->show();
	analysisStack->setCurrentWidget(barsWidget);
}

void OrientationProject::createWeightingWidget()
{
	/*QSurface3DSeries* ser = new QSurface3DSeries();
	SurfaceResult = new MakeSurfaceResponse(ser, this, 180, 360, "Composite");
	QWidget* containerRes = SurfaceResult->getTheMainWidget();
	containerRes->setFixedHeight(330);
	containerRes->setFixedWidth(350);
	wLay->addWidget(containerRes);
	containerRes->show();*/

	//Weighting Control
	WeightingCtrlWid = new QGroupBox(weightingWidget);
	WeightingCtrlWid->setTitle("Weights Control");
	WeightingCtrlWid->setFixedSize(360, 220);
	WeightingCtrlWid->move(10, 0);
	weigthersLayout = new QGridLayout;
	WeightingCtrlWid->setLayout(weigthersLayout);

	QPushButton* ValidButton = new QPushButton("Update Aggregation", weightingWidget);
	ValidButton->setFixedSize(360, 30);
	ValidButton->move(10, 230);
	ValidButton->setToolTip("Re-Calculate Aggregation");
	ValidButton->show();
	connect(ValidButton, SIGNAL(released()), this, SLOT(onAggregateValues()));

	theAggregated.getResponse()->setParent(weightingWidget);
	theAggregated.getResponse()->setProject(this);
	theAggregated.getResponse()->setTitle("Aggregation");
	theAggregated.getResponse()->setSize(360, 300);
	theAggregated.getResponse()->move(10, 270);
	theAggregated.getResponse()->show();
}

void OrientationProject::onShowWeigthingWidget()
{
	//WeightingCtrlWid->show();
	//RotationCtrlWid->hide();
	
	analysisStack->setCurrentWidget(weightingWidget);
}

void OrientationProject::createThresholdingWidget()
{
	QLabel* lab = new QLabel("Coming Soon...\nWorking on it !", thresholdingWidget);
	lab->move(20, 100);
	QFont aFont;
	aFont.setPointSize(15);
	lab->setFont(aFont);
}

void OrientationProject::onShowThresholdingWidget()
{
	WeightingCtrlWid->hide();
	//RotationCtrlWid->hide();
	analysisStack->setCurrentWidget(thresholdingWidget);
}

void OrientationProject::onMakeAttributeWidget()
{
	QFormLayout* aLay = new QFormLayout;
	attributeWidget->setLayout(aLay);

	//connect(attributeWidget, SIGNAL(lastWindowClosed()), this, SLOT(onSaveFaceSelection()));

	QIcon aBackicon;
	aBackicon.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/GoBack.png")), QIcon::Normal, QIcon::Off);
	QPushButton* CancelButton = new QPushButton(attributeWidget);
	CancelButton->setFixedSize(40, 40);
	CancelButton->move(10, 10);
	CancelButton->setIcon(aBackicon);
	CancelButton->setToolTip("Cancel/Go Back");
	CancelButton->show();
	connect(CancelButton, SIGNAL(released()), this, SLOT(onCancelFaceSelection()));

	QIcon aValicon;
	aValicon.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/Validate.png")), QIcon::Normal, QIcon::Off);
	QPushButton* ValidButton = new QPushButton(attributeWidget);
	ValidButton->setFixedSize(40, 40);
	ValidButton->move(60, 10);
	ValidButton->setIcon(aValicon);
	ValidButton->setToolTip("Save Selection");
	ValidButton->show();
	connect(ValidButton, SIGNAL(released()), this, SLOT(onSaveFaceSelection()));

	attribTitle = new QLabel("Select items", attributeWidget);
	attribTitle->setFixedSize(380, 30);
	attribTitle->move(10, 60);
	QFont aFont;
	aFont.setPointSize(9);
	attribTitle->setFont(aFont);

	attribViewList = new QListView(attributeWidget);
	attribViewList->setFixedHeight(250);
	attribViewList->setFixedWidth(380);
	attribViewList->move(10, 100);

	QPushButton* AddFButton = new QPushButton("Add", attributeWidget);
	AddFButton->setFixedSize(180, 30);
	AddFButton->move(10, 360);
	AddFButton->show();
	connect(AddFButton, SIGNAL(released()), this, SLOT(addFaceTo()));

	QPushButton* AddAllFButton = new QPushButton("Add All", attributeWidget);
	AddAllFButton->setFixedSize(180, 30);
	AddAllFButton->move(10, 400);
	AddAllFButton->show();
	connect(AddAllFButton, SIGNAL(released()), this, SLOT(addAllFaceTo()));

	QPushButton* RemFButton = new QPushButton("Remove", attributeWidget);
	RemFButton->setFixedSize(180, 30);
	RemFButton->move(210, 360);
	RemFButton->show();
	connect(RemFButton, SIGNAL(released()), this, SLOT(RemoveFaceFrom()));

	QPushButton* RemAllFButton = new QPushButton("Remove All", attributeWidget);
	RemAllFButton->setFixedSize(180, 30);
	RemAllFButton->move(210, 400);
	RemAllFButton->show();
	connect(RemAllFButton, SIGNAL(released()), this, SLOT(RemoveAllFaceFrom()));
}

void OrientationProject::onShowAttributeWidget()
{
	attribModel = new QStringListModel();
	attribVar = aDoc->getListOfShapes()[aDoc->Shapeid]->getAttributedSurfaces(theRules[SelectedARid]->getID());
	QStringList myList;
	for (int i = 0; i < attribVar.items.size(); i++)
	{
		myList.push_back(aDoc->getListOfShapes()[aDoc->Shapeid]->getSurfaces()[attribVar.items[i]]->getName());
	}
	attribModel->setStringList(myList);
	attribViewList->setModel(attribModel);

	numOfFace = myList.size();
	attribTitle->setText("Select " + theRules[SelectedARid]->getAttribute() + " items (" + QString::number(numOfFace) + ")");


	projectStack->setCurrentWidget(attributeWidget);
	aDoc->SetupSelectionModes();
	EntitySelectionMode = true;

	SurfClickProperties = new QMenu(this);
	QIcon iconv;
	iconv.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/addSurf.png")), QIcon::Normal, QIcon::Off);
	QAction* action1 = new QAction("Add To List", this);
	action1->setIcon(iconv);
	connect(action1, SIGNAL(triggered()), this, SLOT(addFaceTo()));

	QIcon iconr;
	iconr.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/remSurf.png")), QIcon::Normal, QIcon::Off);
	QAction* action2 = new QAction("Remove From List", this);
	action2->setIcon(iconr);
	connect(action2, SIGNAL(triggered()), this, SLOT(RemoveFaceFrom2()));

	QToolBar* toolbarl = new QToolBar;
	toolbarl->addAction(action1);
	toolbarl->addAction(action2);
	QWidgetAction* act = new QWidgetAction(this);
	act->setDefaultWidget(toolbarl);
	SurfClickProperties->addAction(act);
}

void OrientationProject::onUpdateLeftTree()
{
	//Tabb->setFixedHeight(LeftWidget->height());
	LeftWidget->setFixedHeight(mainFrame->height() - 60);
	TreeScroll->setFixedHeight(LeftWidget->height());
	SettingScroll->setFixedHeight(LeftWidget->height());
	AnalysisWidget->setFixedHeight(LeftWidget->height());
	analysisFrame->setFixedHeight(800);
	AnalysisScroll->setFixedHeight(LeftWidget->height()-180);
	//AnalysisScroll->setMaximumHeight(500);

	generalSettings->setFixedSize(LeftWidget->width(), LeftWidget->height());
	orientSettings->setFixedSize(LeftWidget->width(), LeftWidget->height());
	supportSettings->setFixedSize(LeftWidget->width(), LeftWidget->height());

}

void OrientationProject::myTreeClicked(QModelIndex index)
{
	int indrowProj = index.parent().parent().parent().row();
	int indrowPart = index.parent().parent().row();
	int indrowCurrent = index.row();
	if (indrowProj == 0 && index.parent().isValid() && index.parent().parent().isValid())
	{
		//SelectedARid = index.row();

		for (int i = 0; i < theRules.size(); i++)
		{
			if (index.data().toString().contains(theRules[i]->getAttribute(), Qt::CaseInsensitive))
			{
				SelectedARid = i;
				break;
			}
		}

		aDoc->Shapeid = indrowPart;
		QScreen* screen0 = QApplication::screens().at(0);
		QPoint pos0 = QCursor::pos(screen0);
		FeatureProperties->exec(pos0);
	}

	if (index.parent().row() == 0 && !index.parent().parent().isValid())
	{
		//SelectedARid = index.row();

		aDoc->Shapeid = index.row();
		QScreen* screen0 = QApplication::screens().at(0);
		QPoint pos0 = QCursor::pos(screen0);
		ParttoTreeProp->exec(pos0);
	}

	if (index.parent().row() == 1 && !index.parent().parent().isValid())
	{
		//SelectedARid = index.row();

		//aDoc->Shapeid = index.row();
		/*QScreen* screen0 = QApplication::screens().at(0);
		QPoint pos0 = QCursor::pos(screen0);
		XPRTFileProp->exec(pos0);*/
	}
}

void OrientationProject::myTreeDoubleClicked(QModelIndex index)
{
	int indrowProj = index.parent().parent().row();
	int indrowPart = index.parent().row();
	int indrowATR = index.parent().parent().row();
	int indrowCurrent = index.row();
	SelectedARid = index.parent().row();

	int indrow = index.parent().parent().row();

	//if (indrowProj == 0 /*&& indrowATR == 1 && indrowCurrent == 1*/)
	//{
	//	aDoc->Shapeid = indrowPart;
	//	//onShowthisResponse(SelectedARid);
	//}

	//else if (index.parent().row() == 1 && !index.parent().parent().isValid())
	//{
	//	SelectedARid = index.row();
	//	onActionRuleSettings();
	//}

}


//3D Viewer
View* OrientationProject::getViewer()
{
	return myView;
}

void OrientationProject::ActivatetheGrid()
{
	aDoc->PlatformOn();
	aDoc->PlatformVisible = true;
}

void OrientationProject::DeActivatetheGrid()
{
	aDoc->PlatformOff();
	aDoc->PlatformVisible = false;
}

void OrientationProject::getViewClicked()
{
	//aDoc->SomePartisPressed();
	//aDoc->getContext()->InitSelected();

	if (EntitySelectionMode/* && aDoc->getContext()->MoreSelected()*/)
	{
		QTimer::singleShot(150, this, SLOT(executeSelectionMode()));
	}

	else
	{
		QTimer::singleShot(150, this, SLOT(executeSelectionMode()));

	}
}

void OrientationProject::executeSelectionMode()
{
	bool eq = false;

	if (EntitySelectionMode)
	{
		if (aDoc->getListOfShapes()[aDoc->Shapeid]->isOneFaceSelected())
		{
			//addFaceTo();
			onSurfClickedProp();

		}

		else
		{
			//aDoc->getContext()->EraseSelected(true);
			aDoc->getContext()->UnhilightSelected(true);
		}
	}

	else
	{
		bool b = false;
		for (int i = 0; i < aDoc->getListOfShapes().size(); i++)
		{
			if (aDoc->getListOfShapes()[i]->isSelected())
			{
				aDoc->Shapeid = i;
				b = true;
				break;
			}
		}

		if (b)
		{
			viewPtBProp();
		}
		
	}
}

QString OrientationProject::getResourceDirectory()
{
	return QString(":/Coffa_Project/Resources/");
}


//Saving & Opening
void OrientationProject::onSaveProject()
{
	QString fileName;
	if (!hasNameAndPath)
	{
		FilePath = QFileDialog::getSaveFileName(this, tr("Save Project"), "", tr(" COF (*.cof);"));

		if (!FilePath.isNull())
		{
			QFileInfo myFileInfo(FilePath);
			QString aSuffix = myFileInfo.suffix();
			BaseName = myFileInfo.baseName();
			hasNameAndPath = true;
			itemProject->setText(BaseName);
			setWindowTitle(BaseName);
			
		}
	}

	if (hasNameAndPath)
	{
		theNotifier = new NotificationWidget("Saving Project...");
		theNotifier->setParent(this);
		theNotifier->move(this->width() - 370, this->height() - 170);
		theNotifier->show();
		QApplication::processEvents();

		fileName = FilePath;

		QJsonObject Project_obj;
		//
		if (!fileName.isNull())
		{
			QByteArray ba = fileName.toLatin1();
			const char* pFile3 = ba.data();
			if (aDoc->getListOfShapes().size() != 0)
			{
				QJsonArray geometriesArray;
				for (int i = 0; i < aDoc->getListOfShapes().size(); i++)
				{
					aDoc->Shapeid = i;
					aDoc->SaveProject(pFile3);

					QFile thisFile(fileName);
					thisFile.open(QIODevice::ReadWrite | QIODevice::Text);

					QJsonObject aGeom;
					aGeom.insert("name", aDoc->getListOfShapes()[i]->getName());
					aGeom.insert("brep", QString::fromStdString(thisFile.readAll().toStdString()));

					QJsonArray geomResults;
					for each (VarARValues varAR in aDoc->getListOfShapes()[aDoc->Shapeid]->theComputedValues)
					{
						QJsonObject aResult;
						aResult.insert("id", varAR.getID());

						QJsonArray attribArray;
						QList<int> atItems = aDoc->getListOfShapes()[aDoc->Shapeid]->getAttributedSurfaces(varAR.getID()).items;
						for (int m = 0; m < atItems.size(); m++)
						{
							attribArray.append(atItems[m]);
						}
						aResult.insert("attributedFaces", attribArray);

						QJsonArray valArray;
						for (int n = 0; n < varAR.theData.size(); n++)
						{
							QJsonObject aVal;
							aVal.insert("Rx", varAR.theData[n].Rx);
							aVal.insert("Ry", varAR.theData[n].Ry);
							aVal.insert("Rz", varAR.theData[n].Rz);
							aVal.insert("iterator1", varAR.theData[n].iterator1);
							aVal.insert("iterator2", varAR.theData[n].iterator2);
							aVal.insert("globalValue", varAR.theData[n].globalValue);

							valArray.append(aVal);
						}
						aResult.insert("values", valArray);
						geomResults.append(aResult);
					}
					aGeom.insert("computation", geomResults);

					geometriesArray.append(aGeom);
				}

				Project_obj.insert("Geometries", geometriesArray);
			}

			if (ExpertFileExist)
			{
				theNotifier->setMessage("Bundling Action Rules...");
				QJsonArray ARsArray;
				for (int i = 0; i < theRules.size(); i++)
				{
					QJsonObject ar;
					ar.insert("name", theRules[i]->getName());
					ar.insert("label", theRules[i]->getLabel());
					ar.insert("rule", theRules[i]->getRule());
					ar.insert("action", theRules[i]->getActionType());
					ar.insert("id", theRules[i]->getID());
					ar.insert("attribute", theRules[i]->getAttribute());
					ar.insert("color", theRules[i]->getColor());
					ar.insert("shape", theRules[i]->getCadType());
					ar.insert("variable", theRules[i]->getVarType());
					ar.insert("composed", theRules[i]->isComposed());
					ar.insert("relative", theRules[i]->isRelative());

					QJsonArray posArrray;
					for (int n = 0; n < theRules[i]->getPoints().size(); n++)
					{
						QJsonObject myPoints;
						myPoints.insert("x", QJsonValue::fromVariant(theRules[i]->getPoints()[n].x()));
						myPoints.insert("y", QJsonValue::fromVariant(theRules[i]->getPoints()[n].y()));
						posArrray.push_back(myPoints);
					}
					ar.insert("points", posArrray);


					if (theRules[i]->isComposed())
					{
						ar.insert("shape2", theRules[i]->getCadType2());
						ar.insert("variable2", theRules[i]->getVarType2());

						QJsonArray posArrray2;
						for (int n = 0; n < theRules[i]->getPoints2().size(); n++)
						{
							QJsonObject myPoints;
							myPoints.insert("x", QJsonValue::fromVariant(theRules[i]->getPoints2()[n].x()));
							myPoints.insert("y", QJsonValue::fromVariant(theRules[i]->getPoints2()[n].y()));
							posArrray2.push_back(myPoints);
						}
						ar.insert("points2", posArrray2);
					}

					ARsArray.append(ar);
				}

				Project_obj.insert("ActionRules", ARsArray);

			}

			QJsonObject settingElements;
			settingElements.insert("angleStep1", theStep1);
			settingElements.insert("angleStep2", theStep2);
			settingElements.insert("rotationSequence", theSequence);
			settingElements.insert("printerHeight", aDoc->PrinterZ);
			settingElements.insert("printerWidth", aDoc->PrinterX);
			settingElements.insert("printerDepth", aDoc->PrinterY);

			Project_obj.insert("ProjectSetting", settingElements);

			QFile thisFile(fileName);
			thisFile.open(QIODevice::ReadWrite | QIODevice::Text);
			thisFile.resize(0);

			QJsonDocument json_doc;
			json_doc.setObject(Project_obj);
			thisFile.write(json_doc.toJson());

			theNotifier->setMessage("Saved !");
			QTimer::singleShot(2500, theNotifier, SLOT(close()));
		}

		else
		{
			theNotifier->setMessage("Saving Failed !");
			QTimer::singleShot(2500, theNotifier, SLOT(close()));
		}
	}
	//statusBar()->showMessage(QObject::tr("Ready"), 5000);
}

void OrientationProject::ReadProject(QString aProj)
{
	if (!aProj.isNull())
	{
		hasNameAndPath = true;
		//statusBar()->showMessage(QObject::tr("Opening Project... Please Wait"));
		QFile thisFile(aProj);
		thisFile.open(QIODevice::ReadOnly | QIODevice::Text);
		QString fileContent = thisFile.readAll();
		thisFile.close();

		QJsonDocument doc = QJsonDocument::fromJson(fileContent.toUtf8());
		QJsonObject Project_obj = doc.object();

		QJsonArray listRules = Project_obj.value(QString("ActionRules")).toArray();
		QJsonArray listGeom = Project_obj.value(QString("Geometries")).toArray();
		QJsonObject listSetting = Project_obj.value(QString("ProjectSetting")).toObject();

		onLoadExpertFileJSON(listRules);

		theStep1 = listSetting.value(QString("angleStep1")).toInt();
		theStep2 = listSetting.value(QString("angleStep2")).toInt();
		onRestoreAxes();

		theSequence = listSetting.value(QString("rotationSequence")).toInt();
		aDoc->PrinterZ = listSetting.value(QString("printerHeight")).toDouble();
		aDoc->PrinterX = listSetting.value(QString("printerWidth")).toDouble();
		aDoc->PrinterY = listSetting.value(QString("printerDepth")).toDouble();
		heiSpin->setValue(aDoc->PrinterZ);
		widSpin->setValue(aDoc->PrinterX);
		depSpin->setValue(aDoc->PrinterY);

		for each (const QJsonValue & val in listGeom)
		{
			QString geomName = val.toObject().value("name").toString();
			QString geomBrep = val.toObject().value("brep").toString();

			QString brepf = FileDirectory + "/" + "BuffBrep.brep";
			QFile myCAD(brepf);
			myCAD.open(QIODevice::ReadWrite | QIODevice::Text);
			QTextStream BREPFile(&myCAD);

			BREPFile << geomBrep;

			aDoc->AddPart(brepf);

			//FileDirectory.remove(brepf);

			if (!aDoc->getListOfShapes().isEmpty())
			{
				aDoc->getListOfShapes().last()->setName(geomName);
				itemProject->appendRow(aDoc->getListOfShapes().last()->getTreeItem());
				ResultStack->addWidget(aDoc->getListOfShapes().last()->getResultWidget());

				workingPartCombo->addItem(aDoc->getListOfShapes().last()->getName());
				workingPartCombo2->addItem(aDoc->getListOfShapes().last()->getName());
				workingPartCombo3->addItem(aDoc->getListOfShapes().last()->getName());

				ActivateButtons();

				QJsonArray geomResults = val.toObject().value("computation").toArray();
				for each (const QJsonValue & val2 in geomResults)
				{
					QString arID = val2.toObject().value("id").toString();
					QJsonArray atItems = val2.toObject().value("attributedFaces").toArray();
					QJsonArray compVals = val2.toObject().value("values").toArray();

					VarAttribute atVar;
					atVar.isDisplayed = false;
					atVar.id = arID;
					QList<int> its;
					for each (const QJsonValue & it in atItems)
					{
						its.push_back(it.toInt());
					}
					atVar.items = its;
					aDoc->getListOfShapes().last()->setAttributedSurfaces(atVar);

					QList<VarARData> desiValues;
					bool hasNoData = true;
					for each (const QJsonValue & orData in compVals)
					{
						VarARData aData;

						aData.globalValue = orData.toObject().value("globalValue").toDouble();
						aData.iterator1 = orData.toObject().value("iterator1").toInt();
						aData.iterator2 = orData.toObject().value("iterator2").toInt();
						aData.Rx = orData.toObject().value("Rx").toDouble();
						aData.Ry = orData.toObject().value("Ry").toDouble();
						aData.Rz = orData.toObject().value("Rz").toDouble();

						desiValues.push_back(aData);
						hasNoData = false;
					}

					for (int p = 0; p < aDoc->getListOfShapes().last()->theComputedValues.size(); p++)
					{
						if (aDoc->getListOfShapes().last()->theComputedValues[p].getID() == arID)
						{
							aDoc->getListOfShapes().last()->theComputedValues[p].setData(desiValues);
							aDoc->getListOfShapes().last()->theComputedValues[p].setRelativeCalculated(true);
							aDoc->getListOfShapes().last()->theComputedValues[p].getResponse()->shouldUpdate = true;
							aDoc->getListOfShapes().last()->theComputedValues[p].setRange1(180);
							aDoc->getListOfShapes().last()->theComputedValues[p].setStep1(theStep1);
							aDoc->getListOfShapes().last()->theComputedValues[p].setRange2(360);
							aDoc->getListOfShapes().last()->theComputedValues[p].setStep2(theStep2);
							aDoc->getListOfShapes().last()->theComputedValues[p].updateResponse();

							//aDoc->getListOfShapes().last()->theComputedValues[p].updateResponse();
							break;
						}
					}
				}
			}
		}

		//aDoc->drawResponseSurfaces(theSequence, theStep1, theStep2);

		if (!aDoc->getListOfShapes().isEmpty())
		{
			for (int i = 0; i < aDoc->getListOfShapes().size(); i++)
			{
				aDoc->getListOfShapes()[i]->hideAllAttributedSurfaces();
			}
		}
	}
}

void OrientationProject::onExport()
{
	const char* aFile3;
	bool eq = false;
	for (int i = 0; i < ListofExportCH.size(); i++)
	{
		if (ListofExportCH[i]->isChecked())
		{
			eq = true;
		}
	}


	if (eq == true)
	{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Export File"), "", tr(" STL (*.stl);; STEP (*.step);;BREP (*.brep)"));
		QFileInfo myFileInfo(fileName);
		QString aSuffix = myFileInfo.suffix();
		BaseName = myFileInfo.baseName();

		if (!fileName.isNull())
		{
			theNotifier = new NotificationWidget("Exporting Part...");
			theNotifier->setParent(this);
			//theNotifier->setWindowModality(Qt::NonModal);
			theNotifier->move(this->width() - 370, this->height() - 170);
			theNotifier->show();
			QApplication::processEvents();
			for (int i = 0; i < ListofExportCH.size(); i++)
			{
				if (ListofExportCH[i]->isChecked())
				{
					if (aSuffix == "stl")
					{
						QByteArray ba = fileName.toLatin1();
						aFile3 = ba.data();
						aDoc->ExportFileSTL(aFile3, i);
					}

					else if (aSuffix == "step")
					{
						QByteArray ba = fileName.toLatin1();
						aFile3 = ba.data();
						aDoc->ExportFileSTEP(aFile3, i);
					}

					else if (aSuffix == "brep")
					{
						QByteArray ba = fileName.toLatin1();
						aFile3 = ba.data();
						aDoc->WriteBRepFile(aFile3, i);
					}
				}
			}
			ExportDialog->close();
		}
		theNotifier->setMessage("Exportation Done !");
		QTimer::singleShot(2500, theNotifier, SLOT(close()));

	}

	else
	{
		QToolTip::showText(goExportButton->mapToGlobal(QPoint(0, 0)), "Please select one part to export");
	}
}

void OrientationProject::onExportDialog()
{
	ExportDialog = new QDialog(this);
	ExportDialog->setFixedSize(400, 350);
	ExportDialog->show();

	QWidget* awid = new QWidget(ExportDialog);
	awid->move(10, 10);
	awid->setFixedSize(380, 250);
	QVBoxLayout* avl = new QVBoxLayout();
	awid->setLayout(avl);
	awid->show();

	goExportButton = new QPushButton("Export", ExportDialog);
	goExportButton->setFixedSize(100, 30);
	goExportButton->move(250, 300);
	goExportButton->show();
	connect(goExportButton, SIGNAL(released()), this, SLOT(onExport()));

	ListofExportCH.clear();
	if (!aDoc->getListOfShapes().isEmpty())
	{
		QButtonGroup* aGC = new QButtonGroup();
		for (int i = 0; i < aDoc->getListOfShapes().size(); i++)
		{
			QCheckBox* aCheck = new QCheckBox(aDoc->getListOfShapes()[i]->getName());
			if (aDoc->Shapeid == i)
			{
				aCheck->setChecked(1);
			}

			aGC->addButton(aCheck);
			ListofExportCH.push_back(aCheck);
			avl->addWidget(aCheck);
			aCheck->show();
		}

	}

	else if (aDoc->getListOfShapes().isEmpty())
	{
		QLabel* aLab = new QLabel("Message:\nThere is no Geometry to Export\nPlease use Load Part Button to add", ExportDialog);
		aLab->move(10, 20);
		aLab->setFixedSize(380, 60);
		aLab->setStyleSheet("QLabel {background:#ffcf7f; color: #4b4b4b}");
		aLab->show();
		goExportButton->setDisabled(1);
	}

}

//Part Tools
void OrientationProject::viewPtBProp()
{
	QScreen* screen0 = QApplication::screens().at(0);
	QPoint pos0 = QCursor::pos(screen0);
	ParttoBuildProp->exec(pos0);
}

void OrientationProject::onAddPartToPlate()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("All Files(*.step *.stp *.stl *.brep);; STL (*.stl);; STEP (*.step);; STP (*.stp);;BREP (*.brep)"));

	if (!fileName.isEmpty())
	{
		theNotifier = new NotificationWidget("Reading Shape Data...");
		theNotifier->setParent(this);
		theNotifier->move(this->width() - 370, this->height() - 170);
		theNotifier->show();
		QApplication::processEvents();
		aDoc->AddPart(fileName);

		itemProject->appendRow(aDoc->getListOfShapes().last()->getTreeItem());

		theNotifier->setMessage("Initializing Data Binders...");
		workingPartCombo->addItem(aDoc->getListOfShapes().last()->getName());
		workingPartCombo2->addItem(aDoc->getListOfShapes().last()->getName());
		workingPartCombo3->addItem(aDoc->getListOfShapes().last()->getName());

		ResultStack->addWidget(aDoc->getListOfShapes().last()->getResultWidget());

		ActivateButtons();

		theNotifier->close();
		//statusBar()->showMessage(QObject::tr("Ready"));

	}
}

void OrientationProject::onRemovePartFromPlate()
{	
	theNotifier = new NotificationWidget("Removing Shape...");
	theNotifier->setWindowModality(Qt::NonModal);
	theNotifier->setParent(this);
	theNotifier->move(this->width() - 370, this->height() - 170);
	theNotifier->show();
	QApplication::processEvents();

	ResultStack->removeWidget(aDoc->getListOfShapes().at(aDoc->Shapeid)->getResultWidget());
	aDoc->RemovePart();
	itemProject->removeRow(aDoc->Shapeid);
	ParttoBuildProp->hide();
	ParttoTreeProp->hide();

	workingPartCombo->clear();
	workingPartCombo2->clear();
	workingPartCombo3->clear();

	if (!aDoc->getListOfShapes().isEmpty())
	{
		for (int i = 0; i < aDoc->getListOfShapes().size(); i++)
		{
			workingPartCombo->addItem(aDoc->getListOfShapes().at(i)->getName());
			workingPartCombo2->addItem(aDoc->getListOfShapes().at(i)->getName());
			workingPartCombo3->addItem(aDoc->getListOfShapes().at(i)->getName());
		}
	}

	theNotifier->setMessage("Successfully Removed");
	QTimer::singleShot(2500, theNotifier, SLOT(close()));
}

void OrientationProject::onUpdateSelectedPart()
{
	int cur = workingPartCombo->currentIndex();

	if (cur < workingPartCombo2->count())
		workingPartCombo2->setCurrentIndex(cur);

	if (cur < workingPartCombo3->count())
		workingPartCombo3->setCurrentIndex(cur);

	if (cur >= 0 && cur < aDoc->getListOfShapes().size())
	{
		aDoc->Shapeid = cur;

		if (SingleRespWidget->isVisible())
		{
			generateResponses();
			//QApplication::processEvents();
		}
	}
}

void OrientationProject::onUpdateSelectedPart2()
{
	int cur = workingPartCombo2->currentIndex();
	if (cur < workingPartCombo->count())
		workingPartCombo->setCurrentIndex(cur);

	if (cur < workingPartCombo3->count())
		workingPartCombo3->setCurrentIndex(cur);

	if (cur >= 0 && cur < aDoc->getListOfShapes().size())
	{
		aDoc->Shapeid = cur;

		if (SingleRespWidget->isVisible())
		{
			generateResponses();
			//QApplication::processEvents();
		}
	}
}

void OrientationProject::onUpdateSelectedPart3()
{
	int cur = workingPartCombo3->currentIndex();
	if (cur < workingPartCombo->count())
		workingPartCombo->setCurrentIndex(cur);

	if (cur < workingPartCombo2->count())
		workingPartCombo2->setCurrentIndex(cur);

	if (cur >= 0 && cur < aDoc->getListOfShapes().size())
	{
		aDoc->Shapeid = cur;

		if (SingleRespWidget->isVisible())
		{
			generateResponses();
			//QApplication::processEvents();
		}
	}
}

void OrientationProject::onMakeAttributeDialog()
{
	QDialog* aDialog = new QDialog(this);
	aDialog->setFixedSize(300, 350);
	QFormLayout* aLay = new QFormLayout;
	aDialog->setLayout(aLay);

	connect(aDialog, SIGNAL(lastWindowClosed()), this, SLOT(onSaveFaceSelection()));

	attribTitle = new QLabel("Select " + theRules[SelectedARid]->getAttribute() + " items", aDialog);
	attribTitle->setFixedSize(280, 30);
	attribTitle->move(10, 5);
	QFont aFont;
	aFont.setPointSize(9);
	attribTitle->setFont(aFont);

	attribModel = new QStringListModel();
	attribVar = aDoc->getListOfShapes()[aDoc->Shapeid]->getAttributedSurfaces(theRules[SelectedARid]->getID());
	QStringList myList;
	for (int i = 0; i < attribVar.items.size(); i++)
	{
		myList.push_back(aDoc->getListOfShapes()[aDoc->Shapeid]->getSurfaces()[attribVar.items[i]]->getName());
	}
	attribModel->setStringList(myList);

	attribViewList = new QListView(aDialog);
	attribViewList->setFixedHeight(200);
	attribViewList->setFixedWidth(280);
	attribViewList->move(10, 40);
	attribViewList->setModel(attribModel);

	QPushButton* AddFButton = new QPushButton("Add", aDialog);
	AddFButton->setFixedSize(80, 30);
	AddFButton->move(55, 250);
	AddFButton->show();
	connect(AddFButton, SIGNAL(released()), this, SLOT(addFaceTo()));

	QPushButton* AddAllFButton = new QPushButton("Add All", aDialog);
	AddAllFButton->setFixedSize(80, 30);
	AddAllFButton->move(55, 290);
	AddAllFButton->show();
	connect(AddAllFButton, SIGNAL(released()), this, SLOT(addAllFaceTo()));

	QPushButton* RemFButton = new QPushButton("Remove", aDialog);
	RemFButton->setFixedSize(80, 30);
	RemFButton->move(165, 250);
	RemFButton->show();
	connect(RemFButton, SIGNAL(released()), this, SLOT(RemoveFaceFrom()));

	QPushButton* SaveButton = new QPushButton("Save", aDialog);
	SaveButton->setFixedSize(80, 30);
	SaveButton->move(210, 300);
	SaveButton->show();
	//connect(SaveButton, SIGNAL(released()), this, SLOT(onSaveFaceSelection()));
	connect(SaveButton, &QPushButton::released, [=]() {
		aDialog->close();
		onSaveFaceSelection();
	});

	aDialog->show();

	aDoc->SetupSelectionModes();
	EntitySelectionMode = true;
	
}

void OrientationProject::onSaveFaceSelection()
{
	theNotifier = new NotificationWidget("Saving Selection...");
	theNotifier->setWindowModality(Qt::NonModal);
	theNotifier->setParent(this);
	theNotifier->move(this->width() - 370, this->height() - 170);
	theNotifier->show();
	QApplication::processEvents();
	QStringList aList = attribModel->stringList();
	//qDebug()<<aList.size();
	/*if (aList.size() != 0)
	{*/
		QList<int> selectedItems;
		for (int i = 0; i < aList.size(); i++)
		{
			for (int j = 0; j < aDoc->getListOfShapes()[aDoc->Shapeid]->theSurfaces.size(); j++)
			{
				if (aList[i] == aDoc->getListOfShapes()[aDoc->Shapeid]->theSurfaces[j]->getName())
				{
					selectedItems.push_back(j);
					break;
				}
			}
		}
		//qDebug()<<attribVar.id;
		attribVar.items = selectedItems;
		aDoc->getListOfShapes()[aDoc->Shapeid]->setAttributedSurfaces(attribVar);
		//aDoc->getListOfShapes()[aDoc->Shapeid]->showAttributedSurfaces(attribVar.id);
	//}
	aDoc->RestoreSelectionModes();

	EntitySelectionMode = false;
	onBackHome();

	theNotifier->setMessage("Ready !");
	QTimer::singleShot(2000, theNotifier, SLOT(close()));
}

void OrientationProject::onCancelFaceSelection()
{
	QMessageBox msgBox(this);
	msgBox.setText("You are going to discard the modifications.\n");
	msgBox.setInformativeText("Do you want to proceed?");
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::Yes);
	msgBox.setIcon(QMessageBox::Warning);
	connect(msgBox.button(QMessageBox::Yes), &QAbstractButton::pressed, [=]() {
		projectStack->setCurrentWidget(TreeWidget);
	});

	connect(msgBox.button(QMessageBox::No), &QAbstractButton::pressed, [=]() {
		return;
	});
	msgBox.show();
	msgBox.exec();
}

void OrientationProject::addFaceTo()
{
	if (!aDoc->getListOfShapes().isEmpty())
	{
		QList<int> iList = aDoc->getListOfShapes()[aDoc->Shapeid]->getSelectedSurfaces();

		int theId;
		QStringList newList = attribModel->stringList();
		yesToAll = false;
		for (int ii = 0; ii < iList.size(); ii++)
		{
			bool alreadyExist = false;
			for (int j = 0; j < newList.size(); j++)
			{
				if (newList[j] == aDoc->getListOfShapes()[aDoc->Shapeid]->getSurfaces()[iList[ii]]->getName())
				{
					alreadyExist = true;
					break;
				}
			}

			if (alreadyExist)
			{
				if (!yesToAll)
				{
					QMessageBox msgBox(this);
					msgBox.setText("The " + aDoc->getListOfShapes()[aDoc->Shapeid]->getSurfaces()[iList[ii]]->getName() + " has already been added.");
					msgBox.setInformativeText("Press Yes to all to hide this box for the other occurences");
					msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::YesToAll);
					msgBox.setDefaultButton(QMessageBox::Yes);
					msgBox.setIcon(QMessageBox::Warning);
					connect(msgBox.button(QMessageBox::Yes), &QAbstractButton::pressed, [=]() {
						//projectStack->setCurrentWidget(TreeWidget);
					});

					connect(msgBox.button(QMessageBox::YesToAll), &QAbstractButton::pressed, [=]() {
						yesToAll = true;
						return;
					});
					msgBox.show();
					msgBox.exec();
				}
			}

			else
			{
				newList.push_back(aDoc->getListOfShapes()[aDoc->Shapeid]->getSurfaces()[iList[ii]]->getName());
			}
		}

		attribModel->setStringList(newList);
		
		aDoc->getContext()->UnhilightSelected(true);
		aDoc->getContext()->InitSelected();
		SurfClickProperties->close();
		
		numOfFace = newList.size();
		attribTitle->setText("Select " + theRules[SelectedARid]->getAttribute() + " items (" + QString::number(numOfFace) + ")");

		//aDoc->RestoreSelectionModes();
	}

}

void OrientationProject::addAllFaceTo()
{
	if (!aDoc->getListOfShapes().isEmpty())
	{
		QList<int> iList;
		for (int i = 0; i < aDoc->getListOfShapes()[aDoc->Shapeid]->getSurfaces().size(); i++)
		{
			iList.push_back(i);
		}

		int theId;
		QStringList newList = attribModel->stringList();

		if (newList.isEmpty())
		{
			for (int ii = 0; ii < iList.size(); ii++)
			{
				newList.push_back(aDoc->getListOfShapes()[aDoc->Shapeid]->getSurfaces()[iList[ii]]->getName());
			}
			attribModel->setStringList(newList);
		}

		else if (newList.size()== aDoc->getListOfShapes()[aDoc->Shapeid]->getSurfaces().size())
		{
			QMessageBox::warning(this, QObject::tr("Duplicate!!"), "All surfaces have already been added.",
				QObject::tr("OK"), QString::null, QString::null, 0, 0);

		}

		else
		{
			yesToAll = false;
			for (int ii = 0; ii < iList.size(); ii++)
			{
				bool alreadyExist = false;
				for (int j = 0; j < newList.size(); j++)
				{
					if (newList[j] == aDoc->getListOfShapes()[aDoc->Shapeid]->getSurfaces()[iList[ii]]->getName())
					{
						alreadyExist = true;
						break;
					}
				}

				if (alreadyExist)
				{
					if (!yesToAll)
					{
						QMessageBox msgBox(this);
						msgBox.setText("The " + aDoc->getListOfShapes()[aDoc->Shapeid]->getSurfaces()[iList[ii]]->getName() + " has already been added.");
						msgBox.setInformativeText("Press Yes to all to hide this box for the other occurences");
						msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::YesToAll);
						msgBox.setDefaultButton(QMessageBox::Yes);
						msgBox.setIcon(QMessageBox::Warning);
						connect(msgBox.button(QMessageBox::Yes), &QAbstractButton::pressed, [=]() {
							//projectStack->setCurrentWidget(TreeWidget);
						});

						connect(msgBox.button(QMessageBox::YesToAll), &QAbstractButton::pressed, [=]() {
							yesToAll = true;
							return;
						});
						msgBox.show();
						msgBox.exec();
					}
				}

				else
				{
					newList.push_back(aDoc->getListOfShapes()[aDoc->Shapeid]->getSurfaces()[iList[ii]]->getName());
				}
			}
			attribModel->setStringList(newList);
		}	

		aDoc->getContext()->UnhilightSelected(true);
		SurfClickProperties->close();

		numOfFace = newList.size();
		attribTitle->setText("Select " + theRules[SelectedARid]->getAttribute() + " items (" + QString::number(numOfFace) + ")");

		//aDoc->RestoreSelectionModes();
	}
}

void OrientationProject::RemoveFaceFrom()
{
	//Allows Removing a Face from the List
	if (!aDoc->getListOfShapes().isEmpty())
	{
		if (!attribViewList->currentIndex().isValid())
		{
			QMessageBox::warning(this, QObject::tr("No Selection!!"), QObject::tr("Please select one item to remove"),
				QObject::tr("OK"), QString::null, QString::null, 0, 0);
			return;
		}
		//aDoc->getListOfShapes()[aDoc->Shapeid]->getSurfaces()
		QModelIndex myIndex1 = attribViewList->currentIndex();
		int ind = myIndex1.row();
		QStringList newList = attribModel->stringList();
		for (int i = 0; i < newList.size(); i++)
		{
			if (myIndex1.data() == newList[i])
			{
				newList.removeAt(i);
				break;
			}
		}

		attribModel->setStringList(newList);

		numOfFace = newList.size();
		attribTitle->setText("Select " + theRules[SelectedARid]->getAttribute() + " items (" + QString::number(numOfFace)+")");

	}

}

void OrientationProject::RemoveFaceFrom2()
{
	//Allows Removing a Face by the Viewer
	if (!aDoc->getListOfShapes().isEmpty())
	{
		QList<int> iList = aDoc->getListOfShapes()[aDoc->Shapeid]->getSelectedSurfaces();
		QStringList newList = attribModel->stringList();

		for (int ii = 0; ii < iList.size(); ii++)
		{
			bool itexist = false;
			for (int j = 0; j < newList.size(); j++)
			{
				if (newList[j] == aDoc->getListOfShapes()[aDoc->Shapeid]->getSurfaces()[iList[ii]]->getName())
				{
					itexist = true;
					newList.removeAt(j);
					break;
				}
			}

			if (itexist == false)
			{				
				QMessageBox msgBox(this);
				msgBox.setText("The " + aDoc->getListOfShapes()[aDoc->Shapeid]->getSurfaces()[iList[ii]]->getName() + " does not exist in the current list");
				msgBox.setStandardButtons(QMessageBox::Ok);
				msgBox.setDefaultButton(QMessageBox::Ok);
				msgBox.setIcon(QMessageBox::Warning);
				//connect(msgBox.button(QMessageBox::Ok), &QAbstractButton::pressed, [=]() {
				//	//projectStack->setCurrentWidget(TreeWidget);
				//});

				//connect(msgBox.button(QMessageBox::YesToAll), &QAbstractButton::pressed, [=]() {
				//	yesToAll = true;
				//	return;
				//});
				msgBox.show();
				msgBox.exec();
			}

		}
		attribModel->setStringList(newList);
		aDoc->getContext()->UnhilightSelected(true);
		SurfClickProperties->close();

		numOfFace = newList.size();
		attribTitle->setText("Select " + theRules[SelectedARid]->getAttribute() + " items (" + QString::number(numOfFace)+")");

	}

}

void OrientationProject::RemoveAllFaceFrom()
{
	if (!aDoc->getListOfShapes().isEmpty())
	{
		QList<int> iList;
		for (int i = 0; i < aDoc->getListOfShapes()[aDoc->Shapeid]->getSurfaces().size(); i++)
		{
			iList.push_back(i);
		}

		int theId;
		QStringList newList = attribModel->stringList();

		if (!newList.isEmpty())
		{
			for (int ii = 0; ii < iList.size(); ii++)
			{
				for (int j = 0; j < newList.size(); j++)
				{
					if (newList[j] == aDoc->getListOfShapes()[aDoc->Shapeid]->getSurfaces()[iList[ii]]->getName())
					{
						newList.removeAt(j);
						break;
					}
				}
			}
			attribModel->setStringList(newList);
		}

		else if (newList.isEmpty())
		{
			QMessageBox::warning(this, QObject::tr("Empty!!"), "The selection list is empty.",
				QObject::tr("OK"), QString::null, QString::null, 0, 0);

		}

		aDoc->getContext()->UnhilightSelected(true);
		SurfClickProperties->close();

		numOfFace = newList.size();
		attribTitle->setText("Select " + theRules[SelectedARid]->getAttribute() + " items (" + QString::number(numOfFace) + ")");

		//aDoc->RestoreSelectionModes();
	}
}

void OrientationProject::createRotationDialog()
{
	RotDialog = new QDialog(this);
	RotDialog->setParent(this);
	RotDialog->setWindowTitle("Rotate");
	RotDialog->setFixedSize(300, 170);
	QFormLayout* RotForm = new QFormLayout();
	RotDialog->setLayout(RotForm);

	RxSpin = new QDoubleSpinBox();
	RxSpin->setAlignment(Qt::AlignRight);
	RxSpin->setMaximum(360);
	RxSpin->setSuffix(".deg");
	RxSpin->setValue(Rx);
	//RxSpin->setStyleSheet("QDoubleSpinBox{ background: #696969 ; color: white}");
	connect(RxSpin, SIGNAL(valueChanged(double)), SLOT(setValue(int)));
	RotForm->addRow(tr("&X Axis:"), RxSpin);

	RySpin = new QDoubleSpinBox();
	RySpin->setAlignment(Qt::AlignRight);
	RySpin->setMaximum(360);
	RySpin->setSuffix(".deg");
	RySpin->setValue(Ry);
	//RySpin->setStyleSheet("QDoubleSpinBox{ background: #696969 ; color: white}");
	connect(RySpin, SIGNAL(valueChanged(double)), SLOT(setValue(int)));
	RotForm->addRow(tr("&Y Axis:"), RySpin);

	RzSpin = new QDoubleSpinBox();
	RzSpin->setAlignment(Qt::AlignRight);
	RzSpin->setMaximum(360);
	RzSpin->setSuffix(".deg");
	RzSpin->setValue(Rz);
	//RzSpin->setStyleSheet("QDoubleSpinBox{ background: #696969 ; color: white}");
	connect(RzSpin, SIGNAL(valueChanged(double)), SLOT(setValue(int)));
	RotForm->addRow(tr("&Z Axis:"), RzSpin);

	OkButtonRot = new QPushButton("OK", RotDialog);
	OkButtonRot->setFixedSize(60, 30);
	OkButtonRot->move(120, 120);
	OkButtonRot->setShortcut(QObject::tr("ENTER"));
	connect(OkButtonRot, SIGNAL(released()), this, SLOT(onRotatePressed()));

	RotDialog->hide();
	//TrlDialog->close();
	//aDoc->getContext()->UnhilightSelected(true);
	//ParttoBuildProp->hide();
}

void OrientationProject::onShowRotateDialog()
{
	RxSpin->setValue(Rx);
	RySpin->setValue(Ry);
	RzSpin->setValue(Rz);
	RotDialog->show();
	ParttoBuildProp->hide();
}

void OrientationProject::onRotatePressed()
{
	Rx = RxSpin->value();
	Ry = RySpin->value();
	Rz = RzSpin->value();

	if (aDoc->getListOfShapes().size() != 0)
	{
		aDoc->rotateCurrentShape(theSequence, Rx, Ry, Rz);
	}
}

void OrientationProject::createTranslationDialog()
{
	TrlDialog = new QDialog(this);
	TrlDialog->setWindowTitle("Move");
	TrlDialog->setFixedSize(300, 170);
	QFormLayout* TrlForm = new QFormLayout();
	TrlDialog->setLayout(TrlForm);

	TxSpin = new QDoubleSpinBox();
	TxSpin->setAlignment(Qt::AlignRight);
	TxSpin->setMaximum(1000);
	TxSpin->setMinimum(-1000);
	TxSpin->setSuffix(".mm");
	//TxSpin->setStyleSheet("QDoubleSpinBox{ background: #696969 ; color: white}");
	connect(TxSpin, SIGNAL(valueChanged(double)), SLOT(setValue(int)));
	TrlForm->addRow(tr("&X Dir:"), TxSpin);

	TySpin = new QDoubleSpinBox();
	TySpin->setAlignment(Qt::AlignRight);
	TySpin->setMaximum(1000);
	TySpin->setMinimum(-1000);
	TySpin->setSuffix(".mm");
	//TySpin->setStyleSheet("QDoubleSpinBox{ background: #696969 ; color: white}");
	connect(TySpin, SIGNAL(valueChanged(double)), SLOT(setValue(int)));
	TrlForm->addRow(tr("&Y Dir:"), TySpin);

	TzSpin = new QDoubleSpinBox();
	TzSpin->setAlignment(Qt::AlignRight);
	TzSpin->setMaximum(1000);
	TzSpin->setMinimum(0);
	TzSpin->setSuffix(".mm");
	//TzSpin->setStyleSheet("QDoubleSpinBox{ background: #696969 ; color: white}");
	connect(TzSpin, SIGNAL(valueChanged(double)), SLOT(setValue(int)));
	TrlForm->addRow(tr("&Z Dir:"), TzSpin);

	OkButtonTrl = new QPushButton("Apply", TrlDialog);
	OkButtonTrl->setFixedSize(60, 30);
	OkButtonTrl->move(120, 120);
	OkButtonTrl->setShortcut(QObject::tr("Apply"));
	connect(OkButtonTrl, SIGNAL(released()), this, SLOT(onTranslatePressed()));

	TrlDialog->hide();
}

void OrientationProject::onShowTranslateDialog()
{
	TrlDialog->show();
	ParttoBuildProp->hide();
	if (!aDoc->getListOfShapes().isEmpty() && aDoc->Shapeid < aDoc->getListOfShapes().size())
	{
		TxSpin->setValue(aDoc->getListOfShapes()[aDoc->Shapeid]->Tx);
		TySpin->setValue(aDoc->getListOfShapes()[aDoc->Shapeid]->Ty);
		TzSpin->setValue(aDoc->getListOfShapes()[aDoc->Shapeid]->Tz);
	}
}

void OrientationProject::onTranslatePressed()
{
	if (aDoc->getListOfShapes().size() != 0)
	{
		aDoc->getListOfShapes()[aDoc->Shapeid]->translatePart(TxSpin->value(), TySpin->value(), TzSpin->value());
	}
}


//Expert File Tools
void OrientationProject::onLoadExpertFileJSON(QJsonArray anArray)
{
	if (!anArray.isEmpty())
	{
		//QJsonObject Project_obj = anObject;

		QJsonArray listRules = anArray;

		theRules.clear();
		ChoiceCheckList.clear();
		WeightSliderList.clear();
		WeightValues.clear();

		if (itemExpertise->rowCount() > 0)
		{
			itemExpertise->removeRows(0, itemExpertise->rowCount());
		}

		foreach(const QJsonValue & val, listRules)
		{
			QString arName = val.toObject().value("name").toString();
			QString arLabel = val.toObject().value("label").toString();
			QString arAttribute = val.toObject().value("attribute").toString();
			QString arId = val.toObject().value("id").toString();
			QString arContent = val.toObject().value("rule").toString();
			QString arColor = val.toObject().value("color").toString();
			int arAction = val.toObject().value("action").toInt();
			int arShape = val.toObject().value("shape").toInt();
			int arVariable = val.toObject().value("variable").toInt();
			bool arCompo = val.toObject().value("composed").toBool();
			bool arRelative = val.toObject().value("relative").toBool();
			QJsonArray arPoints = val.toObject().value("points").toArray();

			ActionRule* aRule = new ActionRule(arContent);
			aRule->setID(arId);
			aRule->setName(arName);
			aRule->setLabel(arLabel);
			aRule->setAttribute(arAttribute);
			aRule->setColor(arColor);
			aRule->setComposed(arCompo);
			aRule->setRelative(arRelative);
			aRule->setActionType(ActionRule::ACTIONType(arAction));
			aRule->setCadType(ActionRule::CADType(arShape));
			aRule->setVarType(ActionRule::VARType(arVariable));

			for (int i = 0; i < arPoints.size(); i++)
			{
				QJsonObject anObj = arPoints[i].toObject();
				QPointF aPoint;
				aPoint.setX(anObj.value("x").toDouble());
				aPoint.setY(anObj.value("y").toDouble());
				aRule->addPoint(aPoint);
			}

			if (aRule->isComposed())
			{
				int arShape2 = val.toObject().value("shape2").toInt();
				int arVariable2 = val.toObject().value("variable2").toInt();
				QJsonArray arPoints2 = val.toObject().value("points2").toArray();
				aRule->setCadType2(ActionRule::CADType(arShape2));
				aRule->setVarType2(ActionRule::VARType(arVariable2));

				QList<QPointF> funcPoints2;
				for (int i = 0; i < arPoints2.size(); i++)
				{
					QJsonObject anObj = arPoints2[i].toObject();
					QPointF aPoint;
					aPoint.setX(anObj.value("x").toDouble());
					aPoint.setY(anObj.value("y").toDouble());
					aRule->addPoint2(aPoint);
				}
			}

			QCheckBox* aCheck = new QCheckBox(arName);
			ChoiceCheckList.append(aCheck);
			//ARSelectionLayout->addWidget(aCheck);
			theRules.push_back(aRule);

			QStandardItem* itemAR = new QStandardItem(aRule->getName());
			itemAR->setEditable(false);
			QColor aColor;
			aColor.setNamedColor(aRule->getColor());
			QVariant colData(aColor);
			itemAR->setData(colData, Qt::DecorationRole);
			itemExpertise->appendRow(itemAR);
		}

		int gridPosx = 1, gridPosy = 0;
		//QList<QString> visuList;
		visuCombo->clear();
		visuCombo2->clear();
		QIcon aVisuicon;
		aVisuicon.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/shapeVisu.png")), QIcon::Normal, QIcon::Off);
		visuCombo->addItem(aVisuicon, "None");
		visuCombo2->addItem(aVisuicon, "None");


		for (int i = 0; i < theRules.size(); i++)
		{
			WeightValues.push_back(0.0);
			QWidget* aWidg = new QWidget;
			aWidg->setFixedSize(160, 50);
			QHBoxLayout* hlay = new QHBoxLayout;
			aWidg->setLayout(hlay);

			QLabel* LabSlid = new QLabel(theRules[i]->getLabel(), aWidg);
			LabSlid->setFixedSize(100, 20);
			LabSlid->move(10, 0);
			//hlay->addWidget(LabSlid, Qt::AlignLeft);

			QLabel* percentLabel = new QLabel("0%", aWidg);
			percentLabel->setFixedSize(40, 20);
			percentLabel->move(130, 20);
			//percentLabel->setAlignment(Qt::AlignCenter);

			QSlider* mySlider = new QSlider(Qt::Horizontal, aWidg);
			mySlider->move(10, 20);
			mySlider->setFixedSize(120, 30);
			mySlider->setTickPosition(QSlider::TicksBelow);
			mySlider->setTickInterval(10);
			mySlider->setMaximum(100);
			mySlider->setMinimum(0);
			mySlider->setValue(0);
			WeightSliderList.append(mySlider);
			//connect(mySlider, SIGNAL(sliderReleased()), this, SLOT(onWeightersChange()));

			QObject::connect(mySlider, &QSlider::valueChanged, this, [=]() {
				percentLabel->setText(QString::number(mySlider->value()) + "%"); onWeightersChange(); });

			if (gridPosy >= 2)
			{
				gridPosx++;
				gridPosy = 0;
				weigthersLayout->addWidget(aWidg, gridPosx, gridPosy++, Qt::AlignCenter);
			}
			else
			{
				weigthersLayout->addWidget(aWidg, gridPosx, gridPosy++, Qt::AlignCenter);
			}

			//Visualization depending on Var
			if (theRules[i]->getVarType() == ActionRule::normalAngle)
			{
				//visuList.push_back(theRules[i]->getLabel());
				visuCombo->addItem(aVisuicon, theRules[i]->getLabel());
				visuCombo2->addItem(aVisuicon, theRules[i]->getLabel());
			}
		}

		for (int i = 0; i < aDoc->getListOfShapes().size(); i++)
		{
			aDoc->getListOfShapes()[i]->setActionRules(theRules);
		}

		//Context Menu
		FeatureProperties = new QMenu(this);
		QAction* action1 = new QAction("Add/Remove Entities", this);
		FeatureProperties->addAction(action1);
		connect(action1, SIGNAL(triggered()), this, SLOT(onShowAttributeWidget()));

		QAction* action2 = new QAction("Display Entities", this);
		FeatureProperties->addAction(action2);
		connect(action2, SIGNAL(triggered()), this, SLOT(onDisplayEntities()));

		QAction* action3 = new QAction("Hide Entities", this);
		FeatureProperties->addAction(action3);
		connect(action3, SIGNAL(triggered()), this, SLOT(onHideEntities()));

		//statusBar()->showMessage(QObject::tr("Ready"), 2000);
		ExpertFileExist = true;

		createARVizualizer();
	}
}

void OrientationProject::onLoadExpertFileJSON()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Import Expert File"), "", tr("All Files (*.json)"));
	QFileInfo aFinfo(fileName);
	XRDFPath = fileName;
	if (!fileName.isNull())
	{
		QFile thisFile(fileName);
		thisFile.open(QIODevice::ReadOnly | QIODevice::Text);
		QString fileContent = thisFile.readAll();
		thisFile.close();

		QJsonDocument doc = QJsonDocument::fromJson(fileContent.toUtf8());
		QJsonObject Project_obj = doc.object();

		QJsonArray listRules = Project_obj.value(QString("ActionRules")).toArray();
		theRules.clear();
		ChoiceCheckList.clear();
		WeightSliderList.clear();
		WeightValues.clear();

		if (itemExpertise->rowCount() > 0)
		{
			itemExpertise->removeRows(0, itemExpertise->rowCount());
		}

		foreach(const QJsonValue & val, listRules)
		{
			QString arName = val.toObject().value("name").toString();
			QString arLabel = val.toObject().value("label").toString();
			QString arAttribute = val.toObject().value("attribute").toString();
			QString arId = val.toObject().value("id").toString();
			QString arContent = val.toObject().value("rule").toString();
			QString arColor = val.toObject().value("color").toString();
			int arAction = val.toObject().value("action").toInt();
			int arShape = val.toObject().value("shape").toInt();
			int arVariable = val.toObject().value("variable").toInt();
			bool arCompo = val.toObject().value("composed").toBool();
			bool arRelative = val.toObject().value("relative").toBool();
			QJsonArray arPoints = val.toObject().value("points").toArray();

			ActionRule* aRule = new ActionRule(arContent);
			aRule->setID(arId);
			aRule->setName(arName);
			aRule->setLabel(arLabel);
			aRule->setAttribute(arAttribute);
			aRule->setColor(arColor);
			aRule->setComposed(arCompo);
			aRule->setRelative(arRelative);
			aRule->setActionType(ActionRule::ACTIONType(arAction));
			aRule->setCadType(ActionRule::CADType(arShape));
			aRule->setVarType(ActionRule::VARType(arVariable));

			for (int i = 0; i < arPoints.size(); i++)
			{
				QJsonObject anObj = arPoints[i].toObject();
				QPointF aPoint;
				aPoint.setX(anObj.value("x").toDouble());
				aPoint.setY(anObj.value("y").toDouble());
				aRule->addPoint(aPoint);
			}

			if (aRule->isComposed())
			{
				int arShape2 = val.toObject().value("shape2").toInt();
				int arVariable2 = val.toObject().value("variable2").toInt();
				QJsonArray arPoints2 = val.toObject().value("points2").toArray();
				aRule->setCadType2(ActionRule::CADType(arShape2));
				aRule->setVarType2(ActionRule::VARType(arVariable2));

				QList<QPointF> funcPoints2;
				for (int i = 0; i < arPoints2.size(); i++)
				{
					QJsonObject anObj = arPoints2[i].toObject();
					QPointF aPoint;
					aPoint.setX(anObj.value("x").toDouble());
					aPoint.setY(anObj.value("y").toDouble());
					aRule->addPoint2(aPoint);
				}
			}

			QCheckBox* aCheck = new QCheckBox(arName);
			ChoiceCheckList.append(aCheck);
			//ARSelectionLayout->addWidget(aCheck);
			theRules.push_back(aRule);

			QStandardItem* itemAR = new QStandardItem(aRule->getName());
			itemAR->setEditable(false);
			QColor aColor;
			aColor.setNamedColor(aRule->getColor());
			QVariant colData(aColor);
			itemAR->setData(colData, Qt::DecorationRole);
			itemExpertise->appendRow(itemAR);
		}

		int gridPosx = 1, gridPosy = 0;
		visuCombo->clear();
		visuCombo2->clear();
		QIcon aVisuicon;
		aVisuicon.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/shapeVisu.png")), QIcon::Normal, QIcon::Off);
		visuCombo->addItem(aVisuicon, "None");
		visuCombo2->addItem(aVisuicon, "None");

		for (int i = 0; i < theRules.size(); i++)
		{

			WeightValues.push_back(0.0);

			QWidget* aWidg = new QWidget;
			aWidg->setFixedSize(160, 50);
			QHBoxLayout* hlay = new QHBoxLayout;
			aWidg->setLayout(hlay);

			QLabel* LabSlid = new QLabel(theRules[i]->getLabel(), aWidg);
			LabSlid->setFixedSize(100, 20);
			LabSlid->move(10, 0);
			//hlay->addWidget(LabSlid, Qt::AlignLeft);

			QLabel* percentLabel = new QLabel("0%", aWidg);
			percentLabel->setFixedSize(40, 20);
			percentLabel->move(130, 20);
			//percentLabel->setAlignment(Qt::AlignCenter);

			QSlider* mySlider = new QSlider(Qt::Horizontal, aWidg);
			mySlider->move(10, 20);
			mySlider->setFixedSize(120, 30);
			mySlider->setTickPosition(QSlider::TicksBelow);
			mySlider->setTickInterval(10);
			mySlider->setMaximum(100);
			mySlider->setMinimum(0);
			mySlider->setValue(0);
			WeightSliderList.append(mySlider);
			//connect(mySlider, SIGNAL(sliderReleased()), this, SLOT(onWeightersChange()));

			QObject::connect(mySlider, &QSlider::valueChanged, this, [=]() {
				percentLabel->setText(QString::number(mySlider->value()) + "%"); onWeightersChange(); });

			if (gridPosy >= 2)
			{
				gridPosx++;
				gridPosy = 0;
				weigthersLayout->addWidget(aWidg, gridPosx, gridPosy++, Qt::AlignCenter);
			}
			else
			{
				weigthersLayout->addWidget(aWidg, gridPosx, gridPosy++, Qt::AlignCenter);
			}

			//Visualization depending on Var
			if (theRules[i]->getVarType() == ActionRule::normalAngle)
			{
				//visuList.push_back(theRules[i]->getLabel());
				visuCombo->addItem(aVisuicon, theRules[i]->getLabel());
				visuCombo2->addItem(aVisuicon, theRules[i]->getLabel());
			}
		}

		for (int i = 0; i < aDoc->getListOfShapes().size(); i++)
		{
			aDoc->getListOfShapes()[i]->setActionRules(theRules);
		}

		//Context Menu
		FeatureProperties = new QMenu(this);
		QAction* action1 = new QAction("Add/Remove Entities", this);
		FeatureProperties->addAction(action1);
		connect(action1, SIGNAL(triggered()), this, SLOT(onShowAttributeWidget()));

		QAction* action2 = new QAction("Display Entities", this);
		FeatureProperties->addAction(action2);
		connect(action2, SIGNAL(triggered()), this, SLOT(onDisplayEntities()));

		QAction* action3 = new QAction("Hide Entities", this);
		FeatureProperties->addAction(action3);
		connect(action3, SIGNAL(triggered()), this, SLOT(onHideEntities()));

		//statusBar()->showMessage(QObject::tr("Ready"), 2000);
		ExpertFileExist = true;
		createARVizualizer();
	}
}

QList<ActionRule*> OrientationProject::getActionRules()
{
	return theRules;
}

void OrientationProject::onRemoveExpertFile()
{
	itemExpertise->removeRow(0);

	ChoiceCheckList.clear();

	int lastItem = itemProject->rowCount() - 1;
	if (lastItem > -1)
	{
		for (int j = 0; j < lastItem + 1; j++)
		{
			int attribCount = itemProject->child(j, 0)->rowCount();
			/*for (int k = 0; k < attribCount; k++)
			{*/
			itemProject->child(j, 0)->removeRows(0, attribCount);
			//}
		}
	}
	ExpertFileExist = false;
	//onAggregToolsGeneral2();

}

void OrientationProject::createARVizualizer()
{
	ExpertDialog = new QDialog(this);
	ExpertDialog->setWindowTitle("Expertise Visualizer");
	ExpertDialog->setFixedSize(900, 820);
	ExpertDialog->setWindowModality(Qt::WindowModal);

	QWidget* aWidget = new QWidget(ExpertDialog);
	aWidget->move(0, 0);
	aWidget->setObjectName("wid");
	aWidget->setStyleSheet("QWidget#wid{background: rgb(232,232,232);}");

	expertLay = new QVBoxLayout;
	aWidget->setLayout(expertLay);

	expertScroll = new QScrollArea(ExpertDialog);
	expertScroll->setGeometry(0, 0, 900, 750);
	expertScroll->setAlignment(Qt::AlignRight);
	expertScroll->setWidgetResizable(true);
	expertScroll->setWidget(aWidget);

	QPushButton* saveChanges = new QPushButton("Save", ExpertDialog);
	saveChanges->setFixedSize(100, 30);
	saveChanges->move(660, 770);
	connect(saveChanges, SIGNAL(released()), this, SLOT(onSaveAR()));

	QPushButton* cancelChanges = new QPushButton("Cancel", ExpertDialog);
	cancelChanges->setFixedSize(100, 30);
	cancelChanges->move(780, 770);
	connect(cancelChanges, SIGNAL(released()), this, SLOT(onCancelAR()));
}

void OrientationProject::onARVizualizer()
{
	for (int i = 0; i < theRules.size(); i++)
	{
		theRules[i]->makeWidget();
		expertLay->addWidget(theRules[i]->getWidget());
	}

	ExpertDialog->show();
}

void OrientationProject::onSaveAR()
{
	for (int i = 0; i < theRules.size(); i++)
	{
		theRules[i]->updateData();
	}
	ExpertDialog->hide();
}

void OrientationProject::onCancelAR()
{
	ExpertDialog->hide();
}

void OrientationProject::onAddEntitiesToRule()
{
	LeftWidget->hide();
	aDoc->SetupSelectionModes();
	//Context Menu for Adding Surfaces
	SurfClickProperties = new QMenu(this);
	QIcon iconv;
	iconv.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/GenFunc.png")), QIcon::Normal, QIcon::Off);
	QAction* action1 = new QAction("Add To List", this);
	action1->setIcon(iconv);
	connect(action1, SIGNAL(triggered()), this, SLOT(addFaceTo()));

	QIcon iconr;
	iconr.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/CancelFunc.png")), QIcon::Normal, QIcon::Off);
	QAction* action2 = new QAction("Remove From List", this);
	action2->setIcon(iconr);
	connect(action2, SIGNAL(triggered()), this, SLOT(RemoveFaceFrom2()));

	QToolBar* toolbarl = new QToolBar;
	toolbarl->addAction(action1);
	toolbarl->addAction(action2);
	QWidgetAction* act = new QWidgetAction(this);
	act->setDefaultWidget(toolbarl);
	SurfClickProperties->addAction(act);


	SurfListProperties = new QMenu(this);
	QAction* action3 = new QAction("Remove From List", this);
	action3->setIcon(iconr);
	connect(action3, SIGNAL(triggered()), this, SLOT(RemoveFaceFrom2()));

	QToolBar* toolbarl2 = new QToolBar;
	toolbarl2->addAction(action3);
	QWidgetAction* act2 = new QWidgetAction(this);
	act2->setDefaultWidget(toolbarl2);
	SurfListProperties->addAction(act2);

	EntitySelectionMode = true;
	LeftWidget->hide();
}

void OrientationProject::onActionRuleSettings()
{
	LeftWidget->hide();

	ARDocks.at(SelectedARid)->show();
}

void OrientationProject::onSurfClickedProp()
{
	if (ExpertFileExist == true)
	{
		if (EntitySelectionMode == true)
		{
			QScreen* screen0 = QApplication::screens().at(0);
			QPoint pos0 = QCursor::pos(screen0);
			SurfClickProperties->exec(pos0);
		}
	}
}

void OrientationProject::onSurfClicked()
{
	/*if (ExpertFileExist == true)
	{
		if (modifDockWidget->isVisible() && (*theListViews)[aDoc->Shapeid][SelectedARid]->isVisible())
		{
			QList<int> iList = aDoc->Hilightretrieve();
			int theId;
			int labId = 0;
			int ind;
			for (int ii = 0; ii < iList.size(); ii++)
			{
				theId = iList[ii];
				bool itexist = false;
				for (int k = 0; k < ArStringLists->at(SelectedARid).size(); k++)
				{
					QString aValText = ArStringLists->at(SelectedARid).at(k);
					QRegExp rx("(\\d+)");
					int pos = rx.indexIn(aValText);
					if (pos > -1)
					{
						QString value = rx.cap(1);
						labId = value.toInt();

						if (labId == theId)
						{
							ind = k;
							itexist = true;
							break;
						}
					}
					if (itexist == true)
					{
						(*theListViews)[aDoc->Shapeid][SelectedARid]->setCurrentIndex((*theListViews)[aDoc->Shapeid][SelectedARid]->indexAt(QPoint(0, ind)));
					}

				}
			}
		}
	}*/
}

void OrientationProject::onDisplayEntities()
{
	if (!aDoc->getListOfShapes().isEmpty())
	{
		aDoc->getListOfShapes()[aDoc->Shapeid]->showAttributedSurfaces(theRules[SelectedARid]->getID());
	}
}

void OrientationProject::onHideEntities()
{
	if (!aDoc->getListOfShapes().isEmpty())
	{
		aDoc->getListOfShapes()[aDoc->Shapeid]->hideAttributedSurfaces(theRules[SelectedARid]->getID());
	}
}

//Printer
void OrientationProject::onPlatformUpdate()
{
	aDoc->PlatformOff();
	aDoc->Platform();
	aDoc->PlatformOn();
}


//Orientation Settings
void OrientationProject::onAxisChanged()
{
	//The rotation Axes must not be the same
	if (theAxis1 == theAxis2)
	{
		QMessageBox* msgBox = new QMessageBox(this);
		msgBox->setText("Please do not select the same axis for both rotations");
		msgBox->setInformativeText("If you leave this setting, the default\n Sequence XY will be applied ");
		msgBox->setStandardButtons(QMessageBox::Ok);
		msgBox->setDefaultButton(QMessageBox::Ok);
		msgBox->setIcon(QMessageBox::Information);
		connect(msgBox->button(QMessageBox::Ok), &QAbstractButton::pressed, [=]() {
			msgBox->close();
			theSequence = 1;
		});
		msgBox->show();
		msgBox->exec();
	}

	else
	{
		if (theAxis1 == 1 && theAxis2 == 2)
		{
			theSequence = 1;
		}

		else if (theAxis1 == 1 && theAxis2 == 3)
		{
			theSequence = 2;
		}

		else if (theAxis1 == 2 && theAxis2 == 1)
		{
			theSequence = 3;
		}

		else if (theAxis1 == 2 && theAxis2 == 3)
		{
			theSequence = 4;
		}

		else if (theAxis1 == 3 && theAxis2 == 1)
		{
			theSequence = 5;
		}

		else if (theAxis1 == 3 && theAxis2 == 2)
		{
			theSequence = 6;
		}
	}
}

void OrientationProject::onRestoreAxes()
{
	if (theSequence = 1)
	{
		theAxis1 = 1;
		theAxis2 = 2;

		xax1->setChecked(true);
		yax2->setChecked(true);
	}

	else if (theSequence == 2)
	{
		theAxis1 = 1;
		theAxis2 = 3;

		xax1->setChecked(true);
		zax2->setChecked(true);
	}

	else if (theSequence == 3)
	{
		theAxis1 = 2;
		theAxis2 = 1;

		yax1->setChecked(true);
		xax2->setChecked(true);
	}

	else if (theSequence == 4)
	{
		theAxis1 = 2;
		theAxis2 = 3;

		yax1->setChecked(true);
		zax2->setChecked(true);
	}

	else if (theSequence == 5)
	{
		theAxis1 = 3;
		theAxis2 = 1;

		zax1->setChecked(true);
		xax2->setChecked(true);
	}

	else if (theSequence == 6)
	{
		theAxis1 = 3;
		theAxis2 = 2;

		zax1->setChecked(true);
		yax2->setChecked(true);
	}
}

void OrientationProject::RestoreOrientationSettings(QList<double> aList)
{
	if (aList.size() == 6)
	{
		//Angle1Range->setValue(aList[0]);
		//Angle1Step->setValue(aList[1]);
		//Angle2Range->setValue(aList[2]);
		//Angle2Step->setValue(aList[3]);

		////Update
		//aDoc->Angle1Range = 180;
		//aDoc->Angle2Range = 360;
		//aDoc->Angle1Step = theStep1;
		//aDoc->Angle2Step = theStep2;


	}


}

void OrientationProject::ActivateButtons()
{
	for (int i = 0; i < ListofToolBarButton.size(); i++)
	{
		ListofToolBarButton[i]->setEnabled(1);
	}
}


//Support Settings
void OrientationProject::SupportSettingTools()
{
	suppAng = 45;
	suppHeight = 5;
	suppWidth = 0.5;
	suppSpace = 2.0;
	/*QScrollArea* supScroll = new QScrollArea(supportSettings);
	supScroll->setGeometry(0, 10, supportSettings->width(), supportSettings->height());
	supScroll->setAlignment(Qt::AlignRight);
	supScroll->setWidgetResizable(true);
	supScroll->setFrameShape(QFrame::NoFrame);*/

	/*QIcon aValicon;
	aValicon.addPixmap(QPixmap(QString::fromUtf8(":/COFFA/Resources/valid.png")), QIcon::Normal, QIcon::Off);
	QPushButton* ValidPlnButton = new QPushButton(supportWidget);
	ValidPlnButton->setFixedSize(40, 40);
	ValidPlnButton->move(10, 10);
	ValidPlnButton->setIcon(aValicon);
	ValidPlnButton->setToolTip("Ok");
	ValidPlnButton->setStatusTip("Validate Operation");
	ValidPlnButton->show();
	connect(ValidPlnButton, SIGNAL(released()), this, SLOT(onCloseSupportWidget()));*/

	QGroupBox* spinWidgets = new QGroupBox(supportSettings);
	spinWidgets->setTitle("Parameters");
	spinWidgets->move(10, 20);
	spinWidgets->setFixedSize(380, 160);
	QFormLayout* spinLayout = new QFormLayout;
	spinWidgets->setLayout(spinLayout);

	QDoubleSpinBox* angSpin = new QDoubleSpinBox(supportSettings);
	angSpin->setRange(0, 90);
	angSpin->setValue(45);
	angSpin->setSuffix(" deg");
	angSpin->setAlignment(Qt::AlignRight);
	angSpin->setFixedSize(150, 25);
	connect(angSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
		suppAng = angSpin->value();
	});

	QDoubleSpinBox* hSpin = new QDoubleSpinBox(supportSettings);
	hSpin->setRange(0, 1000);
	hSpin->setValue(10);
	hSpin->setSuffix(" mm");
	hSpin->setAlignment(Qt::AlignRight);
	hSpin->setFixedSize(150, 25);
	connect(hSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
		suppHeight = hSpin->value();
	});

	QDoubleSpinBox* wSpin = new QDoubleSpinBox(supportSettings);
	wSpin->setRange(0, 100);
	wSpin->setValue(1.5);
	wSpin->setSuffix(" mm");
	wSpin->setAlignment(Qt::AlignRight);
	wSpin->setFixedSize(150, 25);
	connect(wSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
		suppWidth = wSpin->value();
	});

	QDoubleSpinBox* sSpin = new QDoubleSpinBox(supportSettings);
	sSpin->setRange(0, 100);
	sSpin->setValue(2.0);
	sSpin->setSuffix(" mm");
	sSpin->setAlignment(Qt::AlignRight);
	sSpin->setFixedSize(150, 25);
	connect(sSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
		suppSpace = sSpin->value();
	});

	spinLayout->addRow("Angle", angSpin);
	spinLayout->addRow("Height", hSpin);
	spinLayout->addRow("Thickness", wSpin);
	spinLayout->addRow("Spacing", sSpin);

	QGroupBox* connectWidgets = new QGroupBox(supportSettings);
	connectWidgets->setTitle("Connection Preferences");
	connectWidgets->move(10, 190);
	connectWidgets->setFixedSize(380, 150);
	QVBoxLayout* connectLayout = new QVBoxLayout;
	connectWidgets->setLayout(connectLayout);

	QRadioButton* radio1 = new QRadioButton("Full Link");
	connect(radio1, &QRadioButton::isChecked, [=]() {
		connectionType = 1;
	});
	QRadioButton* radio2 = new QRadioButton("Teeth");
	connect(radio2, &QRadioButton::isChecked, [=]() {
		connectionType = 2;
	});
	QRadioButton* radio3 = new QRadioButton("Mountain Tip");
	connect(radio3, &QRadioButton::isChecked, [=]() {
		connectionType = 3;
	});
	radio1->setChecked(true);

	QDoubleSpinBox* connecSpin = new QDoubleSpinBox(supportSettings);
	connecSpin->setRange(0, 50);
	connecSpin->setValue(connectionSpace);
	connecSpin->setSuffix(" mm");
	connecSpin->setAlignment(Qt::AlignRight);
	connecSpin->setFixedSize(150, 25);
	connect(connecSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
		connectionSpace = connecSpin->value();
	});

	connectLayout->addWidget(radio1);
	connectLayout->addWidget(radio2);
	connectLayout->addWidget(radio3);
	connectLayout->addWidget(connecSpin);

	QGroupBox* perforationWidgets = new QGroupBox(supportSettings);
	perforationWidgets->setTitle("Perforation Preferences");
	perforationWidgets->move(10, 350);
	perforationWidgets->setFixedSize(380, 150);
	QFormLayout* perfLayout = new QFormLayout;
	perforationWidgets->setLayout(perfLayout);

}

void OrientationProject::onShowSupportSettings()
{


}

//Computation Elements
void OrientationProject::onStartComputation()
{
	if (theRules.isEmpty())
	{
		QMessageBox::warning(this, QObject::tr("Setting Error !!"), QObject::tr("Please load an Expert File\nTip: press >>Import Expertise Button and select an expertise file from your directory"),
			QObject::tr("OK"), QString::null, QString::null, 0, 0);
	}

	else if (aDoc->getListOfShapes().isEmpty())
	{
		QMessageBox::warning(this, QObject::tr("Setting Error !!"), QObject::tr("Please load a geometry\nTip: press >>Load Part Button and select a STEP or STL file from your directory"),
			QObject::tr("OK"), QString::null, QString::null, 0, 0);
	}


	else
	{
		choiceDialog = new QDialog(this);
		choiceDialog->setMinimumSize(450, 700);
		//choiceDialog->setStyleSheet("QDialog{background: #696969}");
		choiceDialog->setWindowModality(Qt::WindowModal);

		QScrollArea* aScroll = new QScrollArea(choiceDialog);
		aScroll->setGeometry(0, 0, 450, 700);
		aScroll->setAlignment(Qt::AlignRight);
		aScroll->setWidgetResizable(true);
		aScroll->setFrameShape(QFrame::NoFrame);

		QWidget* mainWid = new QWidget();
		QVBoxLayout* aVlay = new QVBoxLayout();
		mainWid->setLayout(aVlay);
		aScroll->setWidget(mainWid);

		QGroupBox* aBox = new QGroupBox("Select Rules to Compute");
		aBox->setFixedSize(400, 350);
		//aBox->setStyleSheet("QWidget{background: #4b4b4b}");
		QVBoxLayout* aBoxLay = new QVBoxLayout();
		aBox->setLayout(aBoxLay);
		aVlay->addWidget(aBox);

		for (int ii = 0; ii < theRules.size(); ii++)
		{
			aBoxLay->addWidget(ChoiceCheckList.at(ii), Qt::AlignLeft);
		}

		QGroupBox* aBox2 = new QGroupBox("Select Part");
		aBox2->setFixedSize(400, 200);
		//aBox2->setStyleSheet("QWidget{background: #4b4b4b}");
		QVBoxLayout* aBoxLay2 = new QVBoxLayout();
		aBox2->setLayout(aBoxLay2);
		aVlay->addWidget(aBox2);
		ListofExportCH.clear();

		QButtonGroup* aGroupeButton = new QButtonGroup();
		for (int j = 0; j < aDoc->getListOfShapes().size(); j++)
		{
			QCheckBox* aCheck = new QCheckBox(aDoc->getListOfShapes().at(j)->getName());
			if (aDoc->Shapeid == j)
			{
				aCheck->setChecked(1);
			}
			ListofExportCH.push_back(aCheck);
			aBoxLay2->addWidget(aCheck);
			aGroupeButton->addButton(aCheck, j);
			aCheck->show();
		}

		QPushButton* ChoiceFinish = new QPushButton("Compute", choiceDialog);
		ChoiceFinish->setFixedSize(150, 30);
		connect(ChoiceFinish, SIGNAL(released()), this, SLOT(onCriteriaComputation()));
		aVlay->addWidget(ChoiceFinish, 3, Qt::AlignCenter);
		choiceDialog->show();
	}
}

void OrientationProject::onCriteriaComputation()
{
	choiceDialog->close();

	QList<bool> ARofInterest;
	bool atleastOne = false;
	for (int ii = 0; ii < ChoiceCheckList.size(); ii++)
	{
		ARofInterest.push_back(ChoiceCheckList.at(ii)->isChecked());
		if (ChoiceCheckList.at(ii)->isChecked())
		{
			atleastOne = true;
		}
	}


	if (atleastOne == true)
	{
		theAggregated.setStep1(theStep1);
		theAggregated.setStep2(theStep2);
		theAggregated.setRange1(180);
		theAggregated.setRange2(360);
		for (int i = 0; i < ListofToolBarButton.size(); i++)
		{
			ListofToolBarButton.at(i)->setDisabled(1);
		}
		//myComputeProgress();

		for (int i = 0; i < ListofExportCH.size(); i++)
		{
			if (ListofExportCH[i]->isChecked())
			{
				aDoc->Shapeid = i;
				break;
			}
		}
		QApplication::processEvents();

		aDoc->onDesirabilityComputation(theSequence, theStep1, theStep2, ARofInterest);
		generateResponses();

		for (int i = 0; i < ListofToolBarButton.size(); i++)
		{
			ListofToolBarButton.at(i)->setEnabled(1);
		}

		workingPartCombo->setCurrentIndex(aDoc->Shapeid);
		workingPartCombo2->setCurrentIndex(aDoc->Shapeid);
		workingPartCombo3->setCurrentIndex(aDoc->Shapeid);
	}

}

void OrientationProject::onShowComputeForOnePos()
{
	if (theRules.isEmpty())
	{
		QMessageBox::warning(this, QObject::tr("Setting Error !!"), QObject::tr("Please load an Expert File\nTip: press >>Import Expertise Button and select an expertise file from your directory"),
			QObject::tr("OK"), QString::null, QString::null, 0, 0);
	}

	else if (aDoc->getListOfShapes().isEmpty())
	{
		QMessageBox::warning(this, QObject::tr("Setting Error !!"), QObject::tr("Please load a geometry\nTip: press >>Load Part Button and select a STEP or STL file from your directory"),
			QObject::tr("OK"), QString::null, QString::null, 0, 0);
	}

	else
	{
		custChoiceDialog = new QDialog(this);
		custChoiceDialog->setMinimumSize(450, 700);
		custChoiceDialog->setWindowModality(Qt::WindowModal);

		QScrollArea* aScroll = new QScrollArea(custChoiceDialog);
		aScroll->setGeometry(0, 0, 450, 700);
		aScroll->setAlignment(Qt::AlignRight);
		aScroll->setWidgetResizable(true);
		aScroll->setFrameShape(QFrame::NoFrame);

		QWidget* mainWid = new QWidget();
		QVBoxLayout* aVlay = new QVBoxLayout();
		mainWid->setLayout(aVlay);
		aScroll->setWidget(mainWid);

		QGroupBox* aBox = new QGroupBox("Select Rules to Compute");
		aBox->setFixedSize(400, 250);
		//aBox->setStyleSheet("QWidget{background: #4b4b4b}");
		QVBoxLayout* aBoxLay = new QVBoxLayout();
		aBox->setLayout(aBoxLay);
		aVlay->addWidget(aBox);

		for (int ii = 0; ii < theRules.size(); ii++)
		{
			aBoxLay->addWidget(ChoiceCheckList.at(ii), Qt::AlignLeft);
		}

		QGroupBox* aBox2 = new QGroupBox("Select Part");
		aBox2->setFixedSize(400, 100);
		//aBox2->setStyleSheet("QWidget{background: #4b4b4b}");
		QVBoxLayout* aBoxLay2 = new QVBoxLayout();
		aBox2->setLayout(aBoxLay2);
		aVlay->addWidget(aBox2);
		ListofExportCH.clear();

		QButtonGroup* aGroupeButton = new QButtonGroup();
		for (int j = 0; j < aDoc->getListOfShapes().size(); j++)
		{
			QCheckBox* aCheck = new QCheckBox(aDoc->getListOfShapes().at(j)->getName());
			if (aDoc->Shapeid == j)
			{
				aCheck->setChecked(1);
			}
			ListofExportCH.push_back(aCheck);
			aBoxLay2->addWidget(aCheck);
			aGroupeButton->addButton(aCheck, j);
			aCheck->show();
		}

		QGroupBox* ctrlWid = new QGroupBox("Set Angles");
		ctrlWid->setFixedSize(400, 120);
		QFormLayout* ctrlLay = new QFormLayout;
		ctrlWid->setLayout(ctrlLay);

		QDoubleSpinBox* Rxspin0 = new QDoubleSpinBox();
		Rxspin0->setRange(0, 360);
		Rxspin0->setValue(Rx);
		Rxspin0->setFixedHeight(25);
		connect(Rxspin0, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
			Rx = Rxspin0->value();
		});

		QDoubleSpinBox* Ryspin0 = new QDoubleSpinBox();
		Ryspin0->setRange(0, 360);
		Ryspin0->setValue(Ry);
		Ryspin0->setFixedHeight(25);
		connect(Ryspin0, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
			Ry = Ryspin0->value();
		});

		QDoubleSpinBox* Rzspin0 = new QDoubleSpinBox();
		Rzspin0->setRange(0, 360);
		Rzspin0->setValue(Rz);
		Rzspin0->setFixedHeight(25);
		connect(Rzspin0, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double value) {
			Rz = Rzspin0->value();
		});

		ctrlLay->addRow("X-Axis", Rxspin0);
		ctrlLay->addRow("Y-Axis", Ryspin0);
		ctrlLay->addRow("Z-Axis", Rzspin0);

		aVlay->addWidget(ctrlWid);

		custResultBarChart = new QChart();
		custResultBarChart->legend()->hide();

		QChartView* chartView = new QChartView(custResultBarChart);
		chartView->setRenderHint(QPainter::Antialiasing);
		chartView->move(10, 260);
		chartView->setFixedSize(400, 350);

		custBarAxisX = new QBarCategoryAxis();
		custBarAxisX->setLabelsAngle(270);
		custResultBarChart->addAxis(custBarAxisX, Qt::AlignBottom);

		custBarAxisY = new QValueAxis();
		custBarAxisY->setRange(0, 1.0);
		custBarAxisY->setTitleText("Desirability");
		custResultBarChart->addAxis(custBarAxisY, Qt::AlignLeft);


		QPushButton* ChoiceFinish = new QPushButton("Calculate", mainWid);
		ChoiceFinish->setFixedSize(150, 30);
		connect(ChoiceFinish, SIGNAL(released()), this, SLOT(onComputeForOnePos()));
		aVlay->addWidget(ChoiceFinish, 3, Qt::AlignCenter);

		aVlay->addWidget(chartView, 4, Qt::AlignCenter);

		custChoiceDialog->show();

	}
}

void OrientationProject::onComputeForOnePos()
{
	QList<bool> ARofInterest;
	bool atleastOne = false;
	for (int ii = 0; ii < ChoiceCheckList.size(); ii++)
	{
		ARofInterest.push_back(ChoiceCheckList.at(ii)->isChecked());
		if (ChoiceCheckList.at(ii)->isChecked())
		{
			atleastOne = true;
		}
	}


	if (atleastOne == true)
	{
		for (int i = 0; i < ListofToolBarButton.size(); i++)
		{
			ListofToolBarButton.at(i)->setDisabled(1);
		}

		for (int i = 0; i < ListofExportCH.size(); i++)
		{
			if (ListofExportCH[i]->isChecked())
			{
				aDoc->Shapeid = i;
				break;
			}
		}

		aDoc->onCustDesirabilityComputation(theSequence, Rx, Ry, Rz, ARofInterest);

		custChoiceDialog->show();

		QApplication::processEvents();
		for (int i = 0; i < ListofToolBarButton.size(); i++)
		{
			ListofToolBarButton.at(i)->setEnabled(1);
		}

		workingPartCombo->setCurrentIndex(aDoc->Shapeid);

		QBarSeries* custBarSeries = new QBarSeries;
		QBarSet* seti = new QBarSet("Action Rules");

		QStringList categories;
		if (theRules.size() != 0)
		{
			for (int i = 0; i < theRules.size(); i++)
			{
				categories.push_back(theRules[i]->getName());
			}

		}

		int n = 0;
		for each (VarARValues varAR in aDoc->getListOfShapes()[aDoc->Shapeid]->theCustComputedValues)
		{
			double val = 0;
			for (int i = 0; i < varAR.theData.size(); i++)
			{
				if (varAR.theData[i].Rx == Rx && varAR.theData[i].Ry == Ry && varAR.theData[i].Rz == Rz)
				{
					val = varAR.theData[i].globalValue;
				}
			}
			seti->append(val);
		}

		custBarAxisX->clear();
		custBarAxisX->append(categories);
		seti->setColor(QColor(96, 12, 252));;
		custBarSeries->append(seti);

		custResultBarChart->removeAllSeries();
		custResultBarChart->addSeries(custBarSeries);

		custBarSeries->attachAxis(custBarAxisX);
		custBarSeries->attachAxis(custBarAxisY);
	}
}

//Responses
void OrientationProject::onDisplayResponseWidget()
{
	if (!aDoc->getListOfShapes().isEmpty())
	{
		if (!SingleRespWidget->isVisible())
			generateResponses();

		else if (SingleRespWidget->isVisible())
		{
			SingleRespWidget->hide();
		}
	}

	else
	{
		QMessageBox::warning(this, QObject::tr("No Data Found!!"),
			QObject::tr("No Geometry has been found in your Project"),
			QObject::tr("OK"), QString::null, QString::null, 0, 0);
	}
}

void OrientationProject::onSingleResponseTools()
{
	SingleRespWidget = new QWidget(mainFrame);
	SingleRespWidget->move(mainFrame->width()- 570, 85);
	SingleRespWidget->setFixedSize(400, 700);
	//SingleRespWidget->setStyleSheet("QWidget{background: rgba(232, 232, 232, 1);}");

	QLabel* titleLabel = new QLabel(SingleRespWidget);
	titleLabel->move(0, 0);
	titleLabel->setText("  Individual Responses");
	titleLabel->setFixedSize(500, 30);
	titleLabel->setStyleSheet("QLabel{background: rgba(187, 191, 202, 1); color: rgba(73, 54, 100, 1); font-size: 16px;}"); //border-top-left-radius: 15px; border-top-right-radius: 15px;
	titleLabel->show();

	QIcon aHideicon;
	aHideicon.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/closeSurface.png")), QIcon::Normal, QIcon::Off);
	QPushButton* closeResButton = new QPushButton(titleLabel);
	closeResButton->setFixedSize(25, 25);
	closeResButton->move(460, 2.5);
	closeResButton->setIcon(aHideicon);
	closeResButton->setToolTip("Close Response Window");
	closeResButton->show();
	connect(closeResButton, SIGNAL(released()), this, SLOT(onCloseResponseWindow()));

	QIcon updateVisuicon;
	updateVisuicon.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/updateVisu.png")), QIcon::Normal, QIcon::Off);
	QPushButton* visuButton = new QPushButton(titleLabel);
	visuButton->setFixedSize(25, 25);
	visuButton->move(420, 2.5);
	visuButton->setIcon(updateVisuicon);
	visuButton->setToolTip("Update Visualization");
	visuButton->show();
	connect(visuButton, SIGNAL(released()), this, SLOT(onUpdateVisualized()));

	visuCombo = new QComboBox(titleLabel);
	visuCombo->setFixedSize(100, 25);
	visuCombo->move(300, 2.5);
	visuCombo->setToolTip("Select Visualized Response\nThis displays a color\nmap of the response\non the Geometry");
	//connect(visuCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onUpdateVisualized()));

	workingPartCombo2 = new QComboBox(titleLabel);
	workingPartCombo2->setFixedSize(100, 25);
	workingPartCombo2->move(190, 2.5);
	workingPartCombo2->setToolTip("Select Working Part\nThis will automatically\nupdate all data");
	connect(workingPartCombo2, SIGNAL(currentIndexChanged(int)), this, SLOT(onUpdateSelectedPart2()));

	/*visuCombo->setac(aHideicon);*/

	responsesContainer = new QWidget(SingleRespWidget);
	responsesContainer->move(0, 30);
	responsesContainer->setStyleSheet("QWidget {background:rgba(232, 232, 232, 1);}");
	ResultLayout = new QGridLayout();
	layPosx = 1; layPosy = 0;

	ResultStack = new QStackedLayout(SingleRespWidget);
	responsesContainer->setLayout(ResultStack);

	SingleRespScroll = new QScrollArea(SingleRespWidget);
	SingleRespScroll->setGeometry(0, 30, /*myResWid->width() - 20*/800, 330);
	SingleRespScroll->setAlignment(Qt::AlignRight);
	SingleRespScroll->setWidgetResizable(true);
	SingleRespScroll->setFrameShape(QFrame::NoFrame);
	SingleRespScroll->setWidget(responsesContainer);

	/*NoResponseLabel = new QLabel("Message:\nFrom the Tab --> Responses Check the boxes corresponding to the\nresponses you would like to display.", SingleRespWidget);
	NoResponseLabel->move(50, 100);
	NoResponseLabel->setFixedSize(550, 120);
	NoResponseLabel->setFont(aFont);
	NoResponseLabel->setStyleSheet("QLabel {background:#ffcf7f; color: #4b4b4b}");
	NoResponseLabel->show();*/
	SingleRespWidget->hide();
	respWidgetCreated = true;
}

void OrientationProject::onCloseResponseWindow()
{
	SingleRespWidget->hide();
}

void OrientationProject::generateResponses()
{
	if (!aDoc->getListOfShapes().isEmpty())
	{
		SingleRespWidget->show();
		ResultStack->setCurrentWidget(aDoc->getListOfShapes()[aDoc->Shapeid]->getResultWidget());

		/*layPosx = 1; layPosy = 0;

		for (int i = 0; i < ResultLayout->rowCount(); i++)
		{
			for (int j = 0; j < ResultLayout->columnCount(); j++)
			{
				//QLayoutItem *item=ResultLayout->itemAtPosition(i, j);
				////ResultLayout->indexOf(item->widget());
				//ResultLayout->removeItem(item);
				if (ResultLayout!= nullptr)
				{
					ResultLayout->itemAtPosition(i, j)->widget()->hide();
					ResultLayout->removeWidget(ResultLayout->itemAtPosition(i, j)->widget());
				}
			}
		}
		QWidget* item;
		while ((item = ResultLayout->takeAt(0)) != 0) {
			ResultLayout->removeWidget(item);

		}*/

		/*for (int i = 0; i < aDoc->getListOfShapes()[aDoc->Shapeid]->theComputedValues.size(); i++)
		{
			if (layPosy >= 2)
			{
				layPosx++;
				layPosy = 0;
				ResultLayout->addWidget(aDoc->getListOfShapes()[aDoc->Shapeid]->theComputedValues[i].getResponse(),
					layPosx, layPosy++, Qt::AlignCenter);
				aDoc->getListOfShapes()[aDoc->Shapeid]->theComputedValues[i].getResponse()->show();
			}
			else
			{
				ResultLayout->addWidget(aDoc->getListOfShapes()[aDoc->Shapeid]->theComputedValues[i].getResponse(),
					layPosx, layPosy++, Qt::AlignCenter);
				aDoc->getListOfShapes()[aDoc->Shapeid]->theComputedValues[i].getResponse()->show();
			}
		}*/
	}
}

void OrientationProject::onUpdateVisualized()
{
	if (!aDoc->getListOfShapes().isEmpty())
	{
		theNotifier = new NotificationWidget("Processing...");
		theNotifier->setParent(this);
		theNotifier->move(this->width() - 370, this->height() - 170);
		theNotifier->show();
		QApplication::processEvents();

		QString aRule;
		bool found = false;
		for (int i = 0; i < theRules.size(); i++)
		{
			if (theRules[i]->getLabel() == visuCombo->currentText())
			{
				aRule = theRules[i]->getID();
				found = true;
				break;
			}
		}

		if (found)
		{
			aDoc->getListOfShapes().at(aDoc->Shapeid)->visualizeDistribution(aRule);
		}

		else
		{
			aDoc->getListOfShapes().at(aDoc->Shapeid)->visualizeDistribution("none");
		}
		theNotifier->close();
	}

	else if (aDoc->getListOfShapes().isEmpty())
	{
		QMessageBox::warning(this, QObject::tr("Error !!"), QObject::tr("No Shape has been detected\nTip: press >>Load Part Button and select a STEP or STL file from your directory"),
			QObject::tr("OK"), QString::null, QString::null, 0, 0);
	}
}

void OrientationProject::onUpdateVisualized2()
{
	if (!aDoc->getListOfShapes().isEmpty())
	{
		theNotifier = new NotificationWidget("Processing...");
		theNotifier->setParent(this);
		theNotifier->move(this->width() - 370, this->height() - 170);
		theNotifier->show();
		QApplication::processEvents();

		QString aRule;
		bool found = false;
		for (int i = 0; i < theRules.size(); i++)
		{
			if (theRules[i]->getLabel() == visuCombo2->currentText())
			{
				aRule = theRules[i]->getID();
				found = true;
				break;
			}
		}

		if (found)
		{
			aDoc->getListOfShapes().at(aDoc->Shapeid)->visualizeDistribution(aRule);
		}

		else
		{
			aDoc->getListOfShapes().at(aDoc->Shapeid)->visualizeDistribution("none");
		}
		theNotifier->close();
	}

	else if (aDoc->getListOfShapes().isEmpty())
	{
		QMessageBox::warning(this, QObject::tr("Error !!"), QObject::tr("No Shape has been detected\nTip: press >>Load Part Button and select a STEP or STL file from your directory"),
			QObject::tr("OK"), QString::null, QString::null, 0, 0);
	}
}

void OrientationProject::onCreateVisualizationDialog()
{
	visuDialog = new QDialog(this);
	visuDialog->setFixedSize(300, 170);
	visuDialog->setWindowModality(Qt::NonModal);

	QGroupBox* aBox = new QGroupBox("Select Visualizable Rule", visuDialog);
	aBox->setFixedSize(280, 100);
	aBox->move(10, 10);

	QFormLayout* aform = new QFormLayout();
	aBox->setLayout(aform);

	visuCombo2 = new QComboBox(visuDialog);
	visuCombo2->setFixedHeight(25);
	visuCombo2->setToolTip("Select Visualized Response\nThis displays a color\nmap of the response\non the Geometry");

	workingPartCombo3 = new QComboBox(this);
	workingPartCombo3->setFixedHeight(25);
	workingPartCombo3->setToolTip("Select Working Part\nThis will automatically\nupdate all data");
	connect(workingPartCombo3, SIGNAL(currentIndexChanged(int)), this, SLOT(onUpdateSelectedPart3()));

	aform->addRow("Select Rule", visuCombo2);
	aform->addRow("Select Part", workingPartCombo3);

	QPushButton* ChoiceFinish = new QPushButton("Update", visuDialog);
	ChoiceFinish->setFixedSize(150, 25);
	ChoiceFinish->move(75, 130);
	connect(ChoiceFinish, SIGNAL(released()), this, SLOT(onUpdateVisualized2()));
	visuDialog->hide();
}

void OrientationProject::onShowVisualizationDialog()
{
	visuDialog->show();
}

//Aggregator Elements
void OrientationProject::onWeightersChange()
{
	for (int ii = 0; ii < WeightSliderList.size(); ii++)
	{
		WeightValues.replace(ii, (WeightSliderList.at(ii)->value() * 0.01));

		if (WeightValues[ii] == 1)
		{
			for (int jj = 0; jj < WeightSliderList.size(); jj++)
			{
				if (ii != jj)
				{
					WeightSliderList[jj]->setValue(0);
				}
			}
		}
	}
}

void OrientationProject::onRadarDisplay()
{
	int step1i = theStep1;
	int step2i = theStep2;
	if (Rot1Dial->value() % step1i != 0)
	{
		int m = Rot1Dial->value();
		int roundValue = 0;
		for (int i = 0; i < step1i; i++)
		{
			m = m + 1;
			if (m % step1i == 0 && i > step1i / 2)
			{
				roundValue = m - step1i;
				break;
			}
			else if (m % step1i == 0 && i <= step1i / 2)
			{
				roundValue = m;
				break;
			}
		}
		Rot1Dial->setValue(roundValue);
		Rot1 = roundValue;
	}

	//XDLabel->setText(QString::number(Rot1Dial->value()));

	if (Rot2Dial->value() % step2i != 0)
	{
		int m = Rot2Dial->value();
		int roundValue = 0;
		for (int i = 0; i < step2i; i++)
		{
			m = m + 1;
			if (m % step2i == 0 && i > step2i / 2)
			{
				roundValue = m - step2i;
				break;
			}
			else if (m % step2i == 0 && i <= step2i / 2)
			{
				roundValue = m;
				break;
			}
		}
		Rot2Dial->setValue(roundValue);
		Rot2 = roundValue;
	}

	if (theSequence == 1)
	{
		Rx = Rot1;
		Ry = Rot2;
		Rz = 0;
	}

	else if (theSequence == 2)
	{
		Rx = Rot1;
		Ry = 0;
		Rz = Rot2;
	}

	else if (theSequence == 3)
	{
		Rx = Rot2;
		Ry = Rot1;
		Rz = 0;
	}

	else if (theSequence == 4)
	{
		Rx = 0;
		Ry = Rot1;
		Rz = Rot2;
	}

	else if (theSequence == 5)
	{
		Rx = Rot2;
		Ry = 0;
		Rz = Rot1;
	}

	else if (theSequence == 6)
	{
		Rx = 0;
		Ry = Rot2;
		Rz = Rot1;
	}


	if (aDoc->getListOfShapes().size() != 0)
	{
		aDoc->rotateCurrentShape(theSequence, Rx, Ry, Rz);

		QLineSeries* polarSeries = new QLineSeries;
		polarSeries->setColor(QColor(96, 12, 252));
		QPen aPen;
		aPen.setWidth(5);
		aPen.setJoinStyle(Qt::RoundJoin);
		aPen.setColor(QColor(96, 12, 252));
		polarSeries->setPen(aPen);
		QFont labFont;
		labFont.setPointSize(7);
		ResultPolarChart->removeAxis(PolarAxisX);
		PolarAxisX = new QCategoryAxis();
		PolarAxisX->setLabelsFont(labFont);

		PolarAxisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
		PolarAxisX->setRange(0, 360);
		double pos = 360 / theRules.size();
		if (theRules.size() != 0)
		{
			for (int i = 0; i < theRules.size(); i++)
			{
				PolarAxisX->append(theRules[i]->getLabel(), pos * i);
			}
		}

		int i = 0;
		double val1 = 1;
		for each (VarARValues varAR in aDoc->getListOfShapes()[aDoc->Shapeid]->theComputedValues)
		{
			double val = -1;
			for (int i = 0; i < varAR.theData.size(); i++)
			{
				if (varAR.theData[i].Rx == Rx && varAR.theData[i].Ry == Ry && varAR.theData[i].Rz == Rz)
				{
					val = varAR.theData[i].globalValue;
					break;
				}
			}

			//Not Computed Action Rule Considered as Perfect
			if (val == -1)
				val = 1;

			if (i == 0)
				val1 = val;

			*polarSeries << QPointF(pos * i, val);
			i++;
		}

		*polarSeries << QPointF(360, val1);

		ResultPolarChart->addAxis(PolarAxisX, QPolarChart::PolarOrientationAngular);

		ResultPolarChart->removeAllSeries();
		ResultPolarChart->addSeries(polarSeries);

		polarSeries->attachAxis(PolarAxisX);
		polarSeries->attachAxis(PolarAxisY);
	}
}

void OrientationProject::onBarsDisplay()
{
	int step1i = theStep1;
	int step2i = theStep2;
	
	if (Rot11Dial->value() % step1i != 0)
	{
		int m = Rot11Dial->value();
		int roundValue = 0;
		for (int i = 0; i < step1i; i++)
		{
			m = m + 1;
			if (m % step1i == 0 && i > step1i / 2)
			{
				roundValue = m - step1i;
				break;
			}
			else if (m % step1i == 0 && i <= step1i / 2)
			{
				roundValue = m;
				break;
			}
		}
		Rot11Dial->setValue(roundValue);
		Rot1 = roundValue;
	}

	//XDLabel->setText(QString::number(Rot1Dial->value()));

	if (Rot22Dial->value() % step2i != 0)
	{
		int m = Rot22Dial->value();
		int roundValue = 0;
		for (int i = 0; i < step2i; i++)
		{
			m = m + 1;
			if (m % step2i == 0 && i > step2i / 2)
			{
				roundValue = m - step2i;
				break;
			}
			else if (m % step2i == 0 && i <= step2i / 2)
			{
				roundValue = m;
				break;
			}
		}
		Rot22Dial->setValue(roundValue);
		Rot2 = roundValue;
	}

	//YDLabel->setText(QString::number(YDial->value()));
	//YDLabel->setText(QString::number(YDial->value()));

	if (theSequence == 1)
	{
		Rx = Rot1;
		Ry = Rot2;
		Rz = 0;
	}

	else if (theSequence == 2)
	{
		Rx = Rot1;
		Ry = 0;
		Rz = Rot2;
	}

	else if (theSequence == 3)
	{
		Rx = Rot2;
		Ry = Rot1;
		Rz = 0;
	}

	else if (theSequence == 4)
	{
		Rx = 0;
		Ry = Rot1;
		Rz = Rot2;
	}

	else if (theSequence == 5)
	{
		Rx = Rot2;
		Ry = 0;
		Rz = Rot1;
	}

	else if (theSequence == 6)
	{
		Rx = 0;
		Ry = Rot2;
		Rz = Rot1;
	}
	

	if (aDoc->getListOfShapes().size() != 0)
	{
		aDoc->rotateCurrentShape(theSequence, Rx, Ry, Rz);

		QBarSeries* BarSeries = new QBarSeries;
		QBarSet* seti = new QBarSet("Action Rules");

		QStringList categories;
		if (theRules.size() != 0)
		{
			for (int i = 0; i < theRules.size(); i++)
			{
				categories.push_back(theRules[i]->getLabel());
			}

		}

		for each (VarARValues varAR in aDoc->getListOfShapes()[aDoc->Shapeid]->theComputedValues)
		{
			double val = 0;
			for (int i = 0; i < varAR.theData.size(); i++)
			{
				if (varAR.theData[i].Rx == Rx && varAR.theData[i].Ry == Ry && varAR.theData[i].Rz == Rz)
				{
					val = varAR.theData[i].globalValue;
				}
			}
			seti->append(val);
		}

		BarAxisX->clear();
		BarAxisX->append(categories);
		seti->setColor(QColor(96, 12, 252));
		BarSeries->append(seti);

		ResultBarChart->removeAllSeries();
		ResultBarChart->addSeries(BarSeries);

		BarSeries->attachAxis(BarAxisX);
		BarSeries->attachAxis(BarAxisY);
	}
}

void OrientationProject::onAggregateValues()
{
	bool noWeight = true;
	double SumWeight = 0;
	//T = fopen("Weit.txt", "w");
	for (int ii = 0; ii < WeightValues.size(); ii++)
	{
		if (WeightValues.at(ii) != 0)
		{
			SumWeight = SumWeight + WeightValues.at(ii);
			noWeight = false;
		}

	}

	if (noWeight == true)
	{
		QMessageBox::warning(this, QObject::tr("Assign Weights!!"),
			QObject::tr("Please assign weight to at least one rule\nTip: use the sliders"),
			QObject::tr("OK"), QString::null, QString::null, 0, 0);

		return;
	}

	else if (aDoc->getListOfShapes().isEmpty())
	{
		QMessageBox::warning(this, QObject::tr("No Data!!"),
			QObject::tr("Please import parts first\nTip: load part and expert file then press on compute button to choose rules"),
			QObject::tr("OK"), QString::null, QString::null, 0, 0);

		return;
	}

	else
	{
		if (aDoc->getListOfShapes().size() != 0)
		{
			theNotifier = new NotificationWidget("Aggregating Results...");
			theNotifier->setParent(this);
			theNotifier->setWindowModality(Qt::NonModal);
			theNotifier->move(this->width() - 370, this->height() - 170);
			theNotifier->show();
			QApplication::processEvents();

			int step1i = 180/theStep1;
			int step2i = 360/theStep2;

			QList<VarARData> theVals;

			for (int i = 0; i <= step1i; i++)
			{
				for (int j = 0; j <= step2i; j++)
				{
					QList<double> vals;
					int n = 0;
					for each (VarARValues varAR in aDoc->getListOfShapes()[aDoc->Shapeid]->theComputedValues)
					{
						bool found = false;
						double val = 0;
						for (int m = 0; m < varAR.theData.size(); m++)
						{
							if (varAR.theData[m].iterator1 == i && varAR.theData[m].iterator2 == j)
							{
								found = true;
								val = varAR.theData[m].globalValue;

								break;
							}
						}

						if (found)
						{
							val = pow(val, WeightValues[n]);
						}

						else
						{
							val = 1;
						}

						vals.push_back(val);

						n++;
					}
					double totval = 1;
					for (int p = 0; p < vals.size(); p++)
					{
						totval = totval * vals[p];
					}
					VarARData aData;
					aData.globalValue = totval;
					aData.iterator1 = i;
					aData.iterator2 = j;
					theVals.push_back(aData);
				}
			}
			theAggregated.getResponse()->shouldUpdate=true;
			theAggregated.setStep1(theStep1);
			theAggregated.setStep2(theStep2);
			theAggregated.setRange1(180);
			theAggregated.setRange2(360);
			theAggregated.theData.clear();
			theAggregated.setData(theVals);
			theAggregated.updateResponse();

			theNotifier->setMessage("Done !\nClick on any point to see orientation");
			QTimer::singleShot(2500, theNotifier, SLOT(close()));
		}

	}

}

void OrientationProject::onHandler(QPoint aP)
{
	/*QPoint Pos = SurfaceResult->getSurface()->seriesList().at(0)->selectedPoint();

	XChart = ((Pos.x()) * theStep1 * 3.141592654) / 180;
	YChart = (((Pos.y())) * theStep2 * 3.141592654) / 180;

	if (!aDoc->getListOfShapes().empty())
	{
		if (theSequence == 1)
		{
			Rx = Rot1;
			Ry = Rot2;
			Rz = 0;
		}

		else if (theSequence == 2)
		{
			Rx = Rot1;
			Ry = 0;
			Rz = Rot2;
		}

		else if (theSequence == 3)
		{
			Rx = Rot2;
			Ry = Rot1;
			Rz = 0;
		}

		else if (theSequence == 4)
		{
			Rx = 0;
			Ry = Rot1;
			Rz = Rot2;
		}

		else if (theSequence == 5)
		{
			Rx = Rot2;
			Ry = 0;
			Rz = Rot1;
		}

		else if (theSequence == 6)
		{
			Rx = 0;
			Ry = Rot2;
			Rz = Rot1;
		}

		QTimer::singleShot(100, this, SLOT(onHandler2()));
	}*/
}

void OrientationProject::onHandler2()
{
	aDoc->rotateCurrentShape(theSequence, Rx, Ry, Rz);

}

void OrientationProject::onChartPointPress(int cx, int cy)
{
	double XChart = (double)(cx);
	double YChart = (double)(cy);

	if (!aDoc->getListOfShapes().empty())
	{
		if (theSequence == 1)
		{
			Rx = XChart;
			Ry = YChart;
			Rz = 0;
		}

		else if (theSequence == 2)
		{
			Rx = XChart;
			Ry = 0;
			Rz = YChart;
		}

		else if (theSequence == 3)
		{
			Rx = YChart;
			Ry = XChart;
			Rz = 0;
		}

		else if (theSequence == 4)
		{
			Rx = 0;
			Ry = XChart;
			Rz = YChart;
		}

		else if (theSequence == 5)
		{
			Rx = YChart;
			Ry = 0;
			Rz = XChart;
		}

		else if (theSequence == 6)
		{
			Rx = 0;
			Ry = YChart;
			Rz = XChart;
		}

		if (!aDoc->getListOfShapes().isEmpty())
		{
			for (int i = 0; i < aDoc->getListOfShapes().at(aDoc->Shapeid)->theComputedValues.size(); i++)
			{
				aDoc->getListOfShapes().at(aDoc->Shapeid)->theComputedValues[i].setSelectedPoint(QPoint(cx, cy));
			}
		}

		theAggregated.setSelectedPoint(QPoint(cx, cy));
		
		aDoc->rotateCurrentShape(theSequence, Rx, Ry, Rz);

		//onHandler2();
		//QTimer::singleShot(50, this, SLOT(onHandler2()));

		Rot1Dial->setValue(XChart);
		Rot2Dial->setValue(YChart);
		
		Rot11Dial->setValue(XChart);
		Rot22Dial->setValue(YChart);
		onBarsDisplay();
		onRadarDisplay();
		//QTimer::singleShot(50, this, SLOT(onBarsDisplay()));

		//QApplication::processEvents();
	}
}

void OrientationProject::onColorMapSup()
{
	double threRead1 = /*(wDesi->getValFunc().at(0).at(2))*(3.141592654)*/ 20 / 180;
	double threRead2 = /*(wDesi->getValFunc().at(0).at(4))*(3.141592654)*/ 50 / 180;
	//aDoc->myColorView(threRead1, threRead2);
}

//Fitness Checking
void OrientationProject::onFitnessChecker()
{
	aDoc->checkPartFitnessTool();
}


//Support Structures
void OrientationProject::onGenRTS()
{
	aDoc->generateRTS(suppAng, suppSpace, suppSpace);
}

void OrientationProject::onGenSupport()
{

}


//Display SSDR
void OrientationProject::onSSDRTools()
{
	if (!aDoc->getListOfShapes().isEmpty())
	{

		SSDRdockWidget = new QDockWidget("Simulate Support Removal Settings", this);
		addDockWidget(Qt::LeftDockWidgetArea, SSDRdockWidget);
		SSDRdockWidget->setFixedWidth(LeftWidget->width());
		SSDRdockWidget->show();

		QIcon aCanicon;
		aCanicon.addPixmap(QPixmap(QString::fromUtf8(":/Coffa_Project/Resources/CancelFunc.png")), QIcon::Normal, QIcon::Off);
		QPushButton* CancelButton = new QPushButton(SSDRdockWidget);
		CancelButton->setFixedSize(40, 40);
		CancelButton->move(10, 40);
		CancelButton->setIcon(aCanicon);
		CancelButton->setToolTip("Cancel");
		CancelButton->setStatusTip("Cancel Operation");
		CancelButton->show();
		connect(CancelButton, SIGNAL(released()), this, SLOT(onCancelSSDRSim()));

		QWidget* aWid = new QWidget(SSDRdockWidget);
		aWid->move(10, 100);
		aWid->setMinimumSize(350, 100);
		aWid->show();
		QFormLayout* aForm = new QFormLayout();
		aWid->setLayout(aForm);

		AngSSDR = new QDoubleSpinBox();
		AngSSDR->setSingleStep(1.0);
		AngSSDR->setSuffix(".deg");
		AngSSDR->setRange(0.0, 90.0);
		AngSSDR->setAlignment(Qt::AlignRight);
		AngSSDR->setValue(45.0);
		AngSSDR->setStyleSheet("QSpinBox {background: #4b4b4b;}");
		AngSSDR->show();
		//connect(AngSSDR, SIGNAL(valueChanged(int)), this, SLOT(onRangeSettingUpdate(int)));

		ToolSSDR = new QDoubleSpinBox();
		ToolSSDR->setSingleStep(1);
		ToolSSDR->setSuffix(".mm");
		ToolSSDR->setRange(0.1, 500.0);
		ToolSSDR->setValue(10);
		ToolSSDR->setAlignment(Qt::AlignRight);
		ToolSSDR->setStyleSheet("QSpinBox {background: #4b4b4b;}");
		ToolSSDR->show();
		//connect(ToolSSDR, SIGNAL(valueChanged(int)), this, SLOT(onRangeSettingUpdate(int)));

		QPushButton* simButton = new QPushButton("Calculate", SSDRdockWidget);
		simButton->setFixedSize(200, 30);
		simButton->move(80, 180);
		simButton->show();
		connect(simButton, SIGNAL(released()), this, SLOT(onSimulateSSDR()));

		/*QImage Welim(":/Coffa_Project/Resources/GradSSDR.png");
		QPixmap Welpix = QPixmap::fromImage(Welim);
		QLabel* wLabel = new QLabel(SSDRdockWidget);
		wLabel->setPixmap(Welpix);
		wLabel->setFixedSize(109, 330);
		wLabel->setScaledContents(true);
		wLabel->move(100, 330);
		wLabel->show();*/

		aForm->addRow("Supporting Threshold", AngSSDR);
		aForm->addRow("Tool Size (D)", ToolSSDR);
		LeftWidget->hide();


		for (int i = 0; i < ListofToolBarButton.size(); i++)
		{
			ListofToolBarButton[i]->setDisabled(1);
		}
	}

	else
	{
		QMessageBox::warning(this, QObject::tr("No Geometry Loaded !!"), QObject::tr("Please load a geometry to start this Simulation\nTip: press >>Load Part Button and choose a STEP or STL file from your directory"),
			QObject::tr("OK"), QString::null, QString::null, 0, 0);
	}
}

void OrientationProject::onSimulateSSDR()
{
	if (!aDoc->getListOfShapes().isEmpty())
	{
		/*aDoc->getContext()->Remove(aDoc->CompAISRoad, true);
		aDoc->getContext()->Remove(aDoc->CompAISRed, true);
		aDoc->getContext()->Remove(aDoc->CompAISYellow, true);
		aDoc->getContext()->Remove(aDoc->CompAISGreen, true);
		aDoc->getContext()->Remove(aDoc->CompAISBlue, true);*/

		//statusBar()->showMessage(QObject::tr("Progressing... Please Wait"));
		if (aDoc->getListOfShapes().size() != 0)
		{
			aDoc->getListOfShapes().at(aDoc->Shapeid)->simulateSSDR(ToolSSDR->value(), AngSSDR->value());
		}

		
	}

	else
	{
		QMessageBox::warning(this, QObject::tr("No Geometry Loaded !!"), QObject::tr("Please load a geometry to start this Simulation\nTip: press >>Load Part Button and choose a STEP or STL file from your directory"),
			QObject::tr("OK"), QString::null, QString::null, 0, 0);
	}
}

void OrientationProject::onCancelSSDRSim()
{
	//aDoc->getContext()->Remove(aDoc->CompAISBlue, true);

	if (aDoc->getListOfShapes().size() != 0)
	{
		aDoc->getListOfShapes().at(aDoc->Shapeid)->getInteractiveObject()->SetTransparency(0.0);
		aDoc->getContext()->Remove(aDoc->getListOfShapes().at(aDoc->Shapeid)->SSDRObject, true);
	}

	LeftWidget->show();
	SSDRdockWidget->hide();

	for (int i = 0; i < ListofToolBarButton.size(); i++)
	{
		ListofToolBarButton[i]->setEnabled(1);
	}
}
