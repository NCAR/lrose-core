#include "BoundaryView.hh"
//#include "PolarManager.hh"

#include <toolsa/LogStream.hh>

#include <iostream>
#include <fstream>
#include <iterator>
#include <QApplication>

/*
 BoundaryView.cc 

  Created on: Sept-Nov 2019
      Author: Jeff Smith
  Modified on: June 4, 2021 Split into Model-View-Controller 

  BoundaryView draws the current boundary in an image (WorldPlot, painter)

*/

// Creates the boundary editor dialog and associated event slots
BoundaryView::BoundaryView()
{
}


// draws the boundary
void BoundaryView::draw(WorldPlot worldPlot, QPainter &painter,
	vector<Point> &points, float pointBoxScale, bool isFinished,
	BoundaryToolType currentTool, string &color)
{
	LOG(DEBUG) << "enter";
  painter.save();

	QColor boundaryColor(color.c_str());
  // QBrush *yellowBrush = new QBrush(boundaryColor);
  QPen pen = painter.pen();
  pen.setWidth(0); // 3 pixels wide
  pen.setColor(QColor(boundaryColor));
	painter.setPen(pen); // Qt::yellow);

	// bool isFinished = isAClosedPolygon();

  LOG(DEBUG) << "npoints = " << points.size();
	for (int i=1; i < (int) points.size(); i++)
	{
		LOG(DEBUG) << "drawing point " << i << " x,y = " << points[i].x << "," << points[i].y;
		//worldPlot.drawLine(painter, points[i-1].x, points[i-1].y, points[i].x, points[i].y);
		painter.drawLine(points[i-1].x, points[i-1].y, points[i].x, points[i].y);		
		if (isFinished && currentTool == BoundaryToolType::polygon)
		  drawPointBox(worldPlot, painter, points[i], pointBoxScale, color);
	}

  painter.restore();
	LOG(DEBUG) << "exit";
}

// draws a yellow square over a point
// (relevant with the Polygon Tool)
void BoundaryView::drawPointBox(WorldPlot worldPlot, QPainter &painter, Point point,
	float pointBoxScale, string &color)
{
	double x = point.x;
	double y = point.y;

	QColor boundaryColor(color.c_str());
  QBrush yellowBrush(boundaryColor);
	int size = 3 / worldPlot.getXPixelsPerWorld();
	if (size <= 0) size = 1;
	//worldPlot.fillRectangle(painter, yellowBrush, x-(size/2), y-(size/2), size, size);
	painter.fillRect(x-(size/2), y-(size/2), size, size, yellowBrush);

}

/*
// updates the scale of the boundary (in case the user zoomed in/out)
bool BoundaryView::updateScale(double xRange)
{
	float newPointBoxScale = xRange / 450;  //450 is the default range/size if no radar data has been loaded
	bool doUpdate = (newPointBoxScale != pointBoxScale);
	pointBoxScale = newPointBoxScale;
	return(doUpdate);  //only do an update if this value has changed
}

void BoundaryView::clear()
{
	points.clear();
}

// user has dragged circle slider and reset the circle radius
bool BoundaryView::setCircleRadius(int value)
{
	circleRadius = value;
	bool resizeExistingCircle = (currentTool == BoundaryToolType::circle && points.size() > 1);
	if (resizeExistingCircle)  //resize existing circle
		makeCircle(circleOrigin.x, circleOrigin.y, circleRadius);
	return(resizeExistingCircle);
}

// user has dragged the brush slider and reset its size
void BoundaryView::setBrushRadius(int value)
{
	brushRadius = value;
}

// add (x,y) to the current brush shape
void BoundaryView::addToBrushShape(float x, float y)
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
			for (int i=0; i < (int) mergePoints.size(); i++)
			{
				int mergePointsIndexToUse = i+nearestIndex;
				if (mergePointsIndexToUse >= (int) mergePoints.size())
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

float BoundaryView::getBrushWorldRadius()
{
	float scaleFactor = worldScale / 480;  //480 is worldScale on program startup
	scaleFactor = sqrt(scaleFactor);

	return(brushRadius * scaleFactor);
}

// returns the average distance between points in the boundary
// (relevant with the Brush Tool)
int BoundaryView::getAvgDistBetweenPoints()
{
	int cntUsed = 0;
	float totalDist = 0;

	for (int i=1; i < (int) points.size(); i++)
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
void BoundaryView::removePointsExceedingMaxGap()
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

		for (int i=1; i < (int) points.size()-1; i++)
		{
			float dist = points[i].distanceTo(points[i-1]);

			if (dist > maxGap)
			{
                          if (i < (int) points.size()-1)
				{
					float dist2 = points[i+1].distanceTo(points[i-1]);
//cout << i << "_dist=" << dist << " and i+1_dist=" << dist2 << endl;
					if (dist2 > dist)
					{
                                          if (i+2 < (int) points.size())
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
float BoundaryView::getMaxGapInPoints()
{
	float maxDist = -99999;
	for (int i=1; i < (int) points.size(); i++)
	{
			float dist = points[i].distanceTo(points[i-1].x, points[i-1].y);
			if (dist > maxDist)
				maxDist = dist;
	}
	return(maxDist);
}

// reorders (re-indexes) the points in the boundary to point[0] is opposite of (x,y)
// (relevant with the Brush Tool)
void BoundaryView::reorderPointsSoStartingPointIsOppositeOfXY(int x, int y)
{
	//get furthest point from (x,y) and make that point[0]
	eraseLastPoint();

	int furthestIndex = getFurthestPtIndex(x, y);

	vector<Point> tempPoints;
	for (int i=0; i < (int) points.size(); i++)
	{
		int tempIndex = furthestIndex + i;
		if (tempIndex > (int) points.size()-1)
			tempIndex -= points.size();
		tempPoints.push_back(points[tempIndex]);
	}

	points.swap(tempPoints);  //now assign this newly ordered list of points to "points"
	appendFirstPointAsLastPoint();

	//now free memory from tempPoints by swapping with empty vector
	vector<Point> tempVector;
	tempPoints.swap(tempVector);
}
*/
/* erase any points closer than thresholdDistance to (x,y), and return
   the index of the first point erased. Used to merge brush "circles" into each other
   (relevant with the Brush Tool)
*/
/*
int BoundaryView::erasePointsCloseToXYandReturnFirstIndexErased(int x, int y, int thresholdDistance)
{
	bool isDone = false;
	int firstIndexErased = -1;
	int cntErasedPts = 0;

	while (!isDone)
	{
		int indexToErase = -1;
		for (int i=0; i < (int) points.size(); i++)
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
int BoundaryView::getFurthestPtIndex(int x, int y)
{
	float maxDist = -99999;
	float indexAtMaxDist = -1;

	for (int i=0; i < (int) points.size(); i++)
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
void BoundaryView::makeCircle(int x, int y, float radius)
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
void BoundaryView::checkToAddOrDelPoint(float x, float y)
{
	bool isOverExistingPt = isOverAnyPoint(x, y); // Model
	bool isShiftKeyDown = (QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier) == true);
	if (isShiftKeyDown)
	{
		if (isOverExistingPt)
				BoundaryPointEditorView::Instance()->delNearestPoint(x, y);
			else
				BoundaryPointEditor::Instance()->insertPoint(x, y);
	}
}
*/
/*
int BoundaryView::getCircleRadius()
{
	return(circleRadius);
}

int BoundaryView::getBrushRadius()
{
	return(brushRadius);
}
*/














