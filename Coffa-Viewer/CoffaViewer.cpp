#include "CoffaViewer.h"


/////////////////////////////
//////  Main Window  ////////
/////////////////////////////
CoffaViewer::CoffaViewer(QWidget *parent)
    : QMainWindow(parent)
{
	QIcon iconApp;
	iconApp.addPixmap(QPixmap(QString::fromUtf8(":/CoffaViewer/Resources/AppCof.png")), QIcon::Normal, QIcon::Off);
	this->setWindowIcon(iconApp);
	setMinimumSize(600, 500);
	mainFrame = new QFrame(this);
	QVBoxLayout* layout = new QVBoxLayout(mainFrame);
	layout->setMargin(0);
	mainFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	mainFrame->setStyleSheet("QFrame{background-color:rgb(255,255,255);border:0px solid white}");
	setCentralWidget(mainFrame);
	onSetPalette();
	setStyleSheet("QToolTip{background-color: white;border: 1px solid rgba(73, 84, 100,1);border-radius:5px}");

	//Embedding 3D Viewer & Document
	aDoc = createNewDocument();
	myView = new View(aDoc->getContext(), mainFrame);
	layout->addWidget(myView);

	//This Widget will show the loaded shapes
	shapeSelectionWidget = new QWidget();
	shapeSelectionWidget->move(0, 0);
	shapeSelectionWidget->setFixedSize(300, 80);
	shapeSelectionWidget->setObjectName("shapeSelect");
	shapeSelectionWidget->setStyleSheet("QWidget#shapeSelect{background-color: transparent; margin:5px}");

	//QWidget* aWidget = new QWidget(shapeSelectionWidget); //intermediate widget to encapsulate the scroller 
	shapeSelectionLayout = new QVBoxLayout;
	shapeSelectionWidget->setLayout(shapeSelectionLayout);
	shapeSelectionScroller = new QScrollArea(mainFrame);
	shapeSelectionScroller->setGeometry(20, 30, 310, 100);
	shapeSelectionScroller->setAlignment(Qt::AlignRight);
	shapeSelectionScroller->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	shapeSelectionScroller->setWidgetResizable(true);
	shapeSelectionScroller->setStyleSheet("QScrollArea{background-color:rgb(245,245,245);border:1px solid rgb(60, 69, 82)}");//border:1px solid rgb(60, 69, 82)
	shapeSelectionScroller->setWidget(shapeSelectionWidget);
	shapeSelectionScroller->hide();
	//To Detect a Shape is clicked in Viewer
	connect(myView, SIGNAL(PartClicked(int)), this, SLOT(onShapeInViewClicked()));

	//To Activate or DeActivate Plate in Viewer
	connect(myView, SIGNAL(GridClick(int)), this, SLOT(onActivatethePlate()));
	connect(myView, SIGNAL(GridClick2(int)), this, SLOT(onDeActivatethePlate()));

	createTools();
	fitAll();
	initializeAll();
	setMinimumSize(500, 500);
}

void CoffaViewer::onSetPalette()
{
	QApplication::setStyle(QStyleFactory::create("Fusion"));
	QPalette thePalette = qApp->palette();
	thePalette.setColor(QPalette::Window, QColor(255, 255, 255/*244, 244, 242*/));
	thePalette.setColor(QPalette::WindowText, QColor(60, 69, 82));
	thePalette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(150, 150, 150));
	thePalette.setColor(QPalette::Base, QColor(232, 232, 232));//42 Color of some Widgets
	thePalette.setColor(QPalette::AlternateBase, QColor(140, 140, 140));//Not much important
	thePalette.setColor(QPalette::ToolTipBase, QColor(75, 75, 75));
	thePalette.setColor(QPalette::ToolTipText, QColor(60, 69, 82));
	thePalette.setColor(QPalette::Text, QColor(60, 69, 82));
	thePalette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
	thePalette.setColor(QPalette::Dark, QColor(55, 55, 55));
	thePalette.setColor(QPalette::Shadow, QColor(60, 60, 60));

	thePalette.setColor(QPalette::Highlight, QColor(96, 12, 252));
	thePalette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
	thePalette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
	thePalette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(127, 127, 127));

	thePalette.setColor(QPalette::ButtonText, QColor(60, 69, 82));
	thePalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
	thePalette.setColor(QPalette::BrightText, Qt::red);
	thePalette.setColor(QPalette::Link, QColor(42, 130, 218));


	setStyleSheet("QMenu{background-color: white; border:1px solid rgba(73, 84, 100,1);border-radius:4px}");
	setStyleSheet("QToolTip{background-color: white;border: 1px solid rgba(73, 84, 100,1); border-radius:4px}");

	qApp->setPalette(thePalette);

}

void CoffaViewer::resizeEvent(QResizeEvent*)
{
	if (viewToolsDone)
	{
		double moveX = (mainFrame->width() - 60);
		viewToolBar->move(moveX, 30);
		//viewToolBar->setFixedSize(mainFrame->width(), 30);
	}
}

void CoffaViewer::createTools()
{
	QIcon iconLoad;
	iconLoad.addPixmap(QPixmap(QString::fromUtf8(":/CoffaViewer/Resources/import.png")), QIcon::Normal, QIcon::Off);
	QToolButton* ImportButton = new QToolButton(this);
	ImportButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	ImportButton->setIcon(iconLoad);
	ImportButton->setText("Import\nPart");
	ImportButton->setToolTip(QObject::tr("Import Part"));
	connect(ImportButton, SIGNAL(released()), this, SLOT(onImportPart()));
	ListofToolBarButton.push_back(ImportButton);

	QIcon iconExport;
	iconExport.addPixmap(QPixmap(QString::fromUtf8(":/CoffaViewer/Resources/export.png")), QIcon::Normal, QIcon::Off);
	QToolButton* ExportButton = new QToolButton();
	ExportButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	ExportButton->setIcon(iconExport);
	ExportButton->setText("Export\nPart");
	ExportButton->setToolTip("Export the processed part");
	connect(ExportButton, SIGNAL(released()), this, SLOT(onExport()));
	ListofToolBarButton.push_back(ExportButton);

	QIcon iconAbout;
	iconAbout.addPixmap(QPixmap(QString::fromUtf8(":/CoffaViewer/Resources/about.png")), QIcon::Normal, QIcon::Off);
	QToolButton* AboutButton = new QToolButton();
	AboutButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	AboutButton->setIcon(iconAbout);
	AboutButton->setText("About\nCOFFA Viewer");
	AboutButton->setToolTip("About COFFA Viewer");
	connect(AboutButton, SIGNAL(released()), this, SLOT(onAbout()));

	QIcon iconRot;
	iconRot.addPixmap(QPixmap(QString::fromUtf8(":/CoffaViewer/Resources/orient.png")), QIcon::Normal, QIcon::Off);
	QToolButton* RotButton = new QToolButton();
	RotButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	RotButton->setIcon(iconRot);
	RotButton->setText("Rotate\nPart");
	RotButton->setToolTip("Rotate Part");
	connect(RotButton, SIGNAL(released()), this, SLOT(onShowRotDialog()));

	QIcon iconMove;
	iconMove.addPixmap(QPixmap(QString::fromUtf8(":/CoffaViewer/Resources/move.png")), QIcon::Normal, QIcon::Off);
	QToolButton* MoveButton = new QToolButton();
	MoveButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	MoveButton->setIcon(iconMove);
	MoveButton->setText("Move\nPart");
	MoveButton->setToolTip("Move Part");
	connect(MoveButton, SIGNAL(released()), this, SLOT(onShowTrlDialog()));

	QSize iconSize;
	iconSize.setWidth(45);
	iconSize.setHeight(45);
	mainToolBar = addToolBar("Tools");
	mainToolBar->setStyleSheet("QToolBar{background-color:rgb(255,255,255);border:0px solid white}");
	mainToolBar->setIconSize(iconSize);
	mainToolBar->addWidget(ImportButton);
	mainToolBar->addWidget(ExportButton);
	mainToolBar->addWidget(AboutButton);
	mainToolBar->addSeparator();
	mainToolBar->addWidget(RotButton);
	mainToolBar->addWidget(MoveButton);
}

void CoffaViewer::createViewActions()
{
	viewToolBar = new QToolBar(mainFrame);
	viewToolBar->setOrientation(Qt::Vertical);
	viewToolBar->setFixedSize(45, 600);
	viewToolBar->setMovable(0);
	QSize asi;
	asi.setHeight(35);
	asi.setWidth(35);
	viewToolBar->setIconSize(asi);

	QList<QAction*>* aList = myView->getViewActions();

	viewToolBar->addActions(*aList);
	viewToolBar->setStyleSheet("QToolBar{background: rgba(255, 255, 255, 1);}");
	viewToolBar->toggleViewAction()->setVisible(true);

	viewToolsDone = true;
}

void CoffaViewer::initializeAll()
{
	createViewActions();
	createRotationDialog();
	createTranslationDialog();
	//createShapePropsDialog();

	ShapeProp = new QMenu(this);

	QIcon iconrem;
	iconrem.addPixmap(QPixmap(QString::fromUtf8(":/CoffaViewer/Resources/delete.png")), QIcon::Normal, QIcon::Off);
	QAction* action1 = new QAction("Remove", this);
	action1->setIcon(iconrem);
	action1->setToolTip("Remove");
	action1->setShortcut(QObject::tr("DELETE"));
	connect(action1, SIGNAL(triggered()), this, SLOT(onRemovePart()));

	QIcon iconr;
	iconr.addPixmap(QPixmap(QString::fromUtf8(":/CoffaViewer/Resources/orient.png")), QIcon::Normal, QIcon::Off);
	QAction* action2 = new QAction("Rotate", this);
	action2->setIcon(iconr);
	action2->setToolTip("Rotate");
	connect(action2, SIGNAL(triggered()), this, SLOT(onShowRotDialog()));

	QIcon icont;
	icont.addPixmap(QPixmap(QString::fromUtf8(":/CoffaViewer/Resources/move.png")), QIcon::Normal, QIcon::Off);
	QAction* action3 = new QAction("Move", this);
	action3->setIcon(icont);
	action3->setToolTip("Move");
	connect(action3, SIGNAL(triggered()), this, SLOT(onShowTrlDialog()));

	QIcon iconex;
	iconex.addPixmap(QPixmap(QString::fromUtf8(":/CoffaViewer/Resources/export.png")), QIcon::Normal, QIcon::Off);
	QAction* actionexp = new QAction("Export", this);
	actionexp->setIcon(iconex);
	actionexp->setToolTip("Export");
	connect(actionexp, SIGNAL(triggered()), this, SLOT(onExport()));

	QIcon iconinfo;
	iconinfo.addPixmap(QPixmap(QString::fromUtf8(":/CoffaViewer/Resources/shapeInfo.png")), QIcon::Normal, QIcon::Off);
	QAction* actionInfo = new QAction("Properties", this);
	actionInfo->setIcon(iconinfo);
	actionInfo->setToolTip("Shape Properties");
	connect(actionInfo, SIGNAL(triggered()), this, SLOT(onShowShapeProps()));

	QToolBar* toolbarl = new QToolBar;
	
	toolbarl->addAction(action2);
	toolbarl->addAction(action3);
	toolbarl->addSeparator();
	toolbarl->addAction(actionInfo);
	toolbarl->addSeparator();
	toolbarl->addAction(actionexp);
	toolbarl->addSeparator();
	toolbarl->addAction(action1);
	

	QWidgetAction* act = new QWidgetAction(this);
	act->setDefaultWidget(toolbarl);
	ShapeProp->addAction(act);
	ShapeProp->hide();

}



///////////////////////
/////  Viewer  ////////
///////////////////////
//Fits the View to Screen
void CoffaViewer::fitAll()
{
	myView->fitAll();
}

Doc* CoffaViewer::createNewDocument()
{
	return new Doc(this);
}

//Returns the Viewer
View* CoffaViewer::getViewer()
{
    return myView;
}

//Catches Clicks from the 3D Viewer
void CoffaViewer::onShapeInViewClicked()
{
	//This delay has been added to let the Viewer have enough time to handle the selection
	QTimer::singleShot(50, this, SLOT(onExecuteSelection()));
}

//Seeks wether a shape is selected and calls its menu (ShapeProp)
void CoffaViewer::onExecuteSelection()
{
	bool b = false;
	for (int i = 0; i < aDoc->getListOfShapes().size(); i++)
	{
		if (aDoc->getListOfShapes()[i]->isSelected())
		{
			aDoc->Shapeid = i;

			if (!radioButtonList.isEmpty())
			{
				radioButtonList[i]->setChecked(true);
			}

			b = true;
			break;
		}
	}

	if (b)
	{
		//Display Shape Menu
		QScreen* screen0 = QApplication::screens().at(0);
		QPoint pos0 = QCursor::pos(screen0);
		ShapeProp->exec(pos0);
	}
}

//Activates the plate for 3D Printing Reference
void CoffaViewer::onActivatethePlate()
{
	aDoc->PlatformOn();
	aDoc->PlatformVisible = true;
}

//Deactivates the plate for 3D Printing Reference
void CoffaViewer::onDeActivatethePlate()
{
	aDoc->PlatformOff();
	aDoc->PlatformVisible = false;
}



///////////////////////
//Import - Export CAD//
///////////////////////
void CoffaViewer::onImportPart()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Import File"), "", 
		tr("All Files(*.step *.stp *.stl *.brep);; STL (*.stl);; STEP (*.step);; STP (*.stp);;BREP (*.brep)"));

	if (!fileName.isEmpty())
	{
		QApplication::processEvents();
		aDoc->AddPart(fileName);
		aDoc->Shapeid = aDoc->getListOfShapes().size() - 1;
		shapeSelectionScroller->show();

		//Add it top the shape selection widget on the left side
		QRadioButton *aButton = new QRadioButton(aDoc->getListOfShapes().last()->getName());
		shapeSelectionLayout->addWidget(aButton, 0, Qt::AlignLeft);
		radioButtonList.push_back(aButton);
		aButton->setChecked(1);

		int newHeight = 60 + (aDoc->getListOfShapes().size() * 30);
		shapeSelectionWidget->setFixedHeight(newHeight);
		if (newHeight < 450)
			shapeSelectionScroller->setFixedHeight(newHeight + 10);
	}
}

//Allows Selection of a part to export
void CoffaViewer::onExportDialog()
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
			QCheckBox* aCheck = new QCheckBox(aDoc->getListOfShapes().at(i)->getName());
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

void CoffaViewer::onExport()
{
	const char* aFile;
	ShapeProp->hide();

	int ind = aDoc->Shapeid;

	if (ind != -1 && ind < aDoc->getListOfShapes().size() && !radioButtonList.isEmpty())
	{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Export File"), "", tr(" STL (*.stl);; STEP (*.step);;BREP (*.brep)"));
		QFileInfo myFileInfo(fileName);
		QString aSuffix = myFileInfo.suffix();
		QString BaseName = myFileInfo.baseName();

		if (!fileName.isNull())
		{
			if (aSuffix == "stl")
			{
				QByteArray ba = fileName.toLatin1();
				aFile = ba.data();
				aDoc->ExportFileSTL(aFile, ind);
			}

			else if (aSuffix == "step")
			{
				QByteArray ba = fileName.toLatin1();
				aFile = ba.data();
				aDoc->ExportFileSTEP(aFile, ind);
			}

			else if (aSuffix == "brep")
			{
				QByteArray ba = fileName.toLatin1();
				aFile = ba.data();
				aDoc->ExportFileBRep(aFile, ind);
			}
		}
	}

	else
	{
		QToolTip::showText(goExportButton->mapToGlobal(QPoint(0, 0)), "Please select one part to export");
	}
}

void CoffaViewer::onRemovePart()
{
	ShapeProp->hide();

	int ind = aDoc->Shapeid;

	if (ind != -1 && ind < aDoc->getListOfShapes().size() && !radioButtonList.isEmpty())
	{
		shapeSelectionLayout->removeWidget(radioButtonList[ind]);
		delete radioButtonList[ind];
		radioButtonList.removeAt(ind);
		
		int newHeight = 60 + ((aDoc->getListOfShapes().size()-1) * 30);
		shapeSelectionWidget->setFixedHeight(newHeight);
		if (newHeight < 450)
			shapeSelectionScroller->setFixedHeight(newHeight + 10);

	}

	if (!radioButtonList.isEmpty())
	{
		radioButtonList[0]->setChecked(true);
	}

	else
	{
		shapeSelectionScroller->hide();
	}

	aDoc->RemovePart();
}



///////////////////////
//Rotate & Translate //
///////////////////////
void CoffaViewer::createRotationDialog()
{
	QFont aFont;
	aFont.setPointSize(8);
	aFont.setBold(1);

	RotDialog = new QDialog(this);
	RotDialog->setParent(this);
	RotDialog->setWindowTitle("Rotate");
	RotDialog->setFixedSize(400, 400);
	
	/*QGroupBox* firtsAxWidgets = new QGroupBox(RotDialog);
	firtsAxWidgets->setTitle("First Axis");
	firtsAxWidgets->move(10, 10);
	firtsAxWidgets->setFixedSize(380, 80);
	QHBoxLayout* firstAxLayout = new QHBoxLayout;
	firtsAxWidgets->setLayout(firstAxLayout);

	QGroupBox* secAxWidgets = new QGroupBox(RotDialog);
	secAxWidgets->setTitle("Second Axis");
	secAxWidgets->move(10, 100);
	secAxWidgets->setFixedSize(380, 80);
	QHBoxLayout* secAxLayout = new QHBoxLayout;
	secAxWidgets->setLayout(secAxLayout);

	QRadioButton* xax1 = new QRadioButton("X");
	xax1->setChecked(1);
	connect(xax1, &QRadioButton::clicked, [=]() {
		theAxis1 = 1;
		onAxisChanged();
	});
	firstAxLayout->addWidget(xax1);

	QRadioButton* yax1 = new QRadioButton("Y");
	connect(yax1, &QRadioButton::clicked, [=]() {
		theAxis1 = 2;
		onAxisChanged();
	});
	firstAxLayout->addWidget(yax1);

	QRadioButton *zax1 = new QRadioButton("Z");
	connect(zax1, &QRadioButton::clicked, [=]() {
		theAxis1 = 3;
		onAxisChanged();
	});
	firstAxLayout->addWidget(zax1);

	QRadioButton* xax2 = new QRadioButton("X");
	connect(xax2, &QRadioButton::clicked, [=]() {
		theAxis2 = 1;
		onAxisChanged();
	});
	secAxLayout->addWidget(xax2);

	QRadioButton* yax2 = new QRadioButton("Y");
	yax2->setChecked(1);
	connect(yax2, &QRadioButton::toggled, [=]() {
		theAxis2 = 2;
		onAxisChanged();
	});
	secAxLayout->addWidget(yax2);

	QRadioButton* zax2 = new QRadioButton("Z");
	connect(zax2, &QRadioButton::clicked, [=]() {
		theAxis2 = 3;
		onAxisChanged();
	});
	secAxLayout->addWidget(zax2);*/


	QGroupBox* RotationCtrlWid = new QGroupBox(RotDialog);
	RotationCtrlWid->setTitle("Rotation Control");
	RotationCtrlWid->move(10, 10);
	RotationCtrlWid->setFixedSize(380, 150);
	QHBoxLayout* rotCtrlLayout = new QHBoxLayout;
	RotationCtrlWid->setLayout(rotCtrlLayout);

	QWidget* widX = new QWidget;
	widX->setFixedSize(100, 100);

	QLabel* label1 = new QLabel("X Axis", widX);
	label1->setAlignment(Qt::AlignCenter);
	label1->setFixedSize(100, 20);
	label1->move(0, 0);
	RxDial = new QDial(widX);
	RxDial->setFixedSize(80, 80);
	RxDial->move(0, 20);
	RxDial->setMaximum(360);
	RxDial->setMinimum(0);
	RxDial->setSingleStep(1.0);
	RxDial->setPageStep(45);
	RxDial->setNotchesVisible(1);
	RxDial->setNotchTarget(45);
	RxDial->setTracking(1);

	QLabel* XDLabel = new QLabel("0", RxDial);
	XDLabel->setFixedSize(40, 25);
	XDLabel->move(20, 27.5);
	XDLabel->setAlignment(Qt::AlignCenter);
	XDLabel->setFont(aFont);
	XDLabel->setStyleSheet("QLabel {background : transparent; }");
	XDLabel->show();
	connect(RxDial, QOverload<int>::of(&QDial::valueChanged), [=](double value) {
		XDLabel->setText(QString::number(RxDial->value()));
	});
	connect(RxDial, SIGNAL(sliderReleased()), this, SLOT(onRotate()));
	rotCtrlLayout->addWidget(widX, Qt::AlignCenter);

	QWidget* widY = new QWidget;
	widY->setFixedSize(100, 100);
	QVBoxLayout* vlayY = new QVBoxLayout;
	widY->setLayout(vlayY);

	QLabel* label2 = new QLabel("Y Axis", widY);
	label2->setAlignment(Qt::AlignCenter);
	label2->setFixedSize(100, 20);
	label2->move(0, 0);
	RyDial = new QDial(widY);
	RyDial->setFixedSize(80, 80);
	RyDial->move(0, 20);
	RyDial->setMaximum(360);
	RyDial->setMinimum(0);
	RyDial->setSingleStep(1.0);
	RyDial->setPageStep(45);
	RyDial->setNotchesVisible(1);
	RyDial->setNotchTarget(45);

	QLabel* YDLabel = new QLabel("0", RyDial);
	YDLabel->setFixedSize(40, 25);
	YDLabel->move(20, 27.5);
	YDLabel->setAlignment(Qt::AlignCenter);
	YDLabel->setFont(aFont);
	YDLabel->setStyleSheet("QLabel {background : transparent; border-radius:5px;}");
	YDLabel->show();
	connect(RyDial, QOverload<int>::of(&QDial::valueChanged), [=](double value) {
		YDLabel->setText(QString::number(RyDial->value()));
	});
	connect(RyDial, SIGNAL(sliderReleased()), this, SLOT(onRotate()));
	rotCtrlLayout->addWidget(widY, Qt::AlignCenter);


	QWidget* widZ = new QWidget;
	widZ->setFixedSize(100, 100);
	QVBoxLayout* vlayZ = new QVBoxLayout;
	widZ->setLayout(vlayZ);

	QLabel* label3 = new QLabel("Z", widZ);
	label3->setAlignment(Qt::AlignHCenter);
	label3->setFixedSize(100, 20);
	label3->move(0, 0);
	RzDial = new QDial(widZ);
	RzDial->setFixedSize(80, 80);
	RzDial->move(0, 20);
	RzDial->setMaximum(360);
	RzDial->setMinimum(0);
	RzDial->setSingleStep(1.0);
	RzDial->setPageStep(45);
	RzDial->setNotchesVisible(1);
	RzDial->setNotchTarget(45);

	QLabel* ZDLabel = new QLabel("0", RzDial);
	ZDLabel->setFixedSize(40, 25);
	ZDLabel->move(20, 27.5);
	ZDLabel->setAlignment(Qt::AlignCenter);
	ZDLabel->setFont(aFont);
	ZDLabel->setStyleSheet("QLabel {background : transparent; border-radius:5px;}");
	ZDLabel->show();
	connect(RzDial, QOverload<int>::of(&QDial::valueChanged), [=](double value) {
		ZDLabel->setText(QString::number(RzDial->value()));
	});
	connect(RzDial, SIGNAL(sliderReleased()), this, SLOT(onRotate()));
	rotCtrlLayout->addWidget(widZ, Qt::AlignCenter);

	RotDialog->hide();

}

void CoffaViewer::onShowRotDialog()
{
	if (aDoc->Shapeid > -1)
	{
		RxDial->setValue(aDoc->getListOfShapes()[aDoc->Shapeid]->currentRx());
		RyDial->setValue(aDoc->getListOfShapes()[aDoc->Shapeid]->currentRy());
		RzDial->setValue(aDoc->getListOfShapes()[aDoc->Shapeid]->currentRz());
	}
	
	RotDialog->show();
}

void CoffaViewer::onRotate()
{
	if (aDoc->getListOfShapes().size() != 0)
	{
		aDoc->rotateCurrentShape(theSequence, RxDial->value(), RyDial->value(), RzDial->value());
	}
}

void CoffaViewer::createTranslationDialog()
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

	QPushButton* OkButtonTrl = new QPushButton("Apply", TrlDialog);
	OkButtonTrl->setFixedSize(60, 30);
	OkButtonTrl->move(120, 120);
	OkButtonTrl->setShortcut(QObject::tr("Apply"));
	connect(OkButtonTrl, SIGNAL(released()), this, SLOT(onTranslate()));

	TrlDialog->hide();
}

void CoffaViewer::onShowTrlDialog()
{
	TrlDialog->show();
	ShapeProp->hide();
	if (!aDoc->getListOfShapes().isEmpty() && aDoc->Shapeid < aDoc->getListOfShapes().size())
	{
		TxSpin->setValue(aDoc->getListOfShapes()[aDoc->Shapeid]->Tx);
		TySpin->setValue(aDoc->getListOfShapes()[aDoc->Shapeid]->Ty);
		TzSpin->setValue(aDoc->getListOfShapes()[aDoc->Shapeid]->Tz);
	}
}

void CoffaViewer::onTranslate()
{
	if (aDoc->getListOfShapes().size() != 0)
	{
		aDoc->getListOfShapes()[aDoc->Shapeid]->translatePart(TxSpin->value(), TySpin->value(), TzSpin->value());
	}
}

void CoffaViewer::onAxisChanged()
{
	//The rotation Axes must not be the same
	if (theAxis1 == theAxis2)
	{
		QMessageBox* msgBox = new QMessageBox(this);
		msgBox->setText("Please do not select the same axis for the first and second rotations");
		msgBox->setInformativeText("If you leave this setting, the default\n Sequence XYZ will be applied ");
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


////////////////////////
///Access Shape Props///
////////////////////////
void CoffaViewer::onShowShapeProps()
{
	shapePropsDialog = new QDialog(this);
	shapePropsDialog->setFixedSize(400, 250);

	QStringList Vheaders;
	Vheaders.append("Name");
	Vheaders.append("Center");
	Vheaders.append("Area");
	Vheaders.append("Volume");
	Vheaders.append("Dimensions");
	Vheaders.append("Face Count");
	QTableWidget* myTab = new QTableWidget(shapePropsDialog);
	myTab->move(10, 10);
	myTab->setRowCount(6);
	myTab->setColumnCount(1);
	myTab->setColumnWidth(0, 290);
	myTab->setVerticalHeaderLabels(Vheaders);
	myTab->setFixedWidth(380);
	myTab->setFixedHeight(35*6 + 25);
	myTab->setShowGrid(true);

	QTableWidgetItem* item1 = new QTableWidgetItem(aDoc->getListOfShapes().at(aDoc->Shapeid)->getName());
	myTab->setItem(0, 0, item1);
	myTab->setRowHeight(0, 35);

	gp_Pnt p = aDoc->getListOfShapes().at(aDoc->Shapeid)->getCenter();

	QTableWidgetItem* item2 = new QTableWidgetItem("X: "+QString::number(p.X())+
		"  | Y: " + QString::number(p.Y()) + " | Z: " + QString::number(p.Z()));
	myTab->setItem(1, 0, item2);
	myTab->setRowHeight(1, 35);

	QTableWidgetItem* item3 = new QTableWidgetItem(QString::number(aDoc->getListOfShapes().at(aDoc->Shapeid)->getTotalArea()));
	myTab->setItem(2, 0, item3);
	myTab->setRowHeight(2, 35);

	QTableWidgetItem* item4 = new QTableWidgetItem(QString::number(aDoc->getListOfShapes().at(aDoc->Shapeid)->getVolume()));
	myTab->setItem(3, 0, item4);
	myTab->setRowHeight(3, 35);

	double dimX = aDoc->getListOfShapes().at(aDoc->Shapeid)->getXDim();
	double dimY = aDoc->getListOfShapes().at(aDoc->Shapeid)->getYDim();
	double dimZ = aDoc->getListOfShapes().at(aDoc->Shapeid)->getZDim();

	QTableWidgetItem* item5 = new QTableWidgetItem("X: "+QString::number(dimX)+
		"  | Y: " + QString::number(dimY) +"  | Z: " + QString::number(dimZ));
	myTab->setItem(4, 0, item5);
	myTab->setRowHeight(4, 35);

	QTableWidgetItem* item6 = new QTableWidgetItem(QString::number(aDoc->getListOfShapes().at(aDoc->Shapeid)->getFaceCount()));
	myTab->setItem(5, 0, item6);
	myTab->setRowHeight(5, 35);

	shapePropsDialog->show();
}