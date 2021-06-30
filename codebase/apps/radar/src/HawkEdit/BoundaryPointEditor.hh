#ifndef BoundaryPointEditor_HH
#define BoundaryPointEditor_HH

#include "GeneralDefinitions.hh"
#include "WorldPlot.hh"

#include "BoundaryView.hh"
#include "BoundaryPointEditorView.hh"
#include "BoundaryPointEditorModel.hh"
#include "BoundaryToolType.hh"

//#include <QPaintEngine>
//#include <QPushButton>
//#include <QBrush>
//#include <QColor>
//#include <QSlider>
#include <list>

using namespace std;

//
// BoundaryPointEditor.hh
//
//     Created: Sept-Nov 2019 by Jeff Smith
//     Modified: June 4, 2021 Split into Model-View-Controller 
//  Includes Point and PolygonUtils. Implemented as a singleton and stores the points for a boundary


class BoundaryPointEditor : public QObject
{

  Q_OBJECT

  public:

  BoundaryPointEditor(BoundaryPointEditorView *bpeView,
	  BoundaryView *boundaryView);
  ~BoundaryPointEditor();

  //void createBoundaryEditorDialog();	

	string getBoundaryFilePath(string &fieldName, int sweepIndex, string boundaryFileName);
	string getBoundaryDirFromRadarFilePath(string rootBoundaryDir, string radarFilePath);
	vector<Point> getBoundaryPoints(string radarFilePath, string &fieldName, int sweepIndex, string boundaryFileName);
	//string getRootBoundaryDir();
	void setBoundaryDir(string &openFilePath);

  bool evaluatePoint(int worldX, int worldY);
  bool evaluateCursor(bool isShiftKeyDown);
  void evaluateMouseRelease(int mouseReleaseX, int mouseReleaseY, bool isShiftKeyDown);
	static BoundaryPointEditor* Instance();
	void makeCircle(int x, int y, float radius);
	void addToBrushShape(float x, float y);
	void addPoint(float x, float y);
	void insertPoint(float x, float y);
	void delNearestPoint(float x, float y);
	void drawBoundary(WorldPlot worldPlot, QPainter &painter);
	bool isOverAnyPoint(float worldX, float worldY);
	void moveNearestPointTo(float worldX, float worldY);
	bool isAClosedPolygon();
	void checkToAddOrDelPoint(float x, float y);
	void clear();
	void save(int boundaryIndex, string &selectedFieldName,
    int sweepIndex, string &radarFilePath);
	bool load(int boundaryIndex, string &selectedFieldName,
    int sweepIndex, string &radarFilePath);



	void setTool(BoundaryToolType tool);
	BoundaryToolType getCurrentTool();
	bool updateScale(double xRange);
	vector<Point> getWorldPoints();
	bool setCircleRadius(int value);
	void setBrushRadius(int value);
	void setWorldScale(float value);
	bool getIsCircle();
	//int getCircleRadius();
	//int getBrushRadius();
	void showBoundaryEditor();
	//void refreshBoundaries();
	//const char *refreshBoundary(int i);
	void refreshBoundaries(string &openFilePath,
    string &currentFieldName, int currentSweepIndex);

  signals:
  void needSelectedFieldNameSweepIndex(int boundaryIndex);
  void boundaryCircleRadiusChanged(int value);
  void boundaryBrushRadiusChanged(int value);
  void saveBoundary(int boundaryIndex);
  void loadBoundary(int boundaryIndex);

  public slots:

  //void selectedFieldNameSweepIndexSent(int boundaryIndex, QString &selectedFieldName, int sweepIndex);
  void clearBoundaryEditorClick();
	void userClickedPolygonButton();
	void userClickedCircleButton();
	void userClickedBrushButton();

  private:

	vector<Point> getPoints(string boundaryFilePath);


	//int getNearestPointIndex(float x, float y, vector<Point> &pts);
	//float getNearestDistToLineSegment(int x, int y, int segmentPtIndex1, int segmentPtIndex2);
	void coutPoints(vector<Point> &pts);
	//void drawPointBox(WorldPlot worldPlot, QPainter &painter, Point point);
	//void checkToMovePointToOriginIfVeryClose(Point &pt);
	//int getClosestPtIndex(int x, int y);
	//int getFurthestPtIndex(int x, int y);
	//int erasePointsCloseToXYandReturnFirstIndexErased(int x, int y, int thresholdDistance);
	//void reorderPointsSoStartingPointIsOppositeOfXY(int x, int y);
	//void appendFirstPointAsLastPoint();
	//void eraseLastPoint();
	//float getMaxGapInPoints();
	//void removePointsExceedingMaxGap();
	//int getAvgDistBetweenPoints();
	//bool doesLastSegmentIntersectAnyOtherSegment(Point &lastPoint);
	//float getBrushWorldRadius();

	//const float TwoPI = 6.283185;
	float worldScale;
	float CLOSE_DISTANCE = 10;
	float pointBoxScale = 1;
	Point circleOrigin;
	//QPushButton *_clearBtn;
	//QPushButton *_saveBtn;
  //QSlider *_radiusSlider;
	//int circleRadius = 50;
	//int brushRadius = 20;
	int brushToolNextPtDistance = 10;
	Point brushLastOrigin;
	//QBrush *yellowBrush = new QBrush(QColor(255,255,0));
	void coutMemUsage();
	//vector<Point> points;
	//vector<Point> mergePoints;
	//static BoundaryPointEditor* m_pInstance;
	//BoundaryToolType currentTool = BoundaryToolType::brush;

	//void ReadFromFile(const string &file_name);

  string _boundaryDir;	

  BoundaryPointEditorView *_boundaryEditorView;
  BoundaryView *_boundaryView;  
  BoundaryPointEditorModel *_boundaryPointEditorModel;
};


#endif
