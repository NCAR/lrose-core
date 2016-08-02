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


 
#ifndef SOUNDING_MERGE
#define SOUNDING_MERGE   

#include <Spdb/SoundingGet.hh>
#include <Spdb/SoundingPut.hh>

#include <ctime>
#include "Params.hh"
using namespace std;


class SoundingMerge {
 
public:        
  //
  // Handy struct to hold sounding data.
  // Does not include sounding time or ID.
  //
  typedef struct {
    double lat; // Site latitude.
    double lon; // Longitude.
    double alt; // Site altitude.

    int numPoints;
    double badVal;

    const char *siteName;
    const char *source;
    //
    // Actual arrays of data. I use arrays rather than vectors.
    // NULL means no data.
    //
    double *height; 
    double *u; 
    double *v;
    double *w; 
    double *prs;
    double *relHum; 
    double *temp;

  } SoundingDataHolder;
  //
  // Constructor. Makes copy of TDR parameters.
  //
  SoundingMerge(const Params *P);
  //
  // Destructor.
  //
  ~SoundingMerge();
  //
  //
  // Init. Client gives the class the sounding ID and the time,
  // and it looks in the output SPDB database to see if there is
  // already something there. If there is, it is sorted into ascending
  // order with respect to altitude and stored internally.
  // Returns 0 on failure.
  //
  int Init(time_t dataTime, int id);
  //
  //
  // Merge. The real woo-woo of the class.  You pass in the specs for
  // a sounding and it is merged with whatever was stored internally
  // by the init. This is then written out to the SPDB database.
  // Returns 0 on failure.
  //
  int Merge(SoundingDataHolder S);
  //
  // Soundings are merged by comparing heights. If the
  // heights of two levels  match to within epsilon then they
  // are taken to be the same level. Default is 200 m.
  //
  void setMergeEpsilon(double epsilon );
  //
  // Allocate a sounding data structure.
  //
  void AllocateSoundingDataHolder( SoundingDataHolder *S,
						  int numPoints);
  //
  // Free up a SoundingDataHolder structure.
  //
  void FreeSoundingDataHolder( SoundingDataHolder S);
  //
  // Sort a SoundingDataHolder structure into ascending order wrt height.
  //
  void Sort( SoundingDataHolder S);
  //
  private :

  double _epsilon;
  const Params *_params;
  //
  // Include sounding get and sounding put objects.
  //
  SoundingGet _Sget;
  int _productIndex;
  SoundingPut _Sput;
  time_t _dataTime;
  int _id;
  double _lat, _lon, _alt;
  //
  // Method that actually does the merging if we have to.
  //
  void _merge(SoundingDataHolder toBeAdded,
			     SoundingDataHolder &merged);
  //
  // Private function to move a single value across.
  //
  void _moveVal(double *SourceArray,
			       double *DestArray,
			       int SourceIndex,
			       int DestIndex,
			       double DestBadVal,
			       double SourceBadVal);
  //
  // Private function to merge two values into one.
  //
  void _mergeVal(double *preferred,
				double *secondary,
				double *out,
				int prefIndex,
				int secondIndex,
				int outIndex,
				double prefBad,
				double secBad);
  //
  // Private functions to do vertical linear interpolation
  // on a sounding.
  //
  void _interpSounding(SoundingDataHolder S,
				      double maxInterpDist);
  //
  void _interpArray(double *x, double *y,
				   double badVal, int numPoints,
				   double maxInterpDist);

  void _printSounding(SoundingDataHolder S);



};


#endif

