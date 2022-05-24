#ifndef PolygonUtils_HH
#define PolygonUtils_HH

#include "Point.hh"
#include <vector>

using namespace std;

// PolygonUtils contains some useful methods for working with polygons
class PolygonUtils
{
public:

	//returns true if polygon (pts) doesn't have any segments that intersect each other
	static bool isValidPolygon(vector<Point> &pts)
	{
		if (pts.size() < 4)
			return(false);

		for (int i=1; i < (int) pts.size()-1; i++)
                  for (int j=1; j < (int) pts.size()-1; j++)
			{
				bool isCommonPt = (i == j || pts[i-1].equals(pts[j]));

				if (!isCommonPt && PolygonUtils::doSegmentsIntersect(pts[i], pts[i-1], pts[j], pts[j-1], true))
				{
//					cout << "polygon segments intersect! i=" << i << " j=" << j << endl;
//					cout << "(" << pts[i].x << "," << pts[i].y << ") (" << pts[i-1].x << "," << pts[i-1].y << ")   (" << pts[j].x << "," << pts[j].y << ") (" << pts[j-1].x << "," << pts[j-1].y << ")" << endl;
					return(false);
				}
			}
		return(true);
	}

	// if any segments in polygon (pts) intersect, attempt to move points so they no longer intersect
	static void fixInvalidPolygon(vector<Point> &pts)
	{
		if (pts.size() < 4)
			return;

		for (int i=1; i < (int) pts.size()-1; i++)
		{
                  for (int j=1; j < (int) pts.size()-1; j++)
			{
				bool isCommonPt = (i == j || pts[i-1].equals(pts[j]));

				if (!isCommonPt && PolygonUtils::doSegmentsIntersect(pts[i], pts[i-1], pts[j], pts[j-1], true))
				{
					mergeClosestPoints(pts[i], pts[i-1], pts[j], pts[j-1]);
					return;
				}
			}
		}
	}

	//returns true if the given point (p) lies inside polygon (polyPts)
	static bool isPointInsidePolygon(Point &p, vector<Point> &polyPts)
	{
		Point p2;
		p2.x = p.x;
		p2.y = 1000;
		int cntOfCrosses = 0;

		for (int i=0; i <(int)  polyPts.size()-1; i++)
			if (PolygonUtils::doSegmentsIntersect(p, p2, polyPts[i], polyPts[i+1], false))
				cntOfCrosses++;

		bool isOddNumberOfCrosses = (cntOfCrosses % 2 != 0);
		return(isOddNumberOfCrosses);
	}

	// Returns true if line segment 'p1q1' and 'p2q2' intersect.
	static bool doSegmentsIntersect(Point &p1, Point &q1, Point &p2, Point &q2, bool ignoreCommonPoints)
	{
		bool sharesCommonPt = (p1.equals(p2) || p1.equals(q2) || q1.equals(q2));
		if (ignoreCommonPoints && sharesCommonPt)  //if two segments share common point, return false
			return(false);

		// Find the four orientations needed for general and
		// special cases
		int o1 = orientation(p1, q1, p2);
		int o2 = orientation(p1, q1, q2);
		int o3 = orientation(p2, q2, p1);
		int o4 = orientation(p2, q2, q1);

		// General case
		if (o1 != o2 && o3 != o4)
				return true;

		// Special Cases
		// p1, q1 and p2 are colinear and p2 lies on segment p1q1
		if (o1 == 0 && onSegment(p1, p2, q1))
			return true;

		// p1, q1 and q2 are colinear and q2 lies on segment p1q1
		if (o2 == 0 && onSegment(p1, q2, q1))
			return true;

		// p2, q2 and p1 are colinear and p1 lies on segment p2q2
		if (o3 == 0 && onSegment(p2, p1, q2))
			return true;

		 // p2, q2 and q1 are colinear and q1 lies on segment p2q2
		if (o4 == 0 && onSegment(p2, q1, q2))
			return true;

		return false; // Doesn't fall in any of the above cases
	}

private:

	// Given three colinear points p, q, r, the function checks if
	// point q lies on line segment 'pr'
	static bool onSegment(Point &p, Point &q, Point &r)
	{
	    if (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) &&
	    		q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y))
	       return true;

	    return false;
	}

	// To find orientation of ordered triplet (p, q, r).
	// The function returns following values
	// 0 --> p, q and r are colinear
	// 1 --> Clockwise
	// 2 --> Counterclockwise
	static int orientation(Point &p, Point &q, Point &r)
	{
	    // See https://www.geeksforgeeks.org/orientation-3-ordered-points/
	    // for details of below formula.
	    float val = (q.y - p.y) * (r.x - q.x) -	(q.x - p.x) * (r.y - q.y);

	    if (val == 0)
	    	return 0;  // colinear

	    return (val > 0)? 1: 2; // clock or counterclock wise
	}

	// used to fix invalid polygons. Method determines which 2 points are closest to each other
	// and then merges them (assigns both to the midpoint between the 2)
	static void mergeClosestPoints(Point &p1, Point &p2, Point &p3, Point &p4)
	{
		float d12 = p1.distanceTo(p2);
		float d13 = p1.distanceTo(p3);
		float d14 = p1.distanceTo(p4);
		float d23 = p2.distanceTo(p3);
		float d24 = p2.distanceTo(p4);
		float d34 = p3.distanceTo(p4);

		if (d12<d13 && d12<d14 && d12<d23 && d12<d24 && d12<d34)
			mergeToMidPoint(p1,p2);
		else if (d13<d14 && d13<d23 && d13<d24 && d13<d34)
			mergeToMidPoint(p1,p3);
		else if (d14<d23 && d14<d24 && d14<d34)
			mergeToMidPoint(p1,p4);
		else if (d23<d24 && d23<d34)
			mergeToMidPoint(p2,p3);
		else if (d24<d34)
			mergeToMidPoint(p2,p4);
		else
			mergeToMidPoint(p3,p4);
	}

	// sets points a and b to the midpoint between them
	static void mergeToMidPoint(Point &a, Point &b)
	{
		float xMid = (a.x + b.x) / 2;
		float yMid = (a.y + b.y) / 2;
		a.x = xMid;
		b.x = xMid;
		a.y = yMid;
		b.y = yMid;
	}

};

#endif
