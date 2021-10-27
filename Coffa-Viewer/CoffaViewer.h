#pragma once

#include "Doc.h"
#include "View.h"
#include "InteractiveShape.h"

//Opencascade
#include <StlAPI.hxx>
#include <StlAPI_Reader.hxx>
#include <RWStl.hxx>
#include <TopoDS_Shape.hxx>


//Qt
#include <QtWidgets/QStyleFactory>
#include <QtWidgets/QApplication>

#include <stdio.h>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMdiArea>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMenu>
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QFile>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QToolTip>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressDialog>
#include <QtCore/QTimer>
#include <QtGui/QIcon>
#include <QtWidgets/QToolBox>
#include <QtWidgets/QTabWidget>
#include <QtGui/qevent.h>
#include <QtWidgets/QSlider>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QVBoxlayout>
#include <QtWidgets/QStackedLayout>
#include <QtWidgets/QListView>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QComboBox>
#include <QtGui/QStandardItem>
#include <QtGui/QStandardItemModel>
#include <QtWidgets/QTreeView>
#include <QtCore/QStringListModel>
#include <QtWidgets/QDial>
#include <QtWidgets/QWidgetAction>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QMessageBox>


class CoffaViewer : public QMainWindow
{
    Q_OBJECT

public:
    CoffaViewer(QWidget *parent = Q_NULLPTR);
	void onSetPalette();
	void createTools();

	virtual Doc*				createNewDocument();
	View*						getViewer();
	void						createViewActions();
	void						initializeAll();
	void						fitAll();

	void						createRotationDialog();

	

public slots:
	void						onImportPart();
	void						onExport();
	void						onExportDialog();
	void						onRemovePart();

	void						onShapeInViewClicked();
	void						onExecuteSelection();

	void						onShowRotDialog();
	void						onRotate();

	void						onAxisChanged();

protected:
	virtual void                    resizeEvent(QResizeEvent*);

private:
	Doc* aDoc;
	View* myView;
	QFrame* mainFrame;
	QPalette thePalette;
	QToolBar* mainToolBar, *viewToolBar;
	bool viewToolsDone=false;
	QList<QToolButton*> ListofToolBarButton;
	QList<QCheckBox*> ListofExportCH;

	QDialog* ExportDialog;
	QPushButton* goExportButton;

	QMenu* ShapeProp;

	QDialog* RotDialog, * MoveDialog;

	QDial* RxDial, * RyDial, * RzDial;
	double Rx, Ry, Rz;
	QDoubleSpinBox* TxSpin, * TySpin, * TzSpin;

	int theAxis1=1, theAxis2=2, theSequence=1;
};
