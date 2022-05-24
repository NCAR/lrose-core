// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cerrno>
using namespace std;

/*---------------- defines --------------------------------------------------*/

#ifndef LMIN
#define LMIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef LMAX
#define LMAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#define MAX_BINS 2000
#define INIT_CALL -2 /* must be negative */
#define UP 1.0
#define DOWN -1.0
#define NO_DATA 0
#define LOW 2
#define HIGH 3
#define HIT_EDGE 4

typedef struct {
  double min_range, max_range, sum_distance, sum_directional_distance;
  int num_pairs, num_connected,num_directional, num_dir_connected; 
} bin_t;

/*----------------- file scope variables ------------------------------------*/

static double _x_res, _y_res, _diag_res;
static int _nRows, _nCols;
static int use_mask=0;

static char *prog_name = "connectivity";
static bool _debug = false;
static bool _verbose = false;
static char *_demPath = "./dem.txt";
static char *_rasterPath = "./raster.txt";
static char *_rangesPath = "./ranges.txt";
static char *_mapPath = "./map.out";
static char *_connectPath = "./connect.out";
static char *_maskPath = "./MASK";
static int _threshold = 0;
static bool _directional_flag = false;
static bool _zero_flag = false;
static bool _percentile_flag = false;
static bool _flats_flag = false;
static bool _two_directions_flag = false;
static bool _up_down_flag = false;
static bool _useMask = false;

static bin_t bin[MAX_BINS];

/*----------------- file scope functions ------------------------------------*/

static void parseArgs(int argc, char **argv, const char *prog_name);

static void usage(const char *prog_name, FILE *out);

static int read_gis(const char *path,
		    int &ncols, int &nrows,
		    double &minx, double &miny,
		    double &maxx, double &maxy,
		    double &cellsize,
		    double &nodataval,
		    int** &array);

static int** alloc_2d_array (int nrows, int ncols);

static int find_connected(int **, int **, int, int, int);

static void calc_thresh (int **data, int **mask, int *threshold);

static int int_compare(const void *p1, const void *p2);

static double slope(int r1,int r2,int c1,int c2, int **elevation);

static int search_up (int cell_row, int cell_col,
		      int calling_r, int calling_c, 
		      int **elevation, int **up_down_slope,
		      int **connected, int region, 
		      int descriptor);

static int search_down (int cell_row, int cell_col,
			int calling_r, int calling_c, 
			int **elevation, int **up_down_slope,
			int **connected, int region, 
			int descriptor);

static int find_steepest(double *slope_dat, int **elevation, 
			 double *steepest, double *second_steepest, 
			 int r, int c);

/*-------------------- main -------------------------------------------------*/

int main (int argc, char **argv)

{


  /* parse command line args */

  parseArgs(argc, argv, prog_name);

  /* start clock to check performance */

  clock();

  /* read in raster file */

  int ncolsRaster, nrowsRaster;
  double minxRaster, minyRaster;
  double maxxRaster, maxyRaster;
  double cellsizeRaster, nodatavalRaster;
  int **raster;

  if (_debug) {
    cerr << "Reading in Raster file ..." << endl;
  }

  if (read_gis(_rasterPath,
	       ncolsRaster, nrowsRaster,
	       minxRaster, minyRaster,
	       maxxRaster, maxyRaster,
	       cellsizeRaster, nodatavalRaster,
	       raster)) {
    int errNum = errno;
    cerr << "ERROR - connectivity" << endl;
    cerr << "  Cannot read in raster file: " << _rasterPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  double north = maxyRaster;
  double south = minyRaster;
  double east = maxxRaster;
  double west = minxRaster;
  _nCols = ncolsRaster;
  _nRows = nrowsRaster;
  _x_res=(east-west)/(double)_nCols;
  _y_res=(north-south)/(double)_nRows;
  _diag_res=pow(_x_res*_x_res+_y_res*_y_res,0.5);

  int **elevation = NULL;
  int **up_down_slope = NULL;
  if(_directional_flag) {

    up_down_slope = alloc_2d_array(_nRows, _nCols);
    
    /* read in DEM */
    
    int ncolsDem, nrowsDem;
    double minxDem, minyDem;
    double maxxDem, maxyDem;
    double cellsizeDem, nodatavalDem;
    
    if (_debug) {
      cerr << "Reading in DEM file ..." << endl;
    }
    
    if (read_gis(_demPath,
		 ncolsDem, nrowsDem,
		 minxDem, minyDem,
		 maxxDem, maxyDem,
		 cellsizeDem, nodatavalDem,
		 elevation)) {
      int errNum = errno;
      cerr << "ERROR - connectivity" << endl;
      cerr << "  Cannot read in DEM file: " << _demPath << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }

    // check that dem and raster match
    
    if (ncolsDem != ncolsRaster || nrowsDem != nrowsRaster ||
	fabs(minxDem - minxRaster) > 0.001 || fabs(maxxDem - maxxRaster) > 0.001 ||
	fabs(minyDem - minyRaster) > 0.001 || fabs(maxyDem - maxyRaster) > 0.001) {
      cerr << "ERROR - connectivity" << endl;
      cerr << "  DEM and raster grids do not match" << endl;
      cerr << "  Use -debug option to see grid properties" << endl;
      return -1;
    }

  }

  // read in mask if required

  int **mask = NULL;
  if (_useMask) {
    
    int ncolsMask, nrowsMask;
    double minxMask, minyMask;
    double maxxMask, maxyMask;
    double cellsizeMask, nodatavalMask;
    
    if (_debug) {
      cerr << "Reading in mask file ..." << endl;
    }
    
    if (read_gis(_maskPath,
		 ncolsMask, nrowsMask,
		 minxMask, minyMask,
		 maxxMask, maxyMask,
		 cellsizeMask, nodatavalMask,
		 mask)) {
      int errNum = errno;
      cerr << "ERROR - connectivity" << endl;
      cerr << "  Cannot read in mask file: " << _maskPath << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }

    if (ncolsRaster != ncolsMask || nrowsRaster != nrowsMask) {
      cerr << "ERROR - connectivity" << endl;
      cerr << "  Raster and mask grids do not match" << endl;
      cerr << "  Use -debug option to see grid properties" << endl;
      return -1;
    }

  } // if (_useMask)

  /* now read the bin information - do this early cos its quick and 
     we need to check its right */

  FILE *bin_file;
  if((bin_file=fopen(_rangesPath,"r"))==NULL) {
    int errNum = errno;
    cerr << "ERROR - connectivity" << endl;
    cerr << "  Cannot open range bin file: " << _rangesPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  int num_bins=0;
  char in_str[1000];
  while(fgets(in_str,1000,bin_file)!=NULL) {
    if(sscanf(in_str,"%lf%lf",&bin[num_bins].min_range,&bin[num_bins].max_range)<2) {
      int errNum = errno;
      cerr << "ERROR - connectivity" << endl;
      cerr << "  Problem reading range file: " << _rangesPath << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
    if(bin[num_bins].min_range>bin[num_bins].max_range) {
      cerr << "ERROR - connectivity" << endl;
      cerr << "  Problem reading range file: " << _rangesPath << endl;
      cerr << "  Range minimum greater than range maximum" << endl;
      cerr << "  bin index: " << num_bins << endl;
      cerr << "  min_range: " << bin[num_bins].min_range << endl;
      cerr << "  max_range: " << bin[num_bins].max_range << endl;
      return -1;
    }
    num_bins++;
    if (num_bins == MAX_BINS) {
      break;
    }
  }
  fclose (bin_file);

  if (_debug) {
    cerr << "BINS:" << endl;
    for (int ii = 0; ii < num_bins; ii++) {
      cerr << "  index, min_range, max_range: " << ii
	   << ", " << bin[ii].min_range
	   << ", " << bin[ii].max_range << endl;
    }
  }

  // check for bin monotonicity
  
  for(int ii = 1; ii < num_bins; ii++) {
    if(bin[ii].min_range<bin[ii-1].max_range) {
      cerr << "ERROR - connectivity" << endl;
      cerr << "  bin ranges overlap or are not monotonically increasing" << endl;
      cerr << "  index, min_range, max_range: " << ii
	   << ", " << bin[ii].min_range
	   << ", " << bin[ii].max_range << endl;
      cerr << "  Use -debug to see full range bin list" << endl;
      return -1;
    }
  }

  for(int i=0;i<num_bins;i++)
  {
    bin[i].sum_distance=0.;
    bin[i].sum_directional_distance=0.;
    bin[i].num_pairs=0;
    bin[i].num_connected=0;
    bin[i].num_directional=0;
    bin[i].num_dir_connected=0;
  }

  /* these are used to limit the search distance for matched pairs so that
     execution times are improved for big arrays */
  int max_row_dif=(int)(bin[num_bins-1].max_range/_y_res)+1;
  int max_col_dif=(int)(bin[num_bins-1].max_range/_x_res)+1;

  /* allocate and initialise memory for the connected regions */

  int **connected = alloc_2d_array(_nRows, _nCols);
  for (int r=0;r<_nRows;r++) {
    for (int c=0;c<_nCols;c++) {
      connected[r][c]=0;
    }
  }

  /* if the -p flag is set we need to make a copy of the data, sort it and 
     calculate the threshold */

  if(_percentile_flag) {
    if(_threshold<0.||_threshold>100.) {
      cerr << "ERROR - connectivity" << endl;
      cerr << "  percentile threshold must be between 0 and 100" << endl;
      cerr << "  threshold: " << _threshold << endl;
      return -1;
    }
    calc_thresh(raster,mask,&_threshold);
    if (_debug) {
      cerr << "Threshold is : " << _threshold << endl;
    }
  }

  /* now threshold the data - this will result in the array data[_nRows][_nCols]
     being set to :
     NO_DATA (0)  -  ignore pixel
     LOW     (1)  -  pixel <= threshold
     HIGH    (2)  -  pixel >  threshold
  */

  for(int r=0;r<_nRows;r++) {
    for(int c=0;c<_nCols;c++) {
      if(use_mask) {
	if(mask[r][c]) {
	  if(raster[r][c]>_threshold) {
	    raster[r][c]=HIGH;
	  } else {
	    raster[r][c]=LOW;
	  }
	} else {
	  raster[r][c]=NO_DATA;
	}
      } else {
	if(raster[r][c]!=0) {
	  if(raster[r][c]>_threshold) {
	    raster[r][c]=HIGH;
	  } else {
	    raster[r][c]=LOW;
	  }
	} else {
	  raster[r][c]=NO_DATA;
	}
      }
    } // c
  } // r
    
  /* now find the connected regions */

  int region=1;
  for(int r=0;r<_nRows;r++) {
    for(int c=0;c<_nCols;c++) {
      if (raster[r][c]==HIGH && connected[r][c]==0) {
	find_connected (raster,connected,r,c,region);
	region++;
      }
    }
  }

  /* now calculate the connectivity function */

  for(int r=0;r<_nRows;r++) {

    // G_percent(r,_nRows,5);

    for(int c=0;c<_nCols;c++) {

      if(connected[r][c]) {

        double x1=west+((double)c+0.5)*_x_res;
        double y1=north-((double)r+0.5)*_y_res;
        int min_row=LMAX(0,r-max_row_dif);
        int max_row=LMIN(_nRows-1,r+max_row_dif);
        int min_col=LMAX(0,c-max_col_dif);
        int max_col=LMIN(_nCols-1,c+max_col_dif);
  
        /* if we are interested in directional stuff we need to find the up and 
           downslope connected areas now */
        if(_directional_flag) {
          up_down_slope[r][c]=1;
          if(search_up(r, c, INIT_CALL, INIT_CALL, elevation, up_down_slope, 
		       connected, connected[r][c],1)==HIT_EDGE) {
            fprintf(stderr,"\ar.catchments: Warning - dem edge encountered\n");
	  }
          if(search_down(r, c, INIT_CALL, INIT_CALL, elevation, up_down_slope, 
			 connected, connected[r][c],1)==HIT_EDGE) {
            fprintf(stderr,"\ar.catchments: Warning - dem edge encountered\n");
	  }
        }
  
	for(int rr=min_row;rr<=max_row;rr++) {
	  for(int cc=min_col;cc<=max_col;cc++) {
	    if(raster[rr][cc]!=NO_DATA) {
	      double x2=west+((double)cc+0.5)*_x_res;
	      double y2=north-((double)rr+0.5)*_y_res;
	      double distance=pow((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2),0.5);
	      int i=0;
	      while (bin[i].max_range<distance&&i<num_bins-1) i++;

	      if(distance>=bin[i].min_range) {
		bin[i].sum_distance+=distance;
		bin[i].num_pairs++;
		if(connected[r][c]==connected[rr][cc]) bin[i].num_connected++;
		if(_directional_flag && 
		   up_down_slope[r][c]==up_down_slope[rr][cc]) {
		  bin[i].sum_directional_distance+=distance;
		  bin[i].num_directional++;
		  if(connected[r][c]==connected[rr][cc]) bin[i].num_dir_connected++;
		}
	      } // if(distance ...

	    } // if(raster[rr][cc] ...
	  } // cc
	} // rr

        /* now reset the up and downslope connected regions */
        if(_directional_flag) {
          up_down_slope[r][c]=0;
          search_up(r, c, INIT_CALL, INIT_CALL, elevation, up_down_slope, 
		    connected, connected[r][c],0);
          search_down(r, c, INIT_CALL, INIT_CALL, elevation, up_down_slope, 
		      connected, connected[r][c],0);
        }

      } // if(connected[r][c]) {

    } // c
  } // r

  /* now write the result file */

  FILE *connect_file;
  if((connect_file=fopen(_connectPath,"w"))==NULL) {
    int errNum = errno;
    cerr << "ERROR - connectivity" << endl;
    cerr << "  Cannot open connect file for writing: " << _connectPath << endl;
    cerr << "  "  << strerror(errNum) << endl;
    return -1;
  }
  
  for(int i=0;i<num_bins;i++) {
    if(bin[i].num_pairs>0)  {
      fprintf(connect_file," %6.3f  ",
	      (bin[i].min_range + bin[i].max_range) / 2.0);
      fprintf(connect_file," %6.3f  ",
	      bin[i].sum_distance/(double)bin[i].num_pairs);
      fprintf(connect_file," %8.6f  ",
	      (double)bin[i].num_connected/(double)bin[i].num_pairs);
      if(_directional_flag) {
        if(bin[i].num_directional>0) {
          fprintf(connect_file," %6.3f  ",
		  bin[i].sum_directional_distance/(double)bin[i].num_directional);
          fprintf(connect_file," %8.6f  ",
		  (double)bin[i].num_dir_connected/(double)bin[i].num_directional);
        } else {
	  fprintf(connect_file," 0.0    0.0       ");
	}
      } // if(_directional_flag)
      fprintf(connect_file," %6d", bin[i].num_pairs);
      fprintf(connect_file," %6d", bin[i].num_connected);
      fprintf(connect_file," %10g", bin[i].sum_distance);
      if(_directional_flag) {
        fprintf(connect_file," %6d", bin[i].num_directional);
        fprintf(connect_file," %6d", bin[i].num_dir_connected);
        fprintf(connect_file," %10g", bin[i].sum_directional_distance);
      }
      fprintf(connect_file,"\n");
    }
  }
  fclose(connect_file);

  /* now we've just got to write out the cell map if its name was supplied */

  FILE *mapFile;
  if ((mapFile = fopen(_mapPath, "w")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - connectivity" << endl;
    cerr << "  Cannot open map file for writing: " << _mapPath << endl;
    cerr << "  "  << strerror(errNum) << endl;
    return -1;
  }

  fprintf(mapFile, "ncols             %d\n", _nCols);
  fprintf(mapFile, "nrows             %d\n", _nRows);
  fprintf(mapFile, "xllcorner         %g\n", minxRaster);
  fprintf(mapFile, "yllcorner         %g\n", minyRaster);
  fprintf(mapFile, "cellsize          %g\n", cellsizeRaster);
  fprintf(mapFile, "NODATA_value      %g\n", nodatavalRaster);
  for (int rr = 0; rr < _nRows; rr++) {
    for (int cc = 0; cc < _nCols; cc++) {
      fprintf(mapFile, "%d", connected[rr][cc]);
      if (cc == _nCols - 1) {
	fprintf(mapFile, "\n");
      } else {
	fprintf(mapFile, " ");
      }
    } // cc
  } // rr
  fclose(mapFile);
  
  fprintf(stderr,"Execution time %-1.0f seconds\n",clock()/CLOCKS_PER_SEC);

  return 0;

}

/*----------------- parse command line args ---------------------------------*/

static void parseArgs(int argc, char **argv, const char* prog_name)

{

  int iret = 0;

  // loop through args
  
  for (int i =  1; i < argc; i++) {

    if (!strcmp(argv[i], "--") ||
	!strcmp(argv[i], "-h") ||
	!strcmp(argv[i], "-help") ||
	!strcmp(argv[i], "-man")) {
      
      usage(prog_name, stdout);
      exit (0);
      
    } else if (!strcmp(argv[i], "-debug")) {

      _debug = true;
      
    } else if (!strcmp(argv[i], "-verbose")) {

      _debug = true;
      _verbose = true;
      
    } else if (!strcmp(argv[i], "-dem")) {
      
      if (i < argc - 1) {
	_demPath = argv[++i];
      } else {
	iret = -1;
      }

      cerr << "dem path: " << _demPath << endl;
	
    } else if (!strcmp(argv[i], "-raster")) {
      
      if (i < argc - 1) {
	_rasterPath = argv[++i];
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-ranges")) {
      
      if (i < argc - 1) {
	_rangesPath = argv[++i];
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-mask")) {
      
      if (i < argc - 1) {
	_maskPath = argv[++i];
	_useMask = true;
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-map")) {
      
      if (i < argc - 1) {
	_mapPath = argv[++i];
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-connect")) {
      
      if (i < argc - 1) {
	_connectPath = argv[++i];
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-threshold")) {
      
      if (i < argc - 1) {
	if (sscanf(argv[++i], "%d", &_threshold) != 1) {
	  iret = -1;
	}
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-d")) {

      _directional_flag = true;
      
    } else if (!strcmp(argv[i], "-z")) {

      _zero_flag = true;
      
    } else if (!strcmp(argv[i], "-p")) {

      _percentile_flag = true;
      
    } else if (!strcmp(argv[i], "-f")) {

      _flats_flag = true;
      
    } else if (!strcmp(argv[i], "-2")) {

      _two_directions_flag = true;
      
    } else if (!strcmp(argv[i], "-u")) {
      
      _up_down_flag = true;
      
    } // if
    
  } // i

  if (iret) {
    usage(prog_name, stderr);
    exit(-1);
  }

}

/*--------------------- usage -----------------------------------------------*/

static void usage(const char *prog_name, FILE *out)

{
  
  fprintf(out, "%s%s%s%s",
	  "Usage: ", prog_name, " [options as below]\n",
	  "options:\n"
	  "       [ --, -h, -help, -man ] produce this list.\n"
	  "       [ -debug ] print debug messages\n"
	  "       [ -verbose ] print verbose debug messages\n"
	  "       [ -dem ? ] input file path for dem\n"
	  "       [ -raster ? ] input file name for raster\n"
	  "       [ -ranges ? ] input file path for range bins\n"
	  "       [ -mask ? ] specify mask file - optional\n"
	  "       [ -connect ? ] output file path for connectivity function\n"
	  "       [ -map ? ] output file path for map of connected regions\n"
	  "       [ -threshold ? ] threshold value\n"
	  "       [ -d ] consider directional connectivity\n"
	  "       [ -z ] consider zero data within the masked area\n"
	  "       [ -p ] use a percentile to determine threshold\n"
	  "       [ -f ] include flats when doing directional connectivity\n"
	  "       [ -2 ] include two steepest directions when\n"
	  "              doing directional connectivity\n"
	  "       [ -u ] allow directional search to zig-zag up and down\n"
	  "\n");

}

/*--------------------- read_gis --------------------------------------------*/
/*
 * read in data from a GIS-format file
 */

static int read_gis(const char *path,
		    int &ncols, int &nrows,
		    double &minx, double &miny,
		    double &maxx, double &maxy,
		    double &cellsize,
		    double &nodataval,
		    int** &array)

{

  if (_debug) {
    cerr << "Opening gis file for reading: " << path << endl;
  }

  /* open file */

  FILE *in;
  if ((in = fopen(path, "r")) == NULL) {
    fprintf(stderr, "ERROR - connectivity::read_gis\n");
    fprintf(stderr, "  Cannot open file: %s\n", path);
    perror(path);
    return -1;
  }

  /* read in header */

  if (fscanf(in,
	     "ncols %d nrows %d "
	     "xllcorner %lg yllcorner %lg "
	     "cellsize %lg NODATA_value %lg",
	     &ncols, &nrows, &minx, &miny, &cellsize, &nodataval) != 6) {
    fclose(in);
    return -1;
  }
  
  /* read in eol */
  
  char line[1024];
  fgets(line, 1024, in);

  maxx = minx + (ncols - 1) * cellsize;
  maxy = miny + (nrows - 1) * cellsize;

  /* alloc data array */

  array = alloc_2d_array(nrows, ncols);

  /* read in data */
  
  for (int ii = 0; ii < nrows; ii++) {
    for (int jj = 0; jj < ncols; jj++) {
      if (fscanf(in, "%d", &array[ii][jj]) != 1) {
	fclose(in);
	return -1;
      }
    } // jj
    fgets(line, 1024, in);
  } // ii
  
  fclose(in);

  if (_debug) {
    cerr << "  ncols: " << ncols << endl;
    cerr << "  nrows: " << nrows << endl;
    cerr << "  minx: " << minx << endl;
    cerr << "  maxx: " << maxx << endl;
    cerr << "  miny: " << miny << endl;
    cerr << "  maxy: " << maxy << endl;
    cerr << "  cellsize: " << cellsize << endl;
    cerr << "  nodataval: " << nodataval << endl;
  }
  if (_verbose) {
    cerr << "  Values:" << endl;
    for (int ii = 0; ii < nrows; ii++) {
      for (int jj = 0; jj < ncols; jj++) {
	cerr << array[ii][jj];
	if (jj == ncols - 1) {
	  cerr << endl;
	} else {
	  cerr << " ";
	}
      } // jj
    } // ii
  } // verbose

  return 0;

}

/*--------------------- alloc_2d_array --------------------------------------*/

static int** alloc_2d_array (int nrows, int ncols)

{

  int **array = (int **) malloc(nrows * sizeof(int*));
  if (array == NULL) {
    cerr << "ERROR - connectivity::alloc_2d_array" << endl;
    cerr << "  Out of memory" << endl;
    exit(-1);
  }

  for(int r=0; r < nrows; r++) {
    array[r] = (int *) malloc(ncols * sizeof(int));
    if (array[r] == NULL) {
      cerr << "ERROR - connectivity::alloc_2d_array" << endl;
      cerr << "  Out of memory" << endl;
      exit(-1);
    }
  }
  return array;

}

/*--------------------- find_connected --------------------------------------*/

static int find_connected (int **data, int **connected, int r, int c, int region)

{
  int min_row, max_row, min_col, max_col;

  connected[r][c]=region;

  min_row=LMAX(0,r-1);
  max_row=LMIN(_nRows-1,r+1);
  min_col=LMAX(0,c-1);
  max_col=LMIN(_nCols-1,c+1);

  for(r=min_row;r<=max_row;r++)
  for(c=min_col;c<=max_col;c++)
  if (data[r][c]==HIGH && connected[r][c]==0)
    find_connected (data,connected,r,c,region);
}

/*---------------- slope ---------------------------------------*/

static double slope(int r1,int r2,int c1,int c2, int **elevation)

{
  double slope;
  int index, neighbour;

  if(r2!=r1 && c2!=c1)
  {
    slope=((double)(elevation[r2][c2]-elevation[r1][c1]))/_diag_res;
  }
  else if (c2==c1)
  {
    slope=((double)(elevation[r2][c2]-elevation[r1][c1]))/_y_res;
  }
  else
  {
    slope=((double)(elevation[r2][c2]-elevation[r1][c1]))/_x_res;
  }
  return slope;
}

/*------------------ calc_thresh --------------------------------------*/

static void calc_thresh (int **data, int **mask, int *threshold)

{
  unsigned int i, num_pts;
  int r,c;
  int percentile;
  int *copy;
  if((copy=(int*)malloc((_nRows*_nCols+1)*sizeof(int)))==NULL) {
    fprintf(stderr, "Couldn't allocate memory\n");
    exit(-1);
  }
  
  i=1;
  for(r=0;r<_nRows;r++)
  for(c=0;c<_nCols;c++)
  {
    if(use_mask && mask[r][c])
    {
      copy[i]=data[r][c];
      i++;
    }
    else if (data[r][c]!=0)
    {
      copy[i]=data[r][c];
      i++;
    }
  }

  num_pts=i-1;

  qsort(copy, num_pts, sizeof(int), int_compare);

  i=(unsigned int)((double)num_pts*(double)*threshold/100.+0.5);

  *threshold=copy[i];
}

/*--------------------- search_down -----------------------------------*/

static int search_down (int cell_row, int cell_col,
			int calling_r, int calling_c, 
			int **elevation, int **up_down_slope,
			int **connected, int region, 
			int descriptor)

/* cell_row and cell_column refer to the cell we are at */

{

  int hit_edge=0;
  int r, c;
  double steepest, second_steepest;
  double slope_dat[8];
  int i;

  up_down_slope[cell_row][cell_col]=descriptor;

  hit_edge=find_steepest(slope_dat, elevation, &steepest, &second_steepest,
    cell_row,cell_col);

  if(cell_row==0||cell_col==0||cell_row==_nRows-1||cell_col==_nCols-1) hit_edge=1;

  i=0;
  for (r=LMAX(0,cell_row-1);r<=LMIN(_nRows-1,cell_row+1);r++)
  for (c=LMAX(0,cell_col-1);c<=LMIN(_nCols-1,cell_col+1);c++)
  {
    if(r==cell_row && c==cell_col) continue;

    if(slope_dat[i]>0.0 || (slope_dat[i]>=0.0&&_flats_flag))
    {
      if(slope_dat[i]>=steepest ||
        (slope_dat[i]>=second_steepest&&_two_directions_flag))
      {
        if(/*connected[r][c]==region &&*/ up_down_slope[r][c]!=descriptor) 
        {
          hit_edge=search_down(r, c, cell_row, cell_col, elevation, 
            up_down_slope, connected,region,descriptor);
        }
      }
    }
    i++;
  }

  /* this does the up and down zig-zags */
  if(_up_down_flag && up_down_slope[cell_row][cell_col]==descriptor)
  {
    for (r=LMAX(0,cell_row-1);r<=LMIN(_nRows-1,cell_row+1);r++)
    for (c=LMAX(0,cell_col-1);c<=LMIN(_nCols-1,cell_col+1);c++)
    {
      if((elevation[r][c]>elevation[cell_row][cell_col]) ||
        (elevation[r][c]>=elevation[cell_row][cell_col] && _flats_flag))
      {
        if(/*connected[r][c]==region &&*/ up_down_slope[r][c]!=descriptor) 
        {
          hit_edge=search_up (r,c,cell_row,cell_col,elevation,
            up_down_slope, connected,region,descriptor);
        }
      }
    }
  }
  /* end up and down zig zag */

  return hit_edge;
}


/*---------------- search_up ------------------------------------------*/

static int search_up (int cell_row, int cell_col,
		      int calling_r, int calling_c, 
		      int **elevation, int **up_down_slope,
		      int **connected,int region, 
		      int descriptor)

/* cell_row and cell_column refer to the cell we are at */

{
  int hit_edge=0;
  int r, c;
  double steepest, second_steepest, slope_to_calling_cell;
  double slope_dat[8];
  int i;

  hit_edge=find_steepest(slope_dat,elevation,&steepest,&second_steepest,
      cell_row,cell_col);

  if(cell_row==0||cell_col==0||cell_row==_nRows-1||cell_col==_nCols-1) hit_edge=1;

  if(calling_r!=INIT_CALL)  slope_to_calling_cell=
    DOWN*slope(cell_row,calling_r,cell_col,calling_c, elevation);
  else slope_to_calling_cell=1.;

  if(calling_r==INIT_CALL || (slope_to_calling_cell>=steepest) || 
    (slope_to_calling_cell>=second_steepest && _two_directions_flag))
  {
    /* its upslope */
    up_down_slope[cell_row][cell_col]=descriptor;
    for (r=LMAX(0,cell_row-1);r<=LMIN(_nRows-1,cell_row+1);r++)
    for (c=LMAX(0,cell_col-1);c<=LMIN(_nCols-1,cell_col+1);c++)
    {
      if((elevation[r][c]>elevation[cell_row][cell_col]) ||
        (elevation[r][c]>=elevation[cell_row][cell_col] && _flats_flag))
      {
        if(/*connected[r][c]==region && */up_down_slope[r][c]!=descriptor) 
        {
          hit_edge=search_up (r,c,cell_row,cell_col,elevation,
            up_down_slope, connected,region,descriptor);
        }
      }
    }
  }

  /* the next bit does the up and downzig-zag bit */
  i=0;
  if(_up_down_flag && up_down_slope[cell_row][cell_col]==descriptor)
  {
    i=0;
    for (r=LMAX(0,cell_row-1);r<=LMIN(_nRows-1,cell_row+1);r++)
    for (c=LMAX(0,cell_col-1);c<=LMIN(_nCols-1,cell_col+1);c++)
    {
      if(r==cell_row && c==cell_col) continue;
  
      if(slope_dat[i]>0.0 || (slope_dat[i]>=0.0&&_flats_flag))
      {
        if(slope_dat[i]>=steepest ||
          (slope_dat[i]>=second_steepest&&_two_directions_flag))
        {
          if(/*connected[r][c]==region && */up_down_slope[r][c]!=descriptor) 
          {
            hit_edge=search_down(r, c, cell_row, cell_col, elevation, 
              up_down_slope, connected,region,descriptor);
          }
        }
      }
      i++;
    }
  }
  /* the end of the zig-zag bit */

  return hit_edge;
}

/*------------ find_steepest -------------------------------------*/

static int find_steepest(double *slope_dat, int **elevation,
			 double *steepest, double *second_steepest,
			 int r, int c)

{

  int rr, cc;
  int hit_edge=0;
  int i;

  *steepest=-1.;
  *second_steepest=-1.;
  i=0;
 for (rr=LMAX(0,r-1);rr<=LMIN(_nRows-1,r+1);rr++)
  for (cc=LMAX(0,c-1);cc<=LMIN(_nCols-1,c+1);cc++)
  {
    if(rr==r && cc==c) continue;

    if(elevation[rr][cc]<=0)
    {
      hit_edge=1;
      slope_dat[i]=-1.;
      i++;
      continue;
    }

    slope_dat[i]=DOWN*slope(r,rr,c,cc,elevation);

    if(slope_dat[i]>*second_steepest)
    {
      if(slope_dat[i]>*steepest)
      {
        *second_steepest=*steepest;
        *steepest=slope_dat[i];
      }
      else 
      {
        *second_steepest=slope_dat[i];
      }
    }

    i++;

  }
  for(i=0;i<8;i++) slope_dat[i]+=0.01/_diag_res;
  return hit_edge;
}

/*---------------- comparison routine for qsorting ints --------------------*/

static int int_compare(const void *p1, const void *p2)
{
  const int *i1 = (int *) p1;
  const int *i2 = (int *) p2;
  return (int) (*i1 - *i2);
}
