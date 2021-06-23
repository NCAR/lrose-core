#ifndef BoundaryPointEditorView_HH
#define BoundaryPointEditorView_HH

#include "GeneralDefinitions.hh"
#include "BoundaryToolType.hh"

#include "WorldPlot.hh"

#include <QDialog>
#include <QPaintEngine>
#include <QPushButton>
#include <QBrush>
#include <QColor>
#include <QSlider>
#include <QListWidget>
#include <QLabel>

#include <list>

using namespace std;

//
// BoundaryPointEditor.hh
//
//     Created: Sept-Nov 2019 by Jeff Smith
//     Modified: June 2021 moving to MVC structure
//  Includes Point and PolygonUtils. Implemented as a singleton and stores the points for a boundary
//  



class BoundaryPointEditorView : public QDialog
{
	Q_OBJECT

  public:

	BoundaryPointEditorView();
	//string getBoundaryFilePath(string &fieldName, int sweepIndex, string boundaryFileName);
	//string getBoundaryDirFromRadarFilePath(string rootBoundaryDir, string radarFilePath);
	//vector<Point> getBoundaryPoints(string radarFilePath, string &fieldName, int sweepIndex, string boundaryFileName);
	//string getRootBoundaryDir();
	//void setBoundaryDir(string &openFilePath);

	//void makeCircle(int x, int y, float radius);
	//void addToBrushShape(float x, float y);
	//void addPoint(float x, float y);  
	//void insertPoint(float x, float y);
	//void delNearestPoint(float x, float y);
	//void draw(WorldPlot worldPlot, QPainter &painter);
	//bool isOverAnyPoint(float worldX, float worldY);
	//void moveNearestPointTo(float worldX, float worldY);
	//bool isAClosedPolygon();
	//void checkToAddOrDelPoint(float x, float y);
	void closeEvent();
	void clear();
	void save(string path);
	void load(string path);
	void setTool(BoundaryToolType tool, int radius = 0);
	BoundaryToolType getCurrentTool();
	//bool updateScale(double xRange);
	//vector<Point> getWorldPoints();
	bool setCircleRadius(int value);
	void setBrushRadius(int value);
	//void setWorldScale(float value);
	bool getIsCircle();
	int getCircleRadius();
	int getBrushRadius();

	void setBoundaryFile(int boundaryIndex, string  &fileName);

 signals:

	void saveBoundary(int boundaryIndex);
	void loadBoundary(int boundaryIndex);
  bool boundaryCircleRadiusChanged(int newRadius);
  bool boundaryBrushRadiusChanged(int newRadius);

	void userClickedPolygonButton();
	void userClickedCircleButton();
	void userClickedBrushButton();

	//vector<Point> getPoints(string boundaryFilePath);

  void showBoundaryEditor();
  void refreshBoundaries();

  void boundaryPointEditorClosed();
  void clearBoundary(int boundaryIndex);  // TODO: still needs connection


  void selectBoundaryTool(BoundaryToolType tool, int radius = 0);

  int firstBoundaryIndex();
  int lastBoundaryIndex();

  private:


  	/*

	vector<Point> getPoints(string boundaryFilePath);

	BoundaryPointEditor(){};
	int getNearestPointIndex(float x, float y, vector<Point> &pts);
	float getNearestDistToLineSegment(int x, int y, int segmentPtIndex1, int segmentPtIndex2);
	void coutPoints(vector<Point> &pts);
	void drawPointBox(WorldPlot worldPlot, QPainter &painter, Point point);
	void checkToMovePointToOriginIfVeryClose(Point &pt);
	int getClosestPtIndex(int x, int y);
	int getFurthestPtIndex(int x, int y);
	int erasePointsCloseToXYandReturnFirstIndexErased(int x, int y, int thresholdDistance);
	void reorderPointsSoStartingPointIsOppositeOfXY(int x, int y);
	void appendFirstPointAsLastPoint();
	void eraseLastPoint();
	float getMaxGapInPoints();
	void removePointsExceedingMaxGap();
	int getAvgDistBetweenPoints();
	bool doesLastSegmentIntersectAnyOtherSegment(Point &lastPoint);
	float getBrushWorldRadius();

	const float TwoPI = 6.283185;
	float worldScale;
	float CLOSE_DISTANCE = 10;
	float pointBoxScale = 1;
	Point circleOrigin;
	*/
	QPushButton *_clearBtn;
	QPushButton *_saveBtn;
  QSlider *_radiusSlider;
	int circleRadius = 50;
	int brushRadius = 20;
	int brushToolNextPtDistance = 10;
	//Point brushLastOrigin;
	QBrush *yellowBrush = new QBrush(QColor(255,255,0));

	//BoundaryToolType currentTool = BoundaryToolType::brush;

  QPushButton *_boundaryEditorClearBtn;
  QPushButton *_boundaryEditorHelpBtn;
  QPushButton *_boundaryEditorSaveBtn;
  QPushButton *_boundaryEditorPolygonBtn;
  QPushButton *_boundaryEditorCircleBtn;
  QPushButton *_boundaryEditorBrushBtn;
  QListWidget *_boundaryEditorList;
  QLabel *_boundaryEditorInfoLabel;
  bool forceHide = true;
  QSlider *_circleRadiusSlider;
  QSlider *_brushRadiusSlider;

  const int nBoundaries = 5;

  /*
+  QPushButton *_boundaryEditorClearBtn;
+  QPushButton *_boundaryEditorSaveBtn;
+  QListWidget *_boundaryEditorList;
+  */

  //circle radius slider for BoundaryPointEditor
  void _circleRadiusSliderValueChanged(int value);
  void _brushRadiusSliderValueChanged(int value);

   // boundary editor
//void _createBoundaryEditorDialog();
//-  void _showBoundaryEditor();
//-  void _clearBoundaryEditorClick();
  void createBoundaryEditorDialog();

  void clearBoundaryEditorClick();
  void helpBoundaryEditorClick();
  void polygonBtnBoundaryEditorClick();
  void circleBtnBoundaryEditorClick();
  void brushBtnBoundaryEditorClick();
   void onBoundaryEditorListItemClicked(QListWidgetItem* item);
//-  void _saveBoundaryEditorClick();
  void saveBoundaryEditorClick();


  //void _clearBoundaryEditorClick();
  //void onBoundaryEditorListItemClicked(QListWidgetItem* item);
  //void _saveBoundaryEditorClick();


};


#endif
