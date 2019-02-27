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
/////////////////////////////////////////////////////////////
// NWS_WWA.cc
//
// C++ wrapper for NWS Watches, Warnings and Advisories.
//
// Author: Frank Hage,  June 2006
// (C) UCAR 2006. All right reserved.  
//////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <toolsa/DateTime.hh>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/TaStr.hh>

#define NWS_WWA_CORE
#include <rapformats/NWS_WWA.hh>

using namespace std;

////////////////////
// constructor
NWS_WWA::NWS_WWA() { clear(); }

////////////////////
// destructor

NWS_WWA::~NWS_WWA() { }

////////////////////
// clear

void NWS_WWA::clear()
{
  MEM_zero(_hdr);
  _text = "";
  _memBuf.free();
}

///////////////////////////////////////////////////////////
// disassemble()
// Disassembles a buffer, sets the object values.
// Handles byte swapping.
// Returns true on success, false on failure

bool NWS_WWA::disassemble(const void *buf, int len)
{
  if(len < (int) sizeof(nws_wwa_hdr_t)) return false;
  
  // get header
  memcpy((void*) &_hdr, buf, sizeof(nws_wwa_hdr_t));

  _NWS_WWA_from_BE(&_hdr);

  // Locate the text part
  char *t_ptr = (char *) buf;
  t_ptr += sizeof(nws_wwa_hdr_t);

   _text = t_ptr; // Copy the text part 

  return true;

}

///////////////////////////////////////////
// assemble()
// Load up the buffer from the object.
// Handles byte swapping.
//
// returns true on success, false on failure
// Use getErrStr() on failure.

bool NWS_WWA::assemble()
{
   nws_wwa_hdr_t h_buf;
   
  // Initialize the buffer
  _memBuf.free();

  // Create a temporary copy
  h_buf = _hdr;
  h_buf.text_length = _text.size() +1;

  if(h_buf.text_length <= 1) return false;

  // Swap to netweork byte order.
  _NWS_WWA_to_BE(&h_buf);

  // Add the header part to our Mem Buffer.
  _memBuf.add(&h_buf, sizeof(nws_wwa_hdr_t));

  // Add the raw text part.
  _memBuf.add(_text.c_str(),_text.size());
  
  return true;
}

////////////////////////////////////////////////////////
// prints

void NWS_WWA::print(FILE *out) const
{
    fprintf(out, "  ============================================\n");
    fprintf(out, "  NWS_WWA - NWS Watches, Warnings, Advisories\n");
    fprintf(out, "  ============================================\n");

	fprintf(out, "   Issue time = %s\n", utimstr(_hdr.issue_time));
	fprintf(out, "   Expire time = %s\n", utimstr(_hdr.expire_time));
	fprintf(out, "   Hazard type = %s\n",wwa[_hdr.hazard_type].match_str);
	if(_hdr.hazard_sub_type != 0) {
		fprintf(out, "   Hazard Sub type = %s\n",wwa[_hdr.hazard_sub_type].match_str);
	}
        switch (_hdr.action)
        {
           /* New Event */
           case ACT_NEW:
              fprintf(out, "   Action = NEW,  New event\n");
              break;
           /* Event continued */
           case ACT_CON:
              fprintf(out, "   Action = CON, event continued \n");
              break;
           /* Event extended (time)*/
           case ACT_EXT:
              fprintf(out, "   Action = EXT, event extended time \n");
              break;
            /* Event extended (area) */
            case ACT_EXA:
              fprintf(out, "   Action = EXA, event extended area \n");
              break;
            /* Event extended (both time and area) */
            case ACT_EXB:
              fprintf(out, "   Action = EXB, event extended time and area \n");
              break;
            /* Event upgraded */
            case ACT_UPG:
              fprintf(out, "   Action = UPG, event upgraded \n");
              break;
            /* Event cancelled */
            case ACT_CAN:
              fprintf(out, "   Action = CAN, event cancelled \n");
              break;
            /* Event expired */
            case ACT_EXP:
              fprintf(out, "   Action = EXP, notification of expire time of event \n");
              break;
            /* Event correction */
            case ACT_COR:
              fprintf(out, "   Action = COR, event correction \n");
              break;
            /* Event routine */
            case ACT_ROU:
              fprintf(out, "   Action = ROU, event routine  \n");
              break;
            default:
              fprintf(out, "   Action is unknown, not in list of known action types \n");
              break;
        } 
	fprintf(out, "   Motion Time = %s \n",utimstr(_hdr.motion_time));
	fprintf(out, "   Motion Lat, Lon = %g, %g \n",_hdr.motion_lat,_hdr.motion_lon);
	fprintf(out, "   Motion Direction = %ddeg\n",_hdr.motion_dir);
	fprintf(out, "   Motion Speed = %dkts\n",_hdr.motion_kts);

	fprintf(out, "   Bounding Box:\n");
	fprintf(out, "   Min Lat: %g Max Lat: %g\n",_hdr.lat_min,_hdr.lat_max);
	fprintf(out, "   Min Lon: %g Max Lon: %g\n",_hdr.lon_min,_hdr.lon_max);

	fprintf(out, "   %d Boundry points\n",_hdr.num_points);
	fprintf(out, "   Text len = %d\n",_hdr.text_length);

	int i;
	for(i=0 ; i < (int) _hdr.num_points; i++) {
	  fprintf(out,"Point %d Lat: %f, Lon: %f\n",i,_hdr.wpt[i].lat,_hdr.wpt[i].lon);
	}
    fprintf(out, "--------------------------------------------\n");
	fprintf(out, "%s\n",_text.c_str());
    fprintf(out, "--------------------------------------------\n\n");
}

void NWS_WWA::print(ostream &out, string spacer) const
{
    out <<  "============================================" << endl;
    out <<  "NWS_WWA - NWS Watches, Warnings, Advisories" << endl;
    out <<  "============================================" << endl;

	out <<  "   Issue time = " <<  utimstr(_hdr.issue_time) << endl;
	out <<  "   Expire time = " << utimstr(_hdr.expire_time) << endl;
	out <<  "   Hazard type = " << wwa[_hdr.hazard_type].match_str << endl;
	if(_hdr.hazard_sub_type != 0) {
		out <<  "   Hazard Sub type = " << wwa[_hdr.hazard_sub_type].match_str << endl;;
	}
	out <<  "   Bounding Box"  << endl;
	out <<  "   Min Lat: " << _hdr.lat_min << " Max Lat: " << _hdr.lat_max << endl;
	out <<  "   Min Lon: " << _hdr.lon_min << " Max Lon: " << _hdr.lon_max << endl;

	out <<  "   " << _hdr.num_points <<  " Boundry points" << endl;
	out <<  "   Text len = " << _hdr.text_length << endl;

	int i;
	for(i=0 ; i < (int) _hdr.num_points; i++) {
	  out << "Point: " << i << "Lat: " << _hdr.wpt[i].lat <<  "Lon: " << _hdr.wpt[i].lat << endl; 
	}
    out << "  --------------------------------------------" << endl;
	out << _text.c_str() << endl;
    out << "  --------------------------------------------" << endl << endl;;
}

/************************************************************************
 * NWS_WWA_from_BE() - Convert the WWA  information from big-endian format
 *                 to native format.
 */

void NWS_WWA::_NWS_WWA_from_BE(nws_wwa_hdr_t *hdr)
{
	BE_from_array_32(hdr, sizeof(hdr));
}

/************************************************************************
 * NWS_WWA_to_BE() - Convert the WWA header information from native to
 *  big-endian format
 */

void NWS_WWA::_NWS_WWA_to_BE(nws_wwa_hdr_t *hdr)
{
	BE_from_array_32(hdr, sizeof(hdr));
}

