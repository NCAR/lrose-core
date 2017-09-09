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
#include <rapformats/hist_fore.h>
#include <rapformats/acarsXml.hh>
#include <rapformats/Amdar.hh>
#include <rapformats/ac_data.h>
#include <rapformats/ac_posn.h>
#include <rapformats/ac_route.h>
#include <rapformats/ac_georef.hh>
#include <rapformats/bdry.h>
#include <rapformats/flt_path.h>
#include <rapformats/fos.h>
#include <rapformats/ltg.h>
#include <rapformats/pirepXml.hh>
#include <Spdb/sounding.h>
#include <Spdb/PosnRpt.hh>
#include <Spdb/FltRoute.hh>
#include <rapformats/SigAirMet.hh>
#include <rapformats/Taf.hh>
#include <rapformats/station_reports.h>
#include <rapformats/trec_gauge.h>
#include <rapformats/zr.h>
#include <rapformats/zrpf.h>
#include <rapformats/ZVis.hh>
#include <rapformats/ZvisCal.hh>
#include <rapformats/ZvisFcast.hh>
#include <rapformats/VerGridRegion.hh>
#include <rapformats/ComboPt.hh>
#include <rapformats/tstorm_spdb.h>
#include <rapformats/Sndg.hh>
#include <rapformats/Edr.hh>
#include <rapformats/Edr_expanded.hh>
#include <rapformats/GenPoly.hh>
#include <rapformats/acPosVector.hh>
#include <rapformats/UsgsData.hh>
#include <rapformats/NWS_WWA.hh>
#include <rapformats/DsRadarSweep.hh>
#include <rapformats/DsRadarPower.hh>
#include <rapformats/RadarSpectra.hh>
#include <rapformats/WxObs.hh>
#include <rapformats/TaiwanAwos.hh>
#include <rapformats/ChecktimeReport.hh>

using namespace std;

// Constructor

Print::Print(FILE *out, ostream &ostr, bool debug /* = false */) :
  _out(out), _ostr(ostr), _debug(debug)

{
}

// Destructor

Print::~Print()

{

}

///////////////////////////////////////////////////////////////
// CHUNK_HDR

void Print::chunk_hdr(const Spdb::chunk_t &chunk)
{

  bool print_hashed = true;
  string data_type_str; 
  if(chunk.data_type > 1048576) { // These are probably 4 char hashes.
	  data_type_str = Spdb::dehashInt32To4Chars(chunk.data_type);
  } else if (chunk.data_type > 0)  { // A numerical ID
	  print_hashed = false;
  } else {
	  data_type_str = Spdb::dehashInt32To5Chars(chunk.data_type);
  }

  // Check for non printable chars.
  for (size_t i = 0; i < data_type_str.size(); i++) {
    if (!isprint(data_type_str[i])) {
	  print_hashed = false;
    }
  }

  if (print_hashed) {
    fprintf(_out, "  data_type: %d (%s)\n", chunk.data_type, data_type_str.c_str());
  } else {
    fprintf(_out, "  data_type: %d\n", chunk.data_type);
  }

  bool print_hashed2 = true;
  string data_type2_str = Spdb::dehashInt32To4Chars(chunk.data_type2);
  if (data_type2_str.size() < 1) {
    print_hashed2 = false;
  } else {
    for (size_t i = 0; i < data_type2_str.size(); i++) {
      if (!isprint(data_type2_str[i])) {
        print_hashed2 = false;
      }
    }
  }
  if (print_hashed2) {
    fprintf(_out, "  data_type2: %d (%s)\n", chunk.data_type2,
	    Spdb::dehashInt32To4Chars(chunk.data_type2).c_str());
  } else {
    fprintf(_out, "  data_type2: %d\n", chunk.data_type2);
  }

  fprintf(_out, "  valid_time: %s\n", utimstr(chunk.valid_time));
  fprintf(_out, "  expire_time: %s\n", utimstr(chunk.expire_time));
  if (chunk.write_time > 0) {
    fprintf(_out, "  write_time: %s\n", utimstr(chunk.write_time));
  }
  fprintf(_out, "  len: %d\n", chunk.len);


  if (chunk.current_compression == Spdb::COMPRESSION_GZIP) {
    fprintf(_out, "  current_compression: gzip\n");
  } else if (chunk.current_compression == Spdb::COMPRESSION_BZIP2) {
    fprintf(_out, "  current_compression: bzip2\n");
  } else {
    fprintf(_out, "  current_compression: none\n");
  }

  if (chunk.stored_compression == Spdb::COMPRESSION_GZIP) {
    fprintf(_out, "  stored_compression: gzip\n");
  } else if (chunk.stored_compression == Spdb::COMPRESSION_BZIP2) {
    fprintf(_out, "  stored_compression: bzip2\n");
  } else {
    fprintf(_out, "  stored_compression: none\n");
  }
  
  if (chunk.tag.size() > 0) {
    fprintf(_out, "  tag: %s\n", chunk.tag.c_str());
  }

  fprintf(_out, "\n");

}

///////////////////////////////////////////////////////////////
// ASCII: Print ascii data

void Print::ascii(int data_len, void *data)
{

  if (data_len == 0) {
    return;
  }

  char *str = (char *) data;
  // ensure null-terminated the string
  str[data_len - 1] = '\0';
  fprintf(_out, "%s\n", str);
  
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
  pt.print(_ostr);
  
  return;
}

///////////////////////////////////////////////////////////////
// Print generic point data

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
  pt.print(_ostr);
  
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
// AMDAR : Print amdar bulletin

void Print::amdar(int data_len,  void *data)
{

  Amdar amdar;
  if (amdar.disassemble(data, data_len)) {
    cerr << "ERROR - SpdbQuery::Print::amdar" << endl;
    cerr << "  Cannot disassemble chunk" << endl;
    return;
  }
  amdar.print(_ostr, "  ");

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
  fprintf(_out, "Data for wmod aircraft position (%d positions):\n", num_posn);

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
// AC_GEOREF : Print aircraft georef data

void Print::ac_georef(int data_len, void *data)
{

  ac_georef_t *ac_georef_data = (ac_georef_t *) data;
  int num_georef = data_len / sizeof(ac_georef_t);
  
  // Print the aircraft data.
  
  fprintf(_out, "\n");
  fprintf(_out, "Data for aircraft georefs (n = %d):\n", num_georef);
  
  // Check for errors
  
  if (data_len % sizeof(ac_georef_t) != 0) {
    fprintf(_out,
	    "  ERROR:  Chunk contains %d bytes, not an even "
	    "multiple of structure size (%d bytes)\n",
	    data_len, (int) sizeof(ac_georef_t));
  }
  
  for (int ii = 0; ii < num_georef; ii++) {

    BE_to_ac_georef(&ac_georef_data[ii]);
    ac_georef_print(_out, "  ", &ac_georef_data[ii]);
    
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
// AC_ROUTE : Print aircraft route

void Print::ac_route(void *data)
{
  BE_to_ac_route(data);
  ac_route_print(_out, "\t", data);
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

void Print::bdry(void *data)
{

  BDRY_spdb_product_t *bdry_data = (BDRY_spdb_product_t *)data;
  BDRY_spdb_product_from_BE(bdry_data);
  fprintf(_out, "\n");
  fprintf(_out, "Data for boundary:\n");
  BDRY_print_spdb_product(_out, bdry_data, _debug);
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

  fprintf(_out, " Printing history forecast data:\n");
  hf_chunk_native((hf_chunk_t **) &data);
  print_hf_chunk(_out, (hf_chunk_t *) data, data_len);
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
// SYMPROD : Print symbolic product data
//

void Print::symprod(int data_len, void *data)
{
  
  if (data == NULL) {
    cerr << "ERROR - Print::symprod" << endl;
    cerr << "  No data from server." << endl;
    return;
  }

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

void Print::sig_air_met(int data_len, void *data)

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
// TAF : Data in the TAF class
//

void Print::taf(int data_len, void *data)

{

  Taf taf;
  if (taf.disassemble(data, data_len)) {
    cerr << "ERROR - SpdbQuery::Print::taf" << endl;
    cerr << "  Cannot disassemble chunk" << endl;
    return;
  }
  taf.print(_ostr, "  ");

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
// ZV_PROB
//

void Print::zvis_cal(int data_len, void *data)

{

  ZvisCal cal;
  if (cal.disassemble(data, data_len)) {
    cerr << "ERROR - SpdbQuery::Print::zv_prob_cal" << endl;
    cerr << "  Cannot disassemble chunk" << endl;
    return;
  }
  cal.print(_ostr);

}

void Print::zvis_fcast(int data_len, void *data)

{

  ZvisFcast fcast;
  if (fcast.disassemble(data, data_len)) {
    cerr << "ERROR - SpdbQuery::Print::zv_prob_fcast" << endl;
    cerr << "  Cannot disassemble chunk" << endl;
    return;
  }
  fcast.print(_ostr);

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
// SNDG_PLUS : Print sounding plus
//

void Print::sndg_plus(int data_len, void *data)

{

  Sndg sndg;
  if (sndg.disassemble(data, data_len)) {
    cerr << "ERROR - SpdbQuery::Print::sndg_plus" << endl;
    cerr << "  Cannot disassemble chunk" << endl;
    return;
  }
  sndg.print(_ostr);

}

///////////////////////////////////////////////////////////////
// EDR_POINT : Print edr_t 
//

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
// EDR_POINT : Print Edr_t  - this ia an expanded structure to
// allow for east future expansion
//
void Print::EDR_point(int data_len, void *data)

{ 

  EDR Edr;
  if (Edr.disassemble(data, data_len)) {
    cerr << "ERROR - SpdbQuery::Print::EDR_point" << endl;
    cerr << "  Cannot disassemble chunk" << endl;
    return;
  }
  Edr.print(_ostr);

}

///////////////////////////////////////////////////////////////
// VERGRID_REGION: Print Verify Grid Region chunk
///


void Print::vergrid_region(int data_len, void *data)
{
  VerGridRegion *region = new VerGridRegion();
  region->readChunk(data, data_len);
  region->print(_out, "  ");
  delete(region);
  return;
}

//////////////////
// generic polygon

void Print::gen_poly(int data_len, void *data)
{
  GenPoly genPoly;
  if (!genPoly.disassemble(data, data_len)) {
    cerr << "ERROR - SpdbQuery::Print::gen_poly" << endl;
    cerr << "  Cannot disassemble chunk" << endl;
    return;
  }
  genPoly.print(_ostr);

  return;
}

///////////////////////////////////////////////////////////////
//  Print USGS Data (Volcanes and Earthquakes
//

void Print::usgs_data(int data_len, void *data)

{ 
  UsgsData usgsData;
  if (usgsData.disassemble(data, data_len)) {
    cerr << "ERROR - SpdbQuery::Print::usgs_data" << endl;
    cerr << "  Cannot disassemble chunk" << endl;
    return;
  }
  usgsData.print(_ostr, "  ");

}

///////////////////////////////////////////////////////////////
// NWS_WWA

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

//////////////////
// DsRadarPower

void Print::ds_radar_power(int data_len, void *data)
{
  DsRadarPower power;
  if (power.disassemble(data, data_len)) {
    cerr << "ERROR - SpdbQuery::Print::ds_radar_power" << endl;
    cerr << "  Cannot disassemble chunk" << endl;
    return;
  }
  power.print(_ostr, "");

  return;
}

//////////////////
// RadarSpectra

void Print::radar_spectra(int data_len, void *data)
{
  RadarSpectra spectra;
  if (spectra.disassemble(data, data_len)) {
    cerr << "ERROR - SpdbQuery::Print::radar_spectra" << endl;
    cerr << "  Cannot disassemble chunk" << endl;
    return;
  }
  if (_debug) {
    spectra.print(_ostr, "");
  } else {
    spectra.printHeader(_ostr, "");
  }

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
// WxObs : Print Taiwan AWOS/ASOS report

void Print::taiwan_awos(int data_len, void *data)
{

  TaiwanAwos taiwanAwos;
  int retVal = taiwanAwos.disassemble(data, data_len);
  if(retVal == 1) {
    cerr << "ERROR - SpdbQuery::Print::taiwan_awos" << endl;
    cerr << "  Cannot disassemble chunk" << endl;
    cerr << "disassembly failed." << endl;
    return;
  }

  taiwanAwos.print(_ostr);
  return;

}

///////////////////////////////////////////////////////////////
// Checktimes : Print checktime report

void Print::checktimes(int data_len, void *data)
{
  ChecktimeReport report;
  if (!report.disassemble(data, data_len)) {
    cerr << "ERROR - Print::ChecktimeReport" << endl;
    cerr << "  Cannot disassemble chunk." << endl;
    return;
  }

  report.print(_ostr);
  return;
}
