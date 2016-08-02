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
//  Nexrad sub-class for reading from an LDM
//
//  Responsible for reading the input data from a LDM file
//  and parceling out the radar data one nexrad message buffer at a time.
//
//  Terri Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  November 2001
//
//  $Id: NexradLdm.hh,v 1.19 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _NEXRAD_LDM_INC_
#define _NEXRAD_LDM_INC_

#include <string>
#include <cstdio>
#include <didss/DsInputPath.hh>
#include <dsdata/DsTrigger.hh>
#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/TriggerInfo.hh>

#include "Params.hh"
#include "Status.hh"
#include "NexradInput.hh"
using namespace std;


class NexradLdm : public NexradInput
{
public:
   NexradLdm();
  ~NexradLdm();

   //
   // Return 0 upon success, -1 upon failure
   //
   int init( Params& params );

   //
   // Sub-classes are required to provide a read method
   // which sets a pointer to the buffer.  The input buffer
   // is owned by the sub-class NOT by the caller.
   //
   Status::info_t     readNexradMsg( ui08* &buffer, 
                                     bool  &volumeTitleSeen );

private:

   //
   // Originating NEXRAD LDM files contain a maximum of 100 beams 
   // of the standard NEXRAD message size (2432 bytes).
   // MIT's LDM files contain a full volume of NEXRAD packets.
   // In both cases, at the beginning of each new volume,
   // is a title header (24 bytes)
   //
   static const size_t NEX_BUFFER_SIZE = 100 * NEX_PACKET_SIZE;

   //
   // Input data read from the LDM files
   //
   ui08               ldmBuffer[NEX_BUFFER_SIZE];

   //
   // Read physical records from the ldm file
   //
   Status::info_t     readPhysicalRecord( size_t& physicalBytes );

   //
   // Index into the physical record to create logical records.
   // The logical record is only a pointer into the physical record.
   //
   size_t             bytesLeft;
   size_t             byteOffset;
   ui08              *logicalRecord;

   Status::info_t     readLogicalRecord();

   //
   // LDM file handling
   //
   DsInputPath       *fileTrigger;
   DsTrigger         *dsTrigger;
   FILE              *ldmFile;
   bool               fileIsOpen;

   bool               realtimeMode;
   int                realtimeSequenceNumber;
   time_t             realtimeDatatime;
   char               *radarInputDir;
   char               *datedDirFormat;
   char               *fileTimeFormat;
   char               fileNameBuffer[MAX_PATH_LEN];
   int                maxValidAgeMin;
   int                maxElapsedSearchTimeMin;
   int                Qtime;
   int                minFileSize;
   int                decompress_mechanism;
   char               *instance;

   bool               isBuild5;
   bool               noVolumeTitleYet;
   bool               oneFilePerVolume;

   Status::info_t     openNextFile();
   void               closeLdmFile();

   char*              getNextLdmRealtimeFile();
   time_t             parseRealtimeLdmFilename(char *fileName,
					       int *seqNum);
   int                getSignificanceCode (char FormatCode );
   int                getDir (char *dirName, bool nextDir);
   int                getFile(char *directory, 
			      char *fileName);

   void               resetFileSearchParameters();
   void               waitQtime(char *fileName);
   void               _byteSwap4(void *p);

   //
   // File compression
   //
   enum compression_t { UNCOMPRESSED,
                        BZIP2,
                        ZLIB
   };

   compression_t      compression;
   string             tmpPath;

};

#endif
