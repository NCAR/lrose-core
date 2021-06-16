#ifndef BoundaryView_HH
#define BoundaryView_HH

#include "GeneralDefinitions.hh"
#include "WorldPlot.hh"
#include "BoundaryPointEditorModel.hh"

#include <QPaintEngine>
#include <QPushButton>
#include <QBrush>
#include <QColor>
#include <QSlider>
#include <list>

using namespace std;

//
// BoundaryView.hh
//
//     Created: Sept-Nov 2019 by Jeff Smith
//     Modified: June, 2021 to separate into Model-View-Controller classes.
//
// draws a boundary in an image (WorldPlot, painter)



class BoundaryView
{
  public:

	BoundaryView();
	//string getBoundaryFilePath(string &fieldName, int sweepIndex, string boundaryFileName);
	//string getBoundaryDirFromRadarFilePath(string rootBoundaryDir, string radarFilePath);
	//vector<Point> getBoundaryPoints(string radarFilePath, string &fieldName, int sweepIndex, string boundaryFileName);
	//string getRootBoundaryDir();
	//void setBoundaryDir(string &openFilePath);
/*
	void makeCircle(int x, int y, float radius);
	void addToBrushShape(float x, float y);
	void addPoint(float x, float y);
	void insertPoint(float x, float y);
	void delNearestPoint(float x, float y);
	*/
	void draw(WorldPlot worldPlot, QPainter &painter,
	  vector<Point> &points, float pointBoxScale,
	  bool isFinished, BoundaryToolType currentTool, QBrush *yellowBrush);
	/*
	bool isOverAnyPoint(float worldX, float worldY);
	void moveNearestPointTo(float worldX, float worldY);
	bool isAClosedPolygon();
	void checkToAddOrDelPoint(float x, float y);
	void clear();
	void setTool(BoundaryToolType tool);
	BoundaryToolType getCurrentTool();
	bool updateScale(double xRange);
	vector<Point> getWorldPoints();
	bool setCircleRadius(int value);
	void setBrushRadius(int value);
	void setWorldScale(float value);
	bool getIsCircle();
	int getCircleRadius();
	int getBrushRadius();
*/

	void drawPointBox(WorldPlot worldPlot, QPainter &painter, Point point,
		float pointBoxScale, QBrush *yellowBrush);

  private:


  	/*

	vector<Point> getPoints(string boundaryFilePath);

	BoundaryPointEditor(){};
	int getNearestPointIndex(float x, float y, vector<Point> &pts);
	float getNearestDistToLineSegment(int x, int y, int segmentPtIndex1, int segmentPtIndex2);
	void coutPoints(vector<Point> &pts);
	*/
	/*
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

/*
	int circleRadius = 50;
	int brushRadius = 20;
	int brushToolNextPtDistance = 10;
	Point brushLastOrigin;
	QBrush *yellowBrush = new QBrush(QColor(255,255,0));

	BoundaryToolType currentTool = BoundaryToolType::brush;

  bool forceHide = true;

  //circle radius slider for BoundaryPointEditor
  void _circleRadiusSliderValueChanged(int value);
  void _brushRadiusSliderValueChanged(int value);

  void refreshBoundaries();

  void selectBoundaryTool(BoundaryToolType tool);
*/
};


#endif
