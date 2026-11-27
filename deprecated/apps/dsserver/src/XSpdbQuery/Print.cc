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
// Print.cc
//
// Printing class
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
//////////////////////////////////////////////////////////

#include "Print.hh"
#include <Spdb/Symprod.hh>
#include <rapformats/acarsXml.hh>
#include <rapformats/ac_data.h>
#include <rapformats/ac_posn.h>
#include <rapformats/acPosVector.hh>
#include <rapformats/Bdry.hh>
#include <rapformats/ComboPt.hh>
#include <rapformats/DsRadarSweep.hh>
#include <rapformats/Edr.hh>
#include <rapformats/flt_path.h>
#include <rapformats/fos.h>
#include <rapformats/GenPoly.hh>
#include <rapformats/ltg.h>
#include <rapformats/LtgGroup.hh>
#include <rapformats/NWS_WWA.hh>
#include <rapformats/pirepXml.hh>
#include <rapformats/SigAirMet.hh>
#include <rapformats/Sndg.hh>
#include <rapformats/trec_gauge.h>
#include <rapformats/tstorm_spdb.h>
#include <rapformats/VerGridRegion.hh>
#include <rapformats/zr.h>
#include <rapformats/zrpf.h>
#include <rapformats/WxObs.hh>
#include <rapformats/ZVis.hh>

#include <Spdb/sounding.h>
#include <Spdb/PosnRpt.hh>

#include <toolsa/udatetime.h>

using namespace std;

// Constructor

Print::Print(FILE *out, ostream &ostr) :
  _out(out), _ostr(ostr)

{
    ios::sync_with_stdio();
}

// Destructor

Print::~Print()

{

}

///////////////////////////////////////////////////////////////
// CHUNK_HDR

void Print::chunk_hdr(Spdb::chunk_ref_t *hdr)
{
  if(hdr->data_type >= 0) {
	if(hdr->data_type > 1048576) { // Must be a 3 or 4 char ID
      fprintf(_out, "data_type = %d (%s)\n", hdr->data_type,
	    Spdb::dehashInt32To4Chars(hdr->data_type).c_str());
	} else {
      fprintf(_out, "data_type = %d\n", hdr->data_type);
	}
  } else {
    fprintf(_out, "data_type = %d (%s)\n", hdr->data_type,
	  Spdb::dehashInt32To5Chars(hdr->data_type).c_str());
  }
  fprintf(_out, "data_type2 = %d\n", hdr->data_type2);
  fprintf(_out, "valid_time = %s\n", utimstr(hdr->valid_time));
  fprintf(_out, "expire_time = %s\n", utimstr(hdr->expire_time));
  fprintf(_out, "offset = %d\n", hdr->offset);
  fprintf(_out, "len = %d\n", hdr->len);
  return;
}

void Print::chunk_hdr(const Spdb::chunk_t &chunk)
{
  if(chunk.data_type >= 0) {
	if(chunk.data_type > 1048576) { // Must be a 3 or 4 char ID
      fprintf(_out, "data_type = %d (%s)\n", chunk.data_type,
	  Spdb::dehashInt32To4Chars(chunk.data_type).c_str());
	} else {
      fprintf(_out, "data_type = %d\n", chunk.data_type);
	}
  } else {
    fprintf(_out, "data_type = %d (%s)\n", chunk.data_type,
	  Spdb::dehashInt32To5Chars(chunk.data_type).c_str());
  }
  fprintf(_out, "data_type2 = %d\n", chunk.data_type2);
  fprintf(_out, "valid_time = %s\n", utimstr(chunk.valid_time));
  fprintf(_out, "expire_time = %s\n", utimstr(chunk.expire_time));
  fprintf(_out, "len = %d\n", chunk.len);
  return;
}

///////////////////////////////////////////////////////////////
// ASCII: Print ascii data

void Print::ascii(int data_len, void *data)
{
  
  char *str = (char *) data;
  // ensure null-terminated the string
  str[data_len - 1] = '\0';
  fprintf(_out, "\n");
  fprintf(_out, "ASCII data:\n");
  fprintf(_out, "  %s\n", str);
  
  return;
}

///////////////////////////////////////////////////////////////
// Print generic point data

void Print::generic_pt(int data_len, void *data)
{
  
  GenPt pt;
  if (pt.disassemble(data, data_len)) {
    cerr << "ERROR - Print::generic_pt" << endl;
    cerr << "  Cannot disassemble chunk." << endl;
    return;
  }
//  pt.print(_ostr);
  pt.print(_out);
  
  return;
}

///////////////////////////////////////////////////////////////
// ac_vector


void Print::ac_vector(int data_len, void *data)
{

  acPosVector pt;
  if (pt.disassemble(data, data_len)) {
    cerr << "ERROR - Print::generic_pt" << endl;
    cerr << "  Cannot disassemble chunk." << endl;
    return;
  }
  pt.print(_ostr);

  return;
}



///////////////////////////////////////////////////////////////
// Print combo generic point data

void Print::combo_pt(int data_len, void *data)
{
  
  ComboPt pt;
  if (pt.disassemble(data, data_len)) {
    cerr << "ERROR - Print::combo_pt" << endl;
    cerr << "  Cannot disassemble chunk." << endl;
    return;
  }
//  pt.print(_ostr);
  pt.print(_out);
  
  return;
}

///////////////////////////////////////////////////////////////
// STN_REPORT : Print station report

void Print::stn_report(void *data)
{

  station_report_t *report = (station_report_t *)data;
  
  // Swap the report data

  station_report_from_be(report);

  // Now print the fields.

  fprintf(_out, "\n");
  fprintf(_out, "Data for station report:\n");
  print_station_report(_out, "   ", report);
  fprintf(_out, "\n");
  
  return;
}

///////////////////////////////////////////////////////////////
// PIREP : Print PIREP data

void Print::pirep(int data_len, void *data)
{

  // Check for errors
  
  if (data_len < (int) sizeof(pirep_t)) {
    fprintf(_out, "  ERROR:  SpdbQuery::Print.pirep\n");
    fprintf(_out, "  Chunk size too small: %d bytes", data_len);
    fprintf(_out, "  Must be at least: %d bytes", (int) sizeof(pirep_t));
    return;
  }

  pirepXml pirep;
  if (pirep.disassemble(data, data_len)) {
    fprintf(_out, "  ERROR:  SpdbQuery::Print.pirep\n");
    fprintf(_out, "  Cannot disassemble buffer, len: %d bytes", data_len);
    return;
  }

  pirep.print(_out, "  ");
  fprintf(_out, "\n");

}

///////////////////////////////////////////////////////////////
// ACARS : Print acars data

void Print::acars(int data_len,  void *data)
{

  // Check for errors
  
  if (data_len < (int) sizeof(acars_t)) {
    fprintf(_out, "  ERROR:  SpdbQuery::Print.acars\n");
    fprintf(_out, "  Chunk size too small: %d bytes", data_len);
    fprintf(_out, "  Must be at least: %d bytes", (int) sizeof(acars_t));
    return;
  }

  acarsXml acars;
  if (acars.disassemble(data, data_len)) {
    fprintf(_out, "  ERROR:  SpdbQuery::Print.acars\n");
    fprintf(_out, "  Cannot disassemble buffer, len: %d bytes", data_len);
    return;
  }

  acars.print(_out, "  ");
  fprintf(_out, "\n");

}

///////////////////////////////////////////////////////////////
// AC_POSN : Print aircraft position data

void Print::ac_posn(int data_len,  void *data)
{

  ac_posn_t *ac_posn_data = (ac_posn_t *)data;
  int num_posn = data_len / sizeof(ac_posn_t);
  int posn;
  
  // Print the aircraft data.

  fprintf(_out, "\n");
  fprintf(_out, "Data for aircraft position (%d positions):\n", num_posn);

  // Check for errors
  
  if (data_len % sizeof(ac_posn_t) != 0) {
    fprintf(_out,
	    "  ERROR:  Chunk contains %d bytes, not an even "
	    "multiple of structure size (%d bytes)\n",
	    data_len, (int)sizeof(ac_posn_t));
  }
  
  for (posn = 0; posn < num_posn; posn++) {

    // Swap the aircraft data.

    BE_to_ac_posn(&ac_posn_data[posn]);
  
    fprintf(_out, "  position %d\n", posn);
    fprintf(_out, "   lat = %f\n", ac_posn_data[posn].lat);
    fprintf(_out, "   lon = %f\n", ac_posn_data[posn].lon);
    fprintf(_out, "   alt = %f\n", ac_posn_data[posn].alt);
    fprintf(_out, "   callsign = <%s>\n", ac_posn_data[posn].callsign);
    fprintf(_out, "\n");
    
  }
  
  fprintf(_out, "\n");

  return;

}

///////////////////////////////////////////////////////////////
// AC_POSN_WMOD : ac_posn_wmod data

void Print::ac_posn_wmod(int data_len,  void *data)
{

  ac_posn_wmod_t *ac_posn_data = (ac_posn_wmod_t *)data;
  int num_posn = data_len / sizeof(ac_posn_wmod_t);
  
  // Print the aircraft data.

  fprintf(_out, "\n");
  fprintf(_out, "Data for aircraft position (%d positions):\n", num_posn);

  // Check for errors
  
  if (data_len % sizeof(ac_posn_wmod_t) != 0) {
    fprintf(_out,
	    "  ERROR:  Chunk contains %d bytes, not an even "
	    "multiple of structure size (%d bytes)\n",
	    data_len, (int)sizeof(ac_posn_t));
  }
  
  for (int posn = 0; posn < num_posn; posn++) {

    // Swap the aircraft data.

    BE_to_ac_posn_wmod(&ac_posn_data[posn]);
    ac_posn_wmod_print(_out, "  ", &ac_posn_data[posn]);
    
  }
  
  fprintf(_out, "\n");

  return;

}

///////////////////////////////////////////////////////////////
// AC_DATA : Print aircraft data

void Print::ac_data(void *data)
{
  ac_data_t *ac_data = (ac_data_t *)data;
  ac_data_from_BE(ac_data);
  fprintf(_out, "\n");
  fprintf(_out, "Data for aircraft:\n");
  ac_data_print(_out, "   ", ac_data);
  fprintf(_out, "\n");
  return;
}

///////////////////////////////////////////////////////////////
// SPDB_POSN_RPT_ID : Print position report data


void Print::posn_rpt (void *data)
{
  PosnRpt *posn_report = new PosnRpt (data);

  fprintf(_out, "\n");
  fprintf(_out, "Data for position report:\n");
  posn_report->print(_out);
  fprintf(_out, "\n");
  return;
}

///////////////////////////////////////////////////////////////
// BDRY : Print boundary data

void Print::bdry(const int data_len, void *data)
{
  Bdry boundary;
  boundary.disassemble(data, data_len);
  fprintf(_out, "\n");
  fprintf(_out, "Data for boundary:\n");
  boundary.print(_out, true);
  fprintf(_out, "\n");
  return;

}

///////////////////////////////////////////////////////////////
// FLT_PATH : Print flight path data
//

void Print::flt_path(void *data)
{
  
  FLTPATH_path_t *path_data = (FLTPATH_path_t *)data;
  FLTPATH_path_from_BE(path_data);
  fprintf(_out, "\n");
  fprintf(_out, "Data for flight path:\n");
  FLTPATH_print_path(_out, path_data);
  fprintf(_out, "\n");
  return;

}

///////////////////////////////////////////////////////////////
// HISTORY_FORECAST : Print history forecast data
//

void Print::history_forecast(int data_len, void *data)
{   

  fprintf(_out, " Printing history forecast data - Not Supported\n");
  // ier = hf_chunk_native((hf_chunk_t **) &data);
  // print_hf_chunk(_out, (hf_chunk_t *) data, data_len);
}

///////////////////////////////////////////////////////////////
// LTG : Print lightning data
///

void Print::ltg(int data_len, void *data)
{

  // check for extended data

  if (data_len < (int) sizeof(ui32)) {
    return;
  }
  ui32 cookie;
  memcpy(&cookie, data, sizeof(cookie));

  if (cookie == LTG_EXTENDED_COOKIE) {

    int num_strikes = data_len / sizeof(LTG_extended_t);
    LTG_extended_t *ltg_data = (LTG_extended_t *)data;

    for (int strike = 0; strike < num_strikes; strike++) {
      LTG_extended_from_BE(&(ltg_data[strike]));
      fprintf(_out, "\n");
      fprintf(_out, "Data for strike %d:\n", strike);
      LTG_print_extended(_out, &(ltg_data[strike]));
      fprintf(_out, "\n");
    }

  } else {

    int num_strikes = data_len / sizeof(LTG_strike_t);
    LTG_strike_t *ltg_data = (LTG_strike_t *)data;

    for (int strike = 0; strike < num_strikes; strike++) {
      LTG_from_BE(&(ltg_data[strike]));
      fprintf(_out, "\n");
      fprintf(_out, "Data for strike %d:\n", strike);
      LTG_print_strike(_out, &(ltg_data[strike]));
      fprintf(_out, "\n");
    }

  }

}

///////////////////////////////////////////////////////////////
// LTG_GROUP : Print lightning group data
///

void Print::ltg_group(int data_len, void *data)
{
  LtgGroup group;
  
  if (group.disassemble(data, data_len) != 0)
  {
    cerr << "ERROR - SpdbQuery::Print::ltg_group" << endl;
    cerr << "  Cannot disassemble chunk" << endl;
    return;
  }
  
  group.print(_out);
}

///////////////////////////////////////////////////////////////
// SYMPROD : Print symbolic product data
//

void Print::symprod(int data_len, void *data)
{
  
  Symprod prod;
  if (prod.deserialize(data, data_len)) {
    cerr << "ERROR - Print::symprod" << endl;
    cerr << "  Cannot deserialze symprod data." << endl;
    cerr << "  " << prod.getErrStr() << endl;
    return;
  }
  _ostr << endl;
  _ostr << "Data for product: " << endl;
  prod.print(_ostr);
  _ostr << endl;
    
  return;
}

///////////////////////////////////////////////////////////////
// SIGMET : Print SIGMET data
//

void Print::sigmet(void *data)
{

  SIGMET_spdb_t *sigmet_data = (SIGMET_spdb_t *)data;
  SIGMET_spdb_from_BE(sigmet_data);
  fprintf(_out, "\n");
  fprintf(_out, "Data for SIGMET:\n");
  SIGMET_print_spdb(_out, sigmet_data);
  fprintf(_out, "\n");
  return;
}
///////////////////////////////////////////////////////////////
// SIGAIRMET : Data in the SigAirMet class
//
//

void Print::sigAirMet(int data_len, void *data)

{
	  SigAirMet sigmet;
	    if (sigmet.disassemble(data, data_len)) {
		    cerr << "ERROR - SpdbQuery::Print::sigAirMet" << endl;
		    cerr << "  Cannot disassemble chunk" << endl;
		    return;
		  }
		  sigmet.print(_ostr, "  ");
}

///////////////////////////////////////////////////////////////
// TSTORMS : Print TSTORMS data
//

void Print::tstorms(void *data)
{

  tstorm_spdb_buffer_from_BE((ui08 *) data);
  fprintf(_out, "\n");
  fprintf(_out, "Data for TSTORMS:\n");
  tstorm_spdb_print_buffer(_out, "  ", (ui08 *) data);
  fprintf(_out, "\n");
  return;

}

///////////////////////////////////////////////////////////////
// TREC_GAUGE : Print trec_gauge data
//

void Print::trec_gauge(int data_len, void *data)
{

  trec_gauge_handle_t tgauge;
  trec_gauge_init(&tgauge);
  trec_gauge_load_from_chunk(&tgauge, data, data_len);

  fprintf(_out, "\n");
  fprintf(_out, "Data for TREC GAUGE:\n");
  trec_gauge_print(_out, "  ", &tgauge);
  fprintf(_out, "\n");

  trec_gauge_free(&tgauge);

  return;
}

///////////////////////////////////////////////////////////////
// ZR_PARAMS : Print ZR params data
//

void Print::zr_params(void *data)
{

  zr_params_t *zrp = (zr_params_t *) data;
  zr_params_from_BE(zrp);

  fprintf(_out, "\n");
  fprintf(_out, "Data for ZR PARAMS:\n");
  zr_params_print(_out, "  ", zrp);
  fprintf(_out, "\n");

  return;

}

///////////////////////////////////////////////////////////////
// ZRPF : Print zr point forecast data
//

void Print::zrpf(int data_len, void *data)

{

  zrpf_handle_t handle;
  zrpf_handle_init(&handle);
  zrpf_handle_load_from_chunk(&handle, data, data_len);

  fprintf(_out, "\n");
  fprintf(_out, "Data for ZR POINT FORECAST DATA:\n");
  zrpf_handle_print(_out, "  ", &handle);
  fprintf(_out, "\n");

  zrpf_handle_free(&handle);

  return;

}

///////////////////////////////////////////////////////////////
// ZVPF : Print zv point forecast data
//
void Print::zvpf(int data_len, void *data)

{
  ZVis zvis;
  if (zvis.disassemble(data, data_len)) {
    cerr << "ERROR - SpdbQuery::Print::zvpf" << endl;
    cerr << "  Cannot disassemble chunk" << endl;
    return;
  }
  zvis.print(_ostr);

}

///////////////////////////////////////////////////////////////
// SNDG : Print sounding data
///

void Print::sndg(void *data)
{

  SNDG_spdb_product_t *sndg_data = (SNDG_spdb_product_t *)data;
  SNDG_spdb_product_from_BE(sndg_data);

  fprintf(_out, "\n");
  fprintf(_out, "Data for sounding:\n");
  SNDG_print_spdb_product(_out, sndg_data, TRUE);
  fprintf(_out, "\n");

  return;

}

///////////////////////////////////////////////////////////////
void Print::sndg_plus(int data_len, void *data)
{

  Sndg sndg;
  if (sndg.disassemble(data, data_len)) {
    cerr << "ERROR - Print::sndg_plus" << endl;
    cerr << "  Cannot disassemble chunk." << endl;
    return;
  }
//  sndg.print(_ostr);
  sndg.print(_out);

  return;

}

///////////////////////////////////////////////////////////////
void Print::edr_point(int data_len, void *data)

{ 

  Edr edr;
  if (edr.disassemble(data, data_len)) {
    cerr << "ERROR - SpdbQuery::Print::edr_point" << endl;
    cerr << "  Cannot disassemble chunk" << endl;
    return;
  }
  edr.print(_ostr);

}



///////////////////////////////////////////////////////////////
// VERGRID_REGION: Print Verify Grid Region chunk
//

void Print::vergrid_region(int data_len, void *data)
{
  VerGridRegion *region = new VerGridRegion();
  region->readChunk(data, data_len);
  region->print(_out, "  ");
  delete(region);
  return;
}

void Print::gen_poly(int data_len, void *data)
{
   GenPoly genPoly;
   if (!genPoly.disassemble(data, data_len)) {
     cerr << "ERROR - SpdbQuery::Print::gen_poly" << endl;
     cerr << "  Cannot disassemble chunk" << endl;
     return;
   }
//   genPoly.print(_ostr);
   genPoly.print(_out);
 
   return;
}

///////////////////////////////////////////////////////////////
// NWS_WWA
//

void Print::nws_wwa(int data_len, void *data)
{
  NWS_WWA W;
  W.disassemble(data,data_len);

  W.print(_out);
  
  return;
}

//////////////////
// DsRadarSweep


void Print::ds_radar_sweep(int data_len, void *data)
{
  DsRadarSweep sweep;
  if (sweep.disassemble(data, data_len)) {
    cerr << "ERROR - SpdbQuery::Print::ds_radar_sweep" << endl;
    cerr << "  Cannot disassemble chunk" << endl;
    return;
  }
  sweep.print(_ostr, "");

  return;
}

///////////////////////////////////////////////////////////////
// WxObs : Print weather observation

void Print::wx_obs(int data_len, void *data)
{

  WxObs obs;
  obs.disassemble(data, data_len);
  _ostr << "Data for weather observation:\n" << endl;
  obs.print(_ostr, "   ");
  return;

}

///////////////////////////////////////////////////////////////
// wafs_sigwx : Print the XML stored in the WAFS SIGWX databases

void Print::wafs_sigwx(int data_len, void *data)
{

  char *xml_buffer = (char *)data;
  fprintf(_out, "%s\n", xml_buffer);
  return;
  
}

