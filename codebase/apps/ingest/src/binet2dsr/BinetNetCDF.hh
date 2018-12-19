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
////////////////////////////////////////////////////////////////////////////////
//
//  NetCDF class for radar input stream.
//
//  Responsible for reading radar data from a NetCDF file
//  and parceling out the radar data one DsRadarMsg at a time.
//
//  Terri Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  September 2001
//
//  $Id: BinetNetCDF.hh,v 1.9 2018/01/26 20:15:07 jcraig Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _BINET_NetCDF_INC_
#define _BINET_NetCDF_INC_

#include <vector>
#include <utility>
#include <Ncxx/Nc3File.hh>

using namespace std;

//
// Forward class declarations
//
class Params;
class DsInputPath;


class BinetNetCDF
{
public:
   BinetNetCDF();
  ~BinetNetCDF();

   //
   // Return 0 upon success, -1 upon failure
   //
   int init( Params& params, DsInputPath* trigger );

   //
   // Return more detailed status info
   //
   enum inputStatus_t { ALL_OK,
                        BAD_DATA,
                        BAD_FILE,
                        END_OF_DATA
   };

   inputStatus_t readRadarMsg();

   //
   // Handles to the output radar message and its components
   // The input radar message is for internal use only
   // The DsRadarMsg cannot be returned as a const because 
   // when the DsRadarQueue goes to write out the message
   // the serialization of the message modifies the object
   //
   DsRadarMsg&     getRadarMsg(){ return outputRadarMsg; }
   DsRadarFlags&   getRadarFlags(){ return outputRadarFlags; }
   DsRadarParams&  getRadarParams(){ return outputRadarParams; }
   DsRadarBeam&    getRadarBeam(){ return outputRadarBeam; }

   //
   // Do the output radar parameters differ from before?
   //
   bool            radarParamsChanged(){ return outputParamsChanged; }   

private:

   //
   // Radar messaging
   // NOTE: the DsRadarMsg declarations MUST come before the reference
   //       declarations of the components below because of the
   //       constructor initialization.  Be careful when reordering.
   //
   DsRadarMsg                inputRadarMsg;
   DsRadarMsg                outputRadarMsg;

   //
   // Components of the radar messages
   //
   DsRadarFlags&             inputRadarFlags;
   DsRadarParams&            inputRadarParams;
   DsRadarBeam&              inputRadarBeam;
   vector<DsFieldParams*>&   inputRadarFields;

   DsRadarFlags&             outputRadarFlags;
   DsRadarParams&            outputRadarParams;
   DsRadarBeam&              outputRadarBeam;
   vector<DsFieldParams*>&   outputRadarFields;

   //
   // State of the radar parameters
   //
   bool                      inputParamsChanged;
   bool                      outputParamsChanged;
   bool                      overrideRadarLocation;

   //
   // Beam data buffer
   //
   ui08                     *beamData;
   size_t                    beamDataLen;

   void                      allocateBeamData( int inputDataLen );

   //
   // NetCDF file handling
   //
   DsInputPath  *fileTrigger;
   Nc3File       *ncFile;
   bool          fileIsOpen;
   bool          newFileRead;

   inputStatus_t openNextFile();

   //
   // NetCDF error handling
   //
   Nc3Error       ncError;

   const char*   ncErrorMsg(){ return nc_strerror(ncError.get_err()); }

   //
   // State information useful for radar processing
   //
   bool          firstCall;
   time_t        baseTime;
   int           numBeams;
   int           currentVol;    // zero-based(?) index
   int           currentBeam;   // zero-based index
   int           currentTilt;   // zero-based index
   float         currentElev;   // in degrees
   float         targetElev;    // in degrees
   float         maxDeltaElev;  // in degrees


   typedef pair< float, float > ScaleBiasPair;
   vector< ScaleBiasPair* >     inputScaling;

   //
   // Radar processing methods
   //
   inputStatus_t setRadarMsg();
   void          setRadarParams();
   void          setRadarBeam();
   void          setRadarFlags();

   void          shiftRadarMsg();
   void          shiftFieldParams();

   void          clearScaling();
   void          setDataScaling();

   //
   // Fetching beams from files
   //
   inputStatus_t seekValidData();
   void          nextBeam();

   //
   // Associations between field types, names, and units
   //
   typedef struct { int    type;
                    char*  name;
                    char*  units;
                  } fieldInfo_t;

   static const fieldInfo_t FIELD_INFO[];
};

#endif
