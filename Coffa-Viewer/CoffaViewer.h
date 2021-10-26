#pragma once

#include "Doc.h"
#include "View.h"

//Opencascade
#include <StlAPI.hxx>
#include <StlAPI_Reader.hxx>
#include <RWStl.hxx>
#include <TopoDS_Shape.hxx>


//Qt
#include <QtWidgets\QStyleFactory>
#include <QtWidgets/QApplication>

#include <stdio.h>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMdiArea>
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QFile>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/qpushbutton.h>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressDialog>
#include <QtCore/QTimer>
#include <QtGui/QIcon>
#include <QtWidgets/QToolBox>
#include <QtWidgets/QTabWidget>
#include <QtGui/qevent.h>
#include <QtWidgets/qslider.h>
#include <QtDataVisualization/q3dsurface.h>
#include <QtWidgets/qscrollarea.h>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QVBoxlayout>
#include <QtWidgets/QStackedLayout>
#include <QtWidgets/QListView>
#include <QtWidgets/qtoolbutton.h>
#include <QtWidgets/QRadioButton>
#include <QtWidgets\qcombobox.h>
#include <QtGui\QStandardItem>
#include <QtGui\QStandardItemModel>
#include <QtWidgets\QTreeView>
#include <QtCore/QStringListModel>
#include <QtWidgets/qdial.h>


class CoffaViewer : public QMainWindow
{
    Q_OBJECT

public:
    CoffaViewer(QWidget *parent = Q_NULLPTR);
	void onSetPalette();
	void createTabsOfTools();

	virtual Doc*				createNewDocument();
	View*						getViewer();
	void						createViewActions();
	void						fitAll();
	

	void						viewPtBProp();

private:
	Doc* aDoc;
	View* myView;
	QFrame* mainFrame;
	QPalette thePalette;
};
