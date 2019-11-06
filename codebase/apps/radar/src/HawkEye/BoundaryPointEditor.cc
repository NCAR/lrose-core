#include "BoundaryPointEditor.hh"
#include "PolarManager.hh"

#include <iostream>
#include <fstream>
#include <iterator>
#include <QApplication>

//
// BoundaryPointEditor.cc
// Enables users to load/save up to 5 different boundaries. The boundary names are fixed (e.g. "Boundary1")
// in order to simplify saving/loading them (i.e., user's don't have a file save dialog pop up where they
// navigate to a directory in which they have read/write permissions). Instead, all they have to do is click
// Save and the file is written automatically to a location that the boundary editor knows how to read from
// later.
//
// Boundary files are written to the user's home directory underneath "/HawkEyeBoundaries". For example,
// to /home/jeff/HawkEyeBoundaries. For each radar file opened and accessed with the Boundary Editor, a
// unique directory (name) is created underneath this directory based on the hash code of the source radar file path.
// For example, if the radar file is loaded from /media/some/dir/cfrad.20170408_001452.962_to_20170408_002320.954_KARX_Surveillance_SUR.nc,
// this might be hash-coded to "1736437357943458505". The boundary directory will then be
// something like /home/jeff/HawkEyeBoundaries/1736437357943458505
//
// Within that directory will be any boundaries saved by the user. Unique boundaries are saved for each
// field and sweep. So filenames in that directory might be:
//
//   (The user saved 3 different boundaries for field 0, sweep 4)
//   field0-sweep4-Boundary1
//   field0-sweep4-Boundary2
//   field0-sweep4-Boundary3
//
//   (The user saved 1 boundary for field1, sweep 4, and saved one boundary for field2, sweep0)
//   field1-sweep4-Boundary1
//   field2-sweep0-Boundary1
//
//   Fields are zero indexed. For example, we might have fields "DBZ" (0), "REF" (1), "VEL" (2), ...
//   Sweeps are zero indexed. For example, we might have sweeps "0.47" (0), "1.48" (1), "2.49" (2), ...
//
//   So if a radar file has 8 fields, 5 sweeps (and since there are 5 potential boundaries in the screen list),
//   the maximum possible boundaries the user could potentially save to disk in the boundary directory is:
//     8 * 5 * 5 = 200 files
//
//  Created on: Sept-Nov 2019
//      Author: Jeff Smith


// Global static pointer used to ensure a single instance of the class.
BoundaryPointEditor* BoundaryPointEditor::m_pInstance = NULL;


BoundaryPointEditor* BoundaryPointEditor::Instance()
{
   if (!m_pInstance)   // Only allow one instance of class to be generated.
      m_pInstance = new BoundaryPointEditor;
   return m_pInstance;
}

// Set the current tool
void BoundaryPointEditor::setTool(BoundaryToolType tool)
{
	if (tool != currentTool)
		clear();
	currentTool = tool;
}

// Boundary Editor needs to know the world scale so it sizes boundaries correctly on screen
void BoundaryPointEditor::setWorldScale(float value)
{
	worldScale = value;
//	cout << "BoundaryEditor, setting worldScale=" << worldScale << endl;
}

BoundaryToolType BoundaryPointEditor::getCurrentTool()
{
	return(currentTool);
}

// returns the points (which are stored in world coordinates, not pixel locations)
vector<Point> BoundaryPointEditor::getWorldPoints()
{
	return(points);
}

// If given point is very close to the first point, assume the user is closing the polygon
// (relevant with the Polygon Tool)
void BoundaryPointEditor::checkToMovePointToOriginIfVeryClose(Point &pt)
{
	if (points.size() > 2 && pt.distanceTo(points[0]) < CLOSE_DISTANCE)
	{
		pt.x = points[0].x;
		pt.y = points[0].y;
	}
}

// adds a polygon point (relevant with the Polygon Tool)
void BoundaryPointEditor::addPoint(float x, float y)
{
	currentTool == BoundaryToolType::polygon;

	if (!isAClosedPolygon())
	{
		Point pt;
		pt.x = x;
		pt.y = y;

		checkToMovePointToOriginIfVeryClose(pt);

		if (points.size() < 4 || !doesLastSegmentIntersectAnyOtherSegment(pt))
			points.push_back(pt);
		else
			cout << "Invalid point crosses existing line segment! Discarding." << endl;
	//	coutPoints();
	}
}

// return true if the user supplied point results in an intersecting line segment
// (relevant with the Polygon Tool)
bool BoundaryPointEditor::doesLastSegmentIntersectAnyOtherSegment(Point &lastPoint)
{
	cout << "points.size()=" << points.size() << endl;

	for (int i=0; i < points.size()-2; i++)
	{
//		cout << "checking (" << points[i].x << "," << points[i].y << ") (" << points[i+1].x << "," << points[i+1].y << ") and (" << points[points.size()-1].x << "," << points[points.size()-1].y << ") (" << lastPoint.x << "," << lastPoint.y << ")" << endl;
		if (PolygonUtils::doSegmentsIntersect(points[i], points[i+1], points[points.size()-1], lastPoint, true))
			return(true);
	}
	return(false);
}

// If user is holding down the Shift key, they can insert a new point into the polygon
// (relevant with the Polygon Tool)
void BoundaryPointEditor::insertPoint(float x, float y)
{
	Point pt;
	pt.x = x;
	pt.y = y;
	int nearestPtIndex = getNearestPointIndex(x, y, points);
	Point nearestPt = points[nearestPtIndex];

	//remember that the last point is just a duplicate of the first point in a closed polygon, so just ignore this point

	//figure out which of the two possible segments (x,y) is closer to
	int afterPtIndex = nearestPtIndex+1;
	if (afterPtIndex >= points.size()-1)
		afterPtIndex = 0;
	float afterPtDistance = pt.distToLineSegment(nearestPt.x, nearestPt.y, points[afterPtIndex].x, points[afterPtIndex].y);

	int beforePtIndex = nearestPtIndex-1;
	if (beforePtIndex < 0)
		beforePtIndex = points.size()-2;
	float beforePtDistance = pt.distToLineSegment(nearestPt.x, nearestPt.y, points[beforePtIndex].x, points[beforePtIndex].y);

	cout << "INSERT POINT! nearestPtIndex=" << nearestPtIndex << ", insert point at " << x << "," << y << endl;
	cout << "afterPtDist=" << afterPtDistance << ", beforePtDist=" << beforePtDistance << ", beforePtIndex=" << beforePtIndex << ", afterPtIndex=" << afterPtIndex << endl;

	if (afterPtDistance <= beforePtDistance)
	{
		if (afterPtIndex == 0)
			points.insert(points.begin() + points.size()-1, pt);
		else
			points.insert(points.begin() + afterPtIndex, pt);
	}
	else
	{
		if (beforePtIndex == points.size()-1)
			points.insert(points.begin() + points.size()-2, pt);
		else
			points.insert(points.begin() + beforePtIndex+1, pt);
	}
}

// If the user is holding down the Shift key over an existing point, they can
// delete that point from the polygon
// (relevant with the Polygon Tool)
void BoundaryPointEditor::delNearestPoint(float x, float y)
{
  int nearestPtIndex = getNearestPointIndex(x, y, points);
  if (nearestPtIndex == 0 || nearestPtIndex == points.size()-1)
  {
  	eraseLastPoint();
    points.erase(points.begin());  //erase first point

    //now add closing (last) point that is identical to the new first point
    appendFirstPointAsLastPoint();
  }
  else
    points.erase(points.begin() + nearestPtIndex);
	cout << "delete nearest point to " << x << "," << y << endl;
}

// create new point, assign its (x,y) to the first point's values, and add to
// the polygon (effectively "closing" the polygon)
// (relevant with the Polygon Tool)
void BoundaryPointEditor::appendFirstPointAsLastPoint()
{
  Point pt;
  pt.x = points[0].x;
  pt.y = points[0].y;
  points.push_back(pt);
}

void BoundaryPointEditor::eraseLastPoint()
{
	points.erase(points.begin() + points.size()-1);
}

// return the index of the point closest to the given (x,y)
// (relevant with the Polygon Tool)
int BoundaryPointEditor::getNearestPointIndex(float x, float y, vector<Point> &pts)
{
	int nearestPointIndex;
	float nearestDistance = 99999;

	for (int i=0; i < pts.size(); i++)
	{
		float distance = pts[i].distanceTo(x, y);
		if (distance < nearestDistance)
		{
			nearestDistance = distance;
			nearestPointIndex = i;
		}
	}
	return(nearestPointIndex);
}

// find the nearest point to (x,y) and move it exactly to (x,y)
// (relevant with the Polygon Tool)
void BoundaryPointEditor::moveNearestPointTo(float x, float y)
{
	int nearestPointIndex = getNearestPointIndex(x, y, points);
	points[nearestPointIndex].x = x;
	points[nearestPointIndex].y = y;
	if (nearestPointIndex == 0)
	{
		points[points.size()-1].x = x;
		points[points.size()-1].y = y;
	}
}

//return true if (x,y) is very close to any existing point
// (relevant with the Polygon Tool)
bool BoundaryPointEditor::isOverAnyPoint(float x, float y)
{
	for (int i=0; i < points.size(); i++)
		if (points[i].distanceTo(x, y) < CLOSE_DISTANCE)
			return(true);
	return(false);
}

// return true if the vector of points (the polygon) is closed (meaning the first point == last point)
// (relevant with the Polygon Tool)
bool BoundaryPointEditor::isAClosedPolygon()
{
	return(currentTool == BoundaryToolType::polygon && (points.size() > 2) && points[0].equals(points[points.size()-1]));
}

// useful debug method
void BoundaryPointEditor::coutPoints()
{
	cout << points.size() << " total boundary editor points: " << endl;

	for (int i=0; i < points.size(); i++)
		cout << "(" << points[i].x << " " << points[i].y << ") ";
	cout << endl;
}

// draws the boundary
void BoundaryPointEditor::draw(WorldPlot worldPlot, QPainter &painter)
{
	painter.setPen(Qt::yellow);
	bool isFinished = isAClosedPolygon();

	for (int i=1; i < points.size(); i++)
	{
		worldPlot.drawLine(painter, points[i-1].x, points[i-1].y, points[i].x, points[i].y);
		if (isFinished && currentTool == BoundaryToolType::polygon)
		  drawPointBox(worldPlot, painter, points[i]);
	}

}

// draws a yellow square over a point
// (relevant with the Polygon Tool)
void BoundaryPointEditor::drawPointBox(WorldPlot worldPlot, QPainter &painter, Point point)
{
	double x = point.x;
	double y = point.y;

	int size = 6 * pointBoxScale;
	worldPlot.fillRectangle(painter, *yellowBrush, x-(size/2), y-(size/2), size, size);
}

// updates the scale of the boundary (in case the user zoomed in/out)
bool BoundaryPointEditor::updateScale(double xRange)
{
	float newPointBoxScale = xRange / 450;  //450 is the default range/size if no radar data has been loaded
	bool doUpdate = (newPointBoxScale != pointBoxScale);
	pointBoxScale = newPointBoxScale;
	return(doUpdate);  //only do an update if this value has changed
}

void BoundaryPointEditor::clear()
{
	points.clear();
}

// user has dragged circle slider and reset the circle radius
bool BoundaryPointEditor::setCircleRadius(int value)
{
	circleRadius = value;
	bool resizeExistingCircle = (currentTool == BoundaryToolType::circle && points.size() > 1);
	if (resizeExistingCircle)  //resize existing circle
		makeCircle(circleOrigin.x, circleOrigin.y, circleRadius);
	return(resizeExistingCircle);
}

// user has dragged the brush slider and reset its size
void BoundaryPointEditor::setBrushRadius(int value)
{
	brushRadius = value;
}

// add (x,y) to the current brush shape
void BoundaryPointEditor::addToBrushShape(float x, float y)
{
	float brushWorldRadius = getBrushWorldRadius();

	float distToPrevPt = points.size() == 0 ? 0 : brushLastOrigin.distanceTo(x, y);
	if (brushRadius <= 12)
		brushToolNextPtDistance = 5;
	else if (brushRadius <= 14)
		brushToolNextPtDistance = 7;
	else if (brushRadius <= 16)
		brushToolNextPtDistance = 8;
	else
		brushToolNextPtDistance = 10;

	if (points.size() == 0)
	{
		points.clear();
		makeCircle(x, y, brushWorldRadius);
		brushLastOrigin.x = x;
		brushLastOrigin.y = y;
	}
	else if (distToPrevPt > brushToolNextPtDistance) //mouse has moved enough to warrant an update, and initial circle exists so create second circle and merge them
	{
		reorderPointsSoStartingPointIsOppositeOfXY(x, y);

		Point brushPt;
		brushPt.x = x;
		brushPt.y = y;
		bool isPointInsidePolygon = PolygonUtils::isPointInsidePolygon(brushPt, points);

		//next, delete all the existing points that fall inside the new circle whose origin is (x,y)
		int firstIndexErased = erasePointsCloseToXYandReturnFirstIndexErased(x, y, brushWorldRadius);

		if (firstIndexErased == -1 && !isPointInsidePolygon)  //rapid mouse motion so we have jumped completely outside of original circle! start over with a new circle.
		{
			cout << "(" << x << "," << y << ") wasn't in polygon! starting over with circle!" << endl;
			makeCircle(x, y, brushWorldRadius);
		}
		else if (!(firstIndexErased == -1 && isPointInsidePolygon))
		{
			mergePoints.clear();
			int cntPoints = 0;

			//now insert some of the new points
			for (float angle=TwoPI; angle > 0; angle -= 0.35)
			{
				Point pt;
				pt.x = (float)x + (float)brushWorldRadius * cos(angle);
				pt.y = (float)y + (float)brushWorldRadius * sin(angle);
			  cntPoints++;

				//is this new point outside the previous circle? if so, add it to the mergePoints list
				if (pt.distanceTo(brushLastOrigin) > brushWorldRadius && !PolygonUtils::isPointInsidePolygon(pt, points))
					mergePoints.push_back(pt);
			}

			int nearestIndex = getNearestPointIndex(points[firstIndexErased].x, points[firstIndexErased].y, mergePoints);
			nearestIndex++;
			for (int i=0; i < mergePoints.size(); i++)
			{
				int mergePointsIndexToUse = i+nearestIndex;
				if (mergePointsIndexToUse >= mergePoints.size())
					mergePointsIndexToUse -= mergePoints.size();
				points.insert(points.begin() + firstIndexErased++, mergePoints[mergePointsIndexToUse]);
			}
		}

		float maxGap = getMaxGapInPoints();
		float maxGapAllowed = 1.1*brushWorldRadius;

		if (maxGap > maxGapAllowed)  //user may have backed over shape or curved around and intersected it.
		{
//			cout << "Invalid point in shape? maxGap=" << maxGap << ". Attempting to fix polygon shape" << endl;
			removePointsExceedingMaxGap();
		}

		int loopCnt = 0;
		while (loopCnt++ < 10 && !PolygonUtils::isValidPolygon(points))
		{
			PolygonUtils::fixInvalidPolygon(points);
		}
		if (!PolygonUtils::isValidPolygon(points))  //something went wrong, just start over with a new circle
			points.clear();

		brushLastOrigin.x = x;
		brushLastOrigin.y = y;
	}
}

float BoundaryPointEditor::getBrushWorldRadius()
{
	float scaleFactor = worldScale / 480;  //480 is worldScale on program startup
	scaleFactor = sqrt(scaleFactor);

	return(brushRadius * scaleFactor);
}

// returns the average distance between points in the boundary
// (relevant with the Brush Tool)
int BoundaryPointEditor::getAvgDistBetweenPoints()
{
	int cntUsed = 0;
	float totalDist = 0;

	for (int i=1; i < points.size(); i++)
	{
		float dist = points[i].distanceTo(points[i-1]);
		if (dist < getBrushWorldRadius())
		{
			totalDist += dist;
			cntUsed++;
		}
	}

	return(totalDist / cntUsed);
}

// if any points are way off the boundary, remove them
// (relevant with the Brush Tool)
void BoundaryPointEditor::removePointsExceedingMaxGap()
{
	float maxGap = 4 * getAvgDistBetweenPoints();
	if (maxGap < 20)
		maxGap = 20;
//	cout << "removePointsExceedingMaxGap with maxGap=" << maxGap << endl;

	bool done = false;

	while (!done)
	{
		int indexToRemove = -1;
		if (points.size() < 3)  //we've removed too many points in this loop, give up
			return;

		for (int i=1; i < points.size()-1; i++)
		{
			float dist = points[i].distanceTo(points[i-1]);

			if (dist > maxGap)
			{
				if (i < points.size()-1)
				{
					float dist2 = points[i+1].distanceTo(points[i-1]);
//cout << i << "_dist=" << dist << " and i+1_dist=" << dist2 << endl;
					if (dist2 > dist)
					{
						if (i+2 < points.size())
						{
							float dist3 = points[i+2].distanceTo(points[i-1]);
//cout << i << "_dist2=" << dist2 << " and i+2_dist=" << dist3 << endl;
							if (dist3 > dist)
								continue;
						}
						else
							continue;
					}
				}
				indexToRemove = i;
				break;
			}
		}

		if (indexToRemove == -1)
		{
			eraseLastPoint();
			appendFirstPointAsLastPoint();
			return;
		}
		else
		{
    	points.erase(points.begin() + indexToRemove);
//    	cout << "just erased " << indexToRemove << endl;
		}
	}
}

// return the maximum gap (distance) between points in the boundary
// (relevant with the Brush Tool)
float BoundaryPointEditor::getMaxGapInPoints()
{
	float maxDist = -99999;
	for (int i=1; i < points.size(); i++)
	{
			float dist = points[i].distanceTo(points[i-1].x, points[i-1].y);
			if (dist > maxDist)
				maxDist = dist;
	}
	return(maxDist);
}

// reorders (re-indexes) the points in the boundary to point[0] is opposite of (x,y)
// (relevant with the Brush Tool)
void BoundaryPointEditor::reorderPointsSoStartingPointIsOppositeOfXY(int x, int y)
{
	//get furthest point from (x,y) and make that point[0]
	eraseLastPoint();

	int furthestIndex = getFurthestPtIndex(x, y);

	vector<Point> tempPoints;
	for (int i=0; i < points.size(); i++)
	{
		int tempIndex = furthestIndex + i;
		if (tempIndex > points.size()-1)
			tempIndex -= points.size();
		tempPoints.push_back(points[tempIndex]);
	}

	points.swap(tempPoints);  //now assign this newly ordered list of points to "points"
	appendFirstPointAsLastPoint();

	//now free memory from tempPoints by swapping with empty vector
	vector<Point> tempVector;
	tempPoints.swap(tempVector);
}

// erase any points closer than thresholdDistance to (x,y), and return
// the index of the first point erased. Used to merge brush "circles" into each other
// (relevant with the Brush Tool)
int BoundaryPointEditor::erasePointsCloseToXYandReturnFirstIndexErased(int x, int y, int thresholdDistance)
{
	bool isDone = false;
	int firstIndexErased = -1;
	int cntErasedPts = 0;

	while (!isDone)
	{
		int indexToErase = -1;
		for (int i=0; i < points.size(); i++)
		{
			if (points[i].distanceTo(x, y) < thresholdDistance)
			{
				indexToErase = i;
				break;
			}
		}

		if (indexToErase != -1)
		{
			points.erase(points.begin() + indexToErase);
			cntErasedPts++;
			if (firstIndexErased == -1)
				firstIndexErased = indexToErase;
		}
		else
			isDone = true;

		points[points.size()-1].x = points[0].x;
		points[points.size()-1].y = points[0].y;
	}

//	cout << "erased " << cntErasedPts << " points, firstIndexErased=" << firstIndexErased << endl;
	return(firstIndexErased);
}

// return the index of the point furthest from (x,y)
// (relevant with the Brush Tool)
int BoundaryPointEditor::getFurthestPtIndex(int x, int y)
{
	float maxDist = -99999;
	float indexAtMaxDist = -1;

	for (int i=0; i < points.size(); i++)
	{
		float dist = points[i].distanceTo(x, y);
		if (dist > maxDist)
		{
			maxDist = dist;
			indexAtMaxDist = i;
		}
	}

	return(indexAtMaxDist);
}

// makes a circle of points at (x,y) with radius
// (relevant with the Circle Tool)
void BoundaryPointEditor::makeCircle(int x, int y, float radius)
{
	points.clear();
	circleOrigin.x = x;
	circleOrigin.y = y;

	for (float angle=TwoPI; angle > 0; angle -= 0.35)
	{
		Point pt;
		pt.x = (float)x + radius * cos(angle);
		pt.y = (float)y + radius * sin(angle);
		points.push_back(pt);
	}

	points.push_back(points[0]);
}

// User has Shift key down and has clicked mouse, so either insert or delete a point
// (relevant with the Polygon Tool)
void BoundaryPointEditor::checkToAddOrDelPoint(float x, float y)
{
	bool isOverExistingPt = isOverAnyPoint(x, y);
	bool isShiftKeyDown = (QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier) == true);
	if (isShiftKeyDown)
	{
		if (isOverExistingPt)
				BoundaryPointEditor::Instance()->delNearestPoint(x, y);
			else
				BoundaryPointEditor::Instance()->insertPoint(x, y);
	}
}

int BoundaryPointEditor::getCircleRadius()
{
	return(circleRadius);
}

int BoundaryPointEditor::getBrushRadius()
{
	return(brushRadius);
}

// Saves the current boundary to a file with a path described above (top of this file)
// Also saves the tool that was used to create the boundary (so it can be recalled)
void BoundaryPointEditor::save(string path)
{
	cout << "BoundaryPointEditor, saving boundary with " << points.size() << " points to " << path << endl;

	FILE *file;
	file = fopen(path.c_str(), "wb");

	int tool;
	if (currentTool == BoundaryToolType::circle)
		tool = 0;
	else if (currentTool == BoundaryToolType::polygon)
		tool = 1;
	else
		tool = 2; //brush

	//write the header (five ints)
	fwrite(&tool, sizeof(int), 1, file);
	fwrite(&circleRadius, sizeof(int), 1, file);
	fwrite(&circleOrigin.x, sizeof(float), 1, file);
	fwrite(&circleOrigin.y, sizeof(float), 1, file);
	fwrite(&brushRadius, sizeof(int), 1, file);

	//now write the points
	for (int i=0; i < points.size(); i++)
	{
		fwrite(&points[i].x, sizeof(float), 1, file);
		fwrite(&points[i].y, sizeof(float), 1, file);
	}
	fclose(file);
}

// Loads the boundary file given by path
// It also sets the correct editor tool (polygon, circle, or brush) based on what is in the file
void BoundaryPointEditor::load(string path)
{
	ifstream infile(path);
	if (infile.good())
	{
		FILE *file;
		file = fopen(path.c_str(), "rb");

		//get number of points in file (from file size)
		fseek(file, 0L, SEEK_END);
		int headerSize = 5 * sizeof(int);
		int size = ftell(file) - headerSize;
		fseek (file, 0, SEEK_SET);
		int numPoints = size / sizeof(Point);

		//read the header
		int tool;
		fread(&tool, sizeof(int), 1, file);
		fread(&circleRadius, sizeof(int), 1, file);
		fread(&circleOrigin.x, sizeof(float), 1, file);
		fread(&circleOrigin.y, sizeof(float), 1, file);
		fread(&brushRadius, sizeof(int), 1, file);

		//now read each point and add to boundary
		points.clear();
		for (int i=0; i < numPoints; i++)
		{
			float x, y;
			fread(&x, sizeof(float), 1, file);
			fread(&y, sizeof(float), 1, file);

			Point pt;
			pt.x = x;
			pt.y = y;
			points.push_back(pt);
		}
		fclose (file);

		checkToMovePointToOriginIfVeryClose(points[points.size()-1]);

		if (tool == 0)
			currentTool = BoundaryToolType::circle;
		else if (tool == 1)
			currentTool = BoundaryToolType::polygon;
		else
			currentTool = BoundaryToolType::brush;
		cout << "BoundaryPointEditor, read " << points.size() << " points from " << path << endl;
	}
	else
		cout << path << " doesn't exist, skipping..." << endl;
}

/*
void BoundaryPointEditor::coutMemUsage()
{
    double vm_usage     = 0.0;
    double resident_set = 0.0;

    // the two fields we want
    unsigned long vsize;
    long rss;
    {
        std::string ignore;
        std::ifstream ifs("/proc/self/stat", std::ios_base::in);
        ifs >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
                >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
                >> ignore >> ignore >> vsize >> rss;
    }

    long page_size_kb = 1024; // in case x86-64 is configured to use 2MB pages
    vm_usage = vsize / 1024.0;
    resident_set = rss * page_size_kb;
    cout << "VM: " << vm_usage << "; RSS: " << rss << endl;
}
*/
