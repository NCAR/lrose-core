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
#include <rapformats/ac_data.h>
#include <rapformats/ac_posn.h>
#include <rapformats/ac_route.h>
#include <rapformats/acPosVector.hh>
#include <toolsa/udatetime.h>
#include <toolsa/DateTime.hh>

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
  string data_type_str = Spdb::dehashInt32To4Chars(chunk.data_type);
  if (data_type_str.size() != 4) {
    print_hashed = false;
  } else {
    for (size_t i = 0; i < data_type_str.size(); i++) {
      if (!isprint(data_type_str[i])) {
	print_hashed = false;
      }
    }
  }
  if (print_hashed) {
    fprintf(_out, "data_type = %d (%s)\n", chunk.data_type,
	    Spdb::dehashInt32To4Chars(chunk.data_type).c_str());
  } else {
    fprintf(_out, "data_type = %d\n", chunk.data_type);
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
    fprintf(_out, "data_type2 = %d (%s)\n", chunk.data_type2,
	    Spdb::dehashInt32To4Chars(chunk.data_type2).c_str());
  } else {
    fprintf(_out, "data_type2 = %d\n", chunk.data_type2);
  }

  fprintf(_out, "valid_time = %s\n", utimstr(chunk.valid_time));
  fprintf(_out, "expire_time = %s\n", utimstr(chunk.expire_time));
  if (chunk.write_time > 0) {
    fprintf(_out, "write_time = %s\n", utimstr(chunk.write_time));
  }
  fprintf(_out, "len = %d\n", chunk.len);
  return;
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
// AC_POSN : Print aircraft position data

void Print::ac_posn_summary(time_t valid_time, int data_len,  void *data)
{

  ac_posn_t *ac_posn_data = (ac_posn_t *)data;
  int num_posn = data_len / sizeof(ac_posn_t);
  
  // Check for errors
  
  if (data_len % sizeof(ac_posn_t) != 0) {
    fprintf(stderr,
	    "  ERROR:  Chunk contains %d bytes, not an even "
	    "multiple of structure size (%d bytes)\n",
	    data_len, (int)sizeof(ac_posn_t));
    return;
  }
  
  for (int ii = 0; ii < num_posn; ii++) {

    // Swap the aircraft data.

    ac_posn_t posn = ac_posn_data[ii];
    BE_to_ac_posn(&posn);

    DateTime dtime(valid_time);

    fprintf(_out, "%s,%.4d,%.2d,%.2d,%.2d,%.2d,%.2d,%.4f,%.4f,%.4f\n",
	    posn.callsign,
	    dtime.getYear(), dtime.getMonth(), dtime.getDay(),
	    dtime.getHour(), dtime.getMin(), dtime.getSec(),
	    posn.lat, posn.lon, posn.alt);
  }
  
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

void Print::ac_posn_wmod_summary(time_t valid_time, int data_len,  void *data)
{

  ac_posn_wmod_t *ac_posn_data = (ac_posn_wmod_t *)data;
  int num_posn = data_len / sizeof(ac_posn_wmod_t);
  
  // Check for errors
  
  if (data_len % sizeof(ac_posn_wmod_t) != 0) {
    fprintf(stderr,
	    "  ERROR:  Chunk contains %d bytes, not an even "
	    "multiple of structure size (%d bytes)\n",
	    data_len, (int)sizeof(ac_posn_wmod_t));
    return;
  }
  
  for (int ii = 0; ii < num_posn; ii++) {

    // Swap the aircraft data.

    ac_posn_wmod_t posn = ac_posn_data[ii];
    BE_to_ac_posn_wmod(&posn);
    DateTime dtime(valid_time);
    
    fprintf(_out, "%s,%.4d,%.2d,%.2d,%.2d,%.2d,%.2d,%.4f,%.4f,%.4f\n",
	    posn.callsign,
	    dtime.getYear(), dtime.getMonth(), dtime.getDay(),
	    dtime.getHour(), dtime.getMin(), dtime.getSec(),
	    posn.lat, posn.lon, posn.alt);
  }
  
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

