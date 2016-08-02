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
///////////////////////////////////////////////////////
// PtrMgr - class that handles the memory managment
//          associated with data that is to be written
//          out to the netcdf files
//
// Jaimi Yee, RAP, NCAR, P.O.Box 3000, Boulder, CO, 
//   80307-3000, USA
//
// $Id: PtrMgr.cc,v 1.4 2016/03/07 01:23:10 dixon Exp $
//
////////////////////////////////////////////////////////
#include "PtrMgr.hh"

PtrMgr::PtrMgr() 
{
   dataCopied = false;
}

PtrMgr::~PtrMgr() 
{
   if( dataCopied ) {
      delete [] dz;
      delete [] ve;
      delete [] sw;
      delete [] snr;
      delete [] pr;
   }
}

void PtrMgr::setDataPtrs( int maxCells, int numCells, int numRays,
                          SweepData* currentSweep ) 
{
   if( !currentSweep ) 
      return;
   
   if( numCells < maxCells ) {

      //
      // Create new data arrays
      //
      dz  = new short[numCells * numRays];
      ve  = new short[numCells * numRays];
      sw  = new short[numCells * numRays];
      snr = new short[numCells * numRays];
      pr  = new short[numCells * numRays];
      
      //
      // Get pointers to the data we got from
      // the nexrad messages
      //
      short *oldDz  = currentSweep->getDz();
      short *oldVe  = currentSweep->getVe();
      short *oldSw  = currentSweep->getSw();
      short *oldSnr = currentSweep->getSnr();
      short *oldPr  = currentSweep->getPr();
      
      //
      // Copy the appropriate data in to the new, shorter
      // data arrays
      //
      for( int i = 0; i < numRays; i++ ) {
         short *dzPtr    = dz + i * numCells;
         short *oldDzPtr = oldDz + i * maxCells;
         
         memcpy( (void *) dzPtr, (void *) oldDzPtr, 
                  sizeof( short ) * numCells );
         
         short *vePtr    = ve + i * numCells;
         short *oldVePtr = oldVe + i * maxCells;
         
         memcpy( (void *) vePtr, (void *) oldVePtr, 
                 sizeof( short ) * numCells );

         short *swPtr    = sw + i * numCells;
         short *oldSwPtr = oldSw + i * maxCells;
         
         memcpy( (void *) swPtr, (void *) oldSwPtr, 
                 sizeof( short ) * numCells );
         
         short *snrPtr    = snr + i * numCells;
         short *oldSnrPtr = oldSnr + i * maxCells;
         
         memcpy( (void *) snrPtr, (void *) oldSnrPtr, 
                 sizeof( short ) * numCells );

         short *prPtr    = pr + i * numCells;
         short *oldPrPtr = oldPr + i * maxCells;
         
         memcpy( (void *) prPtr, (void *) oldPrPtr, 
                 sizeof( short ) * numCells );
      }

      dataCopied = true;
      
   }
   else {
      
      dz  = currentSweep->getDz();
      ve  = currentSweep->getVe();
      sw  = currentSweep->getSw();
      snr = currentSweep->getSnr();
      pr  = currentSweep->getPr();
   }
   
}

