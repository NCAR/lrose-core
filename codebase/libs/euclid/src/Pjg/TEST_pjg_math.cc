/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1992 - 2001 */
/* ** University Corporation for Atmospheric Research(UCAR) */
/* ** National Center for Atmospheric Research(NCAR) */
/* ** Research Applications Program(RAP) */
/* ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA */
/* ** 2001/11/19 23:15:6 */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/*
 * Name: TEST_pjg_math.cc
 *
 * Purpose:
 *
 *      To test the PJG engine module in the library: euclid
 *
 * Usage:
 *
 *       % TEST_pjg_math
 *
 * Inputs: 
 *
 *       None
 *
 *
 * Author: Mike Dixon
 * Modified from code by Young Rhee for toolsa pjg testing
 */

/*
 * include files
 */

#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <euclid/PjgMath.hh>

// random number generation

#define mod_diff(x,y) (((x) -(y) ) &0x7fffffff)
#define gb_next_rand() (*gb_fptr>=0?*gb_fptr--:gb_flip_cycle())
#define two_to_the_31 ((unsigned long) 0x80000000)

#define TINY_DIST 0.0001
#define SMALL_DIST 0.01

static long A[56] = {-1};
static long *gb_fptr = A;

static long gb_flip_cycle(void);
static void gb_init_rand(long seed);
static double rand_in_range(double minval, double maxval);

/*--------------------------------*/
static int
test_AzimEquiDist(FILE *fp_err)
{

  /*
   * Inputs:  None
   *
   * Returns: 0: on succeed
   *          1: on failure
   *
   * Function:
   *      This routine tests the grid functions
   */

  int retval = 0;

  /*
   * set geometry for test grid
   */

  double origin_lat = rand_in_range(-70.0, 70.0);
  double origin_lon = rand_in_range(-180.0, 180.0);
  double min_x = rand_in_range(-100.0, 100.0);
  double min_y = rand_in_range(-100.0, 100.0);
  double dx = rand_in_range(0.25, 2.0);
  double dy = rand_in_range(0.25, 2.0);
  int nx = 100;
  int ny = 100;
  double offset_lat = rand_in_range(origin_lat - 10.0, origin_lat + 10.0);
  if (offset_lat < -90) {
    offset_lat = -90;
  }
  if (offset_lat > 90) {
    offset_lat = 90;
  }
  double offset_lon = rand_in_range(origin_lon - 20.0, origin_lon + 20.0);
  double false_easting = rand_in_range(0, 100);
  double false_northing = rand_in_range(0, 100);
  
  // check for 0 rotation

  double rotation = 0.0;
  PjgAzimEquidistMath math(origin_lat, origin_lon, rotation);
  math.setOffsetOrigin(offset_lat, offset_lon);

  for (int iy = 0; iy < ny; iy++) {
    for (int ix = 0; ix < nx; ix++) {

      double xx1 = min_x + ix * dx;
      double yy1 = min_y + iy * dy;

      double lat, lon;
      math.xy2latlon(xx1, yy1, lat, lon);
      
      double xx2, yy2;
      math.latlon2xy(lat, lon, xx2, yy2);

      double diff_x = xx1 - xx2;
      double diff_y = yy1 - yy2;
      if (fabs(diff_x) > TINY_DIST || fabs(diff_y) > TINY_DIST) {
        fprintf(fp_err, "================================\n");
        fprintf(fp_err, "FAILED AzimEquiDist test\n");
        fprintf(fp_err, "  xx1, xx2, err    : %15.10f, %15.10f, %15.10f\n",
                xx1, xx2, xx1 - xx2);
        fprintf(fp_err, "  yy1, yy2, err    : %15.10f, %15.10f, %15.10f\n",
                yy1, yy2, yy1 - yy2);
        fprintf(fp_err, "  lat, lon         : %15.10f, %15.10f\n", lat, lon);
        fprintf(fp_err, "  Grid:\n");
        fprintf(fp_err, "  org_lat, org_lon : %15.10f, %15.10f\n",
                origin_lat, origin_lon);
        fprintf(fp_err, "  rotation         : %15.10f\n", rotation);
        fprintf(fp_err, "  min_x, min_y     : %15.10f, %15.10f\n",
                min_x, min_y);
        fprintf(fp_err, "  dx, dy           : %15.10f, %15.10f\n", dx, dy);
        retval = 1;
      }
      
    } // ix
  } // iy

  // check for random rotation

  rotation = rand_in_range(-45.0, 45.0);
  PjgAzimEquidistMath math2(origin_lat, origin_lon, rotation);
  math.setOffsetOrigin(offset_lat, offset_lon);
  math.setOffsetCoords(false_northing, false_easting);
  
  for (int iy = 0; iy < ny; iy++) {
    for (int ix = 0; ix < nx; ix++) {

      double xx1 = min_x + ix * dx;
      double yy1 = min_y + iy * dy;

      double lat, lon;
      math2.xy2latlon(xx1, yy1, lat, lon);
      
      double xx2, yy2;
      math2.latlon2xy(lat, lon, xx2, yy2);

      double diff_x = xx1 - xx2;
      double diff_y = yy1 - yy2;
      if (fabs(diff_x) > TINY_DIST || fabs(diff_y) > TINY_DIST) {
        fprintf(fp_err, "================================\n");
        fprintf(fp_err, "FAILED AzimEquiDist test\n");
        fprintf(fp_err, "  xx1, xx2, err    : %15.10f, %15.10f, %15.10f\n",
                xx1, xx2, xx1 - xx2);
        fprintf(fp_err, "  yy1, yy2, err    : %15.10f, %15.10f, %15.10f\n",
                yy1, yy2, yy1 - yy2);
        fprintf(fp_err, "  lat, lon         : %15.10f, %15.10f\n", lat, lon);
        fprintf(fp_err, "  Grid:\n");
        fprintf(fp_err, "  org_lat, org_lon : %15.10f, %15.10f\n",
                origin_lat, origin_lon);
        fprintf(fp_err, "  rotation         : %15.10f\n", rotation);
        fprintf(fp_err, "  min_x, min_y     : %15.10f, %15.10f\n",
                min_x, min_y);
        fprintf(fp_err, "  dx, dy           : %15.10f, %15.10f\n", dx, dy);
        retval = 1;
      }
      
    } // ix
  } // iy

  /*
   * Done
   */
  
  if(retval) {
    fprintf(stderr, "FAILED test TEST_pjg_math - AzimEquiDist.\n");
  }
    
  return(retval);

}

/*--------------------------------*/
static int
test_LambertAzimEqualArea(FILE *fp_err)
{

  /*
   * Inputs:  None
   *
   * Returns: 0: on succeed
   *          1: on failure
   *
   * Function:
   *      This routine tests the grid functions
   */

  int retval = 0;

  /*
   * set geometry for test grid
   */

  double origin_lat = rand_in_range(-70.0, 70.0);
  double origin_lon = rand_in_range(-180.0, 180.0);
  double min_x = rand_in_range(-100.0, 100.0);
  double min_y = rand_in_range(-100.0, 100.0);
  double dx = rand_in_range(0.25, 2.0);
  double dy = rand_in_range(0.25, 2.0);
  int nx = 100;
  int ny = 100;
  double offset_lat = rand_in_range(origin_lat - 10.0, origin_lat + 10.0);
  if (offset_lat < -90) {
    offset_lat = -90;
  }
  if (offset_lat > 90) {
    offset_lat = 90;
  }
  double offset_lon = rand_in_range(origin_lon - 20.0, origin_lon + 20.0);
  double false_easting = rand_in_range(0, 100);
  double false_northing = rand_in_range(0, 100);
  
  PjgLambertAzimMath math(origin_lat, origin_lon);
  math.setOffsetOrigin(offset_lat, offset_lon);
  math.setOffsetCoords(false_northing, false_easting);

  for (int iy = 0; iy < ny; iy++) {
    for (int ix = 0; ix < nx; ix++) {

      double xx1 = min_x + ix * dx;
      double yy1 = min_y + iy * dy;

      double lat, lon;
      math.xy2latlon(xx1, yy1, lat, lon);
      
      double xx2, yy2;
      math.latlon2xy(lat, lon, xx2, yy2);

      double diff_x = xx1 - xx2;
      double diff_y = yy1 - yy2;
      if (fabs(diff_x) > TINY_DIST || fabs(diff_y) > TINY_DIST) {
        fprintf(fp_err, "================================\n");
        fprintf(fp_err, "FAILED LambertAzimEqualArea test\n");
        fprintf(fp_err, "  xx1, xx2, err    : %15.10f, %15.10f, %15.10f\n",
                xx1, xx2, xx1 - xx2);
        fprintf(fp_err, "  yy1, yy2, err    : %15.10f, %15.10f, %15.10f\n",
                yy1, yy2, yy1 - yy2);
        fprintf(fp_err, "  lat, lon         : %15.10f, %15.10f\n", lat, lon);
        fprintf(fp_err, "  Grid:\n");
        fprintf(fp_err, "  org_lat, org_lon : %15.10f, %15.10f\n",
                origin_lat, origin_lon);
        fprintf(fp_err, "  min_x, min_y     : %15.10f, %15.10f\n",
                min_x, min_y);
        fprintf(fp_err, "  dx, dy           : %15.10f, %15.10f\n", dx, dy);
        retval = 1;
      }
      
    } // ix
  } // iy

  /*
   * Done
   */
  
  if(retval) {
    fprintf(stderr, "FAILED test TEST_pjg_math - LambertAzimEqualArea.\n");
  }
    
  return(retval);

}

/*--------------------------------*/
static int
test_LambertConfConic(FILE *fp_err)
{

  /*
   * Inputs:  None
   *
   * Returns: 0: on succeed
   *          1: on failure
   *
   * Function:
   *      This routine tests the grid functions
   */

  int retval = 0;

  /*
   * set geometry for test grid
   */

  double origin_lat = rand_in_range(-70.0, 70.0);
  double origin_lon = rand_in_range(-180.0, 180.0);
  double deltaLat = rand_in_range(0.0, 10.0);
  double lat1 = origin_lat - deltaLat;
  double lat2 = origin_lat + deltaLat;
  double min_x = rand_in_range(-100.0, 100.0);
  double min_y = rand_in_range(-100.0, 100.0);
  double dx = rand_in_range(0.25, 2.0);
  double dy = rand_in_range(0.25, 2.0);
  int nx = 100;
  int ny = 100;
  double offset_lat = rand_in_range(origin_lat - 10.0, origin_lat + 10.0);
  if (offset_lat < -90) {
    offset_lat = -90;
  }
  if (offset_lat > 90) {
    offset_lat = 90;
  }
  double offset_lon = rand_in_range(origin_lon - 20.0, origin_lon + 20.0);
  double false_easting = rand_in_range(0, 100);
  double false_northing = rand_in_range(0, 100);
  
  PjgLambertConfMath math(origin_lat, origin_lon, lat1, lat2);
  math.setOffsetOrigin(offset_lat, offset_lon);
  math.setOffsetCoords(false_northing, false_easting);

  for (int iy = 0; iy < ny; iy++) {
    for (int ix = 0; ix < nx; ix++) {

      double xx1 = min_x + ix * dx;
      double yy1 = min_y + iy * dy;

      double lat, lon;
      math.xy2latlon(xx1, yy1, lat, lon);
      
      double xx2, yy2;
      math.latlon2xy(lat, lon, xx2, yy2);

      double diff_x = xx1 - xx2;
      double diff_y = yy1 - yy2;
      if (fabs(diff_x) > TINY_DIST || fabs(diff_y) > TINY_DIST) {
        fprintf(fp_err, "================================\n");
        fprintf(fp_err, "FAILED LambertConfConic test\n");
        fprintf(fp_err, "  xx1, xx2, err    : %15.10f, %15.10f, %15.10f\n",
                xx1, xx2, xx1 - xx2);
        fprintf(fp_err, "  yy1, yy2, err    : %15.10f, %15.10f, %15.10f\n",
                yy1, yy2, yy1 - yy2);
        fprintf(fp_err, "  lat, lon         : %15.10f, %15.10f\n", lat, lon);
        fprintf(fp_err, "  Grid:\n");
        fprintf(fp_err, "  org_lat, org_lon : %15.10f, %15.10f\n",
                origin_lat, origin_lon);
        fprintf(fp_err, "  lat1, lat2       : %15.10f, %15.10f\n",
                lat1, lat2);
        fprintf(fp_err, "  min_x, min_y     : %15.10f, %15.10f\n",
                min_x, min_y);
        fprintf(fp_err, "  dx, dy           : %15.10f, %15.10f\n", dx, dy);
        retval = 1;
      }
      
    } // ix
  } // iy

  /*
   * Done
   */
  
  if(retval) {
    fprintf(stderr, "FAILED test TEST_pjg_math - LambertConfConic.\n");
  }
    
  return(retval);

}

/*--------------------------------*/
static int
test_AlbersConic(FILE *fp_err)
{

  /*
   * Inputs:  None
   *
   * Returns: 0: on succeed
   *          1: on failure
   *
   * Function:
   *      This routine tests the grid functions
   */

  int retval = 0;

  /*
   * set geometry for test grid
   */

  double origin_lat = rand_in_range(-70.0, 70.0);
  double origin_lon = rand_in_range(-180.0, 180.0);
  double deltaLat = rand_in_range(0.0, 10.0);
  double lat1 = origin_lat - deltaLat;
  double lat2 = origin_lat + deltaLat;
  double min_x = rand_in_range(-100.0, 100.0);
  double min_y = rand_in_range(-100.0, 100.0);
  double dx = rand_in_range(0.25, 2.0);
  double dy = rand_in_range(0.25, 2.0);
  int nx = 100;
  int ny = 100;
  double offset_lat = rand_in_range(origin_lat - 10.0, origin_lat + 10.0);
  if (offset_lat < -90) {
    offset_lat = -90;
  }
  if (offset_lat > 90) {
    offset_lat = 90;
  }
  double offset_lon = rand_in_range(origin_lon - 20.0, origin_lon + 20.0);
  double false_easting = rand_in_range(0, 100);
  double false_northing = rand_in_range(0, 100);
  
  PjgAlbersMath math(origin_lat, origin_lon, lat1, lat2);
  math.setOffsetOrigin(offset_lat, offset_lon);
  math.setOffsetCoords(false_northing, false_easting);

  for (int iy = 0; iy < ny; iy++) {
    for (int ix = 0; ix < nx; ix++) {

      double xx1 = min_x + ix * dx;
      double yy1 = min_y + iy * dy;

      double lat, lon;
      math.xy2latlon(xx1, yy1, lat, lon);
      
      double xx2, yy2;
      math.latlon2xy(lat, lon, xx2, yy2);

      double diff_x = xx1 - xx2;
      double diff_y = yy1 - yy2;
      if (fabs(diff_x) > TINY_DIST || fabs(diff_y) > TINY_DIST) {
        fprintf(fp_err, "================================\n");
        fprintf(fp_err, "FAILED AlbersConic test\n");
        fprintf(fp_err, "  xx1, xx2, err    : %15.10f, %15.10f, %15.10f\n",
                xx1, xx2, xx1 - xx2);
        fprintf(fp_err, "  yy1, yy2, err    : %15.10f, %15.10f, %15.10f\n",
                yy1, yy2, yy1 - yy2);
        fprintf(fp_err, "  lat, lon         : %15.10f, %15.10f\n", lat, lon);
        fprintf(fp_err, "  Grid:\n");
        fprintf(fp_err, "  org_lat, org_lon : %15.10f, %15.10f\n",
                origin_lat, origin_lon);
        fprintf(fp_err, "  lat1, lat2       : %15.10f, %15.10f\n",
                lat1, lat2);
        fprintf(fp_err, "  min_x, min_y     : %15.10f, %15.10f\n",
                min_x, min_y);
        fprintf(fp_err, "  dx, dy           : %15.10f, %15.10f\n", dx, dy);
        retval = 1;
      }
      
    } // ix
  } // iy

  /*
   * Done
   */
  
  if(retval) {
    fprintf(stderr, "FAILED test TEST_pjg_math - AlbersConic.\n");
  }
    
  return(retval);

}

/*--------------------------------*/
static int
test_PolarStereo(FILE *fp_err)
{

  /*
   * Inputs:  None
   *
   * Returns: 0: on succeed
   *          1: on failure
   *
   * Function:
   *      This routine tests the grid functions
   */

  int retval = 0;

  /*
   * set geometry for test grid
   */

  double offset_lat = rand_in_range(70.0, 90.0);
  double offset_lon = rand_in_range(-180.0, 180.0);
  double tangent_lon = offset_lon + rand_in_range(-10.0, 10.0);
  bool pole_is_north = true;
  double central_scale = rand_in_range(0.5, 1.5);
  double min_x = rand_in_range(-100.0, 100.0);
  double min_y = rand_in_range(-100.0, 100.0);
  double dx = rand_in_range(0.25, 2.0);
  double dy = rand_in_range(0.25, 2.0);
  int nx = 100;
  int ny = 100;
  
  PjgPolarStereoMath math(tangent_lon, pole_is_north, central_scale);
  math.setOffsetOrigin(offset_lat, offset_lon);

  for (int iy = 0; iy < ny; iy++) {
    for (int ix = 0; ix < nx; ix++) {

      double xx1 = min_x + ix * dx;
      double yy1 = min_y + iy * dy;

      double lat, lon;
      math.xy2latlon(xx1, yy1, lat, lon);
      
      double xx2, yy2;
      math.latlon2xy(lat, lon, xx2, yy2);

      double diff_x = xx1 - xx2;
      double diff_y = yy1 - yy2;
      if (fabs(diff_x) > SMALL_DIST || fabs(diff_y) > SMALL_DIST) {
        fprintf(fp_err, "================================\n");
        fprintf(fp_err, "FAILED PolarStereo test\n");
        fprintf(fp_err, "  xx1, xx2, err    : %15.10f, %15.10f, %15.10f\n",
                xx1, xx2, xx1 - xx2);
        fprintf(fp_err, "  yy1, yy2, err    : %15.10f, %15.10f, %15.10f\n",
                yy1, yy2, yy1 - yy2);
        fprintf(fp_err, "  lat, lon         : %15.10f, %15.10f\n", lat, lon);
        fprintf(fp_err, "  Grid:\n");
        fprintf(fp_err, "  org_lat, org_lon : %15.10f, %15.10f\n",
                offset_lat, offset_lon);
        fprintf(fp_err, "  tangent_lon      : %15.10f\n", tangent_lon);
        fprintf(fp_err, "  central_scale    : %15.10f\n", central_scale);
        fprintf(fp_err, "  pole_is_north    : %d\n", pole_is_north);
        fprintf(fp_err, "  min_x, min_y     : %15.10f, %15.10f\n",
                min_x, min_y);
        fprintf(fp_err, "  dx, dy           : %15.10f, %15.10f\n", dx, dy);
        retval = 1;
      }
      
    } // ix
  } // iy

  /*
   * Done
   */
  
  if(retval) {
    fprintf(stderr, "FAILED test TEST_pjg_math - PolarStereo.\n");
  }
    
  return(retval);

}

/*--------------------------------*/
static int
test_ObliqueStereo(FILE *fp_err)
{

  /*
   * Inputs:  None
   *
   * Returns: 0: on succeed
   *          1: on failure
   *
   * Function:
   *      This routine tests the grid functions
   */

  int retval = 0;

  /*
   * set geometry for test grid
   */

  double offset_lat = rand_in_range(10.0, 80.0);
  double offset_lon = rand_in_range(-180.0, 180.0);
  double tangent_lat = offset_lat + rand_in_range(-10.0, 10.0);
  double tangent_lon = offset_lon + rand_in_range(-10.0, 10.0);
  double min_x = rand_in_range(-100.0, 100.0);
  double min_y = rand_in_range(-100.0, 100.0);
  double central_scale = rand_in_range(0.5, 1.5);
  double dx = rand_in_range(0.25, 2.0);
  double dy = rand_in_range(0.25, 2.0);
  int nx = 100;
  int ny = 100;
  
  PjgObliqueStereoMath math(tangent_lat, tangent_lon, central_scale);
  math.setOffsetOrigin(offset_lat, offset_lon);

  for (int iy = 0; iy < ny; iy++) {
    for (int ix = 0; ix < nx; ix++) {

      double xx1 = min_x + ix * dx;
      double yy1 = min_y + iy * dy;

      double lat, lon;
      math.xy2latlon(xx1, yy1, lat, lon);
      
      double xx2, yy2;
      math.latlon2xy(lat, lon, xx2, yy2);

      double diff_x = xx1 - xx2;
      double diff_y = yy1 - yy2;
      if (fabs(diff_x) > TINY_DIST || fabs(diff_y) > TINY_DIST) {
        fprintf(fp_err, "================================\n");
        fprintf(fp_err, "FAILED ObliqueStereo test\n");
        fprintf(fp_err, "  xx1, xx2, err    : %15.10f, %15.10f, %15.10f\n",
                xx1, xx2, xx1 - xx2);
        fprintf(fp_err, "  yy1, yy2, err    : %15.10f, %15.10f, %15.10f\n",
                yy1, yy2, yy1 - yy2);
        fprintf(fp_err, "  lat, lon         : %15.10f, %15.10f\n", lat, lon);
        fprintf(fp_err, "  Grid:\n");
        fprintf(fp_err, "  org_lat, org_lon : %15.10f, %15.10f\n",
                offset_lat, offset_lon);
        fprintf(fp_err, "  tangent_lat      : %15.10f\n", tangent_lat);
        fprintf(fp_err, "  tangent_lon      : %15.10f\n", tangent_lon);
        fprintf(fp_err, "  min_x, min_y     : %15.10f, %15.10f\n",
                min_x, min_y);
        fprintf(fp_err, "  dx, dy           : %15.10f, %15.10f\n", dx, dy);
        retval = 1;
      }
      
    } // ix
  } // iy

  /*
   * Done
   */
  
  if(retval) {
    fprintf(stderr, "FAILED test TEST_pjg_math - ObliqueStereo.\n");
  }
    
  return(retval);

}

/*--------------------------------*/
static int
test_Mercator(FILE *fp_err)
{

  /*
   * Inputs:  None
   *
   * Returns: 0: on succeed
   *          1: on failure
   *
   * Function:
   *      This routine tests the grid functions
   */

  int retval = 0;

  /*
   * set geometry for test grid
   */

  double origin_lat = rand_in_range(-70.0, 70.0);
  double origin_lon = rand_in_range(-180.0, 180.0);
  double min_x = rand_in_range(-100.0, 100.0);
  double min_y = rand_in_range(-100.0, 100.0);
  double dx = rand_in_range(0.25, 2.0);
  double dy = rand_in_range(0.25, 2.0);
  int nx = 100;
  int ny = 100;
  double offset_lat = rand_in_range(origin_lat - 10.0, origin_lat + 10.0);
  if (offset_lat < -90) {
    offset_lat = -90;
  }
  if (offset_lat > 90) {
    offset_lat = 90;
  }
  double offset_lon = rand_in_range(origin_lon - 20.0, origin_lon + 20.0);
  double false_easting = rand_in_range(0, 100);
  double false_northing = rand_in_range(0, 100);
  
  PjgMercatorMath math(origin_lat, origin_lon);
  math.setOffsetOrigin(offset_lat, offset_lon);
  math.setOffsetCoords(false_northing, false_easting);

  for (int iy = 0; iy < ny; iy++) {
    for (int ix = 0; ix < nx; ix++) {

      double xx1 = min_x + ix * dx;
      double yy1 = min_y + iy * dy;

      double lat, lon;
      math.xy2latlon(xx1, yy1, lat, lon);
      
      double xx2, yy2;
      math.latlon2xy(lat, lon, xx2, yy2);

      double diff_x = xx1 - xx2;
      double diff_y = yy1 - yy2;
      if (fabs(diff_x) > TINY_DIST || fabs(diff_y) > TINY_DIST) {
        fprintf(fp_err, "================================\n");
        fprintf(fp_err, "FAILED Mercator test\n");
        fprintf(fp_err, "  xx1, xx2, err    : %15.10f, %15.10f, %15.10f\n",
                xx1, xx2, xx1 - xx2);
        fprintf(fp_err, "  yy1, yy2, err    : %15.10f, %15.10f, %15.10f\n",
                yy1, yy2, yy1 - yy2);
        fprintf(fp_err, "  lat, lon         : %15.10f, %15.10f\n", lat, lon);
        fprintf(fp_err, "  Grid:\n");
        fprintf(fp_err, "  org_lat, org_lon : %15.10f, %15.10f\n",
                origin_lat, origin_lon);
        fprintf(fp_err, "  min_x, min_y     : %15.10f, %15.10f\n",
                min_x, min_y);
        fprintf(fp_err, "  dx, dy           : %15.10f, %15.10f\n", dx, dy);
        retval = 1;
      }
      
    } // ix
  } // iy

  /*
   * Done
   */
  
  if(retval) {
    fprintf(stderr, "FAILED test TEST_pjg_math - Mercator.\n");
  }
    
  return(retval);

}

/*--------------------------------*/
static int
test_TransMercator(FILE *fp_err)
{

  /*
   * Inputs:  None
   *
   * Returns: 0: on succeed
   *          1: on failure
   *
   * Function:
   *      This routine tests the grid functions
   */

  int retval = 0;

  /*
   * set geometry for test grid
   */

  double origin_lat = rand_in_range(-10.0, 10.0);
  double origin_lon = rand_in_range(-150.0, 150.0);
  double central_scale = rand_in_range(0.9, 1.1);
  double min_x = rand_in_range(-100.0, 100.0);
  double min_y = rand_in_range(-100.0, 100.0);
  double dx = rand_in_range(0.25, 2.0);
  double dy = rand_in_range(0.25, 2.0);
  int nx = 100;
  int ny = 100;
  double offset_lat = rand_in_range(origin_lat - 10.0, origin_lat + 10.0);
  if (offset_lat < -90) {
    offset_lat = -90;
  }
  if (offset_lat > 90) {
    offset_lat = 90;
  }
  double offset_lon = rand_in_range(origin_lon - 20.0, origin_lon + 20.0);
  double false_easting = rand_in_range(0, 100);
  double false_northing = rand_in_range(0, 100);

  PjgTransMercatorMath math(origin_lat, origin_lon, central_scale);
  math.setOffsetOrigin(offset_lat, offset_lon);
  math.setOffsetCoords(false_northing, false_easting);

  for (int iy = 0; iy < ny; iy++) {
    for (int ix = 0; ix < nx; ix++) {

      double xx1 = min_x + ix * dx;
      double yy1 = min_y + iy * dy;

      double lat, lon;
      math.xy2latlon(xx1, yy1, lat, lon);
      
      double xx2, yy2;
      math.latlon2xy(lat, lon, xx2, yy2);

      double diff_x = xx1 - xx2;
      double diff_y = yy1 - yy2;
      if (fabs(diff_x) > TINY_DIST || fabs(diff_y) > TINY_DIST) {
        fprintf(fp_err, "================================\n");
        fprintf(fp_err, "FAILED TransMercator test\n");
        fprintf(fp_err, "  xx1, xx2, err    : %15.10f, %15.10f, %15.10f\n",
                xx1, xx2, xx1 - xx2);
        fprintf(fp_err, "  yy1, yy2, err    : %15.10f, %15.10f, %15.10f\n",
                yy1, yy2, yy1 - yy2);
        fprintf(fp_err, "  lat, lon         : %15.10f, %15.10f\n", lat, lon);
        fprintf(fp_err, "  Grid:\n");
        fprintf(fp_err, "  org_lat, org_lon : %15.10f, %15.10f\n",
                origin_lat, origin_lon);
        fprintf(fp_err, "  central_scale    : %15.10f\n", central_scale);
        fprintf(fp_err, "  min_x, min_y     : %15.10f, %15.10f\n",
                min_x, min_y);
        fprintf(fp_err, "  dx, dy           : %15.10f, %15.10f\n", dx, dy);
        retval = 1;
      }
      
    } // ix
  } // iy

  /*
   * Done
   */
  
  if(retval) {
    fprintf(stderr, "FAILED test TEST_pjg_math - TransMercator.\n");
  }
    
  return(retval);

}

/*--------------------------------*/
static int
  test_VertPersp(FILE *fp_err)
{

  /*
   * Inputs:  None
   *
   * Returns: 0: on succeed
   *          1: on failure
   *
   * Function:
   *      This routine tests the grid functions
   */

  int retval = 0;

  /*
   * set geometry for test grid
   */
  
  double origin_lat = rand_in_range(-90.0, 90.0);
  double origin_lon = rand_in_range(-180.0, 180.0);
  double persp_radius = rand_in_range(8000, 30000);
  double min_x = rand_in_range(-100.0, 100.0);
  double min_y = rand_in_range(-100.0, 100.0);
  double dx = rand_in_range(0.25, 2.0);
  double dy = rand_in_range(0.25, 2.0);
  int nx = 100;
  int ny = 100;
  double offset_lat = rand_in_range(origin_lat - 10.0, origin_lat + 10.0);
  if (offset_lat < -90) {
    offset_lat = -90;
  }
  if (offset_lat > 90) {
    offset_lat = 90;
  }
  double offset_lon = rand_in_range(origin_lon - 20.0, origin_lon + 20.0);
  double false_easting = rand_in_range(0, 100);
  double false_northing = rand_in_range(0, 100);
  
  PjgVertPerspMath math(origin_lat, origin_lon, persp_radius);
  math.setOffsetOrigin(offset_lat, offset_lon);
  math.setOffsetCoords(false_northing, false_easting);

  for (int iy = 0; iy < ny; iy++) {
    for (int ix = 0; ix < nx; ix++) {

      double xx1 = min_x + ix * dx;
      double yy1 = min_y + iy * dy;

      double lat, lon;
      math.xy2latlon(xx1, yy1, lat, lon);
      
      double xx2, yy2;
      math.latlon2xy(lat, lon, xx2, yy2);
      
      double diff_x = xx1 - xx2;
      double diff_y = yy1 - yy2;
      if (fabs(diff_x) > TINY_DIST || fabs(diff_y) > TINY_DIST) {
        fprintf(fp_err, "================================\n");
        fprintf(fp_err, "FAILED VertPersp test\n");
        fprintf(fp_err, "  xx1, xx2, err    : %15.10f, %15.10f, %15.10f\n",
                xx1, xx2, xx1 - xx2);
        fprintf(fp_err, "  yy1, yy2, err    : %15.10f, %15.10f, %15.10f\n",
                yy1, yy2, yy1 - yy2);
        fprintf(fp_err, "  lat, lon         : %15.10f, %15.10f\n", lat, lon);
        fprintf(fp_err, "  Grid:\n");
        fprintf(fp_err, "  org_lat, org_lon : %15.10f, %15.10f\n",
                origin_lat, origin_lon);
        fprintf(fp_err, "  persp_radius     : %15.10f\n", persp_radius);
        fprintf(fp_err, "  min_x, min_y     : %15.10f, %15.10f\n",
                min_x, min_y);
        fprintf(fp_err, "  dx, dy           : %15.10f, %15.10f\n", dx, dy);
        retval = 1;
      }
      
    } // ix
  } // iy

  /*
   * Done
   */
  
  if(retval) {
    fprintf(stderr, "FAILED test TEST_pjg_math - VertPersp.\n");
  }
    
  return(retval);

}

static void print_usage(char *prog_name, FILE *out)

{
    fprintf(out, "Usage: %s [-many ?]\n", prog_name);
}

////////////////////////////////////////////////////////
// random number generation

static long gb_flip_cycle(void)
{
  register long *ii, *jj;
  for (ii = &A[1], jj = &A[32]; jj <= &A[55]; ii++, jj++)
    *ii = mod_diff(*ii, *jj);
  for (jj = &A[1]; ii <= &A[55]; ii++, jj++)
    *ii = mod_diff(*ii, *jj);
  gb_fptr = &A[54];
  return A[55];
}

static void gb_init_rand(long seed)

{
  register long i;
  register long prev = seed, next = 1;
  seed = prev = mod_diff(prev, 0);
  A[55] = prev;
  for (i = 21; i; i = (i + 21) % 55)
    {
      A[i] = next;
      next = mod_diff(prev, next);
      if (seed & 1)
	seed = 0x40000000 + (seed >> 1);
      else
	seed >>= 1;
      next = mod_diff(next, seed);
      prev = A[i];
    }
  (void)gb_flip_cycle();
  (void)gb_flip_cycle();
  (void)gb_flip_cycle();
  (void)gb_flip_cycle();
  (void)gb_flip_cycle();
}

/*
 * random number given range
 */

static double rand_in_range(double minval, double maxval)

{

  int randval;
  double normval;

  randval = gb_next_rand();
  normval = ((double) randval / (double)  two_to_the_31);

  return (minval + (maxval - minval) * normval);

}

/*--------------------------------*/

/*
 * main program driver
 */

int
main(int argc, char *argv[])
{

  int retval = 0;
  int i;
  int repeats = 20;
  FILE *fp_err;

  /*
   * check usage
   */

  if (argc > 3 || (argc == 2 && !strcmp(argv[1], "-h"))) {
      print_usage(argv[0], stdout);
      return (0);
  }
  
  if (argc == 2 && !strcmp(argv[1], "-many")) {
      repeats = 1000;
  }

  if (argc > 2 || (argc == 2 && strcmp(argv[1], "-many"))) {
    repeats = atoi(argv[2]);
  }

  /*
   * open errlog file
   */

  const char *logPath = "test_pjg_math.errlog";
  if ((fp_err = fopen(logPath, "w")) == NULL) {
    perror(logPath);
    fprintf(stderr, "Cannot open log file\n");
    return -1;
  }

  /*
   * seed random generator
   */

  gb_init_rand((unsigned int) time(NULL));

  /*
   * Test the individual module subroutines
   */

  for (i = 0; i < repeats; i++) {

    fprintf(stdout, "*** testing, loop num %d ***\n", i);

    if (test_AzimEquiDist(fp_err)) {
      retval = 1;
    }

    if (test_LambertAzimEqualArea(fp_err)) {
      retval = 1;
    }
    
    if (test_LambertConfConic(fp_err)) {
      retval = 1;
    }
    
    if (test_AlbersConic(fp_err)) {
      retval = 1;
    }
    
    if (test_PolarStereo(fp_err)) {
      retval = 1;
    }
    
    if (test_ObliqueStereo(fp_err)) {
      retval = 1;
    }
    
    if (test_Mercator(fp_err)) {
      retval = 1;
    }
    
    if (test_TransMercator(fp_err)) {
      retval = 1;
    }
    
    if (test_VertPersp(fp_err)) {
      retval = 1;
    }
    
//     if (retval)
// 	break;

    fflush(stdout);
    fflush(fp_err);

  }

  if (retval) {
    fprintf(stderr, "PjgMath failed test\n");
    fprintf(stderr, "  Log is in file: %s\n", logPath);
  } else {
    fprintf(stdout, "PjgMath passed test\n");
  }
  
  fclose(fp_err);
  return(retval);
}

