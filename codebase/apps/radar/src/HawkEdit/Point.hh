
#ifndef struct_Point_HH
#define struct_Point_HH


// A boundary is a vector list of Points
struct Point
{
  float x;
  float y;

  //move this point towards given point by some fractional amount
  void lerp(Point pt, float amount)
  {
  	x = x + amount * (float)(pt.x - x);
  	y = y + amount * (float)(pt.y - y);
  }

  //return the distance to (x2,y2)
  float distanceTo(float x2, float y2)
  {
    float dx = (x2 - x);
    float dy = (y2 - y);
    float dist = sqrt(dx*dx + dy*dy);
    return(dist);
  }

  //return the distance to other
  float distanceTo(Point other)
  {
    float dx = (other.x - x);
    float dy = (other.y - y);
    float dist = sqrt(dx*dx + dy*dy);
    return(dist);
  }

  //return the distance to line segment (x1, y1, x2, y2)
  float distToLineSegment(float x1, float y1, float x2, float y2)
  {
  	float A = x - x1;
  	float B = y - y1;
  	float C = x2 - x1;
  	float D = y2 - y1;

  	float dot = A * C + B * D;
  	float len_sq = C * C + D * D;
  	float param = -1;
		if (len_sq != 0) //in case of 0 length line
				param = dot / len_sq;

		float xx, yy;

		if (param < 0)
		{
			xx = x1;
			yy = y1;
		}
		else if (param > 1)
		{
			xx = x2;
			yy = y2;
		}
		else
		{
			xx = x1 + param * C;
			yy = y1 + param * D;
		}

		float dx = x - xx;
		float dy = y - yy;
		float distance = sqrt(dx * dx + dy * dy);
		return distance;
  }

  //returns true if this point == other
  bool equals(Point other)
  {
    return(x == other.x && y == other.y);
  }
};

#endif
