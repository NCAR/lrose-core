#include "BoundaryPointEditor.hh"
//#include "PolarManager.hh"
#include <iostream>
#include <fstream>
#include <iterator>

/*
 * BoundaryPointEditor.cc
 *
 *  Created on: Sep 5, 2019
 *      Author: jeff smith
 */

// Global static pointer used to ensure a single instance of the class.
BoundaryPointEditor* BoundaryPointEditor::m_pInstance = NULL;


BoundaryPointEditor* BoundaryPointEditor::Instance()
{
   if (!m_pInstance)   // Only allow one instance of class to be generated.
      m_pInstance = new BoundaryPointEditor;
   return m_pInstance;
}

vector<Point> BoundaryPointEditor::getWorldPoints()
{
	return(points);
}

void BoundaryPointEditor::checkToMovePointToOriginIfVeryClose(Point &pt)
{
	if (points.size() > 1 && pt.distanceTo(points[0].x, points[0].y) < CLOSE_DISTANCE)
	{
		pt.x = points[0].x;
		pt.y = points[0].y;
	}
}

void BoundaryPointEditor::addPoint(int x, int y)
{
	if (!isPolygonFinished())
	{
		Point pt;
		pt.x = x;
		pt.y = y;

		checkToMovePointToOriginIfVeryClose(pt);

		points.push_back(pt);
	//	coutPoints();
	}
}

void BoundaryPointEditor::insertPoint(int x, int y)
{
	Point pt;
	pt.x = x;
	pt.y = y;
	int nearestPtIndex = getNearestPointIndex(x, y);
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

void BoundaryPointEditor::delNearestPoint(int x, int y)
{
  int nearestPtIndex = getNearestPointIndex(x, y);
  if (nearestPtIndex == 0 || nearestPtIndex == points.size()-1)
  {
    points.erase(points.begin() + points.size()-1);
    points.erase(points.begin());

    //now add closing (last) point that is identical to the new first point
    Point pt;
    pt.x = points[0].x;
    pt.y = points[0].y;
    points.push_back(pt);
  }
  else
    points.erase(points.begin() + nearestPtIndex);
	cout << "delete nearest point to " << x << "," << y << endl;
}

int BoundaryPointEditor::getNearestPointIndex(int x, int y)
{
	int nearestPointIndex;
	float nearestDistance = 99999;

	for (int i=0; i < points.size(); i++)
	{
		float distance = points[i].distanceTo(x, y);
		if (distance < nearestDistance)
		{
			nearestDistance = distance;
			nearestPointIndex = i;
		}
	}
	return(nearestPointIndex);
}

void BoundaryPointEditor::moveNearestPointTo(int x, int y)
{
	int nearestPointIndex = getNearestPointIndex(x, y);
	points[nearestPointIndex].x = x;
	points[nearestPointIndex].y = y;
	if (nearestPointIndex == 0)
	{
		points[points.size()-1].x = x;
		points[points.size()-1].y = y;
	}
}

bool BoundaryPointEditor::isOverAnyPoint(int x, int y)
{
	for (int i=0; i < points.size(); i++)
		if (points[i].distanceTo(x, y) < CLOSE_DISTANCE)
			return(true);
	return(false);
}

bool BoundaryPointEditor::isPolygonFinished()
{
	return( (points.size() > 2) && points[0].equals(points[points.size()-1]));
}

void BoundaryPointEditor::coutPoints()
{
	cout << points.size() << " total boundary editor points: " << endl;

	for (int i=0; i < points.size(); i++)
		cout << "(" << points[i].x << " " << points[i].y << ") ";
	cout << endl;
}

void BoundaryPointEditor::draw(WorldPlot worldPlot, QPainter &painter)
{
	painter.setPen(Qt::yellow);
	bool isFinished = isPolygonFinished();

	for (int i=1; i < points.size(); i++)
	{
		worldPlot.drawLine(painter, points[i-1].x, points[i-1].y, points[i].x, points[i].y);
		if (isFinished)
		  drawPointBox(worldPlot, painter, points[i]);
	}

	// cout << "pointBox.size=" << (6 * pointBoxScale) << endl;
}

void BoundaryPointEditor::drawPointBox(WorldPlot worldPlot, QPainter &painter, Point point)
{
	QBrush brush(QColor("yellow"));
	double x = point.x;
	double y = point.y;

	int size = 6 * pointBoxScale;
	worldPlot.fillRectangle(painter, brush, x-(size/2), y-(size/2), size, size);
}

bool BoundaryPointEditor::updateScale(double xRange)
{
	float newPointBoxScale = xRange / 450;  //450 is the default range/size if no radar data has been loaded
	bool doUpdate = (newPointBoxScale != pointBoxScale);
	pointBoxScale = newPointBoxScale;
	return(doUpdate);  //only do an update if this value has changed
}

void BoundaryPointEditor::clear()
{
        // cout << "clear points" << endl;
	points.clear();
}

void BoundaryPointEditor::save(string path)
{
	cout << "BoundaryPointEditor, saving boundary with " << points.size() << " points to " << path << endl;

	FILE *file;
	file = fopen(path.c_str(), "wb");
	for (int i=0; i < points.size(); i++)
	{
		fwrite(&points[i].x, sizeof(int), 1, file);
		fwrite(&points[i].y, sizeof(int), 1, file);
	}
	fclose(file);
}

void BoundaryPointEditor::load(string path)
{
	ifstream infile(path);
	if (infile.good())
	{
		FILE *file;
		file = fopen(path.c_str(), "rb");

		//get number of points in file (from file size)
		fseek(file, 0L, SEEK_END);
		int size = ftell(file);
		fseek (file, 0, SEEK_SET);
		int numPoints = size / sizeof(Point);

		//read each point and add to boundary
		points.clear();
		for (int i=0; i < numPoints; i++)
		{
			int x, y;
			fread(&x, sizeof(int), 1, file);
			fread(&y, sizeof(int), 1, file);
			addPoint(x, y);
		}
		fclose (file);

		cout << "BoundaryPointEditor, read " << points.size() << " points from " << path << endl;
	}
	else
		cout << path << " doesn't exist, skipping..." << endl;
}

