#pragma once

//#include "Doc.h"
//#include "View.h"

//Opencascade
//#include <StlAPI.hxx>
//#include <StlAPI_Reader.hxx>
//#include <RWStl.hxx>
//#include <TopoDS_Shape.hxx>


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
#include <QtDataVisualization/qdatavisualizationglobal.h>
#include <QtCharts/qbarseries.h>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLineSeries>
#include <QtWidgets/QListView>
#include <QtWidgets/qtoolbutton.h>
#include <QtWidgets/QRadioButton>
#include <QtWidgets\qcombobox.h>
#include <QtGui\QStandardItem>
#include <QtGui\QStandardItemModel>
#include <QtWidgets\QTreeView>
#include <QtCore/QStringListModel>
#include <QtWidgets/qdial.h>
#include <QtCharts/QChartView>


#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonParseError>
#include <QtCore/QJsonValue>


class CoffaViewer : public QMainWindow
{
    Q_OBJECT

public:
    CoffaViewer(QWidget *parent = Q_NULLPTR);
	void onSetPalette();
	void createTabsOfTools();

	void						ReadProject(QString aProj);
	void						createViewActions();
	void						fitAll();
	//View*						getViewer();

	void						viewPtBProp();

private:
	QFrame* mainFrame;
	QPalette thePalette;
};
