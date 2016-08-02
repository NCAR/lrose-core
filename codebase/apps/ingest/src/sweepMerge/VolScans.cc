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
//////////////////////////////////////////////////////////
// VolScans - container class for dbz and vel files that
//            that need to be combined
//
// $Id: VolScans.cc,v 1.6 2016/03/07 01:23:11 dixon Exp $
//
//////////////////////////////////////////////////////////
#include <netcdf.hh>
#include "VolScans.hh"
#include "SweepMerge.hh"
#include "DbzFile.hh"
#include "VelFile.hh"
#include "FileInfo.hh"

VolScans::VolScans( int vNum, string* azName, string* elevName, string* tmName,
                    string* reflName )
{
   azimuthName   = azName;
   elevationName = elevName;
   timeName      = tmName;
   dbzName       = reflName;
   volNum        = vNum;
}

VolScans::~VolScans() 
{
   map< float, DbzFile*, less< float > >::iterator dbzIt;
   for( dbzIt = dbzFiles.begin(); dbzIt != dbzFiles.end(); dbzIt++ ) {
      delete( (*dbzIt).second );
   }
   dbzFiles.erase( dbzFiles.begin(), dbzFiles.end() );

   map< float, VelFile*, less< float > >::iterator velIt;
   for( velIt = velFiles.begin(); velIt != velFiles.end(); velIt++ ) {
      delete( (*velIt).second );
   }
   velFiles.erase( velFiles.begin(), velFiles.end() );
}

int VolScans::addDbzFile( FileInfo& fileInfo, double azTol ) 
{
   float elevation = fileInfo.getElevation();
   
   map< float, DbzFile*, less< float > >::iterator it = 
      dbzFiles.find( elevation );
   
   if( it != dbzFiles.end() ) {
      POSTMSG( ERROR, "There is already a long range scan for volume "
               "%d and elevation %f", volNum, elevation );
      return( FAILURE );
   }
   
   DbzFile *newFile = new DbzFile( fileInfo, azTol );
   
   dbzFiles[elevation] = newFile;

   POSTMSG( DEBUG, "Added dbz file %s", fileInfo.getFilePath()->c_str() );
   
   return( SUCCESS );
}

int VolScans::addVelFile( FileInfo& fileInfo, double elevTol, double timeTol ) 
{
   float elevation = fileInfo.getElevation();
   
   map< float, VelFile*, less<float> >::iterator it = 
      velFiles.find( elevation );
   
   if( it != velFiles.end() ) {
      POSTMSG( ERROR, "There is already a short range scan for volume "
               "%d and elevation %f", volNum, elevation );
      return( FAILURE );
   }
   
   VelFile *newFile = new VelFile( fileInfo, elevTol, timeTol );
   
   velFiles[elevation] = newFile;

   POSTMSG( DEBUG, "Added vel file %s", fileInfo.getFilePath()->c_str() );
   
   return( SUCCESS );
}

int VolScans::createNewScan() 
{
   POSTMSG( DEBUG, "Creating new scans for vol = %d\n", volNum );
   
   map< float, VelFile*, less< float > >::iterator velIt;
   
   for( velIt = velFiles.begin(); velIt != velFiles.end(); velIt++ ) {

      float    elevation      = (*velIt).first;
      VelFile *currentVelFile = (*velIt).second;

      map< float, DbzFile*, less< float > >::iterator dbzIt = 
         dbzFiles.find( elevation );
   
      if( dbzIt == dbzFiles.end() ) {
         POSTMSG( DEBUG, "No matching dbz file - skipping file %s", 
                  currentVelFile->getPath()->c_str() );
         continue;
      }

      DbzFile *currentDbzFile = (*dbzIt).second;

      if( currentVelFile->init( azimuthName, elevationName, 
                                timeName, dbzName ) != SUCCESS ) {
         return( FAILURE );
      }
   
      if( currentDbzFile->init( azimuthName, elevationName,
                                timeName, dbzName ) != SUCCESS ) {
         return( FAILURE );
      }
      
      if( currentVelFile->createNewScan( *currentDbzFile ) != SUCCESS ) {
         return( FAILURE );
      }

      if( currentVelFile->write() != SUCCESS ) {
         return( FAILURE );
      }
      
      if( currentDbzFile->moveFile() != SUCCESS ) {
         return( FAILURE );
      }
      
   }

   return( SUCCESS );
}





