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
///////////////////////////////////////////////////////////////
// TstormGroup class
//   Contains information on a group of storms
//
// $Id: TstormGroup.hh,v 1.13 2017/06/09 16:27:58 prestop Exp $
//////////////////////////////////////////////////////////////
#ifndef _TSTORM_GROUP_HH_
#define _TSTORM_GROUP_HH_

#include <string>
#include <vector>

#include <dsdata/TstormGrid.hh>
#include <rapformats/tstorm_spdb.h>
#include <toolsa/DateTime.hh>

using namespace std;

//
// Forward class declarations
//
class Tstorm;

class TstormGroup {

 public:

   //////////////////
   // Constructors //
   //////////////////

    // Default constructor

   TstormGroup(const bool debug = false);

   TstormGroup(const int n_sides,
	       const time_t data_time,
	       const float dbz_threshold,
	       const float start_azimuth,
	       const float delta_azimuth,
	       const TstormGrid &tstorm_grid,
	       const bool debug = false);

   // Create a storm group from a pointer to a buffer containing storm
   // data in the format described in titan/tstorm_spdb.h.  If the
   // swap_data flag is set, the data in the buffer is swapped (in place)
   // before the storm group is created.

   TstormGroup(const char* groupPtr,
	       const bool swap_data = false,
	       const bool debug = false);

   ////////////////
   // Destructor //
   ////////////////

   ~TstormGroup();


   ////////////////////////////
   // Initialization methods //
   ////////////////////////////

   // Clear the data in the storm group

   void clearData();

   // Set the data in the storm group based on the given buffer.  The
   // current group data is cleared before the new data is added.  The
   // data buffer is assumed to be in the format described in
   // titan/tstorm_spdb.h.  If the swap_data flag is set, the data
   // in the buffer is byte-swapped (in place) into native ordering
   // before processing.

   void setData(const char* groupPtr, const bool swap_data = false );

   // Add the given storm to the storm group.  It is assumed that the
   // new storm is compatible with the group (e.g. has the same grid).
   // After this method call, the TstormGroup object retains control
   // of the Tstorm object, so this object shouldn't be modified or
   // freed by the calling method.

   void addTstorm(Tstorm *new_storm);
  

   //////////////////////////
   // Input/Output methods //
   //////////////////////////

   // Retrieve the number of bytes occupied by this object when stored
   // in an SPDB database.

   int getSpdbNumBytes() const;
  

   // Write the object information to the given buffer in SPDB format,
   // including swapping the bytes to big-endian order.
   //
   // Note that the calling method must call getSpdbNumBytes() and allocate
   // a large enough buffer before calling this routine.

   void writeSpdb(ui08 *buffer) const;

  
   /////////////////////
   // Utility methods //
   /////////////////////

   void sortByAreaDescending();
  

   ////////////////////
   // Access methods //
   ////////////////////

   int getProjType() const 
           { return grid.getProjType(); }
  
   const TstormGrid &getGrid() const { return grid; }
  
   int getNStorms() const { return tstorms.size(); }

	 int getNSides() const { return nSides; }
  
   time_t getDataTime() const { return dataTime; }
   void setDataTime(const time_t data_time) { dataTime = data_time; }

   time_t getExpireTime() const { return expireTime; }
   void setExpireTime(const time_t expire_time) { expireTime = expire_time; }

   float getDbzThreshold() const { return dbzThreshold; }
 
   float getStartAzimuth() const { return startAz; }
  
   float getDeltaAzimuth() const { return deltaAz; }
  
   vector< Tstorm* >&         getTstorms() { return tstorms; }
   

 private:
  
   bool _debug;

   //
   // Data members
   //
   int               nSides;
   time_t            dataTime;
   time_t            expireTime;
   float             dbzThreshold;
   float             startAz;
   float             deltaAz;
   TstormGrid        grid;
   
   //
   // Storms
   // 
   vector< Tstorm* > tstorms;

   //
   // methods
   //
   bool remapNewStorm(Tstorm *new_storm);
};

#endif


    
   
