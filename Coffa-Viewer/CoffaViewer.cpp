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
	mainFrame = new QFrame(this);
	QVBoxLayout* layout = new QVBoxLayout(mainFrame);
	layout->setMargin(0);
	mainFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	setCentralWidget(mainFrame);
	onSetPalette();
	setStyleSheet("QToolTip{background-color: white;border: 1px solid rgba(73, 84, 100,1); border-radius:4px}");


	//Embedding 3D Viewer
	aDoc = createNewDocument();

	myView = new View(aDoc->getContext(), mainFrame);
	layout->addWidget(myView);

	//To detect a shape is clicked
	connect(myView, SIGNAL(PartClicked(int)), this, SLOT(onShapeInViewClicked()));

	//To Activate or DeActivate Plate in Viewer
	connect(myView, SIGNAL(GridClick(int)), this, SLOT(ActivatetheGrid()));
	connect(myView, SIGNAL(GridClick2(int)), this, SLOT(DeActivatetheGrid()));

	createTools();
	fitAll();
	initializeAll();
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
		double moveX = (mainFrame->width() - 500) / 2;
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
	connect(ExportButton, SIGNAL(released()), this, SLOT(onExportDialog()));
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
	connect(MoveButton, SIGNAL(released()), this, SLOT(onShowMoveDialog()));

	QSize iconSize;
	iconSize.setWidth(45);
	iconSize.setHeight(45);
	mainToolBar = addToolBar("Tools");
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
	viewToolBar->setOrientation(Qt::Horizontal);
	viewToolBar->setFixedSize(600, 45);
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
	connect(action3, SIGNAL(triggered()), this, SLOT(onShowTranslateDialog()));

	QIcon iconex;
	iconex.addPixmap(QPixmap(QString::fromUtf8(":/CoffaViewer/Resources/export.png")), QIcon::Normal, QIcon::Off);
	QAction* actionexp = new QAction("Export", this);
	actionexp->setIcon(iconex);
	actionexp->setToolTip("Export");
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



///////////////////////
//Import - Export CAD//
///////////////////////
void CoffaViewer::onImportPart()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Import File"), "", 
		tr("All Files(*.step *.stp *.stl *.brep);; STL (*.stl);; STEP (*.step);; STP (*.stp);;BREP (*.brep)"));

	if (!fileName.isEmpty())
	{
		/*theNotifier = new NotificationWidget("Reading Shape Data...");
		theNotifier->setParent(this);
		theNotifier->move(this->width() - 370, this->height() - 170);
		theNotifier->show();*/
		QApplication::processEvents();
		aDoc->AddPart(fileName);

		//itemProject->appendRow(aDoc->getListOfShapes().last()->getTreeItem());
		//theNotifier->close();

	}
}

void CoffaViewer::onExport()
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
		QString BaseName = myFileInfo.baseName();

		if (!fileName.isNull())
		{
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
						aDoc->ExportFileBRep(aFile3, i);
					}
				}
			}
			ExportDialog->close();
		}
	}

	else
	{
		QToolTip::showText(goExportButton->mapToGlobal(QPoint(0, 0)), "Please select one part to export");
	}
}

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

void CoffaViewer::onRemovePart()
{
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


