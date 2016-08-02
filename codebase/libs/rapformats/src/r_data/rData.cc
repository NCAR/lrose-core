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
//
// $Id: rData.cc,v 1.13 2016/03/03 18:45:40 dixon Exp $
//


#include <math.h>
#include <rapformats/DsRadarMsg.hh>
#include <rapformats/DsRadarParams.hh>
#include <rapformats/DsRadarBeam.hh>
#include <rapformats/DsFieldParams.hh>
#include <rapformats/rData.hh>
#include <toolsa/toolsa_macros.h>
#include <toolsa/mem.h>
#include <toolsa/udatetime.h>
#include <toolsa/str.h>
using namespace std;

rData::rData() 
{
   rayBuf         = NULL;
   rayHdr         = NULL;
   radarHdr       = NULL;
   fieldHdrPtr    = NULL;
   dataPtr        = NULL;
   gateData       = NULL;
   rayLength      = 0;
   nFields        = 0;
   nGates         = 0;
   nFieldsAlloc   = 0;
   nBytesAlloc    = 0;
   fieldHdrLen    = 0;
   fieldDataLen   = 0;
   fieldTotalLen  = 0;
   radarHdrOffset = 0;
   fieldHdrOffset = 0;
   
}

rData::~rData() 
{
  if( rayBuf )
     ufree(rayBuf);
  if( fieldHdrPtr )
     ufree(fieldHdrPtr);
  if( dataPtr )
     ufree(dataPtr);
  if( gateData )
     ufree(gateData);
  
}

void
rData::setRay(DsRadarMsg& dsrMsg, int nGatesCopy)
{
   const DsRadarParams            *radarParams = &(dsrMsg.getRadarParams());
   const DsRadarBeam              *radarBeam   = &(dsrMsg.getRadarBeam());
   const vector< DsFieldParams* > *fieldParams = &(dsrMsg.getFieldParams());

    ui08 *source, *dest;

    int nFieldsDs;
    int ifield, igate;

    //
    // Get number of fields
    //
    nFieldsDs = radarParams->numFields;
    nFields = MIN(nFieldsDs, MAX_N_FLDS);

    //
    // Get the number of gates 
    //
    nGates = MIN(radarParams->numGates, nGatesCopy);
    
    //
    // Compute field and data lengths - data length must be a multiple
    // of four to prevent alignment problems
    //
    fieldHdrLen = sizeof(Field_header);
    fieldDataLen = ((nGates + 3) / 4) * 4;
    fieldTotalLen = fieldHdrLen + fieldDataLen;
    
    //
    // Compute the total ray length
    //
    rayLength = (sizeof(Ray_header) + sizeof(Radar_header) +
	         nFields * fieldTotalLen);
    
    //
    // Allocate and initialize ray buffer and arrays of pointers 
    // into that buffer
    //
    allocRay();

    //
    // Set offsets
    //
    radarHdrOffset = sizeof(Ray_header);
    fieldHdrOffset = radarHdrOffset + sizeof(Radar_header);

    //
    // Set pointers into ray buffer
    //
    rayHdr   = (Ray_header *) rayBuf;
    radarHdr = (Radar_header *) (rayBuf + radarHdrOffset);

    for (ifield = 0; ifield < nFields;
	 ifield++, fieldHdrOffset += fieldTotalLen) {

      fieldHdrPtr[ifield] = (Field_header *) (rayBuf + fieldHdrOffset);
      dataPtr[ifield] = (ui08 *) fieldHdrPtr[ifield] + sizeof(Field_header);
      rayHdr->f_pt[ifield] = fieldHdrOffset;

    } 

    // 
    // Set headers
    //
    setHdrs(*radarParams, *radarBeam, *fieldParams, radarHdrOffset);

    //
    // Translate the data from gate by gate format
    // to field by field format
    //
    for (ifield = 0; ifield < nFields; ifield++) {
      source = radarBeam->data() + ifield;
      dest = dataPtr[ifield];

      for (igate = 0; igate < nGates; igate++) {
         *dest = *source;
         dest++;
         source += nFieldsDs;
      }
   } 
    
}

ui08 * 
rData::getGateData() 
{  
   int ifield, igate;
   ui08 *source, *dest;

   gateData = (ui08 *) umalloc((ui32) rayLength);

   for( ifield = 0; ifield < nFields; ifield++ ) {
      source = dataPtr[ifield];
      dest = gateData + ifield;
      
      for( igate = 0; igate < nGates; igate++ ) {
	 *dest = *source;
	 dest += nFields;
         source++;
      }
   }

   return(gateData);
   
}

void 
rData::printRay() const
{
    date_time_t    dataTime;
    Field_header  *fHdrPtr = fieldHdrPtr[0];

    fprintf(stderr, HDR);
    fprintf(stderr, "\n");             
 
    //
    // Parse the time of the beam
    //
    dataTime.unix_time = rayHdr->time;
    uconvert_from_utime( &dataTime );

    fprintf(stderr, FMT,
            (long) rayHdr->v_cnt,
            (long) rayHdr->f_cnt,                                       
            (double) rayHdr->t_ele / 100.0,
            (double) rayHdr->ele / 100.0,
            (double) (ui16) rayHdr->azi / 100.0,
            (long) fHdrPtr->n_gates,
            (long) fHdrPtr->g_size / 16,
            (long) rayHdr->prf,     
            (long) dataTime.year,
            (long) dataTime.month,              
            (long) dataTime.day,           
            (long) dataTime.hour,
            (long) dataTime.min,
            (long) dataTime.sec);
    fprintf(stderr, "\n");
         
} 


void 
rData::setHdrs(const DsRadarParams& radarParams, const DsRadarBeam& radarBeam,
               const vector< DsFieldParams* >& fieldParams,
               int radarHdrOffset)
{
    int  radarId = (radarParams.radarId & 0x7);

    DsFieldParams*  fparams;   
  
    //
    // Set radar header
    //
    STRncopy(radarHdr->radar_name, radarParams.radarName.c_str(), 32);

    radarHdr->latitude    = (long)radarParams.latitude * 100000;
    radarHdr->longitude   = (long)radarParams.longitude * 100000;
    radarHdr->power       = 0;
    radarHdr->altitude    = (long)radarParams.altitude * 1000;
    radarHdr->pulse_width = (long)radarParams.pulseWidth * 1000;
    radarHdr->beamwidth   = (unsigned short)(0.5*(radarParams.horizBeamWidth +
			         radarParams.vertBeamWidth) * 100);
    
    //
    // Set ray header
    //
    rayHdr->length   = rayLength;
    rayHdr->freq     = (unsigned short)(30000 / radarParams.wavelength);
    rayHdr->prf      = (short)radarParams.pulseRepFreq;
    rayHdr->polar    = 0;
    rayHdr->bad_data = 0;
    rayHdr->time     = radarBeam.dataTime;
    rayHdr->r_id     = radarId | radarParams.scanMode;
    rayHdr->f_cnt    = radarBeam.tiltNum;
    rayHdr->v_cnt    = radarBeam.volumeNum;
    rayHdr->id       = 30303;
    rayHdr->azi      = (ui16) floor(radarBeam.azimuth * 100.0 + 0.5);
    rayHdr->t_ele    = (ui16) floor(radarBeam.targetElev * 100.0 + 0.5);
    rayHdr->ele      = (ui16) floor(radarBeam.elevation * 100.0 + 0.5);
    rayHdr->n_fields = nFields;
    rayHdr->r_h_pt   = radarHdrOffset;
    
    //
    // Set field parameters
    //
    for( int ifield=0; 
	 ifield < nFields && ifield < (int) fieldParams.size();
         ifield++ ) {

      fparams = fieldParams[ifield];
      
      STRncopy(fieldHdrPtr[ifield]->f_name, fparams->name.c_str(), 6);
      STRncopy(fieldHdrPtr[ifield]->f_unit, fparams->units.c_str(), 6);

      fieldHdrPtr[ifield]->data_size = 8;

      fieldHdrPtr[ifield]->scale =
	(si16) floor(fparams->scale * 100.0 + 0.5);

      fieldHdrPtr[ifield]->offset =
	(si16) floor(fparams->bias * 100.0 + 0.5);
      
      fieldHdrPtr[ifield]->range =
	(ui16) (radarParams.startRange * 1000.0 + 0.5);

      fieldHdrPtr[ifield]->g_size =
	(ui16) ((radarParams.gateSpacing * 1000.0) * 16.0 + 0.5);

      fieldHdrPtr[ifield]->n_gates = nGates;
      
    } 
}

void
rData::allocRay()
{
   if (rayLength > nBytesAlloc) {
      if (nBytesAlloc == 0) {
	 rayBuf = (char *) umalloc ((ui32) rayLength);
         nBytesAlloc = rayLength;
      } else {
	 rayBuf = (char *) urealloc (rayBuf, (ui32) rayLength);
	 nBytesAlloc = rayLength;
      }
   }

   memset((void *) rayBuf, 0, (int) rayLength);
   
   if (nFields > nFieldsAlloc) {
      if (nFieldsAlloc == 0) {
	 fieldHdrPtr = (Field_header **) 
	    umalloc ((ui32) (nFields * sizeof(Field_header *)));
	 dataPtr = (ui08 **) umalloc ((ui32) (nFields * sizeof(ui08 *)));
         nFieldsAlloc = nFields;
      } else {
	 fieldHdrPtr = (Field_header **)
            urealloc ((char *) fieldHdrPtr, 
                      (ui32) (nFields * sizeof(Field_header *)));
	 dataPtr = (ui08 **) urealloc ((char *) dataPtr, 
                                      (ui32) (nFields * sizeof(ui08 *)));
         nFieldsAlloc = nFields;
      }
   } 

}


   
