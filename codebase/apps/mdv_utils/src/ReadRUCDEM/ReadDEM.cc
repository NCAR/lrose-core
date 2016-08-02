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
/////////////////////////////////////////////////////////////
// ReadDEM.cc
//
// Reads a DEM header file into a struct.
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb  1999
//
///////////////////////////////////////////////////////////////

#define LINE_LEN 1024

#include <cstdio>
#include <iostream>
#include <cstring>

#include <dataport/swap.h>
#include <toolsa/umisc.h>

#include "ReadDEM.hh"  
using namespace std;

// File scope.
int GetString(const char *Label, char *String, FILE *ifp, int debug);
int GetInt(const char *Label, int *I, FILE *ifp, int debug);
int GetReal(const char *Label, float *F, FILE *ifp, int debug);
int GetLongUnsignedInt(const char *Label, long unsigned *LI, 
		       FILE *ifp, int debug);


// constructor
// Reads the header.
ReadDEM::ReadDEM(const char *BaseName, int debug){



  if (sizeof(short)!=2){
    fprintf(stderr,"Short ints are not two bytes here.\n");
    fprintf(stderr,"I cannot cope.\n");
    Error=1;
    return;
  }

  char FileName[MAX_PATH_LEN];
  //
  // Construe the header filename.
  //
  sprintf(FileName,"%s.HDR",BaseName);
  ifp=fopen(FileName,"ra");

  if (ifp == NULL){
    fprintf(stderr,"%s not found.\n",FileName);
    Error=1;
    return;
  }


  int i; char TmpStr[LINE_LEN];

  i=GetString("BYTEORDER",TmpStr,ifp,debug);
  if (i) {
    fprintf(stderr,"Failed to parse BYTEORDER from %s\n",
	    FileName);
    Error=1;
    return;
  }

  if (!(strncmp(TmpStr,"M",1))){
    SwapBytes=1;
  } else {
    fprintf(stdout,"For Byte order we have %s\n",TmpStr);
    fprintf(stdout,"Do I swap bytes (0=No, 1=Yes) --->");
    fscanf(stdin,"%d",&SwapBytes);
  }

  i=GetString("LAYOUT",TmpStr,ifp,debug);
  if (i) {
    fprintf(stderr,"Failed to parse LAYOUT from %s\n",
	    FileName);
    Error=1; fclose(ifp);
    return;
  }

  if ((strncmp(TmpStr,"BIL",3))){
    fprintf(stderr,"LAYOUT entry confuses me.\n");
    Error=1; fclose(ifp);
    return;
  }

  i=GetLongUnsignedInt("NROWS",&NumRows,ifp,debug);
  i=i+GetLongUnsignedInt("NCOLS",&NumCols,ifp,debug);
  i=i+GetInt("NODATA",&NoData,ifp,debug);

  if (i) {
    fprintf(stderr,"Error parsing integers in header file.\n");
    Error=1;
    return;
  }

  i=GetReal("ULXMAP",&LLLon,ifp,debug);
  i=i+GetReal("ULYMAP",&URLat,ifp,debug);
  i=i+GetReal("XDIM",&dx,ifp,debug);
  i=i+GetReal("YDIM",&dy,ifp,debug);

  if (i) {
    fprintf(stderr,"Error parsing floats in header file.\n");
    Error=1;
    return;
  }

  URLon = LLLon + NumCols * dx;
  LLLat = URLat - NumRows * dy;
  fclose(ifp); // Done with header.

  fprintf(stdout,"%s covers %g -> %g in latitude, %g -> %g in Longitude\n",
	  BaseName,
	  LLLat - dy/2.0,
	  URLat + dy/2.0,
	  LLLon - dx/2.0,
	  URLon + dx/2.0);


  // OK - now open the data file. Leave it open - this is
  // closed by the destructor.

  sprintf(FileName,"%s.DEM",BaseName);
  ifp=fopen(FileName,"rb");

  if (ifp == NULL){
    fprintf(stderr,"%s not found.\n",FileName);
    Error=1;
    return;
  }

  Error=0;

}


////////////////////////////////////////////////////

// Allow the user to access the elevation data
// by lat/lon.

float ReadDEM::Elevation(float lat, float lon, float bad){

  OutsideGrid = 0;
  // return 0 if outside the grid.

  if ((lat > URLat + dy/2.0) ||
      (lat < LLLat - dy/2.0) ||
      (lon < LLLon - dx/2.0) ||
      (lon > URLon + dx/2.0)){

    OutsideGrid = 1;
    return 0.0;
  }

  unsigned long j = (unsigned long)((URLat-lat)/dy);
  unsigned long i = (unsigned long)((lon-LLLon)/dx);

  // Correct roundoff as it may lead
  // to an access violation.

  if (i<0) i=0;        if (j<0) j=0;
  if (i>NumCols-1) i=NumCols-1;  
  if (j>NumRows-1) j=NumRows-1;

  // Seek the indexed value in the file.

  fseek(ifp,sizeof(short)*(i+NumCols*j),SEEK_SET);
  short Alt;
  fread(&Alt,1,sizeof(short),ifp);

  if (SwapBytes){
    unsigned char *b,t;
    b=(unsigned char *)&Alt;
    t=*b;
    *b=*(b+1);
    *(b+1)=t;
  }


  if (Alt==NoData) return bad;

  return float(Alt);


}

float ReadDEM::MaxElevation(float lat, float lon, float bad,
                            float cellDx, float cellDy) {

  OutsideGrid = 0;
  // return 0 if outside the grid.

  if ((lat > URLat + dy/2.0) ||
      (lat < LLLat - dy/2.0) ||
      (lon < LLLon - dx/2.0) ||
      (lon > URLon + dx/2.0)){

    OutsideGrid = 1;
    return 0.0;
  }

  // Figure out how many cells to iterate through.
  //  Note that the iteration increment will be dx / 2,
  //  and 1 is added to include the edges of the cell.
  // 
  long iterX = ((long) (cellDx / dx + 0.5)) * 2 + 1;
  long iterY = ((long) (cellDy / dy + 0.5)) * 2 + 1;

  // Find the corner of the iteration area.
  float baseLon = lon - ((float) (iterX - 1)) * (dx / 2) / 2;
  float baseLat = lat - ((float) (iterY - 1)) * (dy / 2) / 2;

bool spew = false;
// bool spew = true;

if (spew) {
  cout << "Finding MaxElevation at lon: " << lon << " lat: " << lat 
       << " using baseLon: " << baseLon << " baseLat: " << baseLat
       << " using cellDx: " << cellDx << " cellDy: " << cellDy
       << " and iterating over cells (lon, lat): " << iterX << ", " << iterY
       << endl;
}

  float maxVal = 0.0;
  for (int x = 0; x < iterX; x++) {
    float currLon = baseLon + x * (dx / 2);

    for (int y = 0; y < iterY; y++) {
      float currLat = baseLat + y * (dy / 2);;

      float curVal = Elevation(currLat, currLon, bad);

      if ((x == 0 && y == 0) || (curVal > maxVal)) {
        maxVal = curVal;

if (spew) {
  cout << "    Elevation at (" << currLon << ", " << currLat << "):" << curVal
       << endl;
}

      }
// if (spew) {
//   cout << "   Elevation at (" << currLon << ", " << currLat << "):" << curVal
//        << endl;
// }

    }
  }
 
if (spew) {
  cout << "        Max: " << maxVal << endl;
}

  return maxVal;
}



/////////////////////////////////////////

int GetString(const char *Label, char *String, FILE *ifp, int debug){
  //
  // This gets a string from the file - if Label is
  // "BYTEORDER" and the line from the file reads
  // "BYTEORDER    M" then the string is set to "M".
  //
  char Line[LINE_LEN];
  int GotIt=0;
  do{
    fgets(Line,LINE_LEN-1,ifp);
    if (!(strncmp(Label,Line,strlen(Label)))){
      if (debug) fprintf(stderr,"%s",Line);
      GotIt=1;
      sscanf(Line+strlen(Label),"%s",String);
    }

  }while((!(GotIt)) && (!(feof(ifp))));

  if (!(GotIt)){
    fclose(ifp);
    return 1;
  }

  if (debug) fprintf(stderr,"READ %s\n",String);
  return 0;

}

/////////////////////////////////////////

int GetInt(const char *Label, int *I, FILE *ifp, int debug){
  //
  // This gets an int from the file - if Label is
  // "ORDER" and the line from the file reads
  // "ORDER    3" then the int is set to 3.
  //

  char Line[LINE_LEN];
  int GotIt=0;
  do{
    fgets(Line,LINE_LEN-1,ifp);
    if (!(strncmp(Label,Line,strlen(Label)))){
      if (debug) fprintf(stderr,"%s",Line);
      GotIt=1;
      sscanf(Line+strlen(Label),"%d",I);
    }

  }while((!(GotIt)) && (!(feof(ifp))));

  if (!(GotIt)){
    fclose(ifp);
    return 1;
  }

  if (debug) fprintf(stderr,"READ %d\n",*I);
  return 0;

}

/////////////////////////////////////////

int GetLongUnsignedInt(const char *Label, long unsigned *LI, 
		       FILE *ifp, int debug){
  //
  // This gets an int from the file - if Label is
  // "ORDER" and the line from the file reads
  // "ORDER    3" then the int is set to 3.
  //

  char Line[LINE_LEN];
  int GotIt=0;
  do{
    fgets(Line,LINE_LEN-1,ifp);
    if (!(strncmp(Label,Line,strlen(Label)))){
      if (debug) fprintf(stderr,"%s",Line);
      GotIt=1;
      sscanf(Line+strlen(Label),"%ld",LI);
    }

  }while((!(GotIt)) && (!(feof(ifp))));

  if (!(GotIt)){
    fclose(ifp);
    return 1;
  }

  if (debug) fprintf(stderr,"READ %ld\n",*LI);
  return 0;

}


/////////////////////////////////////////

int GetReal(const char *Label, float *F, FILE *ifp, int debug){
  //
  // This gets a float from the file - if Label is
  // "BYTES" and the line from the file reads
  // "BYTES    8.0" then the float is set to 8.0.
  //

  char Line[LINE_LEN];
  int GotIt=0;
  do{
    fgets(Line,LINE_LEN-1,ifp);

    if (!(strncmp(Label,Line,strlen(Label)))){
      if (debug) fprintf(stderr,"%s",Line);
      GotIt=1;
      sscanf(Line+strlen(Label),"%g",F);
    }

  }while((!(GotIt)) && (!(feof(ifp))));

  if (!(GotIt)){
    fclose(ifp);
    return 1;
  }

  if (debug) fprintf(stderr,"READ %g\n",*F);
  return 0;

}

//////////////////////////////////////////////

ReadDEM::~ReadDEM()
{

  fclose(ifp);

}



