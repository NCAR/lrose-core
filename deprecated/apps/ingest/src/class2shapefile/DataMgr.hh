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
//  Ingest class file and output shapefile
//
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  January 2003
//
//  $Id: DataMgr.hh,v 1.10 2016/03/07 01:23:07 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _DATAMGR_INC_
#define _DATAMGR_INC_

#include <vector>
#include <toolsa/file_io.h>
#include <didss/LdataInfo.hh>
#include <toolsa/DateTime.hh>
#include <shapelib/shapefil.h>

#include "Params.hh"
#include "FieldInfo.hh"
using namespace std;


class DataMgr
{
public:
   DataMgr();
  ~DataMgr();

   //
   // Return 0 upon success, -1 upon failure
   //
   int              init( Params &params );
   int              convert();

   char*            getFileSuffix(){ return inputSuffix; }

private:

   //
   // Data ingest
   //
   Path             inputPath;
   char            *inputSuffix;
   FILE            *inputFile;

   bool             firstCall;
   bool             singleLevel;

   char             siteName[64];
   int              siteId;
   double           lat;
   double           lon;
   double           alt;

   int              processFile();
   int              readHeader();
   char*            getHeaderLine( const char* label, char fieldText[] );
   char*            getHeaderContent( const char* lineText,
                                      int contentPosition = -1 );

   int              findColumns();
   int              findFirstData();
   int              processData();
   int              processEvent();

   //
   // Input file indexing
   //
   bool             useIndexFile;
   string           indexFileName;
   string           previousFileName;
   LdataInfo       *ldataIndex;

   //
   // Sounding data
   //
   static const char*  INDEX_FILENAME;
   static const char*  DELIMETER;

   DateTime            fileTime;
   DateTime            launchTime;

   //
   // Fields to be processed and their characteristics
   //
   vector< FieldInfo* > dataFields;
   vector< FieldInfo* > headerFields;

   FieldInfo* getHeaderField( string fieldName, DBFFieldType fieldType,
                              int fieldWidth, int fieldPrecision );

   Params    *_params;
   bool       headerRequested( Params::header_id_t headerId, 
                               string& headerString );

   //
   // Output file management
   //
   bool             writeEventfile;
   DBFHandle        eventDbf;

   bool             writeShapefile;
   DBFHandle        outputDbf;
   SHPHandle        outputShp;
   SHPObject       *shpObject;

   int              createDbfField( FieldInfo *fieldInfo ); 
   int              writeDbfField( int recordNum, FieldInfo *fieldInfo );
   int              writeShapeRecord();
};

#endif
