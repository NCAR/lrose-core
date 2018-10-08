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
/////////////////////////////////////////////////////////////////////////
//  ReadNexrad handles reading the actual nexrad message.
//
//  Once parsed and/or converted to a format we need the data
//  is then passed off to the 
//
//  $Id: ReadNexrad.cc,v 1.18 2018/02/07 23:41:12 jcraig Exp $
//
////////////////////////////////////////////////////////////////////////

#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <bzlib.h>
#include <dataport/bigend.h>
#include <rapformats/swap.h>
#include <toolsa/MemBuf.hh>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <toolsa/utim.h>

#include "ReadNexrad.hh"
using namespace std;


ReadNexrad::ReadNexrad(Params *P)
{
  params = P;
  ingester = new Ingester(P);
}

ReadNexrad::~ReadNexrad()
{
  delete ingester;
}

status_t ReadNexrad::endOfData()
{
  return (ingester->endOfData());
}

int ReadNexrad::readBuffer(ui08 *buffer, size_t physicalBytes, bool volTitleSeen )
{
  volumeTitleSeen = volTitleSeen;
  int msgCount   = 0;
  size_t byteOffset = 0;
  size_t bytesLeft = physicalBytes;
  ui08   *messagePtr;
  
  while(bytesLeft > 0 && bytesLeft <= physicalBytes) {
    
    messagePtr = buffer + byteOffset;
    
    RIDDS_msg_hdr *msgHdr = (RIDDS_msg_hdr*) ( messagePtr + sizeof(RIDDS_ctm_info) );
    BE_to_RIDDS_msg_hdr(msgHdr);
    
    if(params->verbose)
      POSTMSG( DEBUG, "Msg: %d, type: %d, size: %d, numSegs: %d, segNum: %d",
	       msgHdr->seq_num, msgHdr->message_type, msgHdr->message_len,
	       msgHdr->num_message_segs, msgHdr->message_seg_num);
    // Check for message of all 0's.  Skip these empty messages.
    if(msgHdr->message_type > 31) {
      POSTMSG( ERROR, "Msg %d is unknown", msgHdr->message_type);
      return(BAD_DATA);
    }

    //
    // Check for message of all 0's.  Skip these empty messages.
    if(msgHdr->seq_num == 0 && msgHdr->message_len == 0 && msgHdr->message_type == 0) {
      POSTMSG( DEBUG, "Skipping over empty message.");
    } else {

      //
      // If we have only one segment to this message go ahead and process it
      if(msgHdr->num_message_segs == 1) {
	readMsg((ui08*)msgHdr, msgHdr->message_len);
	
      //
      // If we have multiple segments to this message read them all in and
      // add them to a single membuff to process.
      // The 200 check is just to make sure we don't get some huge number
      // which may indicate we have a file that is not nexrad data.
      } else if(msgHdr->num_message_segs < 200 &&
		(msgHdr->message_seg_num == 0 || msgHdr->message_seg_num == 1)) {

	int num_message_segs = msgHdr->num_message_segs;
	if(num_message_segs == 0)  // Hack for build 10 messages
	  num_message_segs = 2;    // First message sometimes lists num segments as 0

	MemBuf *msgBuf = new MemBuf();
	
	msgBuf->load(msgHdr, msgHdr->message_len * 2);// halfwords * 2 = bytes
	
	for(int a = 1; a < num_message_segs; a++) {
	  
	  byteOffset   += NEX_PACKET_SIZE;
	  bytesLeft    -= NEX_PACKET_SIZE;
	  msgCount ++;
	  
	  messagePtr = buffer + byteOffset;
	  
	  RIDDS_msg_hdr *msgHdr2 = (RIDDS_msg_hdr*) ( messagePtr + sizeof(RIDDS_ctm_info) );
	  BE_to_RIDDS_msg_hdr(msgHdr2);
	  
	  if(params->verbose)
	    POSTMSG( DEBUG, "Msg: %d, type: %d, size: %d, numSegs: %d, segNum: %d",
		     msgHdr2->seq_num, msgHdr2->message_type, msgHdr2->message_len,
		     msgHdr2->num_message_segs, msgHdr2->message_seg_num);

	  if(num_message_segs == 2)   // Fix the hack here with the proper num seqments
	    num_message_segs = msgHdr2->num_message_segs;

	  //
	  // Don't add the message header just the data to the membuff
	  msgBuf->add((ui08 *)msgHdr2 + sizeof(RIDDS_msg_hdr),
			  (msgHdr2->message_len*2) - sizeof(RIDDS_msg_hdr));
	  
	  //
	  // This break is needed if we were to remove the check for
	  // seq_num > 0 above. Then messages dont always start with
	  // segment number 1.
	  if(msgHdr2->message_seg_num == msgHdr->num_message_segs)
	    break;
	  
	}
	readMsg((ui08 *)msgBuf->getBufPtr(), msgBuf->getBufLen());
	
	delete msgBuf;
      } 
      
    }

    //
    // Move to the next message
    if(msgHdr->message_type == 31) {
      byteOffset   += (msgHdr->message_len * 2) - 4 + sizeof(RIDDS_msg_hdr);
      bytesLeft    -= (msgHdr->message_len * 2) - 4 + sizeof(RIDDS_msg_hdr);
    } else {
      byteOffset   += NEX_PACKET_SIZE;
      bytesLeft    -= NEX_PACKET_SIZE;
    }
    msgCount ++;
  }
 
  return msgCount;
}

status_t ReadNexrad::readMsg(ui08 *buffer, size_t numberOfBytes)
{
  status_t stat;
  RIDDS_msg_hdr *msgHdr  = (RIDDS_msg_hdr*)buffer;
  
  if ( msgHdr->message_type == DIGITAL_RADAR_DATA ) {

    RIDDS_data_hdr *nexradData = (RIDDS_data_hdr*)(buffer + sizeof(RIDDS_msg_hdr));
    BE_to_RIDDS_data_hdr( nexradData );
    
    stat = ingester->addMsg1Beam(nexradData, volumeTitleSeen);

    return( stat );

  } else if ( msgHdr->message_type == DIGITAL_RADAR_DATA_31 ) {

    RIDDS_data_31_hdr *nexradData = (RIDDS_data_31_hdr *)(buffer + sizeof(RIDDS_msg_hdr));
    if(nexradData->volume_ptr != 68)
      BE_to_RIDDS_data_31_hdr(nexradData);

    if(nexradData->data_blocks > 9 || nexradData->volume_ptr > 68) {
      POSTMSG( ERROR, "Msg 31 Data Header read problem." );
      return(BAD_DATA);
    }

    RIDDS_volume_31_hdr *volData = (RIDDS_volume_31_hdr *)((ui08*)nexradData + nexradData->volume_ptr);
    BE_to_RIDDS_volume_31_hdr(volData);

    RIDDS_elev_31_hdr *elevData = (RIDDS_elev_31_hdr *)((ui08*)nexradData + nexradData->elevation_ptr);
    BE_to_RIDDS_elev_31_hdr(elevData);

    RIDDS_radial_31_hdr *radialData = (RIDDS_radial_31_hdr *)((ui08*)nexradData + nexradData->radial_ptr);
    BE_to_RIDDS_radial_31_hdr(radialData);

    if(volData->block_type != 'R' || elevData->block_type != 'R' || radialData->block_type != 'R')
    {
      POSTMSG( ERROR, "Msg 31 Data Header read problem." );
      return(BAD_DATA);
    }

    ui32 blockPtrs[6];
    blockPtrs[0] = nexradData->ref_ptr;
    blockPtrs[1] = nexradData->vel_ptr;
    blockPtrs[2] = nexradData->sw_ptr;
    blockPtrs[3] = nexradData->zdr_ptr;
    blockPtrs[4] = nexradData->phi_ptr;
    blockPtrs[5] = nexradData->rho_ptr;
    nexradData->ref_ptr = 0;
    nexradData->vel_ptr = 0;
    nexradData->sw_ptr = 0;
    nexradData->zdr_ptr = 0;
    nexradData->phi_ptr = 0;
    nexradData->rho_ptr = 0;
    for(int block = 0; block < nexradData->data_blocks-3; block++) 
    {
      RIDDS_data_31 *blockPtr = (RIDDS_data_31 *)((ui08*)nexradData + blockPtrs[block]);
      BE_to_RIDDS_data_31(blockPtr);
      if(blockPtr->block_name[0] == 'R' && blockPtr->block_name[1] == 'E')
	nexradData->ref_ptr = blockPtrs[block];
      if(blockPtr->block_name[0] == 'V' && blockPtr->block_name[1] == 'E')
	nexradData->vel_ptr = blockPtrs[block];
      if(blockPtr->block_name[0] == 'S' && blockPtr->block_name[1] == 'W')
	nexradData->sw_ptr = blockPtrs[block];
      if(blockPtr->block_name[0] == 'Z' && blockPtr->block_name[1] == 'D')
	nexradData->zdr_ptr = blockPtrs[block];
      if(blockPtr->block_name[0] == 'P' && blockPtr->block_name[1] == 'H')
	nexradData->phi_ptr = blockPtrs[block];
      if(blockPtr->block_name[0] == 'R' && blockPtr->block_name[1] == 'H')
	nexradData->rho_ptr = blockPtrs[block];
    }

    if(params->verbose) {
      POSTMSG( DEBUG, "Msg 31, elevation: %f, azimuth %f, reflData: %d, velData: %d, swData: %d, zdrData: %d, phiData: %d, rhoData: %d" ,
	       nexradData->elevation, nexradData->azimuth, nexradData->ref_ptr, nexradData->vel_ptr, 
	       nexradData->sw_ptr, nexradData->zdr_ptr, nexradData->phi_ptr, nexradData->rho_ptr);
    }
    stat = ingester->addMsg31Beam(nexradData, volumeTitleSeen);

    return( stat );

  } else if( msgHdr->message_type == RDA_STATUS_DATA ) {

    RIDDS_status_data *statusData = new RIDDS_status_data();
    memcpy(statusData, buffer + sizeof(RIDDS_msg_hdr), sizeof(RIDDS_status_data));
    BE_to_RIDDS_status_data(statusData);
    
    ingester->addStatus(statusData);

    return( ALL_OK );

  } else if( msgHdr->message_type == VOLUME_COVERAGE_PATTERN || msgHdr->message_type == VOLUME_COVERAGE_PATTERN2 ) {

    VCP_data_t *vcpData;
    stat = readVcpData(buffer + sizeof(RIDDS_msg_hdr), numberOfBytes - sizeof(RIDDS_msg_hdr), 
		       &vcpData);

    ingester->addVCP(vcpData);

    return( stat );

  } else if( msgHdr->message_type == RDA_ADAPTATION_DATA ) {
    
    RIDDS_adaptation_data *adaptData = new RIDDS_adaptation_data();
    memcpy(adaptData, buffer + sizeof(RIDDS_msg_hdr), sizeof(RIDDS_adaptation_data));
    BE_to_RIDDS_adaptation_data(adaptData);
    
    POSTMSG( DEBUG, "Adaptation Data: seg1lim: %f, seg2lim: %f, seg3lim: %f, seg4lim: %f" ,
	     adaptData->seg1lim, adaptData->seg2lim, adaptData->seg3lim, adaptData->seg4lim);

    ingester->addAdaptation(adaptData);

    return( ALL_OK );
    
  } else if( msgHdr->message_type == CLUTTER_FILTER_BYPASS_MAP ) {
    
    BypassMap_t *bypassMap;
    stat = readBypassMap(buffer + sizeof(RIDDS_msg_hdr), numberOfBytes - sizeof(RIDDS_msg_hdr),
			 &bypassMap);
    
    if(stat == ALL_OK) {
      ingester->addBypassMap(bypassMap);
    }
    
    return( stat );
    
  } else if( msgHdr->message_type == CLUTTER_FILTER_MAP ) {

    ClutterMap_t *clutterMap;
    stat = readClutterMap(buffer + sizeof(RIDDS_msg_hdr), numberOfBytes - sizeof(RIDDS_msg_hdr),
			  &clutterMap);

    if(stat == ALL_OK) {
      ingester->addClutterMap(clutterMap);
    }
    
    return( stat );
  } 

  POSTMSG( DEBUG, "Skipping over message type: %d", msgHdr->message_type );
  return( BAD_DATA );
  
}

status_t ReadNexrad::readVcpData(ui08 *buffer, int bufferSize, VCP_data_t **vcp_data)
{

  RIDDS_VCP_hdr *vcpHdr = (RIDDS_VCP_hdr *) buffer;
  BE_to_RIDDS_vcp_hdr(vcpHdr);

  VCP_data_t *vcpData = new VCP_data_t();

  vcpData->hdr.message_len = vcpHdr->message_len;
  vcpData->hdr.pattern_type = vcpHdr->pattern_type;
  vcpData->hdr.pattern_number = vcpHdr->pattern_number;
  vcpData->hdr.num_elevation_cuts = vcpHdr->num_elevation_cuts;
  vcpData->hdr.clutter_map_group = vcpHdr->clutter_map_group;
  vcpData->hdr.dop_vel_resolution = vcpHdr->dop_vel_resolution;
  vcpData->hdr.pulse_width = vcpHdr->pulse_width;

  POSTMSG( DEBUG, "**VCP Info**\nVCP #: %d, Num Elevation Cuts: %d, Doppler Vel Resolution: %0.2f, Pluse Width %0.2f", 
	   vcpData->hdr.pattern_number, vcpData->hdr.num_elevation_cuts, vcpData->hdr.dop_vel_resolution/4.0, 
	   vcpData->hdr.pulse_width/2.0);

  for(int a = 0; a < vcpHdr->num_elevation_cuts; a++)
  {
    RIDDS_elevation_angle *elevAngle = (RIDDS_elevation_angle *)(buffer + sizeof(RIDDS_VCP_hdr) +
								 (sizeof(RIDDS_elevation_angle) * a));
    BE_to_RIDDS_elevation_angle(elevAngle);
    vcpData->angle[a] = *elevAngle;

    POSTMSG( DEBUG, " Elevation: %f, Channel Config: %d, Waveform Type: %d, Super Res Control: %d, Elev #%d", 
	     (elevAngle->elevation_angle/8.)*(180./4096.), elevAngle->channel_config,
	     elevAngle->waveform_type, elevAngle->super_res_control, a+1);
    POSTMSG( DEBUG, "  Azimuth Rate: %f, Surv PRF #: %d, Surv PRF pulse/radial: %d", 
	     (elevAngle->azimuth_rate/8.)*(22.5/2048), elevAngle->surveillance_prf_num,
	     elevAngle->surveillance_prf_pulse_count);
    POSTMSG( DEBUG, "                           Dopp PRF #: %d, Dopp PRF pulse/radial: %d", 
	     elevAngle->doppler_prf_number1,
	     elevAngle->doppler_prf_pulse_count1);
  }

  *vcp_data = vcpData;
  return( ALL_OK );
}

status_t ReadNexrad::readBypassMap(ui08 *buffer, int bufferSize, BypassMap_t **bypass_Map)
{
  RIDDS_clutter_hdr *bypassHdr = (RIDDS_clutter_hdr*) buffer;
  BE_to_RIDDS_clutter_hdr( bypassHdr );

  POSTMSG( DEBUG, "Bypass Date: %d, Min: %d, numSegs: %d", bypassHdr->julian_date, 
	   bypassHdr->minutes_past_midnight, bypassHdr->num_message_segs);
  
  if(bypassHdr->num_message_segs == 0 || bypassHdr->num_message_segs > 5) {
    POSTMSG( ERROR, "Invalid number of Segments in bypass map");
    return( BAD_DATA );
  }
  
  BypassMap_t *bypassMap = new BypassMap_t();
  bypassMap->julian_date = bypassHdr->julian_date;
  bypassMap->minutes_past_midnight = bypassHdr->minutes_past_midnight;
  bypassMap->num_message_segs = bypassHdr->num_message_segs;
  bypassMap->segment[0] = NULL;
  bypassMap->segment[1] = NULL;
  bypassMap->segment[2] = NULL;
  bypassMap->segment[3] = NULL;
  bypassMap->segment[4] = NULL;

  ui08 *clutterPos = buffer + sizeof(RIDDS_clutter_hdr);  
  int length = (sizeof(RIDDS_msg_hdr) + sizeof(RIDDS_clutter_hdr)) / 2;
  ui16 *segment_num, *op_code;

  for(int a = 0; a < bypassHdr->num_message_segs; a++) {

    segment_num = (ui16 *)clutterPos;
    BE_to_array_16(segment_num, 2);

#ifdef DEBUG_CLUTTER
    POSTMSG( DEBUG, "Segment: %d", *segment_num);
#endif

    clutterPos += 2;
    length ++;
    if(*segment_num != a+1) {
      POSTMSG( ERROR, "Invalid segments number in bypass map");
      return( BAD_DATA );
    }
    bypassMap->segment[a] = new BypassSegment_t();

    for(int b = 0; b < 360; b++) {
      
      for(int c = 0; c < 32; c++) {
	op_code = (ui16 *) clutterPos;
	BE_to_array_16(op_code, 2);

#ifdef DEBUG_CLUTTER
	POSTMSG( DEBUG, "Radial: %d, Range: %d, Code: %d", b, c, *op_code);
#endif

	bypassMap->segment[a]->ranges[(b*32)+c] = *op_code;

	clutterPos += 2;
	length ++;
      }
    }
  }
  *bypass_Map = bypassMap;
  return( ALL_OK );
}

status_t ReadNexrad::readClutterMap(ui08 *buffer, int bufferSize, ClutterMap_t **clutter_Map)
{

  RIDDS_clutter_hdr *clutterHdr =  (RIDDS_clutter_hdr*) buffer;
  BE_to_RIDDS_clutter_hdr( clutterHdr );

  POSTMSG( DEBUG, "Clutter Date: %d, Min: %d, numSegs: %d", clutterHdr->julian_date, 
	   clutterHdr->minutes_past_midnight, clutterHdr->num_message_segs);
  
  if(clutterHdr->num_message_segs == 0 || clutterHdr->num_message_segs > 5) {
    POSTMSG( ERROR, "Invalid number of Segments in clutter map");
    return( BAD_DATA );
  }
  
  ClutterMap_t *clutterMap = new ClutterMap_t();
  clutterMap->julian_date = clutterHdr->julian_date;
  clutterMap->minutes_past_midnight = clutterHdr->minutes_past_midnight;
  clutterMap->num_message_segs = clutterHdr->num_message_segs;
  clutterMap->segment[0] = NULL;
  clutterMap->segment[1] = NULL;
  clutterMap->segment[2] = NULL;
  clutterMap->segment[3] = NULL;
  clutterMap->segment[4] = NULL;


  ui08 *clutterPos = buffer + sizeof(RIDDS_clutter_hdr);  
  int length = (sizeof(RIDDS_msg_hdr) + sizeof(RIDDS_clutter_hdr)) / 2;
  ui16 *num_range_zones, *op_code, *end_range;

  for(int a = 0; a < clutterHdr->num_message_segs; a++) {
    
    clutterMap->segment[a] = new ClutterSegment_t();

    for(int b = 0; b < 360; b++) {
      num_range_zones = (ui16 *)clutterPos;
      BE_to_array_16(num_range_zones, 2);
#ifdef DEBUG_CLUTTER
      POSTMSG( DEBUG, "Segment: %d, Azimuth: %d, numRange: %d", a, b, *num_range_zones);
#endif

      if(*num_range_zones == 0 || *num_range_zones > 20) {
	POSTMSG( ERROR, "Invalid number of range zones in clutter map");
	return( BAD_DATA );
      }
      
      clutterPos += 2;
      length ++;
      for(int c = 0; c < *num_range_zones; c++) {
	op_code = (ui16 *) clutterPos;
	end_range = (ui16 *) (clutterPos + 2);	   
	BE_to_array_16(op_code, 2);
	BE_to_array_16(end_range, 2);

#ifdef DEBUG_CLUTTER
	POSTMSG( DEBUG, "Range: %d, Code: %d, EndRange: %d", c, *op_code, *end_range);
#endif

	clutterMap->segment[a]->ranges[(b*20)+c].op_code = *op_code;
	clutterMap->segment[a]->ranges[(b*20)+c].end_range = *end_range;

	clutterPos += 4;
	length += 2;
      }
      //
      // Fill the rest of the ranges array with zeros
      for(int c = *num_range_zones; c < 20; c++) {
	clutterMap->segment[a]->ranges[(b*20)+c].op_code = 0;
	clutterMap->segment[a]->ranges[(b*20)+c].end_range = 0;
      }

    }
  }
  *clutter_Map = clutterMap;
  return( ALL_OK );
}
