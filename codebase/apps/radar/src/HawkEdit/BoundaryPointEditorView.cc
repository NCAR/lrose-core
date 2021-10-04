#include "BoundaryPointEditorView.hh"
//#include "PolarManager.hh"

#include <toolsa/LogStream.hh>

//#include <iostream>
//#include <fstream>
#include <iterator>
#include <QApplication>
#include <QGridLayout>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>


/*
 BoundaryPointEditorView.cc 

 Enables users to load/save up to 5 different boundaries. The boundary names are fixed (e.g. "Boundary1")
 in order to simplify saving/loading them (i.e., user's don't have a file save dialog pop up where they
 navigate to a directory in which they have read/write permissions). Instead, all they have to do is click
 Save and the file is written automatically to a location that the boundary editor knows how to read from
 later.

 Boundary files are written to the user's home directory underneath "/HawkEyeBoundaries". For example,
 to /home/jeff/HawkEyeBoundaries. For each radar file opened and accessed with the Boundary Editor, a
 unique directory (name) is created underneath this directory based on the hash code of the source radar file path.
 For example, if the radar file is loaded from /media/some/dir/cfrad.20170408_001452.962_to_20170408_002320.954_KARX_Surveillance_SUR.nc,
 this might be hash-coded to "1736437357943458505". The boundary directory will then be
 something like /home/jeff/HawkEyeBoundaries/1736437357943458505

 Within that directory will be any boundaries saved by the user. Unique boundaries are saved for each
 field and sweep. So filenames in that directory might be:

   (The user saved 3 different boundaries for field 0, sweep 4)
   field0-sweep4-Boundary1
   field0-sweep4-Boundary2
   field0-sweep4-Boundary3

   (The user saved 1 boundary for field1, sweep 4, and saved one boundary for field2, sweep0)
   field1-sweep4-Boundary1
   field2-sweep0-Boundary1

   Fields are zero indexed. For example, we might have fields "DBZ" (0), "REF" (1), "VEL" (2), ...
   Sweeps are zero indexed. For example, we might have sweeps "4.47" (0), "3.50" (1), ... "0.47" (4)

   So if a radar file has 8 fields, 5 sweeps (and since there are 5 potential boundaries in the screen list),
   the maximum possible boundaries the user could potentially save to disk in the boundary directory is:
     8 * 5 * 5 = 200 files

  Created on: Sept-Nov 2019
      Author: Jeff Smith
  Modified on: June 4, 2021 Split into Model-View-Controller 

*/

BoundaryPointEditorView::BoundaryPointEditorView(QWidget *parent)
  : QDialog(parent) 
// Creates the boundary editor dialog and associated event slots
//void BoundaryPointEditorView::createBoundaryEditorDialog()
{
  //_boundaryEditorDialog = new QDialog(this);
  setMaximumHeight(368);
  setWindowTitle("Boundary Editor");

  Qt::Alignment alignCenter(Qt::AlignCenter);
  // Qt::Alignment alignRight(Qt::AlignRight);

  QGridLayout *_boundaryEditorDialogLayout = new QGridLayout(this); // _boundaryEditorDialog);
  _boundaryEditorDialogLayout->setVerticalSpacing(4);

  int row = 0;
  _boundaryEditorInfoLabel = new QLabel("Boundary Editor allows you to select\nan area of your radar image", this); // _boundaryEditorDialog);
  _boundaryEditorDialogLayout->addWidget(_boundaryEditorInfoLabel, row, 0, 1, 2, alignCenter);

  _boundaryEditorDialogLayout->addWidget(new QLabel(" ", this)); // _boundaryEditorDialog), ++row, 0, 1, 2, alignCenter);

  QLabel *toolsCaption = new QLabel("Editor Tools:", this); // _boundaryEditorDialog);
  _boundaryEditorDialogLayout->addWidget(toolsCaption, ++row, 0, 1, 2, alignCenter);

  _boundaryEditorPolygonBtn = new QPushButton(this); // _boundaryEditorDialog);
  _boundaryEditorPolygonBtn->setMaximumWidth(130);
  _boundaryEditorPolygonBtn->setText(" Polygon");
  _boundaryEditorPolygonBtn->setIcon(QIcon("images/polygon.png"));
  _boundaryEditorPolygonBtn->setCheckable(true);
  _boundaryEditorPolygonBtn->setFocusPolicy(Qt::NoFocus);
  _boundaryEditorDialogLayout->addWidget(_boundaryEditorPolygonBtn, ++row, 0);
  connect(_boundaryEditorPolygonBtn, SIGNAL(clicked()),
   this, SLOT(polygonBtnBoundaryEditorClick()));

  _boundaryEditorCircleBtn = new QPushButton(this); // _boundaryEditorDialog);
  _boundaryEditorCircleBtn->setMaximumWidth(130);
  _boundaryEditorCircleBtn->setText(" Circle  ");
  _boundaryEditorCircleBtn->setIcon(QIcon("images/circle.png"));
  _boundaryEditorCircleBtn->setCheckable(true);
  _boundaryEditorCircleBtn->setFocusPolicy(Qt::NoFocus);
  _boundaryEditorDialogLayout->addWidget(_boundaryEditorCircleBtn, ++row, 0);
  connect(_boundaryEditorCircleBtn, SIGNAL(clicked()),
   this, SLOT(notImplementedMessage()));
   // this, SLOT(circleBtnBoundaryEditorClick()));

  _circleRadiusSlider = new QSlider(Qt::Horizontal);
  _circleRadiusSlider->setFocusPolicy(Qt::StrongFocus);
  _circleRadiusSlider->setTracking(true);
  _circleRadiusSlider->setSingleStep(1);
  _circleRadiusSlider->setPageStep(0);
  _circleRadiusSlider->setFixedWidth(100);
  _circleRadiusSlider->setToolTip("Set the circle radius");
  _circleRadiusSlider->setMaximumWidth(180);
  _circleRadiusSlider->setValue(50);
  _circleRadiusSlider->setMinimum(8);
  _circleRadiusSlider->setMaximum(200);
  _boundaryEditorDialogLayout->addWidget(_circleRadiusSlider, row, 1);
  connect(_circleRadiusSlider, SIGNAL(valueChanged(int)),
   this, SLOT(_circleRadiusSliderValueChanged(int)));

  _boundaryEditorBrushBtn = new QPushButton(this); // _boundaryEditorDialog);
  _boundaryEditorBrushBtn->setMaximumWidth(130);
  _boundaryEditorBrushBtn->setText(" Brush ");
  _boundaryEditorBrushBtn->setIcon(QIcon("images/brush.png"));
  _boundaryEditorBrushBtn->setCheckable(true);
  _boundaryEditorBrushBtn->setFocusPolicy(Qt::NoFocus);
  _boundaryEditorDialogLayout->addWidget(_boundaryEditorBrushBtn, ++row, 0);
  connect(_boundaryEditorBrushBtn, SIGNAL(clicked()),
   this, SLOT(notImplementedMessage()));
   //this, SLOT(brushBtnBoundaryEditorClick()));

  _brushRadiusSlider = new QSlider(Qt::Horizontal);
  _brushRadiusSlider->setFocusPolicy(Qt::StrongFocus);
  _brushRadiusSlider->setTracking(true);
  _brushRadiusSlider->setSingleStep(1);
  _brushRadiusSlider->setPageStep(0);
  _brushRadiusSlider->setFixedWidth(100);
  _brushRadiusSlider->setToolTip("Set the smart brush radius");
  _brushRadiusSlider->setMaximumWidth(180);
  _brushRadiusSlider->setValue(18);
  _brushRadiusSlider->setMinimum(12);
  _brushRadiusSlider->setMaximum(75);
  _boundaryEditorDialogLayout->addWidget(_brushRadiusSlider, row, 1);
  connect(_brushRadiusSlider, SIGNAL(valueChanged(int)),
   this, SLOT(_brushRadiusSliderValueChanged(int)));

  _boundaryEditorPolygonBtn->setChecked(true);
  _boundaryEditorDialogLayout->addWidget(new QLabel(" ", this)); // _boundaryEditorDialog), ++row, 0, 1, 2, alignCenter);

  _boundaryEditorList = new QListWidget(this); // _boundaryEditorDialog);
  QListWidgetItem *newItem5 = new QListWidgetItem;
  newItem5->setText("Boundary5 <none>");
  _boundaryEditorList->insertItem(0, newItem5);
  QListWidgetItem *newItem4 = new QListWidgetItem;
  newItem4->setText("Boundary4 <none>");
  _boundaryEditorList->insertItem(0, newItem4);
  QListWidgetItem *newItem3 = new QListWidgetItem;
  newItem3->setText("Boundary3 <none>");
  _boundaryEditorList->insertItem(0, newItem3);
  QListWidgetItem *newItem2 = new QListWidgetItem;
  newItem2->setText("Boundary2 <none>");
  _boundaryEditorList->insertItem(0, newItem2);
  QListWidgetItem *newItem1 = new QListWidgetItem;
  newItem1->setText("Boundary1");
  _boundaryEditorList->insertItem(0, newItem1);
  _boundaryEditorDialogLayout->addWidget(_boundaryEditorList, ++row, 0, 1, 2);
  connect(_boundaryEditorList, SIGNAL(itemClicked(QListWidgetItem*)),
   this, SLOT(onBoundaryEditorListItemClicked(QListWidgetItem*)));

  // horizontal layout contains the "Clear", "Help", and "Save" buttons
  QHBoxLayout *hLayout = new QHBoxLayout;
  _boundaryEditorDialogLayout->addLayout(hLayout, ++row, 0, 1, 2);

  _boundaryEditorClearBtn = new QPushButton(this); // _boundaryEditorDialog);
  _boundaryEditorClearBtn->setText("Clear");
  hLayout->addWidget(_boundaryEditorClearBtn);
  connect(_boundaryEditorClearBtn, SIGNAL(clicked()),
   this, SLOT(clearBoundaryEditorClick()));

  _boundaryEditorHelpBtn = new QPushButton(this); // _boundaryEditorDialog);
  _boundaryEditorHelpBtn->setText("Help");
  hLayout->addWidget(_boundaryEditorHelpBtn);
  connect(_boundaryEditorHelpBtn, SIGNAL(clicked()),
   this, SLOT(helpBoundaryEditorClick()));

  _boundaryEditorSaveBtn = new QPushButton(this); // _boundaryEditorDialog);
  _boundaryEditorSaveBtn->setText("Save");
  hLayout->addWidget(_boundaryEditorSaveBtn);
  connect(_boundaryEditorSaveBtn, SIGNAL(clicked()),
   this, SLOT(saveBoundaryEditorClick()));

}

void BoundaryPointEditorView::saveBoundaryEditorClick()
{
  LOG(DEBUG) << "enter";
  // boundary editor should manage it's own display and current row. 
  //if (_boundaryDir.empty())
  //  _boundaryDir = BoundaryPointEditor::Instance()->getRootBoundaryDir();
  //ta_makedir_recurse(_boundaryDir.c_str());
  //TODO: should this be in BoundaryPointEditor ?? 

  int boundaryIndex = _boundaryEditorList->currentRow()+1;
  string boundaryName = "Boundary" + to_string(boundaryIndex);
  _boundaryEditorList->currentItem()->setText(boundaryName.c_str());

  emit saveBoundary(boundaryIndex);

  //BoundaryPointEditor::Instance()->save(boundaryIndex); // getBoundaryFilePath(fileName));
  LOG(DEBUG) << "exit";
}

void BoundaryPointEditorView::clearBoundaryEditorClick()
{
  LOG(DEBUG) << "enter";

  int boundaryIndex = _boundaryEditorList->currentRow()+1;
  string boundaryName = "Boundary" + to_string(boundaryIndex);
  _boundaryEditorList->currentItem()->setText(boundaryName.c_str());

  emit clearBoundary();

  LOG(DEBUG) << "exit";
}

// user clicked on the main menu item "Boundary Editor", so toggle it visible or invisible
void BoundaryPointEditorView::showBoundaryEditor()
{
    if (isVisible())
    {
      clearBoundaryEditorClick();
      setVisible(false);
    }
    else
    {
      if (x() == 0 && y() == 0)
      {
        QPoint pos;
        pos.setX(x() + width() + 5);
        pos.setY(y());
        move(pos);
      }
      setVisible(true);
      raise();

      refreshBoundaries();
    }
  
}

// Set the current tool
void BoundaryPointEditorView::setTool(BoundaryToolType tool, int radius)
{
  selectBoundaryTool(tool, radius);
  /*
	//if (tool != currentTool)
		clear();
	//currentTool = tool;
  switch (tool) {
    case BoundaryToolType::circle: {
      setCircleRadius(radius);
      break;
      }
    case BoundaryToolType::brush: {
      setBrushRadius(radius);
      break;
    }  default:
      ; 
  }
  */
}

/*
// Boundary Editor needs to know the world scale so it sizes boundaries correctly on screen
void BoundaryPointEditorView::setWorldScale(float value)
{
	worldScale = value;
//	cout << "BoundaryEditor, setting worldScale=" << worldScale << endl;
}

BoundaryToolType BoundaryPointEditorView::getCurrentTool()
{
	return(currentTool);
}
*/

/*
// returns the points (which are stored in world coordinates, not pixel locations)
vector<Point> BoundaryPointEditorView::getWorldPoints()
{
	return(points);
}

// If given point is very close to the first point, assume the user is closing the polygon
// (relevant with the Polygon Tool)
void BoundaryPointEditorView::checkToMovePointToOriginIfVeryClose(Point &pt)
{
	if (points.size() > 2 && pt.distanceTo(points[0]) < CLOSE_DISTANCE)
	{
		pt.x = points[0].x;
		pt.y = points[0].y;
	}
}

// adds a polygon point (relevant with the Polygon Tool)
void BoundaryPointEditorView::addPoint(float x, float y)
{
	currentTool = BoundaryToolType::polygon;

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
bool BoundaryPointEditorView::doesLastSegmentIntersectAnyOtherSegment(Point &lastPoint)
{
	cout << "points.size()=" << points.size() << endl;

	for (int i=0; i < (int) points.size()-2; i++)
	{
//		cout << "checking (" << points[i].x << "," << points[i].y << ") (" << points[i+1].x << "," << points[i+1].y << ") and (" << points[points.size()-1].x << "," << points[points.size()-1].y << ") (" << lastPoint.x << "," << lastPoint.y << ")" << endl;
		if (PolygonUtils::doSegmentsIntersect(points[i], points[i+1], points[points.size()-1], lastPoint, true))
			return(true);
	}
	return(false);
}


// If user is holding down the Shift key, they can insert a new point into the polygon
// (relevant with the Polygon Tool)
void BoundaryPointEditorView::insertPoint(float x, float y)
{
	Point pt;
	pt.x = x;
	pt.y = y;
	int nearestPtIndex = getNearestPointIndex(x, y, points);
	Point nearestPt = points[nearestPtIndex];

	//remember that the last point is just a duplicate of the first point in a closed polygon, so just ignore this point

	//figure out which of the two possible segments (x,y) is closer to
	int afterPtIndex = nearestPtIndex+1;
	if (afterPtIndex >= (int) points.size()-1)
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
          if (beforePtIndex == (int) points.size()-1)
			points.insert(points.begin() + points.size()-2, pt);
		else
			points.insert(points.begin() + beforePtIndex+1, pt);
	}
}

// If the user is holding down the Shift key over an existing point, they can
// delete that point from the polygon
// (relevant with the Polygon Tool)
void BoundaryPointEditorView::delNearestPoint(float x, float y)
{
  int nearestPtIndex = getNearestPointIndex(x, y, points);
  if (nearestPtIndex == 0 || nearestPtIndex == (int) points.size()-1)
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
void BoundaryPointEditorView::appendFirstPointAsLastPoint()
{
  Point pt;
  pt.x = points[0].x;
  pt.y = points[0].y;
  points.push_back(pt);
}

void BoundaryPointEditorView::eraseLastPoint()
{
	points.erase(points.begin() + points.size()-1);
}

// return the index of the point closest to the given (x,y)
// (relevant with the Polygon Tool)
int BoundaryPointEditorView::getNearestPointIndex(float x, float y, vector<Point> &pts)
{
	int nearestPointIndex = 0;
	float nearestDistance = 99999;

	for (int i=0; i < (int) pts.size(); i++)
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
void BoundaryPointEditorView::moveNearestPointTo(float x, float y)
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

// Return true if (x,y) is very close to any existing point
// (relevant with the Polygon Tool)
bool BoundaryPointEditorView::isOverAnyPoint(float x, float y)
{
  for (int i=0; i < (int) points.size(); i++)
		if (points[i].distanceTo(x, y) < CLOSE_DISTANCE)
			return(true);
	return(false);
}

// return true if the vector of points (the polygon) is closed (meaning the first point == last point)
// (relevant with the Polygon Tool)
bool BoundaryPointEditorView::isAClosedPolygon()
{
	return(currentTool == BoundaryToolType::polygon && (points.size() > 2) && points[0].equals(points[points.size()-1]));
}

// useful debug method
void BoundaryPointEditorView::coutPoints(vector<Point> &pts)
{
	cout << pts.size() << " total boundary editor points: " << endl;

	for (int i=0; i < (int) pts.size(); i++)
		cout << "(" << pts[i].x << " " << pts[i].y << ") ";
	cout << endl;
}
*/

/*

// updates the scale of the boundary (in case the user zoomed in/out)
bool BoundaryPointEditorView::updateScale(double xRange)
{
	float newPointBoxScale = xRange / 450;  //450 is the default range/size if no radar data has been loaded
	bool doUpdate = (newPointBoxScale != pointBoxScale);
	pointBoxScale = newPointBoxScale;
	return(doUpdate);  //only do an update if this value has changed
}

void BoundaryPointEditorView::clear()
{
	points.clear();
}
*/

// TODO: can we not save the boundaries to files on the local machine?
// instead, save the boundaries to field variables in the data file?
// user clicked on one of the 5 boundaries in the boundary editor list, so load that boundary
void BoundaryPointEditorView::onBoundaryEditorListItemClicked(QListWidgetItem* item)
{

  int boundaryIndex = -1;  // indicates no saved boundary
  string fileName = item->text().toUtf8().constData();
  bool found = (fileName.find("<none>") != string::npos);
  if (!found)
  {
    boundaryIndex = _boundaryEditorList->currentRow()+1;
  }
  emit loadBoundary(boundaryIndex);
    /*
    //if (_boundaryDir.empty())
    //  _boundaryDir = BoundaryPointEditor::Instance()->getRootBoundaryDir();
    BoundaryPointEditor::Instance()->load(getBoundaryFilePath(fileName));

    BoundaryToolType currentTool = BoundaryPointEditor::Instance()->getCurrentTool();
    if (currentTool == BoundaryToolType::circle)
    {
      _circleRadiusSlider->setValue(BoundaryPointEditor::Instance()->getCircleRadius());
      selectBoundaryTool(BoundaryToolType::circle);
    }
    else if (currentTool == BoundaryToolType::brush)
    {
      _brushRadiusSlider->setValue(BoundaryPointEditor::Instance()->getBrushRadius());
      selectBoundaryTool(BoundaryToolType::brush);
    }
    else
    {
      selectBoundaryTool(BoundaryToolType::polygon);
    }

    _ppi->update();   //forces repaint which clears existing polygon
    */
  //}
  
}

/*
// loaded a boundary from storage; update the GUI
// from Model to GUI
bool BoundaryPointEditorView::setCircleRadius(int value)
{

  
	BoundaryToolType currentTool = BoundaryPointEditor::Instance()->getCurrentTool();
	circleRadius = value;
	bool resizeExistingCircle = (currentTool == BoundaryToolType::circle && points.size() > 1);
	if (resizeExistingCircle)  //resize existing circle
		makeCircle(circleOrigin.x, circleOrigin.y, circleRadius);
	return(resizeExistingCircle);
  
}

void BoundaryPointEditorView::setBrushRadius(int value)
{
  //emit boundaryBrushRadiusChanged(value);
	//brushRadius = value;
}
*/

// slots for signals coming from the BoundaryPointEditor Dialog ...
// from GUI to Model
// user has dragged circle slider and reset the circle radius
void BoundaryPointEditorView::_circleRadiusSliderValueChanged(int value) 
{
  emit boundaryCircleRadiusChanged(value);
}
// user has dragged the brush slider and reset its size
void BoundaryPointEditorView::_brushRadiusSliderValueChanged(int value) 
{
  emit boundaryBrushRadiusChanged(value);
}

/*
// add (x,y) to the current brush shape
void BoundaryPointEditorView::addToBrushShape(float x, float y)
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

float BoundaryPointEditorView::getBrushWorldRadius()
{
	float scaleFactor = worldScale / 480;  //480 is worldScale on program startup
	scaleFactor = sqrt(scaleFactor);

	return(brushRadius * scaleFactor);
}

// returns the average distance between points in the boundary
// (relevant with the Brush Tool)
int BoundaryPointEditorView::getAvgDistBetweenPoints()
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
void BoundaryPointEditorView::removePointsExceedingMaxGap()
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
float BoundaryPointEditorView::getMaxGapInPoints()
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
void BoundaryPointEditorView::reorderPointsSoStartingPointIsOppositeOfXY(int x, int y)
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
int BoundaryPointEditorView::erasePointsCloseToXYandReturnFirstIndexErased(int x, int y, int thresholdDistance)
{
	bool isDone = false;
	int firstIndexErased = -1;

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
int BoundaryPointEditorView::getFurthestPtIndex(int x, int y)
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
void BoundaryPointEditorView::makeCircle(int x, int y, float radius)
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
void BoundaryPointEditorView::checkToAddOrDelPoint(float x, float y)
{
	bool isOverExistingPt = isOverAnyPoint(x, y);
	bool isShiftKeyDown = (QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier) == true);
	if (isShiftKeyDown)
	{
		if (isOverExistingPt)
				BoundaryPointEditorView::Instance()->delNearestPoint(x, y);
			else
				BoundaryPointEditor::Instance()->insertPoint(x, y);
	}
}

int BoundaryPointEditorView::getCircleRadius()
{
	return(circleRadius);
}

int BoundaryPointEditorView::getBrushRadius()
{
	return(brushRadius);
}

*/
/*
  boundaryFilePath will be something like "/home/jeff/HawkEyeBoundaries/7996122556911878505/field0-sweep0-Boundary1"
  Method reads the binary file and returns the points within it as a vector<Point>
 */
/*
vector<Point> BoundaryPointEditorView::getPoints(string boundaryFilePath)
{
	vector<Point> pts;

	ifstream infile(boundaryFilePath);
	if (infile.good())
	{
		FILE *file;
		file = fopen(boundaryFilePath.c_str(), "rb");

		//get number of points in file (from file size)
		fseek(file, 0L, SEEK_END);
		int headerSize = 3*sizeof(int) + 2*sizeof(float);
		int numPoints = (ftell(file) - headerSize) / sizeof(Point);
		fseek (file, headerSize, SEEK_SET);

		//now read each point and add to boundary
		for (int i=0; i < numPoints; i++)
		{
			Point pt;
			fread(&pt.x, sizeof(float), 1, file);
			fread(&pt.y, sizeof(float), 1, file);
			points.push_back(pt);
		}
		fclose (file);
	}

	return(pts);
}
*/

/*
string BoundaryPointEditorView::getRootBoundaryDir()
{
	return(rootBoundaryDir);
}
*/
/*
  radarFilePath will be something like "/media/sf_lrose/ncswp_SPOL_RHI_.nc"
  fieldIndex is zero based. E.g., "DBZ" is usually fieldIndex 0, "REF" is fieldIndex 1
  sweepIndex is zero based. E.g., "4.47" might be sweepIndex 0, "3.50" sweepIndex 1, ... "0.47" sweepIndex 4
  boundaryFileName will be one of 5 values ("Boundary1" .. "Boundary5")
  Returns the list of world points as vector<Point>
 */
/*
vector<Point> BoundaryPointEditorView::getBoundaryPoints(string radarFilePath, string &fieldName, int sweepIndex, string boundaryFileName)
{
	string boundaryDir = getBoundaryDirFromRadarFilePath(rootBoundaryDir, radarFilePath);
	string boundaryFilePath = getBoundaryFilePath(boundaryDir, fieldName, sweepIndex, boundaryFileName);
	vector<Point> pts = getPoints(boundaryFilePath);
	return(pts);
}
*/

/*
  rootBoundaryDir will be something like "/home/jeff/HawkEyeBoundaries" (home directory + "/HawkEyeBoundaries")
  radarFilePath will be something like "/media/sf_lrose/ncswp_SPOL_RHI_.nc"
  returns the boundary dir for this radar file (e.g. "/home/jeff/HawkEyeBoundaries/1736437357943458505")
 */
/*
string BoundaryPointEditorView::getBoundaryDirFromRadarFilePath(string rootBoundaryDir, string radarFilePath)
{
	hash<string> str_hash;
	long hash = str_hash(radarFilePath); //e.g., converts "/media/sf_lrose/ncswp_SPOL_RHI_.nc" to something like 1736437357943458505
	stringstream ss;
	ss << hash;
	return(rootBoundaryDir + "/" + ss.str());
}
*/
/*
void BoundaryPointEditorView::setBoundaryDir(string &openFilePath)
{
  if (!openFilePath.empty()) {
    _boundaryDir =
      BoundaryPointEditor::Instance()->getBoundaryDirFromRadarFilePath
      (BoundaryPointEditor::Instance()->getRootBoundaryDir(), openFilePath);
  } else {
    _boundaryDir = BoundaryPointEditor::Instance()->getRootBoundaryDir();
  }
}
*/
/*
  boundaryDir will be something like "/home/jeff/HawkEyeBoundaries/1736437357943458505" (where "1736437357943458505" is the hash code of the radar source filepath)
  fieldIndex is zero based. E.g., "DBZ" is usually fieldIndex 0, "REF" is fieldIndex 1
  sweepIndex is zero based. E.g., "4.47" might be sweepIndex 0, "3.50" sweepIndex 1, ... "0.47" sweepIndex 4
  boundaryFileName will be one of 5 values ("Boundary1" .. "Boundary5")

string BoundaryPointEditorView::getBoundaryFilePath(string &fieldName, int sweepIndex, string boundaryFileName)
{
	return(_boundaryDir + "/field" + fieldName + "-sweep" + to_string(sweepIndex) + "-" + boundaryFileName);
}
*/
void BoundaryPointEditorView::helpBoundaryEditorClick()
{
  QDesktopServices::openUrl(QUrl("https://vimeo.com/369963107"));
}

// Select the given tool and set the hint text, while also un-selecting the other tools
void BoundaryPointEditorView::selectBoundaryTool(BoundaryToolType tool, int radius)
{
  _boundaryEditorPolygonBtn->setChecked(false);
  _boundaryEditorCircleBtn->setChecked(false);
  _boundaryEditorBrushBtn->setChecked(false);

  if (tool == BoundaryToolType::polygon)
  {
    _boundaryEditorPolygonBtn->setChecked(true);
    _boundaryEditorInfoLabel->setText("Polygon: click points over desired area to\ndraw a polygon. Click near the first point\nto close it. Once closed, hold the Shift\nkey to insert/delete points.");
  }
  else if (tool == BoundaryToolType::circle)
  {
    _boundaryEditorCircleBtn->setChecked(true);
    _boundaryEditorInfoLabel->setText("Circle: click on the main window to\ncreate your circle. You can then adjust\nthe radius slider to rescale it to the\ndesired size.");
    _circleRadiusSlider->setValue(radius);
  }
  else
  {
    _boundaryEditorBrushBtn->setChecked(true);
    _boundaryEditorInfoLabel->setText("Brush: adjust slider to set brush size.\nClick/drag the mouse to 'paint' boundary.\nClick inside your shape and drag outwards\nto enlarge a desired region.");
    _brushRadiusSlider->setValue(radius);
  }
}

int BoundaryPointEditorView::firstBoundaryIndex() {
  return 1;
}

int BoundaryPointEditorView::lastBoundaryIndex() {
  return nBoundaries;
}

// User clicked on the polygonBtn
void BoundaryPointEditorView::polygonBtnBoundaryEditorClick()
{
  selectBoundaryTool(BoundaryToolType::polygon);
  //BoundaryPointEditor::Instance()->setTool(BoundaryToolType::polygon);
  //emit ppi_update();
  emit userClickedPolygonButton();
}

// User clicked on the circleBtn
void BoundaryPointEditorView::circleBtnBoundaryEditorClick()
{
  selectBoundaryTool(BoundaryToolType::circle);
  //BoundaryPointEditor::Instance()->setTool(BoundaryToolType::circle);
  emit userClickedCircleButton();
}

// User clicked on the brushBtn
void BoundaryPointEditorView::brushBtnBoundaryEditorClick()
{
  selectBoundaryTool(BoundaryToolType::brush);
  //BoundaryPointEditor::Instance()->setTool(BoundaryToolType::brush);
  emit userClickedBrushButton();
}


// check which (if any) of the 5 boundary files exist, and populate the list accordingly
void BoundaryPointEditorView::refreshBoundaries()
{

  //rename any items that have corresponding file on disk
  //for (int i=1; i <= 5; i++)
  //{
    emit refreshBoundariesEvent();
    //const char *fileName = BoundaryPointEditor::Instance()->refreshBoundary(i);
    //_boundaryEditorList->item(i-1)->setText(fileName.c_str());
  //}

  _boundaryEditorList->setCurrentRow(0);

  if (isVisible())
    onBoundaryEditorListItemClicked(_boundaryEditorList->currentItem());
}


void BoundaryPointEditorView::setBoundaryFile(int boundaryIndex, string &fileName) 
{
  if ((boundaryIndex > 0) && (boundaryIndex < nBoundaries))
  _boundaryEditorList->item(boundaryIndex-1)->setText(fileName.c_str());
}

void BoundaryPointEditorView::closeEvent() {
    emit boundaryPointEditorClosed();
}


void BoundaryPointEditorView::notImplementedMessage() {
      QMessageBox::information(this, "Not Implemented", "Not Implemented");
      selectBoundaryTool(BoundaryToolType::polygon, 0);
}

/*
void BoundaryPointEditorView::coutMemUsage()
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
