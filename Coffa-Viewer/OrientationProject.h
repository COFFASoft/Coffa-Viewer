
#ifndef OrientationProject_H
#define OrientationProject_H

#include "Doc.h"
#include "View.h"
#include "Coffa_MainWindow.h"
#include "ActionRule.h"
#include "VarAttribute.h"
#include "NotificationWidget.h"

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
#include <AIS_InteractiveContext.hxx>
#include <V3d_Viewer.hxx>
#include <QtGui/QIcon>
#include <Graphic3d_GraphicDriver.hxx>
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
#include <TopoDS_Shape.hxx>
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


using namespace QtDataVisualization;
using namespace QtCharts;
class Coffa_MainWindow;
//class View;

class OrientationProject : public QMainWindow
{
	Q_OBJECT

public:
	enum { FileNewId, FileExportId, ImportXPTId, FileXCADId, FileCloseId, FileQuitId, ViewToolId, HelpAboutId, CompId, ShowRespId, CustOrId, CustTrId, ViewStatusId };

	OrientationProject(QWidget* parent, Coffa_MainWindow* ParentApp, QString aProjectName, QString aFileName, QString aFilePath, Qt::WindowFlags wflags);
	OrientationProject(QWidget* parent, Coffa_MainWindow* ParentApp, QString aProjectName, QString aFileName, Qt::WindowFlags wflags, int aMode);

	void createTabsOfTools();

	void						ReadProject(QString aProj);
	void						createViewActions();
	void						fitAll();
	View* getViewer();
	QString						getResourceDirectory();
	void						viewPtBProp();

	static OrientationProject* getApplication();
	static QMdiArea* getWorkspace();
	virtual Doc* createNewDocument();
	void						LeftTree();

	void						printerSettingTools();

	
	//Analysis
	void						onAnalysisTools();



	void createWeightingWidget();

	void createBarsWidget();

	void createThresholdingWidget();

	TCollection_AsciiString				aFile;
	Doc* aDoc;

	QList<QList<QList<QPointF>>> ValueDesiList;

	QList<double>		WeightValues;
	QList<QList<int>>* RangeValues;
	QList<QCheckBox*> ChoiceCheckList;
	Qt::CheckState m_chek;
	QList<QString> ARNameList, ActionRuleList, VariableList, AttributeList, AttributeTypeList, ShapeTypeList;
	

	QList<QStringList>* ArStringLists;
	bool				ExpertFileExist;
	bool				EnvironementExist;
	int nExpertFile;
	QList<Q3DSurface*>* ListOf3DSurfaces;
	QList<QWidget*> ListOf3DSurfWidget;
	QDockWidget* modifDockWidget;

	int					theAxis1 = 1;
	int					theAxis2 = 2;
	int					theSequence = 1;
	double				theStep1 = 5;
	double				theStep2 = 10;

	bool EntitySelectionMode;
	QLabel* ProgressLabel;
	QPushButton* ProgressCancelB;

	QList<ActionRule*> theRules;
protected:
	virtual void                    resizeEvent(QResizeEvent*);

private:
	int					   		  myNbDocuments;

	void                            ProjectToolBar();
	void							ProjectMenuBar();
	QList<QAction*>                 myToolActions;
	QList<QAction*>                 myMaterialActions;
	QList<QAction*>                 myStdActions;
	QList<QAction*>                 myProActions;
	QMenu* myFilePopup;
	QAction* myFileSeparator;
	QToolBar* myStdToolBar, *printToolBar, * orientToolbar, * viewToolBar, * suppToolBar;
	QToolBar* mySupportBar;
	QToolBar* myCasCadeBar;
	QMenu* myWindowPopup;

	double							EstimTime;

public slots:
	void							onInit();
	void							onHideLeftWidget();
	void							onShowLeftWidget();

	void							onShowPrinterSettings();
	void							onShowOrientSettings();

	void onBackHome();

	void							onShowAnalysisTools();
	void							onUpdateSelectedAnalysisMode();
	void createRadarWidget();
	void onShowRadarWidget();
	void							onShowWeigthingWidget();
	void							onShowBarsWidget();
	void							onShowThresholdingWidget();

	void onMakeAttributeWidget();

	void onShowAttributeWidget();

	void							onAddPartToPlate();
	void							onMakeAttributeDialog();
	void							onSaveFaceSelection();
	void onCancelFaceSelection();
	void							onRemovePartFromPlate();
	void							onUpdateSelectedPart();

	void onUpdateSelectedPart2();

	void onUpdateSelectedPart3();

	void							onPlatformUpdate();
	void							onFitnessChecker();

	void							onGenRTS();
	void							onGenSupport();

	void							onShowRotateDialog();
	void							onRotatePressed();
	void createTranslationDialog();
	void							onShowTranslateDialog();
	void							onTranslatePressed();

	void						onUpdateLeftTree();

	void                            ActivatetheGrid();
	void							DeActivatetheGrid();
	void							getViewClicked();

	void executeSelectionMode();

	void OrientationSettingTools();
	
	void SupportSettingTools();

	void onShowSupportSettings();

	void onAxisChanged();
	void onRestoreAxes();

	virtual void                    onAbout();
	virtual void					onCloseWindow();
	void							onViewStatusBar();
	void						onCriteriaComputation();
	void						onShowComputeForOnePos();
	void						onComputeForOnePos();


	void						onWeightersChange();
	void onBarsDisplay();
	void onRadarDisplay();
	void onAggregateValues();
	void						onDisplayResponseWidget();
	void						onSingleResponseTools();
	void onCloseResponseWindow();
	void						generateResponses();
	void						onUpdateVisualized();
	void onUpdateVisualized2();
	void						onCreateVisualizationDialog();
	void onShowVisualizationDialog();

	void						onHandler(QPoint aP);
	void						onHandler2();

	void onChartPointPress(int cx, int cy);

	void					addFaceTo();
	void					addAllFaceTo();
	void					RemoveFaceFrom();
	void					RemoveFaceFrom2();

	void RemoveAllFaceFrom();

	void createRotationDialog();



	void					onExport();
	void					onExportDialog();

	void onSaveProject();

	void					ActivateButtons();

	

	void					onColorMapSup();

	QList<ActionRule*> getActionRules();

	void					onRemoveExpertFile();
	void createARVizualizer();
	void					onARVizualizer();

	void onSaveAR();

	void onCancelAR();

	void			onLoadExpertFileJSON(QJsonArray anArray);
	void			onLoadExpertFileJSON();
	void			onAddEntitiesToRule();
	void			onActionRuleSettings();
	void			onSurfClickedProp();
	void			onSurfClicked();
	void onDisplayEntities();
	void onHideEntities();
	void			myTreeClicked(QModelIndex index);
	void			myTreeDoubleClicked(QModelIndex index);
	void					onStartComputation();

	void RestoreOrientationSettings(QList<double> aList);
	virtual void				closeEvent(QCloseEvent* event);

	void	onSSDRTools();
	void	onSimulateSSDR();
	void	onCancelSSDRSim();

protected:
	NotificationWidget* theNotifier;

	QWidget* LeftWidget;
	QFrame* LeftFrame;
	QStackedLayout* projectStack;
	QPushButton* showButton, * hideButton;
	QPushButton* closeResButton;
	QDockWidget* fileDock, *printDock, * orientDock, * supportDock, * viewDock;
	QList<Doc*>          myDocuments;

	QString mySuffix;
	QString BaseName;
	QDockWidget* SSDRdockWidget;
	QStringList aListString;

	QList<QToolButton*> ListofToolBarButton;

	QWidget* myWidgetTree;
	QTreeView* myTree;
	QStandardItemModel* model;
	QStringListModel* modelString;
	QStandardItem* itemProject, * itemExpertise, * itemGeometry;
	int PosOnWid;

	QMenu* FeatureProperties;
	QMenu* ResponseProperties;
	QMenu* SurfClickProperties, * SurfListProperties;
	QMenu* ParttoBuildProp;
	QMenu* ParttoTreeProp;
	//ExpertFile Reader

	QWidget* RespWid;
	QGridLayout* ResultLayout;
	QHBoxLayout* ResultLayout2;
	int layPosx, layPosy;
	QList<int> AllSeriesDisplayed;
	QList<bool> isContainerDisplayed;

	QDialog* choiceDialog, * custChoiceDialog;
	QProgressBar* myProgressBar;

	QList<QSlider*> WeightSliderList;

	QLabel* SelVarLabel, * SelElementLabel;

	int SelectedARid, CurrentARid;
	//double XChart, YChart;
	QLabel* AggregLabel;

	QTabWidget* Tabb, * Tabb2;
	QWidget* TreeWidget, * SetgWidget, * ResponseWidget, * AnalysisWidget;
	QWidget* generalSettings, * orientSettings, * supportSettings, *attributeWidget;

	QWidget* SingleRespWidget;
	QWidget* respTitleWidget;
	QScrollArea* TreeScroll, * SettingScroll, * ResponseScroll, * AnalysisScroll, * SingleRespScroll;

	QList<QDockWidget*> ARDocks;
	QList<QList<QCheckBox*>> ArchList;
	QList<QList<QCheckBox*>> ArAllSurfchList;

	QList<QCheckBox*> Arch2DViewList;


	//Viewer
	View* myView;
	bool WinCreated=false;
	QToolBar* aToolBar;
	QString ProjectName;

	//Left Tree
	
	QDialog* RotDialog, * TrlDialog;
	QDoubleSpinBox* RxSpin, * RySpin, * RzSpin;
	QDoubleSpinBox* TxSpin, * TySpin, * TzSpin;
	QButtonGroup* aGrButton;
	QPushButton* OkButtonRot;
	QPushButton* OkButtonTrl;

	//Bars
	QChart* ResultBarChart, * custResultBarChart;
	QList<QBarSeries*> theBarSeries;
	bool theBarChartsDoneOnce;
	QBarCategoryAxis* BarAxisX, * custBarAxisX;
	QValueAxis* BarAxisY, * custBarAxisY;
	
	//Polar (radar)
	QPolarChart* ResultPolarChart;
	QCategoryAxis* PolarAxisX;
	QValueAxis* PolarAxisY;

	//Importation
	QList<QString> ListofGeomName;

	//Exportation
	QDialog* ExportDialog;
	QList<QCheckBox*> ListofExportCH;
	QPushButton* goExportButton;

	//Orientation Tools
	QSpinBox* Angle1Range, * Angle2Range, * Angle1Step, * Angle2Step;

	//SSDR Tools
	double SSDRAngle, SSDRTool;
	QDoubleSpinBox* AngSSDR, * ToolSSDR;
	QCheckBox* SSDRRoadMode, * SSDRGradientMode;

	////////
	QDialog* ExpertDialog;
	QScrollArea* expertScroll;
	QVBoxLayout* expertLay;
	//Save Options
	QString XRDFPath;

	QString FilePath;
	bool hasNameAndPath = false;
	QString FileDirectory;
	QComboBox* visuCombo, *visuCombo2;
	QDialog* visuDialog;
	QFile myXRDF;
	//Newly Added
	QVBoxLayout* ARSelectionLayout;
	QStackedLayout* analysisStack;
	QFrame* analysisFrame;
	QWidget* weightingWidget, * barsWidget, * thresholdingWidget, *radarWidget;
	QScrollArea* weightingScroll, * barsScroll, * threScroll;
	QGridLayout* weigthersLayout;
	QGroupBox* WeightingCtrlWid;
	QComboBox* WeightingCombo;

	double Rx = 0, Ry = 0, Rz = 0, Rot1 = 0, Rot2 = 0;
	QStringListModel* attribModel;
	VarAttribute attribVar;
	QListView* attribViewList;
	QLabel* attribTitle;	int numOfFace=0;

	QVBoxLayout* singleRespPartLay;
	QWidget* responsesContainer;
	QStackedLayout* ResultStack;
	QFrame* mainFrame;

	QComboBox* AnalysisModeCombo;
	QComboBox* workingPartCombo;
	QComboBox* workingPartCombo2;
	QComboBox* workingPartCombo3;
	QList<QString> workingPartList;
	QList<QCheckBox*> chosenPartChecks;
	QGroupBox* workingPartBox;
	QVBoxLayout* workingPartLayout;

	//QGroupBox* RotationCtrlWid;
	//QHBoxLayout* rotCtrlLayout;
	QDial* Rot1Dial, * Rot2Dial;
	QDial* Rot11Dial, * Rot22Dial;

	QRadioButton* xax1, * xax2, * yax1, * yax2, * zax1, * zax2;
	QDoubleSpinBox* heiSpin, * widSpin, * depSpin;

	//Support Structures
	double suppAng, suppHeight, suppWidth, suppSpace;
	int connectionType, perforationType;
	double connectionSpace;

	bool leftTreeCreated = false, respWidgetCreated = false;
	VarARValues theAggregated;

	bool yesToAll = false;

};
#endif
