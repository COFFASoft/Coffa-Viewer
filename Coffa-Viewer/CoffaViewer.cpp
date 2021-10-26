#include "CoffaViewer.h"

static CoffaViewer* stApp = 0;
static QMdiArea* stWs = 0;

CoffaViewer::CoffaViewer(QWidget *parent)
    : QMainWindow(parent)
{
	QIcon iconApp;
	iconApp.addPixmap(QPixmap(QString::fromUtf8(":/Coffa-Viewer/Resources/AppCof.png")), QIcon::Normal, QIcon::Off);
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

	connect(myView, SIGNAL(PartClicked(int)), this, SLOT(getViewClicked()));
	connect(myView, SIGNAL(GridClick(int)), this, SLOT(ActivatetheGrid()));
	connect(myView, SIGNAL(GridClick2(int)), this, SLOT(DeActivatetheGrid()));
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


void CoffaViewer::createTabsOfTools()
{
}

void CoffaViewer::createViewActions()
{
}

void CoffaViewer::fitAll()
{
}

Doc* CoffaViewer::createNewDocument()
{
	return new Doc(this);
}

View* CoffaViewer::getViewer()
{
    return myView;
}

void CoffaViewer::viewPtBProp()
{
}
