/*
  boundbox

Implementation of boundingBox and llBoundingBox classes

*/

#include "draw.h"
#include "rdrutils.h"

void boundingBox::clear()
{
  bbMin[0] = bbMin[1] = bbMin[2] = MAXFLOAT;
  bbMax[0] = bbMax[1] = bbMax[2] = -MAXFLOAT;
  valid = false;
}

// an openBox passes all
void boundingBox::openBox()
{
  bbMin[0] = bbMin[1] = bbMin[2] = -MAXFLOAT;
  bbMax[0] = bbMax[1] = bbMax[2] = MAXFLOAT;
  valid = true;
}

void boundingBox::set(float x1, float y1, float z1,
		      float x2, float y2, float z2)
{
  clear();
  test(x1, y1, z1);
  test(x2, y2, z2);
};

void boundingBox::test(float x, float y, float z)
{
  if (x < bbMin[0])
    bbMin[0] = x;
  if (y < bbMin[1])
    bbMin[1] = y;
  if (z < bbMin[2])
    bbMin[2] = z;
  if (x > bbMax[0])
    bbMax[0] = x;
  if (y > bbMax[1])
    bbMax[1] = y;
  if (z > bbMax[2])
    bbMax[2] = z;
  valid = true;
}

void boundingBox::test(boundingBox *bbox)
{
  if (!bbox || !valid)
    return;
  if (bbox->bbMin[0] < bbMin[0])
    bbMin[0] = bbox->bbMin[0];
  if (bbox->bbMin[1] < bbMin[1])
    bbMin[1] = bbox->bbMin[1];
  if (bbox->bbMin[2] < bbMin[2])
    bbMin[2] = bbox->bbMin[2];
  if (bbox->bbMax[0] > bbMax[0])
    bbMax[0] = bbox->bbMax[0];
  if (bbox->bbMax[1] > bbMax[1])
    bbMax[1] = bbox->bbMax[1];
  if (bbox->bbMax[2] > bbMax[2])
    bbMax[2] = bbox->bbMax[2];
  valid = true;
}

bool boundingBox::inBox(float x, float y, float z)
{
  return (inBox(x, y) && 
	  (z >= bbMin[2]) && (z <= bbMax[2]));
}
 
bool boundingBox::inBox(float x, float y)
{
  return valid &&
    (x >= bbMin[0]) && (x <= bbMax[0]) &&
    (y >= bbMin[1]) && (y <= bbMax[1]);
}

bool llBoundingBox::setLatLongRng(float lat, float lng, float rng)
{
  clear();
  LatLongHt templlh(lat, lng);
  LatLongHt destllh;
  if (!LatLongKmNE2LatLong(&templlh, rng, rng, 0, &destllh))
    {
      openBox();
      return false;
    }
  else
    test(destllh.Lat, destllh.Long, destllh.Ht);
  if (!LatLongKmNE2LatLong(&templlh, -rng, -rng, 0, &destllh))
    {
      openBox();
      return false;
    }
  else
    test(destllh.Lat, destllh.Long, destllh.Ht);
  return true;
}
