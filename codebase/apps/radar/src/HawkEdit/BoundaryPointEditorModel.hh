#ifndef BoundaryPointEditorModel_HH
#define BoundaryPointEditorModel_HH

#include "GeneralDefinitions.hh"
#include "BoundaryToolType.hh"
#include "Point.hh"
#include "PolygonUtils.hh"


#include <list>
#include <vector>
#include <string>

using namespace std;

//
// BoundaryPointEditorModel.hh
//
//     Created: Sept-Nov 2019 by Jeff Smith
//  Includes Point and PolygonUtils. Implemented as a singleton and stores the points for a boundary

//
//  
//
class BoundaryPointEditorModel
{
  public:

  BoundaryPointEditorModel();

  //vector<Point> getBoundaryPoints(string radarFilePath, string &fieldName, int sweepIndex, string boundaryFileName);
  vector<Point> getBoundaryPoints(string radarFilePath,
    string &fieldName, int sweepIndex, int boundaryIndex);	

  void evaluateMouseRelease(int worldX, int worldY, bool isShiftKeyDown);
  bool evaluatePoint(int worldX, int worldY);
	void makeCircle(int x, int y, float radius);
	void addToBrushShape(float x, float y);
	void addPoint(float x, float y);
	void insertPoint(float x, float y);
	void delNearestPoint(float x, float y);

	bool isOverAnyPoint(float worldX, float worldY);
	void moveNearestPointTo(float worldX, float worldY);
	bool isAClosedPolygon();
	void checkToAddOrDelPoint(float x, float y, bool isShiftKeyDown);
	void clear();
	void save(int boundaryIndex, string &fieldName, int sweepNumber, 
		string &radarFilePath);
	bool load(int boundaryIndex, string &selectedFieldName,
    int sweepIndex, string &radarFilePath);
	bool load(string path);
	void setTool(BoundaryToolType tool);  // TODO: maybe move to View?
	BoundaryToolType getCurrentTool();
	bool updateScale(double xRange);
	vector<Point> getWorldPoints();
	bool setCircleRadius(int value);
	void setBrushRadius(int value);
	void setWorldScale(float value);
	bool getIsCircle();
	int getCircleRadius();
	int getBrushRadius();
	string &getYellowBrush() { return yellowBrush; };
	float getPointBoxScale() { return pointBoxScale; };

	const char *refreshBoundary(string &radarFilePath,
    string &fieldName, int sweepIndex, int boundaryIndex);

  vector<Point> getPoints(string boundaryFilePath);

  private:


  void save(string &path);
  string getBoundaryFilePath(string &radarFilePath, string &fieldName,
   int sweepIndex, int boundaryIndex);
	string getBoundaryDirFromRadarFilePath(string radarFilePath);
	string getRootBoundaryDir();
	//void setBoundaryDir(string &openFilePath); 	
  string getBoundaryName(int i);	

	string rootBoundaryDir; //  = string(getenv("HOME")) + "/" + "HawkEyeBoundaries";
	
	int getNearestPointIndex(float x, float y, vector<Point> &pts);
	float getNearestDistToLineSegment(int x, int y, int segmentPtIndex1, int segmentPtIndex2);
	void coutPoints(vector<Point> &pts);

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
	int circleRadius = 50;
	int brushRadius = 20;
	int brushToolNextPtDistance = 10;
	Point brushLastOrigin;

	string yellowBrush = "yellow";

	void coutMemUsage();
	vector<Point> points;
	vector<Point> mergePoints;

	BoundaryToolType currentTool = BoundaryToolType::brush;

	void ReadFromFile(vector<Point> &x, const string &file_name);

  //string _boundaryDir;	
};


#endif
