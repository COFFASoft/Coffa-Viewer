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
#include <QtWidgets/QGroupBox>
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
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QTableWidget>


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
	void						createTranslationDialog();

public slots:
	void						onImportPart();
	void						onExport();
	void						onExportDialog();
	void						onRemovePart();

	void						onShapeInViewClicked();
	void						onExecuteSelection();

	void						onActivatethePlate();
	void						onDeActivatethePlate();

	void						onShowRotDialog();
	void						onRotate();

	void						onShowTrlDialog();
	void						onTranslate();

	void						onAxisChanged();

	void						onShowShapeProps();

protected:
	virtual void                    resizeEvent(QResizeEvent*);

private:
	Doc* aDoc; //Working Document
	View* myView; //OCCT 3D Viewer
	QFrame* mainFrame;
	QPalette thePalette; //Embeds the theme of the UI

	QWidget* shapeSelectionWidget;
	QVBoxLayout* shapeSelectionLayout;
	QScrollArea* shapeSelectionScroller;
	QList<QRadioButton*> radioButtonList; //For selecting a shape on the left side widget (shapeSelectionWidget)

	QToolBar* mainToolBar, *viewToolBar; //Tool bars
	bool viewToolsDone=false; //To check whether viewer tools have been built
	QList<QToolButton*> ListofToolBarButton;
	

	QDialog* ExportDialog; //Allows selecting a shape to export
	QList<QCheckBox*> ListofExportCH;
	QPushButton* goExportButton;

	QMenu* ShapeProp; //Menu of shape that shows up in the viewer
	QDialog* shapePropsDialog;
	QLabel* nameLabel;
	QLabel* areaLabel;
	QLabel* volumeLabel;
	QLabel* dimLabel;

	////Rotation and Translation Dialog Boxes////
	QDialog* RotDialog, * TrlDialog; 
	QDial* RxDial, * RyDial, * RzDial;
	double Rx, Ry, Rz;
	QDoubleSpinBox* TxSpin, * TySpin, * TzSpin;

	int theAxis1=1, theAxis2=2, theSequence=1;
};
