/*Class to create and write out mdv files
 */

#ifdef sgi
#include <sys/bsd_types.h>
#endif
#ifdef STDCPPHEADERS
#include <iostream>
#else
#include <iostream.h>
#endif

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "utils.h"
#include <libgen.h>
#include <unistd.h>
//#include "linkedlisttemplate.h"
#include <stdio.h>
//#include <baseData.h>
#include <values.h>
#include <siteinfo.h>
#include <rdrscan.h>
#include "nexrad_port_types.h"
#include "nexrad_structs.h"
#include "nexrad.h"
#include "spinlock.h"
#include "rdrfilter.h"

static char const cvsid[] = "$Id: NexRad.C,v 1.2 2008/02/29 04:44:54 dixon Exp $";

//default constructor
NexRad::NexRad()
{
}
NexRad::NexRad(rdr_scan *scan,
	       NexRadOutpDesc **outputdesc,
	       int outputcount,
	       int pattern,
	       int packet_size,
	       bool debug,
	       int VolumeNo,
	       int seq_num,
	       ui32 fr_seq,
	       int delay_pac, 
	       bool filter_Vel,
	       bool filter_Refl,
	       bool filter_SpWdth,
	       int refl_Rngres_reduce_factor,
	       int vel_Rngres_reduce_factor,
	       int mintimepertilt)
{
  //  int err,sent_no;
  makeSpecWidth = false;
  
  time_t scantime = 0;

  //  this->fd           = fd;

  this->scan         = scan;
  scan_vol_no        = 0;
  scan_tilt_no       = 0;
  volbegin           = 1;
  volend             = 0;
  tiltnumber         = 1;  //according to Terri B's email on 17 Aug 1999
  first_scan         = 1;
  last_scan_angle    = -1;
  this->outputdesc   = outputdesc;
  this->outputcount  = outputcount;
  this->debug        = debug;
  this->filter_Vel   = filter_Vel;
  this->filter_Refl  = filter_Refl;
  this->filter_SpWdth = filter_SpWdth;
  this->pattern      = pattern;
  this->packet_size  = packet_size;
  this->VolumeNo     = VolumeNo;
  this->seq_number   = seq_num;
  this->fr_seq       = fr_seq;
  this->delay_pac    = delay_pac;  //in millisec
  refl_rngres_reduce_factor = refl_Rngres_reduce_factor;
  vel_rngres_reduce_factor = vel_Rngres_reduce_factor;
  min_time_per_tilt  = mintimepertilt;	// default to at least 5 secs per tilt 
  velfilter = reflfilter = specwfilter = 0;
  next_tilt_time     = 0; 

  //send initial 201 packet
  if (scan)
    scantime = scan->FirstTm;
  send_201(scantime);

  //set up initial data
  //Note: should ensure BigEndianness of all this to future proof it
  //set up frame header for UDP transmission
  //memset(&message.ctm_info.word1,'\0',12);
  //zero data hdr initially
  memset(&message,0,sizeof(message_typ));
  message.ctm_info.mess_length  = htons(packet_size);
  //message.ctm_info.mess_length  = 132; 
  message.ctm_info.nframes      = htons(1);
  message.ctm_info.frame_num    = htons(1); //always one packet per message here, until SW
  message.ctm_info.frame_offset = htons(0); //not used?
  //message.ctm_info.data_len     = htons(packet_size -  //redo for data > DATASIZE!!
  //                         sizeof(NEXRAD_frame_hdr) - sizeof(NEXRAD_msg_hdr));
  //Alt'n for WDSS
  message.ctm_info.data_len     = htons(packet_size - sizeof(NEXRAD_frame_hdr));

  message.ctm_info.flags        = htons(0); //not used?
  message.msg_hdr.message_len   = htons((si16)0x4B8); //from doc. 
  
  message.msg_hdr.message_type  = htons((si16)1);
  message.msg_hdr.num_message_segs = htons((si16)1);//if our messages are small enough
  message.msg_hdr.message_seg_num  = htons((si16)1);
                                  
  if (scan->prf)
    message.data_hdr.unamb_range_x10 =  htons((si16)(SPD_OF_LITE/(200*scan->prf)));
                       //see NCAR ridds2mom:reformat2ds.cc:385 for justification
  message.data_hdr.sector_num      = htons(0);  //no idea
  message.data_hdr.sys_gain_cal_const   = htonl((si32)0);  //no idea
  message.data_hdr.atmos_atten_factor   = htons(0);  //no idea
  message.data_hdr.threshold_param      = htons((si16)0); //no idea

}


void NexRad::send_201(time_t scantime)
{
  //  si16 vol_scan_num;
  int local_msg_len;
  
  message_201 msg_201;
  //send NSSL 201 packet for the WDSS people
  time_t curr_time = time(0);
  if (scantime)
    curr_time = scantime;
  memset(&msg_201,0,sizeof(msg_201));
  msg_201.ctm_info.fr_seq       = htonl(fr_seq++);
  msg_201.ctm_info.mess_length  = htons((si16)sizeof(msg_201));//short int data
  local_msg_len                 = sizeof(msg_201);
    msg_201.ctm_info.nframes      = htons((si16)1);
  msg_201.ctm_info.frame_num    = htons((si16)1); //always one packet per msg_201 here, until SW
  msg_201.ctm_info.frame_offset = htons(0); //not used?
  //data is short int, 44 (was 28) for wdss  
  //msg_201.ctm_info.data_len     = (si16)44; 
  //data is short int, 60 (was 44) for WDSS, again! see email wrp:10 & 16
  msg_201.ctm_info.data_len     = htons((si16)(sizeof(message_201) - sizeof(NEXRAD_frame_hdr)));
  msg_201.ctm_info.flags        = htons(0); //not used?
  msg_201.msg_hdr.message_len   = htons((si16)(local_msg_len/2+8));
  
  msg_201.msg_hdr.message_type  = htons((si16)201);  //this is it!!
  if (++seq_number > 0x7fff) seq_number = 0;
  msg_201.msg_hdr.seq_num       = htons((si16)seq_number);
  
  msg_201.msg_hdr.julian_date = htons((si16)getJulian(curr_time));
  msg_201.msg_hdr.millisecs_past_midnight = 
	                        htonl((si32)getMillsecFromMidn(curr_time));
  msg_201.msg_hdr.num_message_segs = htons((si16)1);  //if our messages are small enough
  msg_201.msg_hdr.message_seg_num  = htons((si16)1);
  msg_201.vol_scan_no = htons((si16)VolumeNo);
  memcpy(&data,&msg_201,sizeof(msg_201));
  
  si32 temp_si32;
  char arch2header[24];
  memset(arch2header, 0, 24);
  strcpy(arch2header, "ARCHIVE2.000");
  temp_si32 = htonl((si32)getJulian(curr_time));
  memcpy(arch2header+12, &temp_si32, 4);
  temp_si32 = htonl((si32)getMillsecFromMidn(curr_time));
  memcpy(arch2header+16, &temp_si32, 4);

  //write packet
  for (int sock=0;sock<outputcount;sock++)
     if (outputdesc[sock]->fd > -1)
       {
	 if (outputdesc[sock]->archive2file)
	   outputdesc[sock]->write_beam(arch2header,sizeof(arch2header));
	 else
	   outputdesc[sock]->write_beam(data,sizeof(msg_201));
       }
//  if (debug) print_201(&msg_201);
  
}

//build dataset from scanset
int NexRad::checkWrite()
{
  //try to write as much of volume as poss, 
  //if not finished return NOTDONE (0 = false) else DONE
  rdr_scan *thisscan;
  // rdr_scan *lastscan = NULL;
  int i;
  
  //wait upon next tilt,  when complete write it out
  //build up array of scans with same elevation til new elevation comes in
  //last_angle = scan->set_angle;
  
  for (i = scan_vol_no;i<scan->vol_scans;i++)
    {
      //if scan not yet done, return not done
      if (!(thisscan = goto_scan(i))) 
	{
	  scan_vol_no = i;
	  return NOTDONE;
	}
      
      if (!first_scan && thisscan->set_angle != last_scan_angle) 
	{
	  //write out tilt
	  writeTilt();
	  tiltnumber++;
	  //after writing out contents of tiltarr, start it up with new scan
	  scan_tilt_no = 0;
	}
      tiltarr[scan_tilt_no] = i;
      scan_tilt_no++;
	  
      last_scan_angle = thisscan->set_angle;
      first_scan = 0;
    }
  //at end of volume, write out final tilt
  volend = 1;
  writeTilt();
  next_tilt_time = 0;
  tiltnumber++;
  return DONE;
}


NexRad::~NexRad()
{
  for (int i=0;i<outputcount;i++)
    {
      outputdesc[i]->close_fd();
    }
  if (velfilter)
  {
    delete velfilter;
    velfilter = 0;
  }
  if (reflfilter)
  {
    delete reflfilter;
    reflfilter = 0;
  }
  if (specwfilter)
  {
    delete specwfilter;
    specwfilter = 0;
}
}

rdr_scan* NexRad::goto_scan(int n)
{
  //goto the nth scan (ie, zeroth to num_scans-1th) in scan set starting at rootscan scan
  //return 0 if error or incomplete
  rdr_scan* tempscan;
  int index = 0;
  if (n >= scan->scanSetCount()) return 0;
  
  tempscan = scan->rootScan();
  //if (n == 0) return tempscan;
  while (tempscan && index < n){
    index++;
    tempscan = scan->NextScan(tempscan);
  }
  return tempscan;
}


void NexRad::writeTilt()
{
  //write out tilt array
  int local_data_hdr_ref_ptr = 0;
  int local_data_hdr_vel_ptr = 0;
  int local_data_hdr_sw_ptr  = 0;
  int local_data_hdr_ref_num_gates = 0;
  int local_data_hdr_vel_num_gates = 0;

  time_t curr_time;
  int i, data_ind, gate, az;
  //  int err, sent_no;
  int loc_ref_ptr = 0,loc_vel_ptr = 0,loc_sw_ptr = 0, hdr_ind;
short radlpos = 0;
  exp_buff_rd *refl_dbuffrd = new exp_buff_rd();
  exp_buff_rd *vel_dbuffrd = new exp_buff_rd();
  exp_buff_rd *spectw_dbuffrd = new exp_buff_rd();

  
  rdr_scan *velscan=0,*reflscan=0,*specwscan=0,*tempscan;
  s_radl reflradl,velradl,specwradl;
  data_ind = sizeof(message_typ);
  hdr_ind = sizeof(NEXRAD_data_hdr);
  
  bool delVelScan = false;
  bool delReflScan = false;
  bool delSpecWScan = false;
  
  
  next_tilt_time = time(0) + min_time_per_tilt;	// don't send another tilt until > next_tilt_time

  //vol_coverage_pattern keys into table in, ie, ridds2mom:nexrad_vcp.conf
  //I chose 41 and 42 at random.  there may be an official numbering scheme
  //somewhere.  SD 17/8/99.
  //if (scan->scan_type == VOL) 
  message.data_hdr.vol_coverage_pattern = htons((si16)pattern); 
  //if (scan->scan_type == CompPPI) 
  //  message.data_hdr.vol_coverage_pattern = htons((si16)145); 

  //get moments in tilt
  for (i=0;i<scan_tilt_no;i++)
    {
      //tempscan = scan->goto_scan(tiltarr[i]);  old
      tempscan = goto_scan(tiltarr[i]);
      if (!tempscan)
	{
	  fprintf(stderr,"NexRad::writeTilt: ERROR: tempscan not set. Exitting.\n");
// exiting is BAD	  exit(-1);
	    if (refl_dbuffrd) delete refl_dbuffrd;
	    if (vel_dbuffrd) delete vel_dbuffrd;
	    if (spectw_dbuffrd) delete spectw_dbuffrd;
	    return;
	}
      //SD add filter stuff below 30/8/00
      if (tempscan->data_type == e_vel)
	{      
	  velscan = tempscan;
	  if (filter_Vel)
	    {
	      if (!velfilter)
		velfilter = new rdrfilter;
	      velscan = velfilter->MakeFilteredScan(tempscan);
	      delVelScan = true;
	    }
	}
      
      if (tempscan->data_type == e_refl)
	{      
	  reflscan = tempscan;
	  if (filter_Refl)
	    {
	      if (!reflfilter)
		reflfilter = new rdrfilter;
	      reflscan = reflfilter->MakeFilteredScan(tempscan);
	      delReflScan = true;
	    }
	}
      if (tempscan->data_type == e_spectw)
	{
	  specwscan = tempscan;
	  if (filter_SpWdth)
	    {
	      if (!specwfilter)
		specwfilter = new rdrfilter;
	      specwscan = specwfilter->MakeFilteredScan(tempscan);
	      delSpecWScan = true;
	    }
	}
    }
  if (!reflscan)
    {
      fprintf(stderr,"NexRad::writeTilt: ERROR: No refl scan for this tilt. Exiting.\n");
//      exit(-1);
      if (refl_dbuffrd) delete refl_dbuffrd;
      if (vel_dbuffrd) delete vel_dbuffrd;
      if (spectw_dbuffrd) delete spectw_dbuffrd;
      
      if (delVelScan && velscan &&
	  velscan->ShouldDelete(velfilter, "NexRad::writeTilt")) 
	    delete velscan;
      if (delReflScan && reflscan &&
	reflscan->ShouldDelete(reflfilter, "NexRad::writeTilt")) 
	    delete reflscan;
      if (delSpecWScan && specwscan &&
	specwscan->ShouldDelete(specwfilter, "NexRad::writeTilt")) 
	    delete specwscan;
	return;
  }
  message.data_hdr.ref_ptr = 0;
  message.data_hdr.vel_ptr = 0;
  message.data_hdr.sw_ptr  = 0;

  if (reflscan) 
    {
      message.data_hdr.ref_num_gates   =
	htons((si16)(reflscan->max_rng*1000/(reflscan->rng_res*refl_rngres_reduce_factor)));
      local_data_hdr_ref_num_gates =
	int(reflscan->max_rng*1000/(reflscan->rng_res*refl_rngres_reduce_factor));
      message.data_hdr.ref_gate1       = htons((si16)reflscan->start_rng);
      message.data_hdr.ref_gate_width  = htons((si16)reflscan->rng_res*refl_rngres_reduce_factor);
      message.data_hdr.ref_ptr         = htons((si16)hdr_ind);
      local_data_hdr_ref_ptr           = hdr_ind;
      loc_ref_ptr                      = data_ind;
    }
  
  if (velscan)
    {
      //assume if vel then reflectivity is necessarily provided
      message.data_hdr.vel_num_gates   = 
	htons((si16)(velscan->max_rng*1000/(velscan->rng_res*vel_rngres_reduce_factor)));
      local_data_hdr_vel_num_gates   = 
	int(velscan->max_rng*1000/(velscan->rng_res*vel_rngres_reduce_factor));
      message.data_hdr.vel_gate1       = htons((si16)velscan->start_rng);
      message.data_hdr.vel_gate_width  = htons((si16)velscan->rng_res*vel_rngres_reduce_factor);
      message.data_hdr.velocity_resolution  = htons((si16)2); //ie, 0.5m/s 
      message.data_hdr.nyquist_vel     = htons((si16)(velscan->nyquist*100));
      //assume refl is always here
      message.data_hdr.vel_ptr         = htons((si16)(local_data_hdr_ref_ptr +
					      local_data_hdr_ref_num_gates));
      local_data_hdr_vel_ptr           = local_data_hdr_ref_ptr +
					      local_data_hdr_ref_num_gates;
      loc_vel_ptr                      = loc_ref_ptr + 
	                                      local_data_hdr_ref_num_gates;
      
    }
  
  if (specwscan && makeSpecWidth) 
    {
      //assume if specw then necessarily velocity & refl
      message.data_hdr.sw_ptr         = htons((si16)(local_data_hdr_ref_ptr +
					       local_data_hdr_ref_num_gates +
					       local_data_hdr_vel_num_gates));
      local_data_hdr_sw_ptr           = local_data_hdr_ref_ptr +
					       local_data_hdr_ref_num_gates +
					       local_data_hdr_vel_num_gates;
      loc_sw_ptr                      = loc_ref_ptr + 
	                                       local_data_hdr_ref_num_gates +
					       local_data_hdr_vel_num_gates;
    }
  if (loc_ref_ptr + local_data_hdr_ref_num_gates > packet_size  ||
      loc_vel_ptr + local_data_hdr_vel_num_gates > packet_size  ||
      loc_sw_ptr  + local_data_hdr_vel_num_gates > packet_size)
    {
      //Need to use multiple packets per message
      printf("\nNexRad::writeTilt: ERROR, message too long for packet\n\n");
//      exit(-1);
      if (refl_dbuffrd) delete refl_dbuffrd;
      if (vel_dbuffrd) delete vel_dbuffrd;
      if (spectw_dbuffrd) delete spectw_dbuffrd;
      
      if (delVelScan && velscan &&
	velscan->ShouldDelete(velfilter, "NexRad::writeTilt")) 
	    delete velscan;
      if (delReflScan && reflscan &&
	reflscan->ShouldDelete(reflfilter, "NexRad::writeTilt")) 
	    delete reflscan;
      if (delSpecWScan && specwscan &&
	specwscan->ShouldDelete(specwfilter, "NexRad::writeTilt")) 
	    delete specwscan;
	return;
    }
  
  //try identifying *_ptr with *_data_playback to fix data offset
  message.data_hdr.ref_data_playback = htons((si16)(local_data_hdr_ref_ptr));
  message.data_hdr.vel_data_playback = htons((si16)(local_data_hdr_vel_ptr));
  message.data_hdr.sw_data_playback  = htons((si16)(local_data_hdr_sw_ptr));
  
  int azt;
  if (reflscan) refl_dbuffrd->open(reflscan->DataBuff());
  if (velscan) vel_dbuffrd->open(velscan->DataBuff());
  if (specwscan && makeSpecWidth) spectw_dbuffrd->open(specwscan->DataBuff());
  for (azt=0;azt<361;azt++)
    {
      if (azt == 360) az = 0; //send beam 0 as beam 360 for WDSS, az is for rapic
      else az = azt;
      if (reflscan)
	{
	  reflscan->get_radl_angl(&reflradl,az*10, refl_dbuffrd, &radlpos);
	  if (refl_rngres_reduce_factor != 1)
	    reflradl.RngResReduce(refl_rngres_reduce_factor, RRR_PWR);
	}
      if (velscan) 
	{
	  velscan->get_radl_angl(&velradl,az*10, vel_dbuffrd, &radlpos);
	  if (vel_rngres_reduce_factor != 1)
	    velradl.RngResReduce(vel_rngres_reduce_factor, RRR_MED);
	}
      if (specwscan && makeSpecWidth) 
	{
	  specwscan->get_radl_angl(&specwradl,az*10, spectw_dbuffrd, &radlpos);
	  // CHEAT HERE - ASSUME SAME rngres_reduce as vel
	  if (vel_rngres_reduce_factor != 1)
	    specwradl.RngResReduce(vel_rngres_reduce_factor, RRR_MED);
	}
      //set up new radial message
      message.ctm_info.fr_seq               = htonl(fr_seq++);
      if (++seq_number > 0x7fff) seq_number = 0;
      message.msg_hdr.seq_num = htons((si16)seq_number);
      curr_time = time(0);
      message.msg_hdr.julian_date = htons((si16)getJulian(curr_time));
      message.msg_hdr.millisecs_past_midnight = 
	                       htonl((si32)getMillsecFromMidn(curr_time));
      message.data_hdr.julian_date = htons((si16)getJulian(reflscan->scan_time_t));
      message.data_hdr.millisecs_past_midnight = 
	                       htonl((si32)getMillsecFromMidn(reflscan->scan_time_t));
      message.data_hdr.azimuth     = htons((ui16)azt*4096*8/180);
      message.data_hdr.radial_num  = htons((si16)azt);
      if (volbegin && azt == 0)  //note azt and 360 for WDSS
	      message.data_hdr.radial_status = htons((si16)3);
      else if (volend && azt == 360)
	      message.data_hdr.radial_status = htons((si16)4);
      else if (azt == 0)
	      message.data_hdr.radial_status = htons((si16)0);
      else if (azt == 360)
	      message.data_hdr.radial_status = htons((si16)2);
      else    message.data_hdr.radial_status = htons((si16)1);
      
      message.data_hdr.elevation = htons((ui16)reflscan->set_angle*4096*8/1800);
                         //note set_angle is in 10ths of a degree
      message.data_hdr.elev_num  = htons((si16)tiltnumber);

      memset(&data,'\0',packet_size);
      //copy over header data
      memcpy(&data,&message,data_ind);
      
      //for each gate in radial do:
      if (reflscan) 
	{
	  for (gate = 0;gate<reflradl.data_size;gate++)
	    {
	      if (reflradl.data[gate] < scan->NumLevels &&
		  scan->LvlTbls(e_refl) &&
		  reflradl.data[gate] != 0 ) //if 0 leave as such
		data[loc_ref_ptr+gate] = (unsigned char)
		  (((int)(scan->LvlTbls(e_refl)->val(reflradl.data[gate])))+32)*2+2;
	      
	    }
	}
      
      if (velscan)
	{
	  for (gate = 0;gate<velradl.data_size;gate++)
	    {
	      if (velradl.data[gate] > velradl.numlevels -1  ||
		  velradl.data[gate] == 0)
		data[loc_vel_ptr+gate]  = 0; // no data value, 
	      else
		data[loc_vel_ptr+gate]  =
		  (char)  
		  ((int)(((velradl.data[gate]-velradl.numlevels/2)*
			  velscan->nyquist/
			  (velradl.numlevels/2-1)+63.5)*2.0+2.0)
		   & 0xff);
	      //0 = no data,1-7 = -ve (towards),8 = no vel,9-15 = +ve (away)
	    }
	}
      
      if (specwscan && makeSpecWidth)  //this one is a guess,  redo when spectrum width is available
	{
	  for (gate = 0;gate<specwradl.data_size;gate++)
	    {
	      if (specwradl.data[gate] > specwradl.numlevels -1 ||
		  specwradl.data[gate] == 0)
		data[loc_sw_ptr+gate]  = 0; // no data value, 
	      else
		data[loc_sw_ptr+gate]  =
		  (char)  
		  ((int)(((specwradl.data[gate]-specwradl.numlevels/2)*
			  velscan->nyquist/
			  (specwradl.numlevels/2-1)+63.5)*2.0+2.0)
		   & 0xff);
	      //0 = no data, 1 - 7 = -ve (towards), 8 = no vel, 9 - 15 = +ve (away)
	    }
	}

      //debug
//      if (debug && ((volend && azt == 360) || (volbegin && azt == 0))) 
//	print_data_buffer(data,&message);
      
      //deal with data, send radial to file descriptors
      //delay for 10 millisec
      sec_delay(delay_pac/1000);  //delay for NEX_DELAY millisecs
      for (int sock=0;sock<outputcount;sock++)
	if (outputdesc[sock]->fd > -1)
	  { 
	    if (outputdesc[sock]->archive2file) // files use 4 byte shorter frame header
	      outputdesc[sock]->write_beam(data+4,packet_size);
	    else
	      outputdesc[sock]->write_beam(data,packet_size);
	  }
      volbegin = 0;
    }

  /*
   * limit the rate at which tilts can be written out
   */
  while (time(0) < next_tilt_time)
  {
    printf("t");
    fflush(stdout);
    
      sec_delay(0.5);	// wait 1 second
  }

  if (refl_dbuffrd) delete refl_dbuffrd;
  if (vel_dbuffrd) delete vel_dbuffrd;
  if (spectw_dbuffrd) delete spectw_dbuffrd;
  
  if (delVelScan && velscan &&
    velscan->ShouldDelete(velfilter, "NexRad::writeTilt")) 
	delete velscan;
  if (delReflScan && reflscan &&
    reflscan->ShouldDelete(reflfilter, "NexRad::writeTilt")) 
	delete reflscan;
  if (delSpecWScan && specwscan &&
    specwscan->ShouldDelete(specwfilter, "NexRad::writeTilt")) 
        delete specwscan;
    
}


void NexRad::writeLast()
  {
  int local_data_hdr_ref_ptr = 0;
  int local_data_hdr_vel_ptr = 0;
  int local_data_hdr_sw_ptr  = 0;
  int local_data_hdr_ref_num_gates = 0;
  int local_data_hdr_vel_num_gates = 0;

  //write out tilt array to signal a last scan for a volume. This is in case there is
  //a problem at rapic end, to signal to nexrad downstream to finish volume.
  time_t curr_time = time(0);
  int data_ind;
  int loc_ref_ptr = 0,loc_vel_ptr = 0, hdr_ind;
  //  int loc_sw_ptr = 0;
  
  //  rdr_scan *velscan=0,*reflscan=0,*specwscan=0,*tempscan;
  rdr_scan *reflscan=0;

  data_ind = sizeof(message_typ);
  hdr_ind = sizeof(NEXRAD_data_hdr);
  
  if (scan->scan_type == VOL) 
    message.data_hdr.vol_coverage_pattern = htons((si16)pattern); 
  message.data_hdr.sw_ptr  = htons(0);
  local_data_hdr_sw_ptr    = 0;
  reflscan = scan->rootScan();
  
  message.data_hdr.ref_num_gates   =
       htons((si16)(reflscan->max_rng*1000/(reflscan->rng_res*refl_rngres_reduce_factor)));
  local_data_hdr_ref_num_gates =
       int(reflscan->max_rng*1000/(reflscan->rng_res*refl_rngres_reduce_factor));
  message.data_hdr.ref_gate1       = htons((si16)reflscan->start_rng);
  message.data_hdr.ref_gate_width  = htons((si16)reflscan->rng_res*refl_rngres_reduce_factor);
  message.data_hdr.ref_ptr         = htons((si16)hdr_ind);
  local_data_hdr_ref_ptr           = hdr_ind;
  loc_ref_ptr                      = data_ind;
  
  // NO velscan EXISTS HERE, JUST USE SOME reflscan PROPERTIES, HOPEFULLY SHOULDN'T MATTER - ALREADY A FAULT CONDITION ANYWAY
  message.data_hdr.vel_num_gates   = 
    htons((si16)(reflscan->max_rng*1000/(reflscan->rng_res*vel_rngres_reduce_factor)));
  local_data_hdr_vel_num_gates   = 
	   int(reflscan->max_rng*1000/(reflscan->rng_res*vel_rngres_reduce_factor));
  message.data_hdr.vel_gate1       = htons((si16)reflscan->start_rng);
  message.data_hdr.vel_gate_width  = htons((si16)reflscan->rng_res*vel_rngres_reduce_factor);
  message.data_hdr.velocity_resolution  = htons((si16)2); //ie, 0.5m/s 
  message.data_hdr.nyquist_vel     = htons((si16)(reflscan->nyquist*100));
  message.data_hdr.vel_ptr         = htons((si16)(local_data_hdr_ref_ptr +
					    local_data_hdr_ref_num_gates));
  local_data_hdr_vel_ptr           = local_data_hdr_ref_ptr +
					      local_data_hdr_ref_num_gates;
  loc_vel_ptr                      = loc_ref_ptr + 
                                              local_data_hdr_ref_num_gates;
  
  
  //try identifying *_ptr with *_data_playback to fix data offset
  message.data_hdr.ref_data_playback = htons((si16)(local_data_hdr_ref_ptr));
  message.data_hdr.vel_data_playback = htons((si16)(local_data_hdr_vel_ptr));
  message.data_hdr.sw_data_playback  = htons((si16)(local_data_hdr_sw_ptr));

  //set up new radial message
  message.ctm_info.fr_seq               = htonl(fr_seq++);
  if (++seq_number > 0x7fff) seq_number = 0;
  message.msg_hdr.seq_num = htons((si16)seq_number);
  message.msg_hdr.julian_date = htons((si16)getJulian(curr_time));
  message.msg_hdr.millisecs_past_midnight = 
    htonl((si32)getMillsecFromMidn(curr_time));
  message.data_hdr.julian_date = htons((si16)getJulian(reflscan->scan_time_t));
  message.data_hdr.millisecs_past_midnight = 
    htonl((si32)getMillsecFromMidn(reflscan->scan_time_t));
  message.data_hdr.azimuth     = htons(0);
  message.data_hdr.radial_num  = htons(0);
  
  message.data_hdr.radial_status = htons((si16)4);  //endvol status, what this is for!
  
  message.data_hdr.elevation = htons((ui16)reflscan->set_angle*4096*8/1800);
  //note set_angle is in 10ths of a degree
  message.data_hdr.elev_num  = htons((si16)tiltnumber);
  
  memset(&data,'\0',packet_size);
  //copy over header data, leave data zero!
  memcpy(&data,&message,data_ind);
  
  //deal with data, send radial to file descriptors
  for (int sock=0;sock<outputcount;sock++)
    if (outputdesc[sock]->fd > -1) 
      {
	if (outputdesc[sock]->archive2file) // files use 4 byte shorter frame header
	  outputdesc[sock]->write_beam(data+4,packet_size);
	else
	  outputdesc[sock]->write_beam(data,packet_size);
      }
}

int NexRad::getJulian(time_t time)
{
  //return Julian date since 1/1/1970
  //get unix time of last midnight, divide by 3600*2400, round to nearest int
  struct tm *timestr;
  int secFromMidnight;
  
  timestr =  gmtime(&time);
  secFromMidnight =  timestr->tm_hour*3600 +
                     timestr->tm_min*60 +
                     timestr->tm_sec;
  return (int)(time - secFromMidnight + 12*3600)/(24*3600) + 1;  //rounded day
}

int NexRad::getMillsecFromMidn(time_t time)
{
  //return milliseconds from last midnight
  struct tm *timestr;
  
  timestr =  gmtime(&time);
  return  (timestr->tm_hour*3600 +
           timestr->tm_min*60    +
           timestr->tm_sec        )*1000;
}

void NexRad::print_data_buffer(char *data,message_typ *message)
{
  //Note: message contents are in NETWORK endianness, so may be screwed on Intel.
  NEXRAD_data_hdr *riddsData = &(message->data_hdr);
  
  
  int i,offset;
  offset = sizeof(NEXRAD_frame_hdr) + sizeof(NEXRAD_msg_hdr);
  
  //print out message header
  printf("NEXRAD_frame_hdr  =\n"
  "   fr_seq = %d\n"       
  "   mess_length = %d\n"     
  "   nframes = %d\n"         
  "   frame_num = %d\n"       
  "   frame_offset = %d\n"    
  "   data_len = %d\n"        
  "   flags = %d\n",
     message->ctm_info.fr_seq,       
     message->ctm_info.mess_length,     
     message->ctm_info.nframes,         
     message->ctm_info.frame_num,       
     message->ctm_info.frame_offset,    
     message->ctm_info.data_len,        
     message->ctm_info.flags);

  printf("NEXRAD_msg_hdr =\n" 
     "   message_len = %d\n"  
     "   message_type = %d\n" 
     "   seq_num = %d\n"      
     "   julian_date = %d\n"  
     "   millisecs_past_midnight = %d\n" 
     "   num_message_segs = %d\n"	
     "   message_seg_num = %d\n",
     message->msg_hdr.message_len,  
     message->msg_hdr.message_type, 
     message->msg_hdr.seq_num,      
     message->msg_hdr.julian_date,  
     message->msg_hdr.millisecs_past_midnight, 
     message->msg_hdr.num_message_segs,	
     message->msg_hdr.message_seg_num);
 
  //print out header data from data buffer
  printf("RIDDS_data_hdr *riddsData =\n"
    "	millisecs_past_midnight = %d\n"
    "	julian_date = %d\n"   
    "	unamb_range_x10 = %d\n"
    "	azimuth = %d\n"        
    "	radial_num = %d\n"     
    "	radial_status = %d\n"  
    "	elevation = %d\n"   
    "	elev_num = %d\n"    
    "	ref_gate1 = %d\n"   
    "	vel_gate1 = %d\n"   
    "	ref_gate_width = %d\n"
    "	vel_gate_width = %d\n"
    "	ref_num_gates = %d\n" 
    "	vel_num_gates = %d\n" 
    "	sector_num = %d\n"  
    "	sys_gain_cal_const = %d\n" 
    "	ref_ptr = %d\n"      
    "	vel_ptr = %d\n"      
    "	sw_ptr = %d\n"       
    "	velocity_resolution = %d\n"  
    "	vol_coverage_pattern = %d\n" 
    "	ref_data_playback = %d\n"
    "	vel_data_playback = %d\n"
    "	sw_data_playback = %d\n" 
    "	nyquist_vel = %d\n"     
    "	atmos_atten_factor = %d\n" 
       "	threshold_param = %d\n",
    riddsData->millisecs_past_midnight,
    riddsData->julian_date,   
    riddsData->unamb_range_x10,
    riddsData->azimuth,        
    riddsData->radial_num,     
    riddsData->radial_status,  
    riddsData->elevation,   
    riddsData->elev_num,    
    riddsData->ref_gate1,   
    riddsData->vel_gate1,   
    riddsData->ref_gate_width,
    riddsData->vel_gate_width,
    riddsData->ref_num_gates, 
    riddsData->vel_num_gates, 
    riddsData->sector_num,  
    riddsData->sys_gain_cal_const, 
    riddsData->ref_ptr,      
    riddsData->vel_ptr,      
    riddsData->sw_ptr,       
    riddsData->velocity_resolution,  
    riddsData->vol_coverage_pattern, 
    riddsData->ref_data_playback,
    riddsData->vel_data_playback,
    riddsData->sw_data_playback, 
    riddsData->nyquist_vel,     
    riddsData->atmos_atten_factor, 
    riddsData->threshold_param);


printf("\n\nRefl Data buffer is:\n");
if (riddsData->ref_ptr)for (i=riddsData->ref_ptr;i<riddsData->ref_ptr+riddsData->ref_num_gates;i++)
  printf(" %d",data[offset+i]);

printf("\n\nVel Data buffer is:\n");
if (riddsData->vel_ptr) for (i=riddsData->vel_ptr;i<riddsData->vel_ptr+riddsData->vel_num_gates;i++)
  printf(" %d",data[offset+i]);

printf("\n");
}

void NexRad::print_201(message_201 *message)
{
  
  //print out message header
  //Note: message contents are in NETWORK endianness, so may be screwed on Intel.
  printf("\n201_frame_hdr  =\n"
  "   fr_seq = %d\n"       
  "   mess_length = %d\n"     
  "   nframes = %d\n"         
  "   frame_num = %d\n"       
  "   frame_offset = %d\n"    
  "   data_len = %d\n"        
	 "   flags = %d\n",
     message->ctm_info.fr_seq,       
     message->ctm_info.mess_length,     
     message->ctm_info.nframes,         
     message->ctm_info.frame_num,       
     message->ctm_info.frame_offset,    
     message->ctm_info.data_len,        
     message->ctm_info.flags);

  printf("201_msg_hdr =\n" 
     "   message_len = %d\n"  
     "   message_type = %d\n" 
     "   seq_num = %d\n"      
     "   julian_date = %d\n"  
     "   millisecs_past_midnight = %d\n" 
     "   num_message_segs = %d\n"	
     "   message_seg_num = %d\n",
     message->msg_hdr.message_len,  
     message->msg_hdr.message_type, 
     message->msg_hdr.seq_num,      
     message->msg_hdr.julian_date,  
     message->msg_hdr.millisecs_past_midnight, 
     message->msg_hdr.num_message_segs,	
     message->msg_hdr.message_seg_num);
  printf("201 vol_scan_no = %d\n\n",message->vol_scan_no);
}


NexRadOutpDesc::NexRadOutpDesc()
{
  //init for file, opens later
  fd = -1;
  fs_flag = NEXRAD_AVAIL;
  archive2file = false;
  
}


NexRadOutpDesc::NexRadOutpDesc(char *address,int port)
{
  //construct new NexRadOutpDesc from IP addr and port number
  strcpy(this->address,address);
  this->port = port;
  fd = -1;
  fs_flag = NEXRAD_SOCKET;
}

NexRadOutpDesc::~NexRadOutpDesc()
{
  if (fd > -1) close(fd);
}

void NexRadOutpDesc::close_fd()
{
  if (fd >= 0) {
    close(fd); 
    fd = -1;
    }
}

void NexRadOutpDesc::open_socket(int local_port)
{
  
  if (fd >= 0)
    close_fd();	    // fd is open, close it
  if (fs_flag == NEXRAD_SOCKET)
    {
      fd = open_output_udp(local_port);
      if (fd == -1)
	{
	  fprintf(stderr,"NexRadOutpDesc::open_socket: ERROR Cannot open %s:%d for write\n",
		  address, port );
	}
    }
  //debug
  //printf("NexRadOutpDesc::open_socket: address = %s, port = %d, loc_port = %d, fd = %d\n",
  // address,port,local_port,fd);
  
  
}

void NexRadOutpDesc::open_file(char *filename)
{
  int filed;
  mode_t mode = (mode_t) 0664;  //urw grw or

    if (fd >= 0)
	close_fd();	    // fd is open, close it
    filed = open(filename,O_WRONLY | O_CREAT,mode);
  if (filed == -1)
    {
      fprintf(stderr,"NexRadOutpDesc::open_file: ERROR Cannot open %s for write\n",
	      filename );
      perror("NexRadOutpDesc::open_file");
    }
  strcpy(filepath,filename);
  fd = filed;
}
      
void NexRadOutpDesc::write_beam(char *data,int datasize)
{
  int count = 0,limit = NEXRAD_SOCKET_LIMIT;
  
  if (fd < 0)
  {
      fprintf(stderr, "NexRadOutpDesc::write_beam - ERROR trying to write to fd = -1\n");
    return;
  }
  if (fs_flag == NEXRAD_SOCKET)
    {
      while (count < limit && 
	     sendto(fd,data,datasize,0,
		  (struct sockaddr *) &Out_address, sizeof(Out_address)) == -1)
	{
	  fprintf(stderr,"NexRadOutpDesc::write_beam: Status writing UDP data, port = %d, address = %s\n",port,address);
	  perror("NexRadOutpDesc::write_beam");
	  switch (errno) {
		case EBADF:
		case ENOTSOCK:
		case EFAULT:
		case EMSGSIZE:
		case EISCONN:
		case EACCES:
			count = limit;
			break;
		}
	  sleep(1);
	  count++;
	}
      if (count >= limit)
	{
	  fprintf(stderr,"NexRadOutpDesc::write_beam: Error writing UDP data, exceeded try limit, removing output socket from list\n");
	  fs_flag = NEXRAD_AVAIL;
	}
    }

  if (fs_flag == NEXRAD_FILE && fd != -1 && write(fd,data,datasize) == -1)
    {
      fprintf(stderr,"NexRadOutpDesc::write_beam: Error writing file, removing from output list, fd = %d\n",fd);
      perror("NexRadOutpDesc::write_beam");
      
      close_fd();
    }
}

	  

int NexRadOutpDesc::open_output_udp(int loc_port)

{

  int option;
  struct sockaddr_in local_addr;
  int Udp_fd;
  /*
   * open socket
   */

  if  ((Udp_fd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
    fprintf(stderr, "NexRad::open_output_udp: ERROR Could not open UDP socket, port %d\n", port);
    return (-1);
  }
  
  /*
   * bind local address to the socket
   */
  
  memset(&local_addr,0,sizeof(struct sockaddr_in));
  local_addr.sin_port = htons (loc_port);  
  //local_addr.sin_port = htons (MYPORT);
  local_addr.sin_family = AF_INET;
  local_addr.sin_addr.s_addr = htonl (INADDR_ANY);
  
  if (bind (Udp_fd, (struct sockaddr *) &local_addr, 
	    sizeof (local_addr)) < 0) {
    fprintf(stderr, "NexRad::open_output_udp: ERROR Could not bind UDP socket, port %d\n", loc_port);
    perror("NexRadMgr");
    close (Udp_fd);
    return (-1);
  }
  
  /*
   * set socket for broadcast
   */
  
  option = 1;
  //option = 0;  //turn broadcast off
  /*SD debug: try no broadcast
  if (setsockopt(Udp_fd, SOL_SOCKET, SO_BROADCAST,
		 (char *) &option, sizeof(option)) < 0) {
    fprintf(stderr,"NexRadMgr::open_output_udp: ERROR Could not set broadcast on - setsockopt error");
    perror("NexRadMgr");
    return (-1);
  }
  */
  /*
   * set up destination address structure
   */

  memset(&Out_address,0,sizeof(struct sockaddr_in) );
#if (defined SUNOS4) || (defined SUNOS5) || (defined sun)
  Out_address.sin_addr.S_un.S_addr = inet_addr(address);
#else
  if (inet_aton(address, &Out_address.sin_addr) == 0) {
  //if (inet_aton(INADDR_BROADCAST, &Out_address.sin_addr) == 0) {
    fprintf(stderr, "NexRadMgr::open_output_udp: ERROR Cannot translate address: %s - may be invalid\n",
            address);
    perror("NexRadMgr");
    return(-1);
  }
#endif
  //Out_address.sin_addr.s_addr = htonl(INADDR_BROADCAST);
  
  Out_address.sin_family = AF_INET;
  Out_address.sin_port = htons (port);
  
  return (Udp_fd);
}
