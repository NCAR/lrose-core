///////////////////////////////////////////////////
// rotated lat/lon grid support routines
//
// rotated_grid_get_lat_lons is based off of the
// ipolates library function GDSWIZCD
//
///////////////////////////////////////////////////

#include <cmath>
#include <algorithm>

#include "Grib2Nc.hh"

using namespace std;


#define RAD_PER_DEG 0.017453293	/* radians per degree */

#define DEG_PER_RAD 57.29577951 /* degrees per radian */

#define DPR DEG_PER_RAD 

double sign(double a, double b) 
{
  if(b >= 0) {
    return abs(a);
  } else {
    return (double)-1.0*abs(a);
  }
}

int rotated_grid_get_lat_lons(Grib2Nc::GridInfo gridInfo, float *rlon, float *rlat)
{
  if(rlat == NULL || rlon == NULL)
    return 0;
  double rlat1 = gridInfo.minx;
  double rlon1 = gridInfo.miny;
  double rlat0 = gridInfo.proj_origin_lat;
  double rlon0 = gridInfo.proj_origin_lon;
  double rlat2 = gridInfo.lat2;
  double rlon2 = gridInfo.tan_lon;
  int irot = 1;
  int im = gridInfo.ny;
  int jm = gridInfo.nx;
  //int iscan = 0;  assumed
  //int jscan = 1;  assumed
  double hi = 1;
  double hj = 1;
  double slat1 = sin(rlat1/DPR);
  double clat1 = cos(rlat1/DPR);
  double slat0 = sin(rlat0/DPR);
  double clat0 = cos(rlat0/DPR);
  double hs = sign(1.0, fmod(rlon1-rlon0+180+3600,360.0) - 180.0);
  double clon1 = cos((rlon1-rlon0)/DPR);
  double slatr = clat0*slat1-slat0*clat1*clon1;
  double clatr = sqrt(1-pow(slatr,2));
  double clonr = (clat0*clat1*clon1+slat0*slat1)/clatr;
  double rlatr = DPR*asin(slatr);
  double rlonr = hs*DPR*acos(clonr);
  double wbd = rlonr;
  double sbd = rlatr;
  double slat2 = sin(rlat2/DPR);
  double clat2 = cos(rlat2/DPR);
  double hs2 = sign(1.0, fmod(rlon2-rlon0+180+3600,360.0) - 180.0);
  double clon2 = cos((rlon2-rlon0)/DPR);
  slatr = clat0*slat2-slat0*clat2*clon2;
  clatr = sqrt(1-pow(slatr,2));
  clonr = (clat0*clat2*clon2+slat0*slat2)/clatr;
  double nbd = DPR*asin(slatr);
  double ebd = hs2*DPR*acos(clonr);
  double dlats = (nbd-sbd)/(float)(jm-1);
  double dlons = (ebd-wbd)/(float)(im-1);
  double xmin = 0.0;
  double xmax = im+1;
  double ymin = 0.0;
  double ymax = jm+1;

  int npts = im*jm;
  float xpts, ypts;
  int i,j;
  double slat, clat, clon;
  for(int n = 1; n <= npts; n++) {
    j = (n-1)/im+1;
    i = n-im*(j-1);
    //i = (n-1)/jm+1;
    //j = n-jm*(i-1);
    xpts = i;
    ypts = j;

    rlonr = wbd+(xpts-1)*dlons;
    rlatr = sbd+(ypts-1)*dlats;
    if(rlonr <= 0.0) {
      hs = -1.0*hi;
    } else {
      hs = hi;
    }
    clonr = cos(rlonr/DPR);
    slatr = sin(rlatr/DPR);
    clatr = cos(rlatr/DPR);
    slat = clat0*slatr+slat0*clatr*clonr;
    if(slat <= -1.0) {
      clat = 0.0;
      clon = cos(rlon0/DPR);
      rlon[n-1] = 0.0;
      rlat[n-1] = -90.0;
    } else {
      if(slat >= 1.0) {
	clat = 0.0;
	clon = cos(rlon0/DPR);
	rlon[n-1] = 0;
	rlat[n-1] = 90.0;
      } else {
	clat = sqrt(1-pow(slat,2));
	clon = (clat0*clatr*clonr-slat0*slatr)/clat;
	clon = min(max((double)clon,(double)-1.0), (double)1.0);
	rlon[n-1] = fmod(rlon0+hs*DPR*acos(clon)+3600,(float)360.0);
	rlat[n-1] = DPR*asin(slat);
	if(rlon[n-1] > 180.0)
	  rlon[n-1] -= 360.0;
      }
    }
    
  }
/*
  for(int n = 1; n <= npts; n++) {
    j = (n-1)/im+1;
    i = n-im*(j-1);
    cout << "\t"<<n<<"\t"<<i<<"\t"<<j<<"\t"<< rlon[n-1] << "\t" << rlat[n-1] << endl;
  }
*/
  return 1;
}
