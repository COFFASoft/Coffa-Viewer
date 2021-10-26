
/*#ifndef VIEW_H
#define VIEW_H
#include <QtCore/QObject>
#include <QtWidgets/QWidget>
#include <QtWidgets/QAction>
#include <QtCore/QList>
#include <AIS_InteractiveContext.hxx>
#include <AIS_ViewController.hxx>
#include <V3d_View.hxx>

class TopoDS_Shape;
class QRubberBand;

class __declspec(dllexport) View :public QWidget, protected AIS_ViewController
{
	Q_OBJECT
public:
	View(Handle(AIS_InteractiveContext) theContext, QWidget* parent);
	~View();
	
protected:
	enum CurrentAction3d {
		CurAction3d_Nothing, CurAction3d_DynamicZooming,
		CurAction3d_WindowZooming, CurAction3d_DynamicPanning,
		CurAction3d_GlobalPanning, CurAction3d_DynamicRotation};
		
public:
	enum ViewAction {
		ViewFitAllId, ViewZoomId, ViewPanId, ViewRotationId, ViewAxoId, ViewFrontId, 
		ViewBackId, ViewTopId, ViewBottomId, ViewLeftId, ViewRightId, ViewResetId, GridId};
	enum RaytraceAction { ToolRaytracingId, ToolShadowsId, ToolReflectionsId, ToolAntialiasingId };
	
	virtual void                  init();
	bool                          dump(Standard_CString theFile);
	QList<QAction*>*              getViewActions();
	void                          noActiveActions();
	//bool                          isShadingMode();

	void                          EnableRaytracing();
	void                          DisableRaytracing();

	void                          SetRaytracedShadows(bool theState);
	void                          SetRaytracedReflections(bool theState);
	void                          SetRaytracedAntialiasing(bool theState);
	//int								getSelectBool();
	bool                          IsRaytracingMode() const { return myIsRaytracing; }
	bool                          IsShadowsEnabled() const { return myIsShadowsEnabled; }
	bool                          IsReflectionsEnabled() const { return myIsReflectionsEnabled; }
	bool                          IsAntialiasingEnabled() const { return myIsAntialiasingEnabled; }
	gp_Pnt						MousePoint;
	double mX, mY;
	Handle(V3d_View)                myView;
	virtual QPaintEngine*         paintEngine() const;
signals:
	void                          selectionChanged();
Q_SIGNALS:
	void							MoveDone(int);
	void							SurfaceClicked(int);
	void							PartClicked(int);
	void							GridClick(int);
	void							GridClick2(int);
public slots:
	void                          fitAll();
	void                          fitArea();
	void                          zoom();
	void                          pan();
	void                          globalPan();
	void                          front();
	void                          back();
	void                          top();
	void                          bottom();
	void                          left();
	void                          right();
	void                          axo();
	void                          rotation();
	void                          reset();
	void                          hlrOn();
	void                          hlrOff();
	void							GridOn();
	void                          updateToggled(bool);
	void                          onBackground();
	void                          onEnvironmentMap();
	void                          onRaytraceAction();

protected:
	virtual void                  paintEvent(QPaintEvent*);
	virtual void                  resizeEvent(QResizeEvent*);
	virtual void                  mousePressEvent(QMouseEvent*);
	virtual void                  mouseReleaseEvent(QMouseEvent*);
	virtual void                  mouseMoveEvent(QMouseEvent*);
	virtual void                  wheelEvent(QWheelEvent*);
	
	//! Setup mouse gestures.
	void defineMouseGestures();

	//! Set current action.
	void setCurrentAction(CurrentAction3d theAction);

	Handle(V3d_View)&                     getView();
	Handle(AIS_InteractiveContext)&       getContext();
	void                                  activateCursor(const CurrentAction3d);
	CurrentAction3d                       getCurrentMode();
	void                                  updateView();
	int Emitter1;
	int Emitter2;
	int Emitter3;
	int Emitter4;
	int Emitter5;

private:
	void                          initCursors();
	void                          initViewActions();

private:
	bool                            myIsRaytracing;
	bool                            myIsShadowsEnabled;
	bool                            myIsReflectionsEnabled;
	bool                            myIsAntialiasingEnabled;

	bool                            myDrawRect;           // set when a rect is used for selection or magnify 
	
	Handle(AIS_InteractiveContext)  myContext;
	Handle(V3d_View)			aView;
	Handle(AIS_InteractiveContext)  aContext;
	AIS_MouseGestureMap             myDefaultGestures;
	Graphic3d_Vec2i                 myClickPos;
	CurrentAction3d                 myCurrentMode;
	Standard_Integer                myXmin;
	Standard_Integer                myYmin;
	Standard_Integer                myXmax;
	Standard_Integer                myYmax;
	Standard_Real                   myCurZoom;
	Standard_Boolean                myHlrModeIsOn;
	QList<QAction*>*                myViewActions;
	QList<QAction*>*                myRaytraceActions;
	QMenu*                          myBackMenu;
	//QRubberBand*                    myRectBand; //!< selection rectangle rubber band
	int								mySelectedBool;
	Aspect_GridType aGridtype;
	Aspect_GridDrawMode aGridDrawmode;
};
#endif*/

#ifndef VIEW_H
#define VIEW_H

#include <QtCore/QObject>
#include <QtWidgets/QWidget>
#include <QtWidgets/QAction>
#include <QtCore/QList>
#include <AIS_InteractiveContext.hxx>
#include <AIS_ViewController.hxx>
#include <V3d_View.hxx>

class TopoDS_Shape;
class QRubberBand;
//class Coffa;

class View : public QWidget, protected AIS_ViewController
{
	Q_OBJECT
protected:
	enum CurrentAction3d {
		CurAction3d_Nothing, CurAction3d_DynamicZooming,
		CurAction3d_WindowZooming, CurAction3d_DynamicPanning,
		CurAction3d_GlobalPanning, CurAction3d_DynamicRotation
	};

public:
	enum ViewAction { ViewFitAllId, ViewZoomId, ViewPanId, ViewRotationId, ViewAxoId, ViewFrontId,
		ViewBackId, ViewTopId, ViewBottomId, ViewLeftId, ViewRightId, ViewResetId, GridId };

	enum RaytraceAction { ToolRaytracingId, ToolShadowsId, ToolReflectionsId, ToolAntialiasingId };

	View(Handle(AIS_InteractiveContext) theContext, QWidget* parent);

	~View();

	virtual void                  init();
	bool                          dump(Standard_CString theFile);
	QList<QAction*>* getViewActions();
	QList<QAction*>* getRaytraceActions();
	void                          noActiveActions();
	//bool                          isShadingMode();

	void                          EnableRaytracing();
	void                          DisableRaytracing();

	void                          SetRaytracedShadows(bool theState);
	void                          SetRaytracedReflections(bool theState);
	void                          SetRaytracedAntialiasing(bool theState);

	bool                          IsRaytracingMode() const { return myIsRaytracing; }
	bool                          IsShadowsEnabled() const { return myIsShadowsEnabled; }
	bool                          IsReflectionsEnabled() const { return myIsReflectionsEnabled; }
	bool                          IsAntialiasingEnabled() const { return myIsAntialiasingEnabled; }

	static QString                GetShapeType(TopAbs_ShapeEnum aShapeType);

	virtual QPaintEngine* paintEngine() const;

signals:
	void                          selectionChanged();

Q_SIGNALS:
	void							MoveDone(int);
	void							SurfaceClicked(int);
	void							PartClicked(int);
	void							GridClick(int);
	void							GridClick2(int);

public slots:
	void                          fitAll();
	void                          fitArea();
	void                          zoom();
	void                          pan();
	void                          globalPan();
	void                          front();
	void                          back();
	void                          top();
	void                          bottom();
	void                          left();
	void                          right();
	void                          axo();
	void                          rotation();
	void                          reset();
	void                          hlrOn();
	void                          hlrOff();
	void						  GridOn();
	void                          updateToggled(bool);
	void                          onBackground();
	void                          onEnvironmentMap();
	void                          onRaytraceAction();

protected:
	virtual void                  paintEvent(QPaintEvent*);
	virtual void                  resizeEvent(QResizeEvent*);
	virtual void                  mousePressEvent(QMouseEvent*);
	virtual void                  mouseReleaseEvent(QMouseEvent*);
	virtual void                  mouseMoveEvent(QMouseEvent*);
	virtual void                  wheelEvent(QWheelEvent*);


	Handle(V3d_View)& getView();
	Handle(AIS_InteractiveContext)& getContext();
	void                                  activateCursor(const CurrentAction3d);
	CurrentAction3d                       getCurrentMode();
	void                                  updateView();

	int Emitter1;
	int Emitter2;
	int Emitter3;
	int Emitter4;
	int Emitter5;

	//! Setup mouse gestures.
	void defineMouseGestures();

	//! Set current action.
	void setCurrentAction(CurrentAction3d theAction)
	{
		myCurrentMode = theAction;
		defineMouseGestures();
	}

	//! Handle selection changed event.
	void OnSelectionChanged(const Handle(AIS_InteractiveContext)& theCtx,
		const Handle(V3d_View)& theView) Standard_OVERRIDE;

private:
	void                          initCursors();
	void                          initViewActions();
	
	void						initRaytraceActions();

private:
	bool                            myIsRaytracing;
	bool                            myIsShadowsEnabled;
	bool                            myIsReflectionsEnabled;
	bool                            myIsAntialiasingEnabled;

	Handle(V3d_View)                myView;
	Handle(AIS_InteractiveContext)  myContext;
	AIS_MouseGestureMap             myDefaultGestures;
	Graphic3d_Vec2i                 myClickPos;
	CurrentAction3d                 myCurrentMode;
	Standard_Real                   myCurZoom;
	QList<QAction*>* myViewActions;
	QList<QAction*>* myRaytraceActions;
	QMenu* myBackMenu;

	Aspect_GridType aGridtype;
	Aspect_GridDrawMode aGridDrawmode;

};

#endif