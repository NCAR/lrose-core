#include <ctime>
#include <cmath>
#include <sys/time.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <Radx/HrdData.hh>
#include <Radx/ByteOrder.hh>

using namespace std;

//////////////////////////////////////////////////////
// main header

void HrdPrint(const hrd_header_t &hdr,
              ostream &out)

{
  
  out << "=========== hrd_header_t ===========" << endl;
  out << "  header_flag: " << hdr.header_flag << endl;
  out << "  sizeof_header: " << hdr.sizeof_header << endl;
  out << "  tape_num: " << hdr.tape_num << endl;
  out << "  hd_fmt_ver: " << hdr.hd_fmt_ver << endl;
  out << "  year: " << hdr.year << endl;
  out << "  month: " << hdr.month << endl;
  out << "  day: " << hdr.day << endl;
  out << "  hour: " << hdr.hour << endl;
  out << "  minute: " << hdr.minute << endl;
  out << "  second: " << hdr.second << endl;
  out << "  LF_menu: " << Radx::makeString(hdr.LF_menu, 16) << endl;
  out << "  TA_menu: " << Radx::makeString(hdr.TA_menu, 16) << endl;
  out << "  Data_menu[16]: " << Radx::makeString(hdr.Data_menu, 16) << endl;
  out << "  nav_system: " << hdr.nav_system << endl;
  out << "  LU_tape_drive: " << hdr.LU_tape_drive << endl;
  out << "  aircraft_id: " << hdr.aircraft_id << endl;
  out << "  flight_id[8]: " << Radx::makeString(hdr.flight_id, 8) << endl;
  out << "  data_header_len: " << hdr.data_header_len << endl;
  out << "  ray_header_len: " << hdr.ray_header_len << endl;
  out << "  time_zone_offset: " << hdr.time_zone_offset << endl;
  out << "=====>> LOWER FUSELAGE RADAR <<=====" << endl;
  HrdPrint(hdr.radar_lf, out);
  out << "==========>> TAIL RADAR <<==========" << endl;
  HrdPrint(hdr.radar_ta, out);
  if (strlen(hdr.comments) > 0) {
    out << "=======>> COMMENTS" << endl;
    cerr << hdr.comments << endl;
  }
  out << "====================================" << endl;
  
}

void HrdSwap(hrd_header_t &hdr)

{

  ByteOrder::swap16(&hdr.header_flag, 11 * sizeof(Radx::si16));
  ByteOrder::swap16(&hdr.word_36, 4 * sizeof(Radx::si16));
  ByteOrder::swap16(&hdr.data_header_len, 37 * sizeof(Radx::si16));
  ByteOrder::swap16(hdr.words_85_to_100, 16 * sizeof(Radx::si16));
  HrdSwap(hdr.radar_lf);
  HrdSwap(hdr.radar_ta);
  
}

//////////////////////////////////////////////////////
// radar info header

void HrdPrint(const hrd_radar_info_t &radar,
              ostream &out)

{
  
  out << "========= hrd_radar_info_t =========" << endl;

  out << "  sample_size: " << radar.sample_size << endl;
  out << "  DSP_flag: " << radar.DSP_flag << endl;
  out << "  refl_slope_x4096: " << radar.refl_slope_x4096 << endl;
  out << "  refl_noise_thr_x16: " << radar.refl_noise_thr_x16 << endl;
  out << "  clutter_cor_thr_x16: " << radar.clutter_cor_thr_x16 << endl;
  out << "  SQI_thr: " << radar.SQI_thr << endl;
  out << "  width_power_thr_x16: " << radar.width_power_thr_x16 << endl;
  out << "  calib_refl_x16: " << radar.calib_refl_x16 << endl;
  out << "  AGC_decay_code: " << radar.AGC_decay_code << endl;
  out << "  dual_PRF_stabil_delay: " << radar.dual_PRF_stabil_delay << endl;
  out << "  thr_flags_uncorr_refl: " << radar.thr_flags_uncorr_refl << endl;
  out << "  thr_flags_vel: " << radar.thr_flags_vel << endl;
  out << "  thr_flags_width: " << radar.thr_flags_width << endl;
  out << "  data_mode: " << radar.data_mode << endl;
  out << "  range_b1: " << radar.range_b1 << endl;
  out << "  variable_spacing_flag: " << radar.variable_spacing_flag << endl;
  out << "  bin_spacing_xe3: " << radar.bin_spacing_xe3 << endl;
  out << "  num_input_bins: " << radar.num_input_bins << endl;
  out << "  range_avg_state: " << radar.range_avg_state << endl;
  out << "  b1_adjust_xe4: " << radar.b1_adjust_xe4 << endl;
  out << "  num_output_bins: " << radar.num_output_bins << endl;
  out << "  PRT_noise_sample: " << radar.PRT_noise_sample << endl;
  out << "  range_noise_sample: " << radar.range_noise_sample << endl;
  out << "  log_rec_noise_x64: " << radar.log_rec_noise_x64 << endl;
  out << "  I_A2D_offset_x256: " << radar.I_A2D_offset_x256 << endl;
  out << "  Q_A2D_offset_x256: " << radar.Q_A2D_offset_x256 << endl;
  out << "  waveln_xe4: " << radar.waveln_xe4 << endl;
  out << "  pulse_width_xe8: " << radar.pulse_width_xe8 << endl;
  out << "  PRF: " << radar.PRF << endl;
  out << "  DSS_flag: " << radar.DSS_flag << endl;
  out << "  trans_recv_number: " << radar.trans_recv_number << endl;
  out << "  transmit_power: " << radar.transmit_power << endl;
  out << "  gain_control_flag: " << radar.gain_control_flag << endl;
  out << "  scan_mode: " << radar.scan_mode << endl;
  out << "  sweep_speed_x10: " << radar.sweep_speed_x10 << endl;
  out << "  tilt_angle: " << radar.tilt_angle << endl;
  out << "  sector_center: " << radar.sector_center << endl;
  out << "  sector_width: " << radar.sector_width << endl;

  out << "====================================" << endl;
  
}

void HrdSwap(hrd_radar_info_t &radar)

{
  ByteOrder::swap16(&radar, sizeof(radar));
}

//////////////////////////////////////////////////////
// data record header

void HrdPrint(const hrd_data_rec_header_t &rec,
              ostream &out)

{
  
  out << "====== hrd_data_rec_header_t =======" << endl;

  out << "  data_record_flag: " << rec.data_record_flag << endl;
  out << "  sizeof_rec: " << rec.sizeof_rec << endl;
  out << "  sweep_num: " << rec.sweep_num << endl;
  out << "  rec_num: " << rec.rec_num << endl;
  out << "  radar_num: " << (int) rec.radar_num << endl;
  out << "  rec_num_flag: " << (int) rec.rec_num_flag << endl;

  out << "====================================" << endl;
  
}

void HrdSwap(hrd_data_rec_header_t &rec)

{
  ByteOrder::swap16(&rec, sizeof(rec) - 2);
}

//////////////////////////////////////////////////////
// ray header

void HrdPrint(const hrd_ray_header_t &ray,
              ostream &out)

{
  
  out << "======== hrd_ray_header_t ==========" << endl;

  out << "  sizeof_ray: " << ray.sizeof_ray << endl;
  out << "  field_code: 0x" << hex << (int) ray.field_code << dec << endl;
  if (ray.field_code & 0x80) {
    out << "    reflectivity" << endl;
  }
  if (ray.field_code & 0x40) {
    out << "    velocity" << endl;
  }
  if (ray.field_code & 0x20) {
    out << "    width" << endl;
  }
  if (ray.field_code & 0x10) {
    out << "    from TA DSP" << endl;
  }
  if (ray.field_code & 0x08) {
    out << "    from LF DSP" << endl;
  }
  if (ray.field_code & 0x04) {
    out << "    time-series" << endl;
  }
  out << "  year: " << (int) ray.year << endl;
  out << "  month: " << (int) ray.month << endl;
  out << "  day: " << (int) ray.day << endl;
  out << "  ray_code: " << (int) ray.ray_code << endl;
  out << "  hour: " << (int) ray.hour << endl;
  out << "  minute: " << ray.minute << endl;
  out << "  seconds: " << (double) ray.seconds_x100 / 100.0 << endl;
  out << "  latitude: " << HrdAngle(ray.latitude) << endl;
  out << "  longitude: " << HrdAngle(ray.longitude) << endl;
  out << "  altitude_xe3: " << ray.altitude_xe3 << endl;
  out << "  ac_vew: " << ray.ac_vew_x10 / 10.0 << endl;
  out << "  ac_vns: " << ray.ac_vns_x10 / 10.0 << endl;
  out << "  ac_vud: " << ray.ac_vud_x10 / 10.0 << endl;
  out << "  ac_ui: " << ray.ac_ui_x10 / 10.0 << endl;
  out << "  ac_vi: " << ray.ac_vi_x10 / 10.0 << endl;
  out << "  ac_wi: " << ray.ac_wi_x10 / 10.0 << endl;
  out << "  RCU_status: " << ray.RCU_status << endl;
  out << "  elevation: " << HrdAngle(ray.elevation) << endl;
  out << "  azimuth: " << HrdAngle(ray.azimuth) << endl;
  out << "  ac_pitch: " << HrdAngle(ray.ac_pitch) << endl;
  out << "  ac_roll: " << HrdAngle(ray.ac_roll) << endl;
  out << "  ac_drift: " << HrdAngle(ray.ac_drift) << endl;
  out << "  ac_heading: " << HrdAngle(ray.ac_heading) << endl;

  out << "====================================" << endl;
  
}

void HrdSwap(hrd_ray_header_t &ray)

{
  ByteOrder::swap16(&ray.sizeof_ray, 1 * sizeof(Radx::ui16));
  ByteOrder::swap16(&ray.minute, 18 * sizeof(Radx::si16));
}

//////////////////////////////////////////////////////
// print rays

void HrdPrintRays(const hrd_data_rec_header_t &rec,
                  unsigned char *recBuf,
                  int recLen,
                  bool printData,
                  ostream &out)
  
{
  
  unsigned char *ptr = recBuf;
  unsigned char *endPtr = ptr + recLen;
  
  while (ptr < endPtr) {

    // print header
    
    hrd_ray_header_t rayHdr;
    memcpy(&rayHdr, ptr, sizeof(rayHdr));
    HrdSwap(rayHdr);
    HrdPrint(rayHdr, out);
    
    if (rayHdr.sizeof_ray < sizeof(rayHdr)) {
      // done
      return;
    }

    // print data if required

    if (printData) {

      unsigned char *dataBuf = ptr + sizeof(rayHdr);
      int dataLen = rayHdr.sizeof_ray - sizeof(rayHdr);
      
      if (rayHdr.field_code & 0x10) {
      
        // TA data

        bool haveDbz = (rayHdr.field_code & 0x80);
        bool haveVel = (rayHdr.field_code & 0x40);
        bool haveWidth = (rayHdr.field_code & 0x20);
        
        HrdPrintTaData(dataBuf, dataLen,
                       haveDbz, haveVel, haveWidth,
                       out);

      } else if (rayHdr.field_code & 0x08) {

        // LF data

        HrdPrintLfData(dataBuf, dataLen, out);
        
      }

    }
    
    // move to next ray
    
    ptr += rayHdr.sizeof_ray;
    
  }

}

//////////////////////////////////////////////////////
// print LF data

void HrdPrintLfData(unsigned char *dataBuf,
                    int dataLen,
                    ostream &out)

{

  // uncompress the data

  RadxBuf buf;
  HrdUncompress(dataBuf, dataLen, buf);
  HrdPrintFieldData("LF DBZ",
                    (unsigned char *) buf.getPtr(),
                    buf.getLen(),
                    out);

}
  
//////////////////////////////////////////////////////
// print TA data

void HrdPrintTaData(unsigned char *dataBuf,
                    int dataLen,
                    bool haveDbz,
                    bool haveVel,
                    bool haveWidth,
                    ostream &out)

{

  // uncompress the data

  RadxBuf buf;
  HrdUncompress(dataBuf, dataLen, buf);

  // compute the stride for the data in the raw array

  int stride = 0;
  if (haveDbz) stride++;
  if (haveVel) stride++;
  if (haveWidth) stride++;

  // load up field data and print out



  HrdPrintFieldData("LF DBZ",
                    (unsigned char *) buf.getPtr(),
                    buf.getLen(),
                    out);

  

}

//////////////////////////////////////////////////////
// print field data

void HrdPrintFieldData(const string &label,
                       unsigned char *data,
                       int nGates,
                       ostream &out)
  
{

  out << "========================================================" << endl;
  out << "Ray data for: " << label << endl;
  out << "nGates: " << nGates << endl;

  int printed = 0;
  int count = 1;
  double prevVal = HrdDbzVal(data[0]);
  for (int ii = 1; ii < nGates; ii++) {
    double val = HrdDbzVal(data[ii]);
    if (val != prevVal) {
      HrdPrintPacked(out, count, prevVal, Radx::missingFl64);
      printed++;
      if (printed > 7) {
        out << endl;
        printed = 0;
      }
      prevVal = val;
      count = 1;
    } else {
      count++;
    }
  } // ii
  HrdPrintPacked(out, count, prevVal, Radx::missingFl64);
  out << endl;
  out << "========================================================" << endl;

}
  
//////////////////////////////////////////////////////
// uncompress packed data

void HrdUncompress(unsigned char *dataBuf,
                   int dataLen,
                   RadxBuf &buf)
  
{

  buf.clear();
  Radx::ui16 *comp = (Radx::ui16 *) dataBuf;
  int nwords = dataLen / 2;

  for (int ii = 0; ii < nwords; ii++) {

    Radx::ui16 word = comp[ii];
    ByteOrder::swap16(&word, sizeof(word), true);
    int msbit = word & 0x8000;
    int lbits = word & 0x7fff;

    if (msbit == 0) {

      if (lbits == 1) {
        // end of ray
        return;
      }
      
      // zeros

      Radx::ui16 zero = 0;
      for (int jj = 0; jj < lbits; jj++) {
        buf.add(&zero, sizeof(zero));
      } // jj

    } else {

      for (int jj = 0; jj < lbits; jj++) {
        ii++;
        word = comp[ii];
        buf.add(&word, sizeof(word));
      } // jj

    } // if (msbit)

  } // ii

}

////////////////////////////////
// get DBZ value from scaled int

double HrdDbzVal(int ival) 
{

  if (ival == 0) {
    return Radx::missingFl64;
  }

  return (ival - 64.0) / 2.0;

}

////////////////////////////////
// get VEL value from scaled int

double HrdVelVal(int ival) 
{

  if (ival == 0) {
    return Radx::missingFl64;
  }

  return (ival - 64.0) / 2.0;

}

////////////////////////////////
// get WIDTH value from scaled int

double HrdWidthVal(int ival) 
{

  if (ival == 0) {
    return Radx::missingFl64;
  }

  return (ival - 64.0) / 2.0;

}

/////////////////////////////////////////////////////////////////
// print in packed format, using count for identical data values

void HrdPrintPacked(ostream &out, int count,
                    double val, double missing)

{
  
  char outstr[1024];
  if (count > 1) {
    out << count << "*";
  }
  if (val == missing) {
    out << "MISS ";
  } else {
    if (fabs(val) > 0.01) {
      sprintf(outstr, "%.3f ", val);
      out << outstr;
    } else if (val == 0.0) {
      out << "0.0 ";
    } else {
      sprintf(outstr, "%.3e ", val);
      out << outstr;
    }
  }
}

//////////////////////////////////////////////////////
// compute angle as double

double HrdAngle(int binaryAngle)
{
  return ((double) binaryAngle * BINARY_TO_DEGREES);
}

#ifdef JUNK
  
/* c------------------------------------------------------------------------ */

int upk_hrd16 (unsigned short *dd,
               int bad_val,
               unsigned short *ss,
               unsigned short *zz,
               int *bads)
{

  /*
   * routine to unpacks actual data assuming MIT/HRD compression
   */

  unsigned short *us=ss;
  int ii=0, mm, nn, loop=0, nw=0, mark, last_mm=0;
  static int count=0;
  
  count++;
  *bads = 0;

  while(1) {
    loop++;
    if(*us == END_OF_COMPRESSION || *us == 0)
      return(nw);
    last_mm = mm;
    mm = nn = *us & MASK15;

    if( *us & SIGN16 ) {	/* data! */
      us++;
      if(us+nn >= zz) {
        return(nw);
      }
      *bads = 0;
      for(; nn--;) {
        nw++;
        *dd++ = *us++;
      }
    }	
    else {			/* fill with nulls */
      if(*bads) {
        return(nw);	/* some other kind of termination flag */
      }
      if( nw +nn > 1536 ) {
        return(nw);
      }
      *bads = nn;
      us++;
      for(; nn--;) {
        nw++;
        *dd++ = bad_val;
      }
    }
  }
}

/* c------------------------------------------------------------------------ */

int 
  upk_hrd16LE ( /* hostIsLittleEndian() version! */
          unsigned short *dd,
          int bad_val,
          unsigned short *ss,
          unsigned short *zz,
          int *bads
                )
{
  /*
   * routine to unpacks actual data assuming MIT/HRD compression
   */
  unsigned short *us=ss, rlcw;
  int ii=0, mm, nn, loop=0, nw=0, mark, last_mm=0;
  static int count=0;
  unsigned char *aa, *bb;

  aa = (unsigned char *)&rlcw;

  count++;
  *bads = 0;

  while(1) {
    loop++;
    /* nab the run length code word
     */
    bb = (unsigned char *)us;
    *aa = *(bb+1);
    *(aa+1) = *bb;

    if(rlcw == END_OF_COMPRESSION || rlcw == 0)
      return(nw);
    last_mm = mm;
    mm = nn = rlcw & MASK15;

    if( rlcw & SIGN16 ) {	/* data! */
      us++;
      if(us+nn >= zz) {
        return(nw);
      }
      *bads = 0;
# ifdef probably_not
      swack_short(us, dd, nn);
# else
      /* the data is really byte data so it shouldn't be swapped
       */
      memcpy(dd, us, nn*sizeof(short));
# endif
      nw += nn;
      dd += nn;
      us += nn;
    }	
    else {			/* fill with nulls */
      if(*bads) {
        return(nw);	/* some other kind of termination flag */
      }
      if( nw +nn > 1536 ) {
        return(nw);
      }
      *bads = nn;
      us++;
      for(; nn--;) {
        nw++;
        *dd++ = bad_val;
      }
    }
  }
}

#endif

#ifdef JUNK

char *hrd_buf=NULL;
char *comments;

static struct hrd_header *head=NULL, *xhead;
static struct hrd_radar_info *LF, *xLF;
static struct hrd_radar_info *TA, *xTA;
static struct hrd_data_rec_header *hdrh, *xhdrh;
static struct hrd_ray_header *hrh, *xhrh;

static struct mc_ray_header *mrh, *xmrh=NULL;
static short *mc_flags = NULL;
static int32_t *mc_lvals = NULL;

//static struct hrd_general_info *hgi;
static struct generic_radar_info *gri;
static struct hrd_useful_items *hui;
static struct hrd_generic_header *gh, xgh;

static float Aft_ccw_limit=-26., Aft_cw_limit=-10.;
static float Fore_ccw_limit=10., Fore_cw_limit=26.;

static struct HRD_for_each_radar *hfer[MAX_RADARS];
static struct id_stack_entry *Top_id_stack=NULL;
static struct dd_input_filters *difs;
static struct dd_stats *dd_stats=NULL;
static struct input_read_info *irq;
static char preamble[24], postamble[64];
static char *pbuf=NULL, **pbuf_ptrs=NULL;
static int pbuf_max=1200;
static struct d_limits *d_limits=NULL;
static int lf_surveilence_gs = 300;
static char *current_file_name;


/* c------------------------------------------------------------------------ */

void 
  hrd_dd_conv (int interactive_mode)
{
  int n, rn, count=0, mark, trip=111, nok_count=0;

  if(!count) {
    hrdx_ini();		/* initialize and read in first ray */
  }

  if(interactive_mode) {
    //    dd_intset();
    hrd_positioning();
  }
  hrd_reset();


  /*
   * loop through the data
   */

  while(1){
    count++;
    if(count >= trip) {
      mark = 0;
    }

    if(difs->stop_flag &&
       (difs->abrupt_start_stop || hui->new_sweep)) {
      printf("Stop time!\n");
      hrd_print_stat_line(count);
      break;
    }
    if(hrd_select_ray()) {
      rn = hui->current_radar_ndx;

      if(hui->new_sweep) {
        if(difs->max_sweeps) {
          if(++hfer[rn]->sweep_count > difs->max_sweeps)
            break;
        }
        if(hui->new_vol && difs->max_vols) {
          if(++hfer[rn]->vol_count > difs->max_vols)
            break;
        }
      }
      if(difs->max_beams) {
        if(++hfer[rn]->ray_count > difs->max_beams )
          break;
      }
      if(!difs->catalog_only) {
        hrd_nab_data();
      }
      dd_stuff_ray();	/* pass it off to dorade code */
    }
    else {
      if(!(nok_count++ % 1000)) {
        hrd_print_stat_line(count);
      }
    }
    hui->new_sweep = NO;
    hui->new_vol = NO;

    if((n = hrd_next_ray()) < 1) {
      printf("Break on input status: %d\n", n);
      break;
    }
  }
}

/* c------------------------------------------------------------------------ */

void 
  hrd_fix_navs (void)
{
  /* routine to correct position information */
  int i;
  float f, f_pitch, x=0, y=0, z=0;
  double d;

  if(hui->hrd_code & LF_DATA_BIT) {
    /* don't apply corrections if LF radar */
    return;
  }
  gri->roll += hui->roll_correction;
  gri->azimuth += hui->azimuth_correction + gri->roll;
  gri->pitch += hui->pitch_correction;
  gri->drift += hui->drift_correction;
  gri->elevation += hui->elevation_correction;

  if( gri->drift > 25. )
    gri->track = gri->heading +25.;
  else if( gri->drift < -25. )
    gri->track = gri->heading -25.;
  else
    gri->track = gri->heading +gri->drift;

  gri->tilt = gri->elevation;

  /*
   * routine "correct" determines
   * 
   * x=pitch axis (right wing positive) perp. to track
   * y=roll axis (nose positive) parallel to track
   * z=yaw axis (up positive) perp. to track
   * 
   * and azimuth is calculated from these values and
   * made relative to true north
   * 
   */
  return;
}

/* c------------------------------------------------------------------------ */

void 
  hrd_gen_luts (void)
{
  /*
   * generate lookup tables for the various fields
   */
  int i, j, mark;
  short *ss;
  float step, nyq;

  for(i=0; i < gri->num_fields_present; i++ ) {
    ss = hui->hrd_lut[i];

    if( gri->fields_present[i] & REFLECTIVITY_BIT ) {
      gri->dd_scale[i] = 100.; /* for creating 16-bit data */
      gri->dd_offset[i] = 0;
      *ss++ = EMPTY_FLAG;
      for(j=1; j < 256; j++ ) {
        *ss++ = (short)(DD_SCALE((j-64)*.5, gri->dd_scale[i]
				 , gri->dd_offset[i]));
      }
    }
    else if( gri->fields_present[i] & VELOCITY_BIT ) {
      nyq = gri->nyquist_vel;
      step = nyq/127.;
      gri->dd_scale[i] = 100.;
      gri->dd_offset[i] = 0;
      *ss++ = EMPTY_FLAG;
      for(j=1; j < 256; j++ ) {
        *ss++ = (short)(DD_SCALE((j-128)*step, gri->dd_scale[i]
				 , gri->dd_offset[i]));
      }
    }
    else if( gri->fields_present[i] & WIDTH_BIT ) {
      nyq = gri->nyquist_vel;
      step = nyq/256.;
      gri->dd_scale[i] = 100.;
      gri->dd_offset[i] = 0;
      *ss++ = EMPTY_FLAG;
      for(j=1; j < 256; j++ ) {
        *ss++ = (short)(DD_SCALE(((float)j*step), gri->dd_scale[i]
				 , gri->dd_offset[i]));
      }
    }
    mark = 0;
  }
}

/* c------------------------------------------------------------------------ */

void 
  hrd_grab_header_info (void)
{
  char *c;
    
  if( head == NULL ) {
    head = (struct hrd_header *)malloc(2048);
    c = (char *)head;
    c += 100*sizeof(short);
    LF = (struct hrd_radar_info *)c;
    c += 300*sizeof(short);
    TA = (struct hrd_radar_info *)c;
    c += 300*sizeof(short);
    comments = c;
  }
  if(!hostIsLittleEndian()) {
    memcpy((char *)head, hrd_buf, 2048 ); /* Save header */
  }
  else {
    c = hrd_buf;
    //       hrd_crack_header(c, head, (int)0);  //Jul 26, 2011
    hrd_crack_header(c, (char *)head, (int)0);  //Jul 26, 2011 need to confirm
    c += 100*sizeof(short);
    //       hrd_crack_radar(c, LF, (int)0);  //Jul 26, 2011
    hrd_crack_radar(c, (char *)LF, (int)0);  //Jul 26, 2011 need to confirm
    c += 300*sizeof(short);
    //       hrd_crack_radar(c, TA, (int)0);  //Jul 26, 2011
    hrd_crack_radar(c, (char *)TA, (int)0);  //Jul 26, 2011 need to confirm
    c += 300*sizeof(short);
  }
  gri->nav_system = head->nav_system;
  hui->new_header = YES;
  hui->header_count++;
  dd_stats->vol_count++;
  hui->vol_count++;
  hui->new_vol = YES;
  hui->new_sweep = YES;
}

/* c------------------------------------------------------------------------ */

void 
  hrdx_ini (void)
{
  int ii, jj, n, nt;
  CHAR_PTR a, b;
  char string_space[256];//, str_ptrs[32]; //Jul 26, 2011
  char *str_ptrs[32];  //Jul 26, 2011 pass to function -> char *str_ptrs[]
  struct HRD_ray_que *rqnext, *rqlast;
  struct hrd_info_que *iqnext, *iqlast;

  gh = &xgh;
  dd_min_rays_per_sweep();	/* trigger min rays per sweep */
  difs = dd_return_difs_ptr();
  dd_stats = dd_return_stats_ptr();
  gri = return_gri_ptr();

  if(a=get_tagged_string("PROJECT_NAME")) {
    strcpy(gri->project_name, a);
  }
  else {
    strcpy(gri->project_name, "UNKNOWN");
  }
  if(a=get_tagged_string("SITE_NAME")) {
    strcpy(gri->site_name, a);
  }
  else {
    strcpy(gri->site_name, "UNKNOWN");
  }

  hui = (struct hrd_useful_items *)malloc(sizeof(struct hrd_useful_items));
  memset((char *)hui, 0, sizeof(struct hrd_useful_items));
  hui->sweep_trip_delta = 100.;

  for(ii=0; ii < HRD_INFO_QUE_COUNT; ii++) {
    iqnext = (struct hrd_info_que *)malloc(sizeof(struct hrd_info_que));
    memset(iqnext, 0, sizeof(struct hrd_info_que));
    if(ii) {
      iqnext->last = iqlast;
      iqlast->next = iqnext;
    }
    else {
      hui->info_que = iqnext;
    }
    hui->info_que->last = iqnext;
    iqnext->next = hui->info_que;
    iqnext->radar_id = -1;
    iqlast = iqnext;
  }
  hui->bang_zap = HRD_BANG_ZAP;
  if((a=get_tagged_string("FIRST_GOOD_GATE"))) {
    if((ii=atoi(a)) > 0)
      hui->bang_zap = ii-1;
  }
  printf("First good gate is %d\n", hui->bang_zap);
    
  for(ii=0; ii < MAX_RADARS; ii++ ) {
    hfer[ii] = (struct HRD_for_each_radar *)
      malloc(sizeof(struct HRD_for_each_radar));
    memset((char *)hfer[ii], 0, sizeof(struct HRD_for_each_radar));
    hfer[ii]->radar_num = -1;
    for(jj=0; jj < HRD_RAY_QUE_COUNT; jj++) {
      rqnext = (struct HRD_ray_que *)malloc(sizeof(struct HRD_ray_que));
      memset(rqnext, 0, sizeof(struct HRD_ray_que));
      if(jj) {
        rqnext->last = rqlast;
        rqlast->next = rqnext;
      }
      else {
        hfer[ii]->ray_que = rqnext;
      }
      rqnext->rotation_angle = -360.;
      hfer[ii]->ray_que->last = rqnext;
      rqnext->next = hfer[ii]->ray_que;
      rqlast = rqnext;
    }
  }
  if(hostIsLittleEndian()) {
# ifdef notneeded
    head = xhead = (struct hrd_header *)malloc(sizeof(struct hrd_header));
    memset(xhead, 0, sizeof(struct hrd_header));
    LF = xLF = (struct hrd_radar_info *)malloc(sizeof(struct hrd_radar_info));
    memset(xLF, 0, sizeof(struct hrd_radar_info));
    TA = xTA = (struct hrd_radar_info *)malloc(sizeof(struct hrd_radar_info));
    memset(xTA, 0, sizeof(struct hrd_radar_info));
# endif
    hdrh = xhdrh = (struct hrd_data_rec_header *)malloc(sizeof(struct hrd_data_rec_header));
    memset(xhdrh, 0, sizeof(struct hrd_data_rec_header));
  }
  hrh = xhrh = (struct hrd_ray_header *)malloc(sizeof(struct hrd_ray_header));
  memset(xhrh, 0, sizeof(struct hrd_ray_header));


  if(a=get_tagged_string("SWEEP_TRIP_ANGLE")) {
    hui->sweep_trip_angle = atof(a);
    hui->options |= SWEEP_TRIP_ANGLE;
    printf( "Sweep trip angle: %.2f\n", hui->sweep_trip_angle);
  }
  if(a=get_tagged_string("SWEEP_TRIP_DELTA")) {
    hui->sweep_trip_delta = atof(a);
    hui->options |= SWEEP_TRIP_DELTA;
  }
  printf( "Sweep trip delta: %.2f\n", hui->sweep_trip_delta);

  if(a=getenv("MC_DATA")) {
    hui->options |= MC_DATA;
    if (hostIsLittleEndian()) {
      mrh = xmrh = (struct mc_ray_header *)
        malloc(sizeof(struct mc_ray_header));
      memset(xmrh, 0, sizeof(struct mc_ray_header));
    }
    dd_set_unique_vol_id();
    mc_flags = (short *)malloc (64 * sizeof (short));
    memset (mc_flags, 0, 64 * sizeof (short));
    mc_lvals = (int32_t *)malloc (2048 * sizeof (int32_t));
    memset (mc_lvals, 0, 2048 * sizeof (int32_t));
  }
  printf( "MC_DATA option set\n");

  if(a=get_tagged_string("HRD_RANGE_DELAY")) {
    hui->hrd_range_delay = atoi(a);
  }
  if(a=get_tagged_string("HRD_IO_TYPE")) {
    if(strstr(a, "BINARY")) {
      hui->io_type = BINARY_IO;
    }
  }

  if(a=get_tagged_string("AFT_ANGLE_LIMITS")) {
    strcpy(string_space, a);
    nt = dd_tokens(string_space, str_ptrs);
    dd_get_lims(str_ptrs, 0, nt, &d_limits);

    if(d_limits->lower != -MAX_FLOAT && d_limits->upper != MAX_FLOAT) {
      Aft_ccw_limit = d_limits->lower;
      Aft_cw_limit = d_limits->upper;
    }
  }
  printf("Aft_angle_limits: %.1f %.1f\n", Aft_ccw_limit, Aft_cw_limit);

  if(a=get_tagged_string("FORE_ANGLE_LIMITS")) {
    strcpy(string_space, a);
    nt = dd_tokens(string_space, str_ptrs);
    dd_get_lims(str_ptrs, 0, nt, &d_limits);

    if(d_limits->lower != -MAX_FLOAT && d_limits->upper != MAX_FLOAT) {
      Fore_ccw_limit = d_limits->lower;
      Fore_cw_limit = d_limits->upper;
    }
  }
  printf("Fore_angle_limits: %.1f %.1f\n", Fore_ccw_limit, Fore_cw_limit);

  if(a=get_tagged_string("KEEP_ORTHOGONAL_DATA")) {
    hui->options |= KEEP_ORTHOGONAL_DATA;
  }

  hui->azimuth_correction=0.;
  hui->elevation_correction=0.;
  hui->pitch_correction=0.;
  hui->roll_correction=0.;
  hui->drift_correction=0.;
  hui->min_ray_size=sizeof(struct hrd_ray_header)+sizeof(short);
  hui->max_rec_size = 8192;
  hui->new_header = NO;
  hui->header_count = 0;
  hui->radar_count = 0;

  gh = (struct hrd_generic_header *)hrd_buf;
  hdrh = (struct hrd_data_rec_header *)hrd_buf;

  gri->binary_format = DD_16_BITS;
  gri->missing_data_flag = EMPTY_FLAG;
  gri->source_format = HRD_FMT;
  gri->num_fields_present=0;
  gri->h_beamwidth = 1.35;
  gri->v_beamwidth = 1.90;
  gri->polarization = DD_POLAR_VERTICAL;
  gri->rcv_bandwidth = 2.;	/* MHz */
  gri->num_freq = 1;
  gri->num_ipps = 1;
  gri->compression_scheme = NO_COMPRESSION;

  for(ii=0; ii < HRD_MAX_FIELDS; ii++ ) {
    /* lookup tables */
    hui->hrd_lut[ii] = (short *)malloc(256*sizeof(short));
  }
  /*
   * open up the input file
   */
  irq = dd_return_ios(HRD_IO_INDEX, HRD_FMT);

  if(a=get_tagged_string("HRD_VOLUME_HEADER")) {
    /* nab a volume header from another file
     */
    dd_input_read_open(irq, a);
    n = hrd_next_ray();
    dd_input_read_close(irq);
  }
  if((a=get_tagged_string("SOURCE_DEV"))) {
    a = dd_establish_source_dev_names((char *)"HRD", a);
    a = dd_next_source_dev_name((char *)"HRD");
    current_file_name = a;
    dd_input_read_open(irq, a);
  }

  hrd_next_ray();
}
/* c------------------------------------------------------------------------ */

int hrd_inventory(struct input_read_info *irq)
{
  /* the purpose of this routine is to facilitate a more detailed
   * examination of the data
   */
  int ii=0, jj, max=gri_max_lines();
  int nn, ival;
  float val;
  char str[256];


  for(;;) {
    for(ii=0; ii < max; ii++) {
      if((nn = hrd_next_ray()) < 1)
        break;
      hui->new_sweep = NO;
      hui->new_vol = NO;
      sprintf(preamble, "%2d", irq->top->b_num);
      hrd_try_grint(gri, 0, preamble, postamble);
    }
    if(nn < 1)
      break;
    printf("Hit <ret> to continue: ");
    nn = getreply(str, sizeof(str));
    if(cdcode(str, nn, &ival, &val) != 1 || ival) {
      break;
    }
  }
  return(ival);
}
/* c------------------------------------------------------------------------ */

void 
  hrd_nab_data (void)
{
  int i, j, k, n, ng, mark, final_bad_count;
  short words[3072];
  int bins[HRD_MAX_GATES];
  short *dst, *lut;
  unsigned short *ss, *zz;
  static int count=0, trip=234;


  count++;
  if (hui->options & MC_DATA) {
    hrd_upk_mc_data ();
    return;
  }

  ss = (unsigned short *)(hui->hrd_ray_pointer
                          + sizeof(struct hrd_ray_header));
  zz = (unsigned short *)(hui->hrd_ray_pointer+hrh->sizeof_ray);

  if(hostIsLittleEndian()) {
    i = upk_hrd16LE((unsigned short *)words, 0, ss, zz, &final_bad_count);  //Jul 26, 2011
  }
  else {
    i = upk_hrd16((unsigned short *)words, 0, ss, zz, &final_bad_count);  //Jul 26, 2011
  }
  /*
   * seperate and   rescale the data
   */
  for(i=0; i < gri->num_fields_present; i++ ) {
    ng = gri->num_bins;
    /* unpack and rescale the data */
    k = hui->raw_field_position[i];
    hrd_upk_data( k, hui->num_raw_fields
                  , gri->num_bins, (unsigned char *)words, (unsigned int *)bins );  //Jul 26, 2011
    lut = hui->hrd_lut[i];
    dst = gri->scaled_data[i];
    if(hui->hrd_range_delay) {
      /* get set up for range delay
       */
      j = hui->hrd_range_delay;
      ng -= j;
    }
    else {
      j = 0;
    }	
    /* zap ? gates due to big bang containination */
    for(n=j+hui->bang_zap; j < n; j++ ) {
      *dst++ = EMPTY_FLAG;
    }
    for(; j < ng; j++ ) {
      *dst++ = *(lut+bins[j]);
    }
    for(; ng++ < gri->num_bins;) {
      *dst++ = EMPTY_FLAG;
    }
  }
}
/* c------------------------------------------------------------------------ */

void 
  hrd_nab_info (void)
{
  /*
   * stash all the useful info from hrd headers in a struct
   */
  int ii, jj, mark, new_radar=NO;
  int c1, c2, c3, c4, its_changed=NOTHING;
  static int count=0, trip=0, drn; /* drn: DORADE radar number */
  double diff, d;
  static float rot1=79., rot2=82.;
  float elev, ra, rb, rot;
  struct HRD_for_each_radar *hur;

  count++;
  drn = hui->current_radar_ndx;
  hur = hfer[drn];		/* pointer to info for last radar */

  gri->time = hrd_time_stamp(hrh, gri->dts);
  gri->altitude_agl = gri->altitude = hrh->altitude_xe3;
  gri->latitude = BA2F( hrh->latitude );
  gri->longitude = BA2F( hrh->longitude );

  /* c...mark */
  d = fmod((double)BA2F(hrh->azimuth)+360., (double)360.);
  gri->rotation_angle = gri->corrected_rotation_angle = gri->azimuth = d;
  gri->tilt = gri->elevation = BA2F( hrh->elevation );
  gri->heading = BA2F( hrh->ac_heading );
  gri->drift = BA2F( hrh->ac_drift );
  gri->pitch = BA2F( hrh->ac_pitch );
  gri->roll = BA2F( hrh->ac_roll );
  gri->vns = US10(hrh->ac_vns_x10);
  gri->vew = US10(hrh->ac_vew_x10);
  gri->vud = US10(hrh->ac_vud_x10);
  gri->ui = US10(hrh->ac_ui_x10);
  gri->vi = US10(hrh->ac_vi_x10);
  gri->wi = US10(hrh->ac_wi_x10);

  if(gri->azimuth > rot1 && gri->azimuth < rot2) {
    mark = 0;
  }
  hui->hrd_code = ((short)hrh->code) << 8;
  hui->radar_id = TAIL_RADAR_ID;
  hrd_fix_navs();

  if(c1=in_sector((float)gri->elevation, Aft_ccw_limit, Aft_cw_limit)) {
    hui->radar_id = T_AFT_RADAR_ID;
  }
  if(c2=in_sector((float)gri->elevation, Fore_ccw_limit, Fore_cw_limit)) {
    hui->radar_id = T_FOR_RADAR_ID;
  }
  if(!(c1 || c2)) {
    mark = 0;
  }
  c3 = fabs((double)gri->elevation) < ORTHOGONAL_TOLERANCE;

  /* attempt to toss out junk beams */
  hui->ok_ray = (c1 || c2) ? YES : NO;

  if( hui->hrd_code & LF_DATA_BIT ) {
    hui->radar_id = LF_RADAR_ID;
    if(LF->bin_spacing_xe3 < lf_surveilence_gs) {
      hui->radar_id |= LF_SECTOR_RADAR_ID;
    }
    hui->ok_ray = YES;
  }
  if(!hui->ok_ray && KEEP_ORTHOGONAL_DATA SET_IN hui->options) {
    hui->ok_ray = YES;
  }
  /*
   * see if anything major has changed
   */
  if(hui->new_header) {
    its_changed |= NEW_HEADER;
  }
  if(hui->hrd_code != hur->hrd_code) {
    its_changed |= DIFFERENT_CODE;
  }
  if(hui->info_que->radar_id != hui->radar_id) {
    its_changed |= DIFFERENT_RADAR;
  }

  if(its_changed) {
    hui->info_que = hui->info_que->next;
    hui->info_que->radar_id = hui->radar_id;
    /*
     * assign a dorade radar number
     * i.e. we want to indicate to dorade which of
     * n radars we are talking about
     */
    for(ii=0,drn = -1; ii < MAX_RADARS; ii++) {
      if(hfer[ii]->radar_num < 0)
        continue;
      if( hfer[ii]->this_radar_id == hui->radar_id ) {
        drn = gri->dd_radar_num = hfer[ii]->radar_num;
        hur = hfer[drn];
        break;
      }
    }
    new_radar = drn == -1 ? YES : NO;
    /*
     * construct the radar name
     */
    if( hui->hrd_code & TA_DATA_BIT ) {
      hui->hri = TA;		/* point to radar specific info */
      if( hui->radar_id == TAIL_RADAR_ID ) {
        strcpy( gri->radar_name, "TL" );
        gri->radar_type = DD_RADAR_TYPE_AIR_TAIL;
        gri->scan_mode = DD_SCAN_MODE_AIR;
      }
      else if( hui->radar_id == T_AFT_RADAR_ID ) {
        strcpy( gri->radar_name, "TA" );
        gri->radar_type = DD_RADAR_TYPE_AIR_AFT;
        gri->scan_mode = DD_SCAN_MODE_AIR;
      }
      else {
        strcpy( gri->radar_name, "TF" );
        gri->radar_type = DD_RADAR_TYPE_AIR_FORE;
        gri->scan_mode = DD_SCAN_MODE_AIR;
      }
    }
    else {			/* lower fuselage */
      hui->hri = LF;
      if(hui->radar_id & LF_SECTOR_RADAR_ID) {
        strcpy( gri->radar_name, "LFV" );
        gri->scan_mode = DD_SCAN_MODE_PPI;
      }
      else {
        strcpy( gri->radar_name, "LFS" );
        gri->scan_mode = DD_SCAN_MODE_SUR;
      }
      gri->radar_type = DD_RADAR_TYPE_AIR_LF;
      gri->h_beamwidth = 1.1;
      gri->v_beamwidth = 4.1;
    }
    sprintf(gri->radar_name+strlen(gri->radar_name)
            , "%d", head->aircraft_id);
    strcat(gri->radar_name, "P3");

    if(new_radar) {
      ii = hui->radar_count++;
      hfer[ii]->this_radar_id = hui->radar_id;
      drn = gri->dd_radar_num = hfer[ii]->radar_num =
        dd_assign_radar_num(gri->radar_name);
      dd_radar_selected(gri->radar_name, gri->dd_radar_num, difs);
      hui->new_sweep = YES;
      hur = hfer[drn];
      strcpy(hur->radar_name, gri->radar_name);
    }
    /*
     * update fields present
     */
    gri->num_fields_present = hui->num_raw_fields = 0;
	
    if( hui->hrd_code & REFLECTIVITY_BIT ) {
      ii = gri->num_fields_present++;
      hui->raw_field_position[ii] = hui->num_raw_fields++;
      gri->fields_present[ii] = REFLECTIVITY_BIT;
      gri->dd_scale[ii] = 100.;
      gri->dd_offset[ii] = 0;
      gri->field_id_num[ii] = DZ_ID_NUM;
      strcpy( gri->field_name[ii], "DZ      " );
      strncpy( gri->field_long_name[ii]
               , "Reflectivity factor                                     "
               , 40);
      strcpy( gri->field_units[ii], "dBz     " );
    }
    if( hui->hrd_code & VELOCITY_BIT ) {
      ii = gri->num_fields_present++;
      jj = hui->raw_field_position[ii] = hui->num_raw_fields++;
      gri->fields_present[ii] = VELOCITY_BIT;
      gri->dd_scale[ii] = 100.;
      gri->dd_offset[ii] = 0;
      gri->field_id_num[ii] = VE_ID_NUM;
      strcpy( gri->field_name[ii], "VE" );
      strncpy( gri->field_long_name[ii]
               , "Doppler velocity                                         "
               , 40);
      strcpy( gri->field_units[ii], "m/s     " );
    }
    if( hui->hrd_code & WIDTH_BIT ) {
      ii = gri->num_fields_present++;
      hui->raw_field_position[ii] = hui->num_raw_fields++;
      gri->fields_present[ii] = WIDTH_BIT;
      gri->dd_scale[ii] = 100.;
      gri->dd_offset[ii] = 0;
      gri->field_id_num[ii] = SW_ID_NUM;
      strcpy( gri->field_name[ii], "SPEC_WDT" );
      strncpy( gri->field_long_name[ii]
               , "Spectral width                                            "
               , 40);
      strcpy( gri->field_units[ii], "m/s     " );
    }
    hrd_update_header_info();
    hrd_gen_luts();
    hui->new_header = NO;
    hur->hrd_code = hui->hrd_code;

    for(ii=0; ii < gri->num_fields_present; ii++ ) {
      gri->actual_num_bins[ii] = gri->num_bins;
    }
  }
  hui->current_radar_ndx = drn;

  ra = hur->ray_que->rotation_angle;
  hur->ray_que = hur->ray_que->next;
  rb = hur->ray_que->rotation_angle = gri->rotation_angle;
  hur->ray_que->real_diff =  hur->ray_que->diff = diff = angdiff(ra, rb);
  hur->ray_que->elevation = gri->elevation;
  hur->ray_que->time = gri->time;


  if(fabs(diff + hur->delta_angle_sum) > 360.) {
    its_changed |= OVER_ROTATED;
  }
  if(fabs(diff) > hui->sweep_trip_delta) {
    its_changed |= SWEEP_TRIP_DELTA;
  }
  if(ii = in_sector(hui->sweep_trip_angle, ra, rb)) {
    its_changed |= SWEEP_TRIP_ANGLE;
  }
  if(fabs(diff) > .3 && fabs(hur->ray_que->last->diff) > .3 &&
     ((diff + hur->ray_que->last->diff) * hur->delta_angle_sum) < 0) {
    /* sign change in this ray and the previous ray
     * implies antenna reversing (sector scans we hope) */
    its_changed |= ANTENNA_DIRECTION;
  }
  if(hur->hrd_sweep_num != hdrh->sweep_num) {
    its_changed |= SWEEP_NUMBER;
    gri->source_sweep_num = hdrh->sweep_num;
  }

  if(its_changed) {
    /* see if we really have a new sweep
     */
    if(DIFFERENT_RADAR SET_IN its_changed) {
      if(hui->radar_id & LF_RADAR_ID) {
        if(SWEEP_NUMBER SET_IN its_changed) {
          hui->new_sweep = YES;
        }
      }
      else if(!(hui->info_que->last->radar_id & LF_RADAR_ID)) {
        /* antenna shifting between fore and aft */
        hui->new_sweep = YES;
      }
      else if(hui->info_que->last->last->radar_id != hui->radar_id) {
        /*
         * the prev ray was LF data, but the LF data
         * really seperates data from two different radars
         */
        hui->new_sweep = YES;
      }
    }
    else {
      if(OVER_ROTATED SET_IN its_changed) {
        hui->new_sweep = YES;
      }
      if(ANTENNA_DIRECTION SET_IN its_changed) {
        hui->new_sweep = YES;
      }
      /*
        if(SWEEP_TRIP_DELTA SET_IN hui->options
        && SWEEP_TRIP_DELTA SET_IN its_changed) {
      */
      if(SWEEP_TRIP_DELTA SET_IN its_changed) {
        hui->new_sweep = YES;
      }
      if(SWEEP_TRIP_ANGLE SET_IN hui->options
         && SWEEP_TRIP_ANGLE SET_IN its_changed) {
        hui->new_sweep = YES;
      }
    }

    if(hui->new_sweep == YES) {
      dd_stats->sweep_count++;
      hur->sweep_num++;
      hur->ray_num = 0;
      diff = hur->ray_que->diff = hur->delta_angle_sum = 
        hur->fixed_angle_sum = 0;
    }
    hur->hrd_sweep_num = hdrh->sweep_num;
  }

  gri->sweep_num = hur->sweep_num;
  hur->delta_angle_sum += diff;
  hur->fixed_angle_sum += gri->elevation;
  hur->ray_num++;
  hur->ray_count++;
  hrd_merge_std(gri);
  /*
   * try to do a running average of elevations to get a fixed angle
   */
  gri->fixed = hur->fixed_angle_sum/(float)hur->ray_num;
}
/* c------------------------------------------------------------------------ */

int 
  hrd_next_block (void)
{
  /* returns a pointer to the next block of hrd data
   * which corresponds to a physical record on the tape
   */
  struct hrd_generic_header *hgh;
  static int sgh=sizeof(struct hrd_generic_header), count=0, bp=298;
  int ii, mm, nn, mark, eof_count=0;
  int size=-1;
  char *aa, *bb, *cc = NULL;
  char mess[256];


  if(!count++) {
    mark = 0;
  }
    
  if(count >= bp) {
    mark = 0;
  }

  for(;;) {
    if(irq->top->bytes_left < hui->min_ray_size) {
      if(irq->io_type == BINARY_IO && irq->top->bytes_left > 0) {
        dd_io_reset_offset(irq, irq->top->offset
                           + irq->top->sizeof_read
                           - irq->top->bytes_left);
        irq->top->bytes_left = 0;
      }
      dd_logical_read(irq, FORWARD);
      if(irq->top->read_state < 0) {
        printf("Last read: %d\n", irq->top->read_state);
        nn = irq->top->read_state;
        dd_input_read_close(irq);
        /*
         * see if there is another file to read
         */
        if(aa = dd_next_source_dev_name((char *)"HRD")) {
          current_file_name = aa;
          if((ii = dd_input_read_open(irq, aa)) <= 0) {
            return(size);
          }
        }
        else {
          return(size);
        }
        continue;
      }
      else if(irq->top->eof_flag) {
        eof_count++;
        dd_stats->file_count++;
        printf( "EOF number: %d at %.2f MB\n"
                , dd_stats->file_count
                , dd_stats->MB);

        irq->top->bytes_left = 0;
        if(eof_count > 3) {
          return(size);
        }
        continue;
      }
      else {
        eof_count = 0;
      }
    }
    aa = irq->top->at;
	
    if(hostIsLittleEndian()) {
      gh = &xgh;
      //	   swack_short(aa, gh, 2);  //Jul 26, 2011
      swack_short(aa, (char *)gh, 2);  //Jul 26, 2011
    }
    else {
      gh = (struct hrd_generic_header *)aa;
    }
    if (hui->options & MC_DATA) {
      return (irq->top->sizeof_read);
    }

    if(!(gh->header_id == 0 || gh->header_id == 1)) {
      sprintf(mess, 
              "Illegal header id: %d  size: %d  sizeof_read: %d %s %s"
              , gh->header_id, gh->sizeof_rec
              , irq->top->sizeof_read
              , current_file_name
              , dts_print(d_unstamp_time(gri->dts)));
      dd_append_cat_comment(mess);
      printf("%s\n", mess);

      if(irq->io_type != BINARY_IO) {
        irq->top->bytes_left = 0;
        continue;
      }
      if(gh->sizeof_rec > hui->max_rec_size) {
        dd_input_read_close(irq);
        if(aa = dd_next_source_dev_name((char *)"HRD")) {
          if((ii = dd_input_read_open(irq, aa)) <= 0) {
            return(size);
          }
        }
        else {
          return(size);
        }
        irq->top->bytes_left = 0;
        continue;
      }
      /* otherwise try to reposition after the current bad block
       */
      dd_io_reset_offset(irq, irq->top->offset
                         + irq->top->sizeof_read
                         - irq->top->bytes_left
                         + gh->sizeof_rec);
      irq->top->bytes_left = 0;
      continue;
    }
    if(irq->io_type == BINARY_IO) {
      if(gh->sizeof_rec > irq->top->bytes_left) {
        dd_io_reset_offset(irq, irq->top->offset
                           + irq->top->sizeof_read
                           - irq->top->bytes_left);
        irq->top->bytes_left = 0;
      }
      else {
        break;
      }
    }
    else {
      break;		/* for non binary io */
    }
  }
  size = gh->sizeof_rec;

  return(size);
}
/* c------------------------------------------------------------------------ */

int 
  hrd_next_ray (void)
{
  /* read in the next ray */
  static int whats_left=0, rec_num=0, ray_num=0;
  int ii, nn, eof_count=0, got_it=NO, size, mark;
  struct io_packet *dp;
  char *aa;
  DD_TIME dts;
  double dtime;
  int mc_data_really = NO, ndata, nwords16;


  if(irq->top->bytes_left < hui->min_ray_size) {
    /* could be a rewind of a file skip
     */
    whats_left = 0;
  }

  for(;;) {
    if(dd_control_c()) {
      dd_reset_control_c();
      return(-1);
    }
    if(whats_left < hui->min_ray_size) {
      if((size = hrd_next_block()) < hui->min_ray_size) {
        return(-1);
      }
      hrd_buf = irq->top->at;

      if(hostIsLittleEndian()) {
        //	       swack_short(irq->top->at, gh, 2);  //Jul 26, 2011
        swack_short(irq->top->at, (char *)gh, 2);  //Jul 26, 2011
        hdrh = xhdrh;
        //	       hrd_crack_hdrh(irq->top->at, hdrh, (int)0);  //Jul 26, 2011
        hrd_crack_hdrh(irq->top->at, (char *)hdrh, (int)0);  //Jul 26, 2011
      }
      else {
        gh = (struct hrd_generic_header *)irq->top->at;
        hdrh = (struct hrd_data_rec_header *)irq->top->at;
      }
      dd_stats->MB += BYTES_TO_MB(size);
      dd_stats->rec_count++;
      rec_num++;
      if (hui->options & MC_DATA) {
        if( strncmp (irq->top->buf, "P3", 2) == 0 &&
            irq->top->read_state == 2048 ) {
          hrd_grab_header_info();
        }
        else if (irq->top->read_state == 2048) {
          /* this is the cell vector */
        }
        else if (irq->top->read_state == 84) {
          /* ray header (all floats) */
          if (hostIsLittleEndian()) {
            //		     swack_long (irq->top->buf, xmrh, 21);  //Jul 26, 2011
            swack_long (irq->top->buf, (char *)xmrh, 21);  //Jul 26, 2011
          }
          else {
            mrh = (struct mc_ray_header *)irq->top->buf;
          }
          /* populate hrd equivalent struct */

#define SCALEANG (1./ANGSCALE)
#define F2BA(x) ((short)(((x) < 0) ? (x)*SCALEANG-.5 : (x)*SCALEANG +.5))
          hrh->year = head->year;
          hrh->month = head->month;
          hrh->day = head->day;
          hrh->sizeof_ray = irq->top->read_state;
          ii = REFLECTIVITY_BIT;
          ii |= VELOCITY_BIT;
          ii |= WIDTH_BIT;
          ii |= TA_DATA_BIT;
          hrh->code = (unsigned char)(ii >> 8);

          gri->source_sweep_num = (int) mrh->ntasweep;  //Jul 26, 2011 casting
          hrh->latitude     = F2BA (mrh->latitude);    /* (binary angle) */
          hrh->longitude    = F2BA (mrh->longitude);   /* (binary angle)  */
          hrh->altitude_xe3 = (short) (mrh->altitude_xe3);
          /* these velocities are scaled by 100. */
          hrh->ac_vew_x10   = (short) (.1 *  (mrh->ac_vew_x10));   /* east-west velocity */
          hrh->ac_vns_x10   = (short) (.1 *  (mrh->ac_vns_x10));   /* north-south velocity */
          hrh->ac_vud_x10   = (short) (.1 *  (mrh->ac_vud_x10));   /* vertical velocity */
          hrh->ac_ui_x10    = (short) (.1 *  (mrh->ac_ui_x10));    /* east-west wind */
          hrh->ac_vi_x10    = (short) (.1 *  (mrh->ac_vi_x10));    /* north-south wind */
          hrh->ac_wi_x10    = (short) (.1 *  (mrh->ac_wi_x10));    /* vertical wind */
          hrh->elevation    = F2BA (mrh->elevation);   /* (binary angle) */
          hrh->azimuth      = F2BA (mrh->azimuth);     /* (binary angle) word_12 */
          hrh->ac_pitch     = F2BA (mrh->ac_pitch);    /* (binary angle) */
          hrh->ac_roll      = F2BA (mrh->ac_roll);     /* (binary angle) */
          hrh->ac_drift     = F2BA (mrh->ac_drift);    /* (binary angle) */
          hrh->ac_heading   = F2BA (mrh->ac_heading);  /* (binary angle) word_16 */
          //Jul 26, 2011 start
          //		  hrh->hour         =  (mrh->hour);                                           
          //		  hrh->minute       =  (mrh->minute);          
          //		  hrh->seconds_x100 =  (mrh->seconds_x100);    
          hrh->hour         =  static_cast<char>(mrh->hour);                                           
          hrh->minute       =  static_cast<char>(mrh->minute);          
          hrh->seconds_x100 =  static_cast<char>(mrh->seconds_x100);    
          //Jul 26, 2011 end
          mark = 0;
        }
        else {
          /* flags plus data */
          mc_data_really = YES;
          nwords16 = (int)mrh->nwords16;
          ndata = (int)mrh->ndata;
          nn = nwords16 * sizeof (short);

          if (hostIsLittleEndian()) {
            swack_short (irq->top->buf, (char *)mc_flags, nwords16);  //Jul 26, 2011
            swack_long (irq->top->buf+nn, (char *)mc_lvals, ndata);  //Jul 26, 2011
          }
          else {
            memcpy (mc_flags, irq->top->buf, nn);
            memcpy (mc_lvals, irq->top->buf+nn, ndata*sizeof(int32_t));
          }
        }

        irq->top->bytes_left = 0;
        whats_left = 0;
        if (mc_data_really)
        { break;}
        continue;
      }
      else if( gh->header_id == 0 && gh->sizeof_rec == 2048 ) {
        /* Header!
         */
        hrd_grab_header_info();
        irq->top->at += gh->sizeof_rec;
        irq->top->bytes_left -= gh->sizeof_rec;
        whats_left = 0;
      }
      else if( gh->header_id == 1 ) {
        /* Data!
         */
        nn = sizeof(struct hrd_data_rec_header);
        if(hostIsLittleEndian()) {
          //		   hrd_crack_hrh(irq->top->at +nn, hrh, (int)0);  //Jul 26, 2011
          hrd_crack_hrh(irq->top->at +nn, (char*)hrh, (int)0);  //Jul 26, 2011
        }
        else {
          hrh = (struct hrd_ray_header *)(irq->top->at +nn);
        }
        if(hrh->year < 0 || hrh->year > 125 || hrh->month < 1 ||
                                                            hrh->month > 12 || hrh->day < 1 || hrh->day > 31 ||
           hrh->hour < 0 || hrh->hour >= 24 || hrh->minute < 0 ||
                                                             hrh->minute > 60) {
          irq->top->at += gh->sizeof_rec;
          irq->top->bytes_left -= gh->sizeof_rec;
          whats_left = 0;
        }
        else {
          whats_left = hdrh->sizeof_rec -nn;
          irq->top->at += nn;
          irq->top->bytes_left -= nn;
          break;
        }
      }
      else {
        /* Illegal header id! */
        whats_left = 0;
        mark = 0;
      }
    }
    else {
      if(hostIsLittleEndian()) {
        //	      hrd_crack_hrh(irq->top->at, hrh, (int)0);  //Jul 26, 2011
        hrd_crack_hrh(irq->top->at, (char *)hrh, (int)0);  //Jul 26, 2011
      }
      else {
        hrh = (struct hrd_ray_header *)irq->top->at;
      }
      if(hrh->year < 0 || hrh->year > 115 || hrh->month < 1 ||
                                                          hrh->month > 12 || hrh->day < 1 || hrh->day > 31 ||
         hrh->hour < 0 || hrh->hour >= 24 || hrh->minute < 0 ||
                                                           hrh->minute > 60) {
        irq->top->bytes_left += whats_left; /* of the current block */
        irq->top->at += whats_left;
        whats_left = 0;
      }
      else {
        break;
      }
    }
  }

  dd_stats->ray_count++;
  ray_num++;

  hui->hrd_ray_pointer = irq->top->at;
  dp = dd_return_next_packet(irq);
  dp->len = hrh->sizeof_ray;
  irq->top->at += hrh->sizeof_ray;
  if (!(hui->options & MC_DATA)) {
    irq->top->bytes_left -= hrh->sizeof_ray;
    whats_left -= hrh->sizeof_ray;
  }

  if(head)
    hrd_nab_info();

  return((int)hrh->sizeof_ray);
}
/* c------------------------------------------------------------------------ */

void 
  hrd_positioning (void)
{
  int ii, mm, jj, kk, mark, direction;
  int nn, ival;
  float val;
  char str[256];
  DD_TIME dts;
  static double skip_secs=0;
  double d, dtarget;
  struct io_packet *dp, *dp0;
  static int view_char=0, view_char_count=200;


  hrd_try_grint(gri, 1, preamble, postamble);

  pbuf = (char *)malloc(80*pbuf_max);
  memset(pbuf, 0, 80*pbuf_max);
  pbuf_ptrs = (char **)malloc(sizeof(char *)*pbuf_max);
  memset(pbuf_ptrs, 0, sizeof(char *)*pbuf_max);
  *pbuf_ptrs = pbuf;


 menu2:
  printf("\n\
-2 = Exit\n\
-1 = Begin processing\n\
 0 = Skip rays\n\
 1 = Inventory\n\
 2 = Skip files \n\
 3 = Rewind\n\
 4 = 16-bit integers\n\
 5 = hex display\n\
 6 = dump as characters\n\
 7 = Skip records\n\
 8 = Forward time skip\n\
 9 = Set input limits\n\
10 = Main header\n\
11 = LF radar header\n\
12 = TAil radar header\n\
13 = Data record header\n\
14 = Ray data\n\
Option = "
         );
  nn = getreply(str, sizeof(str));
  if( cdcode(str, nn, &ival, &val) != 1 || ival < -2 || ival > 14 ) {
    if(ival == -2) exit(1);
    printf( "\nIllegal Option!\n" );
    goto menu2;
  }

  if(ival == -1) {
    free(pbuf);
    free(pbuf_ptrs);
    return;
  }
  else if(ival < 0)
    exit(0);

  hui->new_sweep = NO;
  hui->new_vol = NO;

  if(ival == 0) {
    printf("Type number of rays to skip:");
    nn = getreply(str, sizeof(str));
    if(cdcode(str, nn, &ival, &val) != 1 || fabs((double)val) > K64) {
      printf( "\nIllegal Option!\n" );
      goto menu2;
    }
    if(ival > 0) {
      for(ii=0; ii < ival; ii++) {
        hui->new_sweep = NO;
        hui->new_vol = NO;
        if((jj=hrd_next_ray()) < 0)
          break;
      }
    }
    else if(ival < 0) {
      /* figure out the number of rays we can really back up
       */
      nn = -ival > irq->packet_count ? irq->packet_count-1 : -ival;
      dp0 = dp = dd_return_current_packet(irq);
      for(ii=0; ii < nn; ii++, dp=dp->last) {
        /* see if the sequence num for this packet
         * agrees with the sequence num for the input buf
         * if so, the data has not been stomped on
         */
        if(!dp->ib || dp->seq_num != dp->ib->seq_num)
          break;
      }
      if(ii) {
        dp = dp->next;	/* loop pushed us one too far */
        /*
         * figure out how many buffers to skip back
         */
        kk = ((dp0->ib->b_num - dp->ib->b_num + irq->que_count)
              % irq->que_count) +1;
        dd_skip_recs(irq, BACKWARD, kk);
        irq->top->at = dp->at;
        irq->top->bytes_left = dp->bytes_left;
      }
      if(ii < -ival) {
        /* did not achive goal */
        printf("Unable to back up %d rays. Backed up %d rays\n"
               , -ival, ii);
        printf("Use record positioning to back up further\n");
      }
    }
    hrd_next_ray();
    hrd_try_grint(gri, 1, preamble, postamble);
  }
  else if(ival == 1) {
    if((nn = hrd_inventory(irq)) == -2)
      exit(0);
  }
  else if(ival == 2 && irq->io_type == BINARY_IO) {
  }
  else if(ival == 2) {
    printf("Type skip: ");
    nn = getreply(str, sizeof(str));
    if(cdcode(str, nn, &ival, &val) != 1 || fabs((double)val) > K64) {
      printf( "\nIllegal Option!\n" );
      goto menu2;
    }
    if(ival) {
      direction = ival >= 0 ? FORWARD : BACKWARD;
      dd_skip_files(irq, direction, ival >= 0 ? ival : -ival);
    }
    hrd_next_ray();
    hrd_try_grint(gri, 1, preamble, postamble);
  }
  else if(ival == 3) {
    dd_rewind_dev(irq);
    hrd_next_ray();
    hrd_try_grint(gri, 1, preamble, postamble);
  }
  else if(ival == 4) {
    if(gri_start_stop_chars(&view_char, &view_char_count) >= 0) {
      printf("\n");
      ctypeu16((unsigned char *)irq->top->buf, view_char, view_char_count);  //Jul 26, 2011
    }
  }
  else if(ival == 5) {
    if(gri_start_stop_chars(&view_char, &view_char_count) >= 0) {
      printf("\n");
      ezhxdmp(irq->top->buf, view_char, view_char_count);
    }
  }
  else if(ival == 6) {
    if(gri_start_stop_chars(&view_char, &view_char_count) >= 0) {
      printf("\n");
      ezascdmp(irq->top->buf, view_char, view_char_count);
    }
  }
  else if(ival == 7) {
    printf("Type record skip # or hit <return> to read next rec:");
    nn = getreply(str, sizeof(str));
    if(cdcode(str, nn, &ival, &val) != 1 || fabs((double)val) > K64) {
      printf( "\nIllegal Option!\n" );
      goto menu2;
    }
    direction = ival >= 0 ? FORWARD : BACKWARD;
    dd_skip_recs(irq, direction, ival > 0 ? ival : -ival);
    hrd_next_ray();
    nn = irq->top->sizeof_read;
    printf("\n Read %d bytes\n", nn);
    hrd_try_grint(gri, 1, preamble, postamble);
  }
  else if(ival == 8) {
    dtarget = 0;
    dd_clear_dts(&dts);

    printf("Type target time (e.g. 14:22:55 or +2h, +66m, +222s): ");
    nn = getreply(str, sizeof(str));
    if(cdcode(str, nn, &ival, &val) == 1 && !ival) {
      /* assume they have hit a <return>
       */
      if(skip_secs > 0)
        dtarget = gri->time + skip_secs; 
    }
    else if(skip_secs = dd_relative_time(str)) {
      dtarget = gri->time + skip_secs;
    }
    else if(kk = dd_crack_datime(str, nn, &dts)) {
      if(!dts.year) dts.year = gri->dts->year;
      if(!dts.month) dts.month = gri->dts->month;
      if(!dts.day) dts.day = gri->dts->day;
      dtarget = d_time_stamp(&dts);
    }
    if(!dtarget) {
      printf( "\nCannot derive a time from %s!\n", str);
      goto menu2;
    }
    printf("Skipping ahead %.3f secs\n", dtarget-gri->time);
    /* loop until the target time
     */
    for(mm=1;; mm++) {
      if((nn = hrd_next_ray()) <= 0 || gri->time >= dtarget ||
         dd_control_c()) {
        break;
      }
      if(!(mm % 1000))
        hrd_try_grint(gri, 1, preamble, postamble);
      mark = 0;
    }
    if(nn)
      hrd_try_grint(gri, 1, preamble, postamble);
  }
  else if(ival == 9) {
    gri_nab_input_filters(gri->time, difs, 1);
  }
  else if(ival == 10) {
    if(head)
      hrd_print_head();
  }
  else if(ival == 11) {
    if(head)
      hrd_print_hri(LF);
  }
  else if(ival == 12) {
    if(head)
      hrd_print_hri(TA);
  }
  else if(ival == 13) {
    hrd_print_hdrh();
  }
  else if(ival == 14) {
    hrd_print_hrh(pbuf_ptrs);
  }
  preamble[0] = '\0';

  goto menu2;


}
/* c------------------------------------------------------------------------ */

void 
  hrd_print_hdrh (void)
{
  char *aa=pbuf, **cptr=pbuf_ptrs;


  *cptr = aa;

  sprintf(*(cptr++), " ");
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "data_record_flag %d ", hdrh->data_record_flag); 	       
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "sizeof_rec       %d ", hdrh->sizeof_rec);       	       
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "sweep_num        %d ", hdrh->sweep_num);        	       
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "rec_num          %d ", hdrh->rec_num);          	       
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "radar_num        %d ", hdrh->radar_num);        	       
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "rec_num_flag     %d ", hdrh->rec_num_flag);     	       
  *cptr = (aa += strlen(aa)+1);

  *cptr = NULL;

  gri_print_list(pbuf_ptrs);
}
/* c------------------------------------------------------------------------ */

void 
  hrd_print_head (void)
{
  char *aa=pbuf, **cptr=pbuf_ptrs;
  char str[32];


  *cptr = aa;

  sprintf(*(cptr++), " ");
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "header_flag       %d ", head->header_flag      );
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "sizeof_header     %d ", head->sizeof_header    );
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "tape_num          %d ", head->tape_num         );
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "hd_fmt_ver        %d ", head->hd_fmt_ver       );
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "word_5            %d ", head->word_5           );
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "year              %d ", head->year             );
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "month             %d ", head->month            );
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "day               %d ", head->day              );
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "hour              %d ", head->hour             );
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "minute            %d ", head->minute           );
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "second            %d ", head->second           );
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "LF_menu           %s "
          , str_terminate(str, head->LF_menu, 16));
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "TA_menu           %s "
          , str_terminate(str, head->TA_menu , 16));
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "Data_menu         %s "
          , str_terminate(str, head->Data_menu, 16));
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "word_36           %d ", head->word_36          );
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "nav_system        %d ", head->nav_system       );
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "LU_tape_drive     %d ", head->LU_tape_drive    );
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "aircraft_id       %d ", head->aircraft_id      );
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "flight_id         %s "
          , str_terminate(str, head->flight_id, 8));
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "data_header_len   %d ", head->data_header_len  );
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "ray_header_len    %d ", head->ray_header_len   );
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "time_zone_offset  %d ", head->time_zone_offset );
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "word_47           %d ", head->word_47          );
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), " ");
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "word_80           %d ", head->word_80          );
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "project_id        %s "
          , str_terminate(str, head->project_id, 8));
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "word_85           %d ", head->word_85          );

  *cptr = NULL;

  gri_print_list(pbuf_ptrs);
}
/* c------------------------------------------------------------------------ */

void 
  hrd_print_hrh (char **ptrs)
{
  int ii, jj, gg, nf;
  char **pt=ptrs;
  char str[32];
  float unscale[7], bias[7];

  sprintf(*pt, " ");
  *(pt+1) = *pt +strlen(*pt) +1;
  sprintf(*(++pt), "Contents of ray header");
  *(pt+1) = *pt +strlen(*pt) +1;
  sprintf(*(++pt), "sizeof_ray   %d ", hrh->sizeof_ray);     	       
  *(pt+1) = *pt +strlen(*pt) +1;
  sprintf(*(++pt), "code         %2x hex", hrh->code);           	       
  *(pt+1) = *pt +strlen(*pt) +1;
  sprintf(*(++pt), "year         %d ", hrh->year);           	       
  *(pt+1) = *pt +strlen(*pt) +1;
  sprintf(*(++pt), "month        %d ", hrh->month);          	       
  *(pt+1) = *pt +strlen(*pt) +1;
  sprintf(*(++pt), "day          %d ", hrh->day);            	       
  *(pt+1) = *pt +strlen(*pt) +1;
  sprintf(*(++pt), "raycode      %2x hex", hrh->raycode);        	       
  *(pt+1) = *pt +strlen(*pt) +1;
  sprintf(*(++pt), "hour         %d ", hrh->hour);           	       
  *(pt+1) = *pt +strlen(*pt) +1;
  sprintf(*(++pt), "minute       %d ", hrh->minute);         	       
  *(pt+1) = *pt +strlen(*pt) +1;
  sprintf(*(++pt), "seconds_x100 %5.2f ", hrh->seconds_x100);   	       
  *(pt+1) = *pt +strlen(*pt) +1;
  sprintf(*(++pt), "latitude     %9.4f ", BA2F(hrh->latitude));       	       
  *(pt+1) = *pt +strlen(*pt) +1;
  sprintf(*(++pt), "longitude    %9.4f ", BA2F(hrh->longitude));      	       
  *(pt+1) = *pt +strlen(*pt) +1;
  sprintf(*(++pt), "altitude_xe3 %d ", hrh->altitude_xe3);   	       
  *(pt+1) = *pt +strlen(*pt) +1;
  sprintf(*(++pt), "ac_vew_x10   %6.1f ", .1*hrh->ac_vew_x10);     	       
  *(pt+1) = *pt +strlen(*pt) +1;
  sprintf(*(++pt), "ac_vns_x10   %6.1f ", .1*hrh->ac_vns_x10);     	       
  *(pt+1) = *pt +strlen(*pt) +1;
  sprintf(*(++pt), "ac_vud_x10   %6.1f ", .1*hrh->ac_vud_x10);     	       
  *(pt+1) = *pt +strlen(*pt) +1;
  sprintf(*(++pt), "ac_ui_x10    %6.1f ", .1*hrh->ac_ui_x10);      	       
  *(pt+1) = *pt +strlen(*pt) +1;
  sprintf(*(++pt), "ac_vi_x10    %6.1f ", .1*hrh->ac_vi_x10);      	       
  *(pt+1) = *pt +strlen(*pt) +1;
  sprintf(*(++pt), "ac_wi_x10    %6.1f ", .1*hrh->ac_wi_x10);      	       
  *(pt+1) = *pt +strlen(*pt) +1;
  sprintf(*(++pt), "RCU_status   %4x hex ", hrh->RCU_status);     	       
  *(pt+1) = *pt +strlen(*pt) +1;
  sprintf(*(++pt), "elevation    %7.2f ", BA2F(hrh->elevation));      	       
  *(pt+1) = *pt +strlen(*pt) +1;
  sprintf(*(++pt), "azimuth      %7.2f ", BA2F(hrh->azimuth));        	       
  *(pt+1) = *pt +strlen(*pt) +1;
  sprintf(*(++pt), "ac_pitch     %7.2f ", BA2F(hrh->ac_pitch));       	       
  *(pt+1) = *pt +strlen(*pt) +1;
  sprintf(*(++pt), "ac_roll      %7.2f ", BA2F(hrh->ac_roll));        	       
  *(pt+1) = *pt +strlen(*pt) +1;
  sprintf(*(++pt), "ac_drift     %7.2f ", BA2F(hrh->ac_drift));       	       
  *(pt+1) = *pt +strlen(*pt) +1;
  sprintf(*(++pt), "ac_heading   %7.2f ", BA2F(hrh->ac_heading));                    
  *(pt+1) = *pt +strlen(*pt) +1;
    
  hrd_nab_data();
  nf = gri->num_fields_present < 7 ? gri->num_fields_present : 7;

  sprintf(*(++pt), " ");
  *(pt+1) = *pt +strlen(*pt) +1;
  sprintf(*(++pt), "Contents of data fields");
  *(pt+1) = *pt +strlen(*pt) +1;
  sprintf(*(++pt), "name ");

  for(ii=0; ii < nf; ii++) {
    sprintf(*pt+strlen(*pt), "        %s"
            , str_terminate(str, gri->field_name[ii], 8));
    unscale[ii] = gri->dd_scale[ii] ? 1./gri->dd_scale[ii] : 1.;
    bias[ii] = gri->dd_offset[ii];
  }
  *(pt+1) = *pt +strlen(*pt) +1;

  for(gg=0; gg < gri->num_bins; gg++) {
    sprintf(*(++pt), "%4d)", gg);

    for(ii=0; ii < nf; ii++) {
      sprintf(*pt+strlen(*pt), "%10.2f",
              DD_UNSCALE(*(gri->scaled_data[ii]+gg), unscale[ii]
                         , bias[ii]));
    }
    *(pt+1) = *pt +strlen(*pt) +1;
  }
  *(++pt) = NULL;
  gri_print_list(ptrs);
}
/* c------------------------------------------------------------------------ */

void 
  hrd_print_hri (struct hrd_radar_info *hri)
{
  char *aa=pbuf, **cptr=pbuf_ptrs;


  *cptr = aa;
    
  sprintf(*(cptr++), " ");
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "sample_size           %d ", hri->sample_size);          
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "DSP_flag              %d ", hri->DSP_flag);             
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "refl_slope_x4096      %d ", hri->refl_slope_x4096);     
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "refl_noise_thr_x16    %d ", hri->refl_noise_thr_x16);   
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "clutter_cor_thr_x16   %d ", hri->clutter_cor_thr_x16);  
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "SQI_thr               %d ", hri->SQI_thr);              
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "width_power_thr_x16   %d ", hri->width_power_thr_x16);  
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "calib_refl_x16        %d ", hri->calib_refl_x16);       
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "AGC_decay_code        %d ", hri->AGC_decay_code);       
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "dual_PRF_stabil_delay %d ", hri->dual_PRF_stabil_delay);
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "thr_flags_uncorr_refl %d ", hri->thr_flags_uncorr_refl);
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "word_112              %d ", hri->word_112);             
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "thr_flags_vel         %d ", hri->thr_flags_vel);        
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "thr_flags_width       %d ", hri->thr_flags_width);      
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "data_mode             %d ", hri->data_mode);            
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "word_116              %d ", hri->word_116);             
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), " ");						       
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "word_140              %d ", hri->word_140);             
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "range_b1              %d ", hri->range_b1);             
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "variable_spacing_flag %d ", hri->variable_spacing_flag);
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "bin_spacing_xe3       %d ", hri->bin_spacing_xe3);      
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "num_input_bins        %d ", hri->num_input_bins);       
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "range_avg_state       %d ", hri->range_avg_state);      
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "b1_adjust_xe4         %d ", hri->b1_adjust_xe4);        
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "word_147              %d ", hri->word_147);             
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "word_148              %d ", hri->word_148);             
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "num_output_bins       %d ", hri->num_output_bins);      
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "word_150              %d ", hri->word_150);             
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "PRT_noise_sample      %d ", hri->PRT_noise_sample);     
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "range_noise_sample    %d ", hri->range_noise_sample);   
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "log_rec_noise_x64     %d ", hri->log_rec_noise_x64);    
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "I_A2D_offset_x256     %d ", hri->I_A2D_offset_x256);    
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "Q_A2D_offset_x256     %d ", hri->Q_A2D_offset_x256);    
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "word_156              %d ", hri->word_156);             
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), " ");						       
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "word_171              %d ", hri->word_171);             
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "waveln_xe4            %d ", hri->waveln_xe4);           
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "pulse_width_xe8       %d ", hri->pulse_width_xe8);      
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "PRF                   %d ", hri->PRF);                  
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "word_175              %d ", hri->word_175);             
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "DSS_flag              %d ", hri->DSS_flag);             
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "trans_recv_number     %d ", hri->trans_recv_number);    
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "transmit_power        %d ", hri->transmit_power);       
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "gain_control_flag     %d ", hri->gain_control_flag);    
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "word_180              %d ", hri->word_180);             
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), " ");						       
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "word_190              %d ", hri->word_190);             
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "scan_mode             %d ", hri->scan_mode);            
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "sweep_speed_x10       %d ", hri->sweep_speed_x10);      
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "tilt_angle            %d ", hri->tilt_angle);           
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "sector_center         %d ", hri->sector_center);        
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "sector_width          %d ", hri->sector_width);         
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), "word_196              %d ", hri->word_196);             
  *cptr = (aa += strlen(aa)+1);
  sprintf(*(cptr++), " ");						       
  *cptr = (aa += strlen(aa)+1);

  *cptr = NULL;

  gri_print_list(pbuf_ptrs);
}
/* c------------------------------------------------------------------------ */

void 
  hrd_print_stat_line (int count)
{
  time_t i;
  double d;
  DD_TIME dts;

  d = gri->time;
  dts.time_stamp = gri->time;
  d_unstamp_time(&dts);
  i = (time_t)d;
  printf("%5d %3d  %s  %.3f %s f:%2d r:%4d %.2f MB\n"
         , count
         , hdrh->sweep_num
         , gri->radar_name
         , dts.time_stamp
         , dts_print(d_unstamp_time(&dts))
         , dd_stats->file_count
         , dd_stats->rec_count
         , dd_stats->MB
         );
}
/* c------------------------------------------------------------------------ */

void 
  hrd_reset (void)
{
  int ii;

  /* make sure we start up again on a new scan
   * and a new volume
   */
  for(ii=0; ii < hui->radar_count; ii++) {
    hfer[ii]->vol_count = 
      hfer[ii]->ray_count = hfer[ii]->sweep_count = 0;
    hfer[ii]->sweep_num++;
    if(ii == hui->current_radar_ndx ? 1 : 0) {
      gri->sweep_num = hfer[ii]->sweep_num;
    }
    else {
      hfer[ii]->hrd_sweep_num = -1;
      hfer[ii]->ray_que->rotation_angle = -360;
      hfer[ii]->ray_que->diff = 0;
      hfer[ii]->delta_angle_sum = 0;
      hfer[ii]->fixed_angle_sum = 0;
    }
  }
# ifdef maybe_not
  for(ii=0; ii < HRD_INFO_QUE_COUNT; ii++) {
  }
# endif
  hui->info_que->last->last->radar_id = hui->info_que->last->radar_id = -1;
  gri->vol_num = ++hui->header_count;
  hui->new_sweep = YES;
  hui->new_vol = YES;
  difs->stop_flag = difs->abrupt_start_stop = NO;
}
/* c------------------------------------------------------------------------ */

int 
  hrd_select_ray (void)
{
  int i, ok, mark;

  ok = hui->ok_ray;

  if((hui->hrd_code & TIME_SERIES_BIT))
    ok = NO;

  if(gri->time >= difs->final_stop_time)
    difs->stop_flag = YES;

  if(difs->num_time_lims) {
    for(i=0; i < difs->num_time_lims; i++ ) {
      if(gri->time >= difs->times[i]->lower &&
         gri->time <= difs->times[i]->upper)
        break;
    }
    if(i == difs->num_time_lims)
      ok = NO;
  }

  if(!difs->radar_selected[gri->dd_radar_num]) 
    ok = NO;

  if(difs->sweep_skip) {
    if(!((gri->sweep_num-1) % difs->sweep_skip)) {
      mark = 0;
    }
    else
      ok = NO;
  }

  if(difs->num_modes) {
    if(difs->modes[gri->scan_mode]) {
      mark = 0;
    }
    else
      ok = NO;
  }

  return(ok);
}
/* c------------------------------------------------------------------------ */

double 
  hrd_time_stamp (struct hrd_ray_header *hrh, DD_TIME *dts)
{

  double d;
  int yy = hrh->year;

  dts->day_seconds =
    (double)hrh->seconds_x100*.01 +60.*hrh->minute +3600.*hrh->hour;
  yy = yy < 70 ? yy +2000 : yy +1900;
  dts->year = yy;
  dts->month = hrh->month;
  dts->day = hrh->day;
  d = d_time_stamp(dts);
  return(d);
}
/* c------------------------------------------------------------------------ */

void 
  hrd_try_grint (struct generic_radar_info *gri, int verbose, char *preamble, char *postamble)
{
  if(head)
    gri_interest(gri, verbose, preamble, postamble);
  else {
    printf("No HRD header record yet...skip to the next file\n");
  }
}
/* c------------------------------------------------------------------------ */

void 
  hrd_update_header_info (void)
{
  int ii;
  int at150 = 256 - hui->hrd_range_delay;
  int at300 = 384 - hui->hrd_range_delay;
  float r;
  double dd, prt0, prt1;

  gri->vol_num = hui->header_count;
  /* keep distances in meters */
  gri->range_b1 = hui->hri->range_b1*1000.
    + hui->hri->b1_adjust_xe4*.1;
  gri->bin_spacing = hui->hri->bin_spacing_xe3;
  gri->wavelength = hui->hri->waveln_xe4*.0001; /* meters! */
  gri->PRF = hui->hri->PRF;
  gri->pulse_width = hui->hri->pulse_width_xe8*1.e-8; /* seconds */
}
 else {
   gri->pulse_width = hui->hri->pulse_width_xe8*1.e-9; /* seconds */
 }
gri->peak_power = hui->hri->transmit_power
  /(gri->pulse_width * gri->PRF);

gri->freq[0] = SPEED_OF_LIGHT/gri->wavelength *1.e-9; /* GHz */
prt0 = gri->ipps[0] =  1./gri->PRF;

if( hui->hri->DSP_flag & PRF_MASK ) {
  /* 2/3 or 3/4 PRF */
  dd = hui->hri->DSP_flag & DUAL_TWO_THIRDS_PRF ? .6666667 : .75;
  gri->ipps[1] = prt1 = 1./( gri->PRF * dd );
  gri->nyquist_vel = gri->wavelength * .25 / ( prt1 - prt0 );
  gri->num_ipps = 2;
 }
 else {
   gri->num_ipps = 1;
   gri->nyquist_vel = gri->wavelength*gri->PRF*.25;
 }
    
gri->num_bins = hui->hri->num_output_bins;
gri->sweep_speed = RPM_TO_DPS(hui->hri->sweep_speed_x10*.1);
strncpy(gri->project_name, head->project_id, 8 );
gri->project_name[8] = '\0';
strncpy(gri->flight_id, head->flight_id, 8 );

r = hui->hri->range_b1*1000. + hui->hri->b1_adjust_xe4*.1;

if(hui->hri->variable_spacing_flag) {
  /* set up range values */
  for(ii=0; ii < at150; ii++) { 
    gri->range_value[ii] = r;
    r += 75.;
  }
  for(; ii < at300; ii++) {
    gri->range_value[ii] = r;
    r += 150.;
  }
  for(; ii < 512; ii++) {
    gri->range_value[ii] = r;
    r += 300.;
  }
 }
 else {
   for(ii=0; ii < hui->hri->num_output_bins; ii++ ) {
     gri->range_value[ii] = r;
     r += hui->hri->bin_spacing_xe3;
   }
 }
}
/* c------------------------------------------------------------------------ */

//void hrd_upk_data (int fn, int nf, int nb, short *src, unsigned int *dst)  //Jul 26, 2011
void hrd_upk_data (int fn, int nf, int nb, unsigned char *src, unsigned int *dst)
{
  /*
   * unpacks uncompressed hrd data
   * where fn is the field number requested
   * where nf is the number of fields
   * and "nb" is the number of bins
   */
  int j, mark;

  for(j=0,src+=fn; j < nb; j++,src+=nf) {
    *dst++ = *src;
  }
}
/* c------------------------------------------------------------------------ */

void 
  hrd_upk_mc_data (void)
{
  /*
   * unpacks compressed mc data
   * "ng" is the number of bins
   * inside mc_data are 64 shorts followed by "ng" or less 32-bit longs
   */
  float ff;
  unsigned int ii, jj=0, kk, nn, gg, ng, mark, nbits;
  unsigned int mask9 = 0x1ff, mask14 = 0x3fff;
  unsigned short flag, mask1 = 0x8000;
  short *dbz, *vel, *sw, sval, *src16;
  unsigned int *src32, ival;
  int nwords16 = (int)mrh->nwords16;
  int ndata = (int)mrh->ndata;

#define S100(x) ((x)*100.0+0.5)
   
  ng = gri->num_bins;

  for(ii=0; ii < gri->num_fields_present; ii++ ) {
    if (gri->fields_present[ii] == REFLECTIVITY_BIT) {
      dbz = gri->scaled_data[ii];
    }
    if (gri->fields_present[ii] == VELOCITY_BIT) {
      vel = gri->scaled_data[ii];
    }
    if (gri->fields_present[ii] == WIDTH_BIT) {
      sw = gri->scaled_data[ii];
    }
  }
  mark = 0;

  for(gg = 0, jj = 0; gg < ndata ; gg++) {
    ii = gg/16;
    nbits = gg % 16;
    flag = (unsigned short)(mask1 >> nbits);

    if (mc_flags[ii] & (unsigned short)(mask1 >> nbits)) {
      /*
        memcpy (&ival, src32+jj++, sizeof (int32_t));
      */
      ival = mc_lvals[jj++];

      ff = .1 * ((ival) & mask9);
      sw[gg] = (short)S100 (ff);
      ff = .01 * ( (int)((ival >> 9) & mask14) -8000);
      vel[gg] = (short)S100 (ff);
      ff = .25 * ( (int)((ival >> 23) & mask9) -120);
      dbz[gg] = (short)S100 (ff);
    }
    else {
      dbz[gg] = vel[gg] = sw[gg] = EMPTY_FLAG;
    }
  }

  mark = 0;
  for (; gg < ng; gg++)
  { dbz[gg] = vel[gg] = sw[gg] = EMPTY_FLAG; }
}

/* c------------------------------------------------------------------------ */

int 
  upk_hrd16 (unsigned short *dd, int bad_val, unsigned short *ss, unsigned short *zz, int *bads)
{
  /*
   * routine to unpacks actual data assuming MIT/HRD compression
   */
  unsigned short *us=ss;
  int ii=0, mm, nn, loop=0, nw=0, mark, last_mm=0;
  static int count=0;

  count++;
  *bads = 0;

  while(1) {
    loop++;
    if(*us == END_OF_COMPRESSION || *us == 0)
      return(nw);
    last_mm = mm;
    mm = nn = *us & MASK15;

    if( *us & SIGN16 ) {	/* data! */
      us++;
      if(us+nn >= zz) {
        return(nw);
      }
      *bads = 0;
      for(; nn--;) {
        nw++;
        *dd++ = *us++;
      }
    }	
    else {			/* fill with nulls */
      if(*bads) {
        return(nw);	/* some other kind of termination flag */
      }
      if( nw +nn > 1536 ) {
        return(nw);
      }
      *bads = nn;
      us++;
      for(; nn--;) {
        nw++;
        *dd++ = bad_val;
      }
    }
  }
}
/* c------------------------------------------------------------------------ */

int 
  upk_hrd16LE ( /* hostIsLittleEndian() version! */
          unsigned short *dd,
          int bad_val,
          unsigned short *ss,
          unsigned short *zz,
          int *bads
                )
{
  /*
   * routine to unpacks actual data assuming MIT/HRD compression
   */
  unsigned short *us=ss, rlcw;
  int ii=0, mm, nn, loop=0, nw=0, mark, last_mm=0;
  static int count=0;
  unsigned char *aa, *bb;

  aa = (unsigned char *)&rlcw;

  count++;
  *bads = 0;

  while(1) {
    loop++;
    /* nab the run length code word
     */
    bb = (unsigned char *)us;
    *aa = *(bb+1);
    *(aa+1) = *bb;

    if(rlcw == END_OF_COMPRESSION || rlcw == 0)
      return(nw);
    last_mm = mm;
    mm = nn = rlcw & MASK15;

    if( rlcw & SIGN16 ) {	/* data! */
      us++;
      if(us+nn >= zz) {
        return(nw);
      }
      *bads = 0;
      /* the data is really byte data so it shouldn't be swapped
       */
      memcpy(dd, us, nn*sizeof(short));
      nw += nn;
      dd += nn;
      us += nn;
    }	
    else {			/* fill with nulls */
      if(*bads) {
        return(nw);	/* some other kind of termination flag */
      }
      if( nw +nn > 1536 ) {
        return(nw);
      }
      *bads = nn;
      us++;
      for(; nn--;) {
        nw++;
        *dd++ = bad_val;
      }
    }
  }
}

/* DSP flag definitions */
#define     RANGE_NORMALIZATION  0x1 /* bit 0 */
#define DOPL_CHAN_SPEKL_REMOVER  0x2 /* bit 1 */
#define   LOG_CHAN_SPEKL_REMOVER 0x4 /* bit 2 */
#define      PULSE_AT_END_OF_RAY 0x8 /* bit 3 */
#define    PULSE_AT START_OF RAY 0x10 /* bit 4 */
#define                  USE_AGC 0x40 /* bit 6 */
#define                 PRF_MASK 0x300 /* bits 8-9 */
#define      DUAL_TWO_THIRDS_PRF 0x100	/* bit 8 */
#define   DUAL_THREE_FOURTHS_PRF 0x200	/* bit 9 */

#define         MAX_ID_STACK 7
#define           MAX_RADARS 5
#define   MIN_RAYS_PER_SWEEP 6
#define    HRD_RAY_QUE_COUNT 11
#define   HRD_INFO_QUE_COUNT 7
#define         HRD_IO_INDEX 1

#define              NOTHING 0
#define    ANTENNA_DIRECTION 0x0001
#define      DIFFERENT_RADAR 0x0004
#define       DIFFERENT_CODE 0x0008
#define         SWEEP_NUMBER 0x0010
#define           NEW_HEADER 0x0020
#define     SWEEP_TRIP_ANGLE 0x0040
#define     SWEEP_TRIP_DELTA 0x0080
#define         OVER_ROTATED 0x0100
#define KEEP_ORTHOGONAL_DATA 0x0200
#define              MC_DATA 0x0400

#endif

