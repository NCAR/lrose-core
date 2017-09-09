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

#ifndef _MDV_FILE_INC_
#define _MDV_FILE_INC_

#include <string>
#include <vector>
#include <ctime>
#include <cstdio>
#include <toolsa/ldata_info.h>
#include <Mdv/mdv/mdv_file.h>
#include <toolsa/Path.hh>
#include <Mdv/mdv/MdvMasterHdr.hh>
#include <Mdv/mdv/MdvVlevel.hh>
using namespace std;

//
// Forward class declarations
//
class Grid;
class MdvField;
class MdvChunk;
class MdvRead;

class MdvFile {
   static int test;
public:

   //
   // Input mdv file constructor
   //
   MdvFile( const char* topDir );

   //
   // Empty and Copy constructors
   //
   MdvFile();
   MdvFile( const MdvFile& );
  ~MdvFile();

   MdvFile& operator= ( const MdvFile &source );
   void copy( const MdvFile &source );           

   //
   // Setting general information on the file
   //
   void            setDebug(bool deb)   { _isDebug = deb; }
   void            setVerbose(bool ver) { _isVerbose = ver;
                                          if(ver) setDebug(ver); }

   void            setFields( vector< pair< string*, Grid* >* >& sourceFields);
   MdvField*       addField( const string &name, const Grid &data );

   int             setVlevel( int index, const MDV_vlevel_header_t & vlevel );

   void            setChunks( vector< MdvChunk* > &chunks );
   void            addChunk( MdvChunk &chunk );

   inline  void    setSensor( double lat, double lon, double alt=0.0 )
                            { masterHdr.setSensor( lat, lon, alt ); }
   inline  void    setTime( const time_t when ){ masterHdr.setTime( when ); }
   inline  void    setDescription( const char *name, 
                                   const char *source=NULL, 
                                   const char *desc=NULL )
                                 { masterHdr.setDescription( name, source, 
                                                             desc ); }

   // Clears all of the master header, then recalculates required stuff:
   //   Num Fields, Num Vlevels, Num Chunks, Max Geometry.
   //   
   void            setMasterHeader(const MDV_master_header_t & header);

   //
   // Fetching stuff
   //
   int          getNumFields() const { return fields.size(); }
   MdvField *getField( const char* fieldName );
   MdvField *getField( const int fieldNum );

   MdvVlevel * getVlevel(unsigned int index) 
   { return (index >= vlevels.size() ? (MdvVlevel *) NULL : &vlevels[index]); }

   inline  char*   getSource(){ return masterHdr.getSource(); }

   MDV_master_header_t & getMasterHeader() { return masterHdr.getInfo(); }


   //
   // Doing the real work
   //
   int     write( time_t timeStamp, int encodeType = MDV_PLANE_RLE8 );
   int     write( FILE *outputFile, int encodeType = MDV_PLANE_RLE8 );

   //
   // Ingest single field from named file.
   //   The field contains the src field number -- the field to get from file.
   //
   int     read( const string & dataFile,
                 int    fieldIndex,
                 MDV_master_header_t & replyMasterHeader,
                 string & errString );

   //
   // Ingest all fields present in this MdvFile object from named file.
   //
   int     read( const string & dataFile,
                 string & errString );

   // Read all fields from a single mdv file on disk, creating a new
   //   MdvField for each. Uses the passed-in grid for geometry.
   //   Resets the master header info on the file.
   // 
   // NOTE:
   //   Allocates a Grid for each field found in the file.
   //      These need to be deleted by the caller!
   // 
   int     readAllFields(const string & dataFile,
                         const MdvField & protoField, string & errString);

   //
   // Initialization value contant
   //
   static const time_t NEVER;

protected:
   int _getHandle(const string & dataFile, MDV_handle_t & handle);
   int _readField(MdvRead & mdvReader,
                  MdvField & field, string & errString);

private:

   //
   // Output management
   //
   string                  topDir;
   Path                    outputPath;
   char                    tmpOutputPath[MAX_PATH_LEN];

   FILE*                   getOutputFile( time_t when );

   //
   // Low-level mdv structures
   //
   MdvMasterHdr            masterHdr;
   LDATA_handle_t          ldataIndex;

   //
   // Mdv data
   //
   vector< MdvField* >     fields;
   vector< MdvVlevel >     vlevels;
   vector< MdvChunk* >     chunks;

   bool _isDebug;
   bool _isVerbose;

   void                    init();
   void                    clearData();
   void                    setFields( vector< MdvField* > &fields );
   void                    checkGeometry( const Grid &g );
};

#endif
