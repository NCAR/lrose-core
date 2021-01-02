#include <malloc.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cerrno>
#include <cmath>
using namespace std;

/*----------------- file scope variables ------------------------------------*/

static double _xRes, _yRes, _diagRes;
static int _nRows, _nCols;

static char *prog_name = "flowlength";
static bool _debug = false;
static bool _verbose = false;
static char *_demPath = "./dem.txt";
static char *_vegPath = "./veg.txt";
static char *_flowlenGisPath = "./flowlen.gis.txt";
static char *_flowlenDataPath = "./flowlen.data.txt";
static double _threshold = 0.0;
static bool _noDiag = false;

/*----------------- file scope functions ------------------------------------*/

static void parseArgs(int argc, char **argv, const char *prog_name);

static void usage(const char *prog_name, FILE *out);

static int read_gis(const char *path,
		    int &ncols, int &nrows,
		    double &minx, double &miny,
		    double &maxx, double &maxy,
		    double &cellsize,
		    double &nodataval,
		    double ** &array);

static int write_gis(const char *path,
		     int ncols, int nrows,
		     double minx, double miny,
		     double cellsize,
		     double nodataval,
		     double **array);

static int write_flowlen_data(const char *path,
			      int ncols, int nrows,
			      double **array);

static double** alloc_2d_double_array (int nrows, int ncols);

static int** alloc_2d_int_array (int nrows, int ncols);

static int find_incr_len(double **elevation,
			 int startRow, int startCol,
			 int &endRow, int &endCol,
			 double &incrLen);

static void compute_flow_lengths(double **veg,
				 double **elevation,
				 double **flowlen,
				 double &meanLen);

/*-------------------- main -------------------------------------------------*/

int main (int argc, char **argv)

{


  /* parse command line args */

  parseArgs(argc, argv, prog_name);

  /* read in veg file */

  int ncolsVeg, nrowsVeg;
  double minxVeg, minyVeg;
  double maxxVeg, maxyVeg;
  double cellsizeVeg, nodatavalVeg;
  double **veg;

  if (_debug) {
    cerr << "Reading in Veg file ..." << endl;
  }

  if (read_gis(_vegPath,
	       ncolsVeg, nrowsVeg,
	       minxVeg, minyVeg,
	       maxxVeg, maxyVeg,
	       cellsizeVeg, nodatavalVeg,
	       veg)) {
    int errNum = errno;
    cerr << "ERROR - flowlength" << endl;
    cerr << "  Cannot read in veg file: " << _vegPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // set global variables

  _nCols = ncolsVeg;
  _nRows = nrowsVeg;
  if (_nCols == 0 || _nRows == 0) {
    cerr << "ERROR - flowlength" << endl;
    cerr << "  Bad VEG file: " << _vegPath << endl;
    cerr << "  nRows: " << _nRows << endl;
    cerr << "  nCols: " << _nCols << endl;
  }
  _xRes=(maxxVeg-minxVeg)/(double)(_nCols - 1);
  _yRes=(maxyVeg-minyVeg)/(double)(_nRows - 1);
  _diagRes=pow(_xRes*_xRes+_yRes*_yRes,0.5);

  // read in DEM
  
  double **elevation = NULL;
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
    cerr << "ERROR - flowlength" << endl;
    cerr << "  Cannot read in DEM file: " << _demPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  // check that dem and veg match
  
  if (ncolsDem != ncolsVeg || nrowsDem != nrowsVeg ||
      fabs(minxDem - minxVeg) > 0.001 || fabs(maxxDem - maxxVeg) > 0.001 ||
      fabs(minyDem - minyVeg) > 0.001 || fabs(maxyDem - maxyVeg) > 0.001) {
    cerr << "ERROR - flowlength" << endl;
    cerr << "  DEM and veg grids do not match" << endl;
    cerr << "  Use -debug option to see grid properties" << endl;
    return -1;
  }

  // compute flow lengths for each grid point

  double **flowlen = alloc_2d_double_array (_nRows, _nCols);
  double meanLen = 0.0;
  compute_flow_lengths(veg, elevation, flowlen, meanLen);

  // print mean flow len

  fprintf(stdout, "Mean flow len: %g\n", meanLen);

  // write flow lengths gis file

  if (write_gis(_flowlenGisPath,
		_nCols, _nRows,
		minxVeg, minyVeg,
		cellsizeVeg, nodatavalVeg,
		flowlen)) {
    cerr << "ERROR - flowlength" << endl;
    cerr << "  Cannot write output GIS file: " << _flowlenGisPath << endl;
    return -1;
  }

  // write flow lengths data file
  
  if (write_flowlen_data(_flowlenDataPath,
			 _nCols, _nRows,
			 flowlen)) {
    cerr << "ERROR - flowlength" << endl;
    cerr << "  Cannot write output data file: " << _flowlenDataPath << endl;
    return -1;
  }

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
	
    } else if (!strcmp(argv[i], "-veg")) {
      
      if (i < argc - 1) {
	_vegPath = argv[++i];
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-flowlen_gis")) {
      
      if (i < argc - 1) {
	_flowlenGisPath = argv[++i];
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-flowlen_data")) {
      
      if (i < argc - 1) {
	_flowlenDataPath = argv[++i];
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-threshold")) {
      
      if (i < argc - 1) {
	if (sscanf(argv[++i], "%lg", &_threshold) != 1) {
	  iret = -1;
	}
      } else {
	iret = -1;
      }
	
    } else if (!strcmp(argv[i], "-nodiag")) {

      _noDiag = true;
      
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
	  "       [ -veg ? ] input file name for vegetation coverage\n"
	  "       [ -flowlen_gis ? ] output GIS file with flow lengths\n"
	  "       [ -flowlen_data ? ] output data file with flow lengths\n"
	  "       [ -threshold ? ] threshold value\n"
	  "       [ -nodiag ] do not allow flow in diagonal directions.\n"
	  "         By default, flow can follow diagonals.\n"
	  "\n");

}

/*--------------------- alloc_2d_double_array -----------------------------*/

static double** alloc_2d_double_array (int nrows, int ncols)

{

  double **array = (double **) malloc(nrows * sizeof(double*));
  if (array == NULL) {
    cerr << "ERROR - flowlength::alloc_2d_array" << endl;
    cerr << "  Out of memory" << endl;
    exit(-1);
  }

  for(int r=0; r < nrows; r++) {
    array[r] = (double *) malloc(ncols * sizeof(double));
    if (array[r] == NULL) {
      cerr << "ERROR - flowlength::alloc_2d_array" << endl;
      cerr << "  Out of memory" << endl;
      exit(-1);
    }
  }
  return array;

}

/*--------------------- alloc_2d_int_array -----------------------------*/

static int** alloc_2d_int_array (int nrows, int ncols)

{

  int **array = (int **) malloc(nrows * sizeof(int*));
  if (array == NULL) {
    cerr << "ERROR - flowlength::alloc_2d_array" << endl;
    cerr << "  Out of memory" << endl;
    exit(-1);
  }

  for(int r=0; r < nrows; r++) {
    array[r] = (int *) malloc(ncols * sizeof(int));
    if (array[r] == NULL) {
      cerr << "ERROR - flowlength::alloc_2d_array" << endl;
      cerr << "  Out of memory" << endl;
      exit(-1);
    }
  }
  return array;

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
		    double ** &array)

{

  if (_debug) {
    cerr << "Opening gis file for reading: " << path << endl;
  }

  /* open file */

  FILE *in;
  if ((in = fopen(path, "r")) == NULL) {
    fprintf(stderr, "ERROR - flowlength::read_gis\n");
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

  array = alloc_2d_double_array(nrows, ncols);

  /* read in data */
  
  for (int ii = 0; ii < nrows; ii++) {
    for (int jj = 0; jj < ncols; jj++) {
      if (fscanf(in, "%lg", &array[ii][jj]) != 1) {
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

/*--------------------- write_gis ----------------------------------------*/
/*
 * write data to GIS-format file
 */

static int write_gis(const char *path,
		     int ncols, int nrows,
		     double minx, double miny,
		     double cellsize,
		     double nodataval,
		     double **array)

{

  if (_debug) {
    cerr << "Opening gis file for writing: " << path << endl;
  }
  
  /* open file */

  FILE *out;
  if ((out = fopen(path, "w")) == NULL) {
    fprintf(stderr, "ERROR - flowlength::write_gis\n");
    fprintf(stderr, "  Cannot open file for writing: %s\n", path);
    perror(path);
    return -1;
  }

  /* write header */

  fprintf(out, "ncols %d\n", ncols);
  fprintf(out, "nrows %d\n", nrows);
  fprintf(out, "xllcorner %g\n", minx);
  fprintf(out, "yllcorner %g\n", miny);
  fprintf(out, "cellsize %g\n", cellsize);
  fprintf(out, "NODATA_value %g\n", nodataval);

  /* write data */
  
  for (int ii = 0; ii < nrows; ii++) {
    for (int jj = 0; jj < ncols; jj++) {
      fprintf(out, "%12g",  array[ii][jj]);
      if (jj == ncols - 1) {
	fprintf(out, "\n",  array[ii][jj]);
      } else {
	fprintf(out, " ",  array[ii][jj]);
      }
    } // jj
  } // ii
  
  fclose(out);

  return 0;

}

/*--------------------- write_flowlen_data ------------------------------*/
/*
 * write flowlen data to simple ascii file
 */

static int write_flowlen_data(const char *path,
			      int ncols, int nrows,
			      double **array)

{
  
  if (_debug) {
    cerr << "Opening data file for writing: " << path << endl;
  }
  
  /* open file */

  FILE *out;
  if ((out = fopen(path, "w")) == NULL) {
    fprintf(stderr, "ERROR - flowlength::write_flowlen_data\n");
    fprintf(stderr, "  Cannot open file for writing: %s\n", path);
    perror(path);
    return -1;
  }

  /* write data */
  
  for (int ii = 0; ii < nrows; ii++) {
    for (int jj = 0; jj < ncols; jj++) {
      fprintf(out, "%g\n",  array[ii][jj]);
    } // jj
  } // ii
  
  fclose(out);

  return 0;

}

/*------------ find incremental length ------------------------------*/

static int find_incr_len(double **elevation,
			 int startRow, int startCol,
			 int &endRow, int &endCol,
			 double &incrLen)

{

  // initialize

  endRow = startRow;
  endCol = startCol;
  incrLen = 0.0;
  double startElev = elevation[startRow][startCol];
  double endElev = startElev;
  double maxSlope = 0.0;

  // search diagonals first if required

  //quitado if (!_noDiag) {

   // look north
  
  if (startRow > 0) {
    double elev = elevation[startRow - 1][startCol];
    double slope = (startElev - elev) / _yRes;
    if (slope >= maxSlope) {
      endElev = elev;
      endRow = startRow - 1;
      endCol = startCol;
      incrLen = _yRes*pow(1+slope*slope,0.5);
      maxSlope = slope;
    }
  }

      // look north-west
    
    if (startRow > 0 && startCol > 0) {
      double elev = elevation[startRow - 1][startCol - 1];
      double slope = (startElev - elev) / _diagRes;
      if (slope >= maxSlope) {
	endElev = elev;
	endRow = startRow - 1;
	endCol = startCol - 1;
	incrLen = _diagRes*pow(1+slope*slope,0.5);
	maxSlope = slope;
      }
    }

   // look north-east
    
    if (startRow > 0 && startCol < (_nCols - 1)) {
      double elev = elevation[startRow - 1][startCol + 1];
      double slope = (startElev - elev) / _diagRes;
      if (slope >= maxSlope) {
	endElev = elev;
	endRow = startRow - 1;
	endCol = startCol + 1;
	incrLen = _diagRes*pow(1+slope*slope,0.5);
	maxSlope = slope;
      }
    }

   // look west
  
  if (startCol > 0) {
    double elev = elevation[startRow][startCol - 1];
    double slope = (startElev - elev) / _xRes;
    if (slope >= maxSlope) {
      endElev = elev;
      endRow = startRow;
      endCol = startCol - 1;
      incrLen = _xRes*pow(1+slope*slope,0.5);
      maxSlope = slope;
    }
  }

  // look east
  
  if (startCol < _nCols - 1) {
    double elev = elevation[startRow][startCol + 1];
    double slope = (startElev - elev) / _xRes;
    if (slope >= maxSlope) {
      endElev = elev;
      endRow = startRow;
      endCol = startCol + 1;
      incrLen = _xRes*pow(1+slope*slope,0.5);
      maxSlope = slope;
    }
  }
    
  
      // look south-west
    
    if (startRow < (_nRows - 1) && startCol > 0) {
      double elev = elevation[startRow + 1][startCol - 1];
      double slope = (startElev - elev) / _diagRes;
      if (slope >= maxSlope) {
	endElev = elev;
	endRow = startRow + 1;
	endCol = startCol - 1;
	incrLen = _diagRes*pow(1+slope*slope,0.5);
	maxSlope = slope;
      }
    }
        
          // look south-east
    
    if (startRow < (_nRows - 1) && startCol < (_nCols - 1)) {
      double elev = elevation[startRow + 1][startCol + 1];
      double slope = (startElev - elev) / _diagRes;
      if (slope >= maxSlope) {
	endElev = elev;
	endRow = startRow + 1;
	endCol = startCol + 1;
	incrLen = _diagRes*pow(1+slope*slope,0.5);
	maxSlope = slope;
      }
    }

 

  //} // if (!_noDiag) quitado

  // then search adjacent grid points

 
  // look south
  
  if (startRow < _nRows - 1) {
    double elev = elevation[startRow + 1][startCol];
    double slope = (startElev - elev) / _yRes;
    if (slope >= maxSlope) {
      endElev = elev;
      endRow = startRow + 1;
      endCol = startCol;
      incrLen = _yRes*pow(1+slope*slope,0.5);
      maxSlope = slope;
    }
  }



  if (endRow == startRow && endCol == startCol) {
    return -1;
  }
  return 0;

}

///////////////////////////////////////////////////////
// compute flow lengths

static void compute_flow_lengths(double **veg,
				 double **elevation,
				 double **flowlen,
				 double &meanLen)
  
{

  // alloc array for keeping track of points visited

  int **visited = alloc_2d_int_array(_nRows, _nCols);

  double sumLen = 0.0;
  int count = 0;

  for (int rr = 0; rr < _nRows; rr++) {

    for (int cc = 0; cc < _nCols; cc++) {

      // clear visited array

      for (int vrr = 0; vrr < _nRows; vrr++) {
	memset(visited[vrr], 0, _nCols * sizeof(int));
      }
      
      int rStart = rr;
      int cStart = cc;
      int rEnd, cEnd;
      double totLen = 0.0;
      double incrLen;
      
      // find incremental length from this point to the next
      // point downhill
      
      while (find_incr_len(elevation, rStart, cStart,
			   rEnd, cEnd, incrLen) == 0) {
	
	// break out if vegetation exceeds threshold at start point
	
	if (veg[rStart][cStart] > _threshold) {
	  break;
	}

	// break out if we have already visited this point

	if(visited[rStart][cStart]) {
	  break;
	}
	visited[rStart][cStart] = 1;
	
	// total up the length
	
	totLen += incrLen;
	
	// move to next point downhill
	
	rStart = rEnd;
	cStart = cEnd;
	
      } // while
      
      // set flow len to total

      flowlen[rr][cc] = totLen;
      sumLen += totLen;
      count++;

      if (_verbose) {
	cerr << "rr, cc, rStart, rEnd, cStart, cEnd, totLen, sumLen: "
	     << rr << ", " << cc << ", "
	     << rStart << ", " << rEnd << ", "
	     << cStart << ", " << cEnd << ", "
	     << totLen << ", " << sumLen << endl;
      }

    } // cc
    
  } // rr

  if (count > 0) {
    meanLen = sumLen / count;
  } else {
    meanLen = 0.0;
  }

}


