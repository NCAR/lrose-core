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
// NWS_WWA.hh
//
// C++ class for dealing with 3NWS Watches, Warnings and Advisories
//
// Frank Hage, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// July 2002
//////////////////////////////////////////////////////////////

#ifndef _NWS_WWA_hh
#define _NWS_WWA_hh


#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <toolsa/MemBuf.hh>
#include <dataport/port_types.h>
using namespace std;

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <dataport/port_types.h>

#define NWS_WWA_NAN -999999.999

/* A Magic cookie to identify specific data structure */
enum nws_wwa_msg_id_t{ NWS_WWA_ID = 27000 };

#define NUM_NWS_WWA_TYPES 80  // Number of possible types.
#define NWS_WWA_MAX_POINTS 64 // Boundry points

/* Hazardouus Weather Type Bitwize flags - Taken from NOAA/NWS definitions */
enum hazard_type_t {
	HW_TOWAR,  /* Tornado Warning */
	HW_TOWAT,  /* Tornado Watch */
	HW_STWAR, /* Severe ThunderStorm Warning */
	HW_STWAT, /* Severe ThunderStorm Watch */
	HW_SWS, /* Severe Weather Statement */
	HW_FFWAR, /* Flash Flood Warning */
	HW_FFWAT, /* Flash Flood Watch */
	HW_TSWAR, /* Tropical Storm Warning */
	HW_TSWAT, /* Tropical Storm Watch */
	HW_HUWAR, /* Hurricane Warning */
	HW_HUWAT, /* Hurricane Watch */
	HW_FLWAR, /* Flood Warning */
	HW_FLWAT, /* Flood Watch */
	HW_WSWAR, /* Winter Storm Warning */
	HW_WSWAT, /* Winter Storm Watch*/
	HW_SMWAR, /* Special Marine Warning (Squalls,	Tornados) */
	HW_WCWAR, /* Wind Chill Warning*/
	HW_WCADV, /* Wind Chill Advisory */
	HW_EHWAR, /* Excessive Heat Warning */
	HW_HEADV, /* Heat Advisory */
	HW_HWWAR, /* High Wind Warning */
	HW_WIADV, /* Wind Advisory */
	HW_DFADV, /* Dense Fog Advisory */
	HW_FZWAR, /* Freeze Warning */
	HW_FRADV, /* Frost Advisory */
	HW_LSWAR, /* LakeShore (Flooding) Warning */
	HW_BSADV, /* Blowing Snow Advisory */
	HW_RAWAR, /* Radiological Hazard Warning */
	HW_NUWAR, /* Nuclear Hazard Warning */
	HW_HMWAR, /* Hazardous Materials Warning */
	HW_VOWAR, /* Volcano Warning */
	HW_EAWAR, /* Earthquake Warning */
	HW_FIWAR, /* Fire Warning */

		// Winter Storms Sub type
	WS_BLWAR,	/* Blizard Warning */
	WS_HSWAR, /* Heavy Snow Warning */
	WS_LEWAR, /* Lake Effect Snow Warning */
	WS_ISWAR, /* Ice Storm Warning */
	WS_HLWAR, /* Heavy sLeet Warning */
	WS_SNADV, /* Snow Advisory */
	WS_LEADV, /* Lake Efeect Snow Advisory */
	WS_SLADV, /* SLeet Advisory */
};

enum action_t {
  ACT_NEW, /* New Event */
  ACT_CON, /* Event continued */
  ACT_EXT, /* Event extended (time)*/
  ACT_EXA, /* Event extended (area) */
  ACT_EXB, /* Event extended (both time and area) */
  ACT_UPG, /* Event upgraded */
  ACT_CAN, /* Event cancelled */
  ACT_EXP, /* Event expired */
  ACT_COR, /* Event correction */
  ACT_ROU  /* Event Routine */
}; 


typedef struct {
	char *match_str;
	enum hazard_type_t code;
	char *color;
} nws_wwa_match_code_t;

typedef struct {
	fl32 lat; // latitude
	fl32 lon; // longitude
} nws_wwa_wpt_t;


// ////////////////////////////////////////////////////////////////////
// SPDB NWS Watches and Warnings Message Structure is as follows:
// 1. nws_wwa_hdr_t
// 2. num_chars of original NWS Text - Including Null termination.

typedef struct {

	enum nws_wwa_msg_id_t  msg_id;	    /* message identifier - Usually NWS_WWA */

	ui32 issue_time;	/* Time of warning - Seconds since Jan 1 0Z 1970 */

	ui32 expire_time;	/* Time when warning expires */

	enum hazard_type_t hazard_type;	/* Weather Hazard */
	
	enum hazard_type_t hazard_sub_type;	/* Winter Storms Subtype */
	
	ui32 num_points;   // Number of Lat- Long points for polygon.
	
	ui32 text_length;  // Number of characters of warning text -including null.

	// TIME...MOT...LOC info
	ui32 motion_time; // Time stamp in the TIME...MOT...LOC line
	ui32 motion_dir;  // Degrees from true North
	ui32 motion_kts;  // Motion Speed in Knots. 
        enum action_t action; // Warning action: NEW CON EXT EXA EXB UPG CAN EXP COR ROU
	ui32 unused_int_spares[53];   // fill out struct to 256 bytes.

	fl32 lat_min;                // Latitude Bounds.
	fl32 lat_max;
	fl32 lon_min;                // Longitude Bounds.
	fl32 lon_max;

	fl32 motion_lat;
	fl32 motion_lon;
	
	fl32 unused_float_spares[58]; // fill out struct to 512 bytes.

	nws_wwa_wpt_t wpt[NWS_WWA_MAX_POINTS]; // fill out struct to 1024 bytes.

} nws_wwa_hdr_t;


class NWS_WWA {

public:

  // WWA Data elements
  nws_wwa_hdr_t _hdr;  

  string  _text;  // Contains the raw text.

  MemBuf _memBuf;  // For outgoing product.

  // constructor

  NWS_WWA();

  // destructor

  ~NWS_WWA();

  //////////////////////// set methods /////////////////////////

  // clear all data members

  void clear();

  ///////////////////////////////////////////
  // assemble()
  // Load up the buffer from the object.
  // Handles byte swapping.
  // Returns true on success, false on failure
  
  bool assemble();

  // get the assembled buffer info
  
  const void *getBufPtr() const { return _memBuf.getPtr(); }
  int getBufLen() const { return _memBuf.getLen(); }

  ///////////////////////////////////////////////////////////
  // disassemble()
  // Disassembles a buffer, sets the values in the object.
  // Handles byte swapping.
  // Returns true on success, false on failure
  
  bool disassemble(const void *buf, int len);
  
  /////////////////////////
  // print
  
  void print(ostream &out, string spacer = "") const;
  void print(FILE *out) const;
  
  // byte swapping

  static void _NWS_WWA_from_BE(nws_wwa_hdr_t *hdr);
  static void _NWS_WWA_to_BE(nws_wwa_hdr_t *hdr);
  
protected:

};

#ifdef NWS_WWA_CORE 

// TEXT to Type and Color Lookup table
// Note - These colors seem to be official See
// http://webdev1.weather.gov/wwa_colors/colors.htm
// and http://www.srh.noaa.gov/oun/news/newmapcolors.php
// -F. Hage June 2006

#define WWA_COLOR_STR_LEN 16
#define WWA_LBL_STR_LEN 32

static char wwa_colors[NUM_NWS_WWA_TYPES][WWA_COLOR_STR_LEN] = {
  "#FF0000\0", "#FFFF00\0", "#FFA500\0", "#DB7093\0", "#00FFFF\0",
  "#8B0000\0", "#32CD32\0", "#B22222\0", "#F08080\0", "#DC143C\0",
  "#FF00FF\0", "#00FF00\0", "#2E8B57\0", "#FF69B4\0", "#FFA500B\0",
  "#DB7093\0", "#B0C4DE\0", "#AFEEEE\0", "#C71585\0", "#FF7F50\0",
  "#DAA520\0", "#D2B48C\0", "#708090\0", "#483D8B\0", "#228B22\0",
  "#B0E0E6\0", "#4B0082\0", "#4B0082\0", "#4B0082\0", "#696969\0",
  "#8B4513\0", "#A0522D\0", "#FF4500\0", "#8A2BE2\0", "#008B8B\0",
  "#8B008B\0", "#87CEEB\0", "#6699CC\0", "#48D1CC\0", "#7B68EE\0"
};

static char wwa_labels[NUM_NWS_WWA_TYPES][WWA_LBL_STR_LEN] = {
  "TORNADO WARNING\0", "TORNADO WATCH\0", "SEVERE THUNDERSTORM WARNING\0", 
  "SEVERE THUNDERSTORM WATCH\0", "SEVERE WEATHER STATEMENT\0", "FLASH FLOOD WARNING\0", 
  "FLASH FLOOD WATCH\0", "TROPICAL STORM WARNING\0", "TROPICAL STORM WATCH\0", 
  "HURRICANE WARNING\0", "HURRICANE WATCH\0", "FLOOD WARNING\0", 
  "FLOOD WATCH\0", "WINTER STORM WARNING\0", "WINTER STORM WATCH\0", 
  "SPECIAL MARINE WARNING\0", "WIND CHILL WARNING\0", "WIND CHILL ADVISORY\0", 
  "EXCESSIVE HEAT WARNING\0", "HEAT ADVISORY\0", "HIGH WIND WARNING\0", 
  "WIND ADVISORY\0", "DENSE FOG ADVISORY\0", "FREEZE WARNING\0", 
  "LAKESHORE WARNING\0", "BLOWING SNOW ADVISORY\0", "RADIOLOGICAL HAZARD WARNING\0", 
  "NUCLEAR HAZARD WARNING\0", "HAZARDOUS MATERIALS WARNING\0", "VOLCANO WARNING\0", 
  "EARTHQUAKE WARNING\0", "FIRE WARNING\0", "BLIZZARD WARNING\0", 
  "HEAVY SNOW WARNING\0", "LAKE EFFECT SNOW WARNING\0", "ICE STORM WARNING\0", 
  "HEAVY SLEET WARNING\0", "SNOW ADVISORY\0","LAKE EFFECT SNOW ADVISORY\0", 
  "SLEET ADVISORY\0",
  "Tornado Warning\0", "Tornado Watch\0", "Severe Thunderstorm Warning\0",
  "Severe Thunderstorm Watch\0", "Severe Weather Statement\0", "Flash Flood Warning\0",
  "Flash Flood Watch\0", "Tropical Storm Warning\0", "Tropical Storm Watch\0",
  "Hurricane Warning\0", "Hurricane Watch\0", "Flood Warning\0",
  "Flood Watch\0", "Winter Storm Warning\0", "Winter Storm Watch\0",
  "Special Marine Warning\0", "Wind Chill Warning\0", "Wind Chill Advisory\0",
  "Excessive Heat Warning\0", "Heat Advisory\0", "High Wind Warning\0",
  "Wind Advisory\0", "Dense Fog Advisory\0", "Freeze Warning\0",
  "Lakeshore warning\0", "Blowing Snow Advisory\0", "Radiological Hazard Warning\0",
  "Nuclear Hazard Warning\0", "Hazardous Materials Warning\0", "Volcano Warning\0",
  "Earthquake Warning\0", "Fire Warning\0", "Blizzard Warning\0",
  "Heavy Snow Warning\0", "Lake Effect Snow Warning\0", "Ice Storm Warning\0",
  "Heavy Sleet Warning\0", "Snow Advisory\0","Lake Effect Snow Advisory\0",
  "Sleet Advisory\0"
};


nws_wwa_match_code_t wwa[NUM_NWS_WWA_TYPES] = {
  { wwa_labels[0], HW_TOWAR, wwa_colors[0] },
	{ wwa_labels[1], HW_TOWAT, wwa_colors[1] },
	{ wwa_labels[2], HW_STWAR, wwa_colors[2] },
	{ wwa_labels[3], HW_STWAT, wwa_colors[3] },
	{ wwa_labels[4], HW_SWS, wwa_colors[4] },
	{ wwa_labels[5], HW_FFWAR, wwa_colors[5] },
	{ wwa_labels[6], HW_FFWAT, wwa_colors[6] },
	{ wwa_labels[7], HW_TSWAR, wwa_colors[7] },
	{ wwa_labels[8], HW_TSWAT, wwa_colors[8] },
	{ wwa_labels[9], HW_HUWAR, wwa_colors[9] },
	{ wwa_labels[10], HW_HUWAT, wwa_colors[10] },
	{ wwa_labels[11], HW_FLWAR, wwa_colors[11] },
	{ wwa_labels[12], HW_FLWAT, wwa_colors[12] },
	{ wwa_labels[13], HW_WSWAR, wwa_colors[13] },
	{ wwa_labels[14], HW_WSWAT, wwa_colors[14] },
	{ wwa_labels[15], HW_SMWAR, wwa_colors[15] },
	{ wwa_labels[16], HW_WCWAR, wwa_colors[16] },
	{ wwa_labels[17], HW_WCADV, wwa_colors[17] },
	{ wwa_labels[18], HW_EHWAR, wwa_colors[18] },
	{ wwa_labels[19], HW_HEADV, wwa_colors[19] },
	{ wwa_labels[20], HW_HWWAR, wwa_colors[20] },
	{ wwa_labels[21], HW_WIADV, wwa_colors[21] },
	{ wwa_labels[22], HW_DFADV, wwa_colors[22] },
	{ wwa_labels[23], HW_FZWAR, wwa_colors[23] },
	{ wwa_labels[24], HW_LSWAR, wwa_colors[24] },
	{ wwa_labels[25], HW_BSADV, wwa_colors[25] },
	{ wwa_labels[26], HW_RAWAR, wwa_colors[26] },
	{ wwa_labels[27], HW_NUWAR, wwa_colors[27] },
	{ wwa_labels[28], HW_HMWAR, wwa_colors[28] },
	{ wwa_labels[29], HW_VOWAR, wwa_colors[29] },
	{ wwa_labels[30], HW_EAWAR, wwa_colors[30] },
	{ wwa_labels[31], HW_FIWAR, wwa_colors[31] },	
	{ wwa_labels[32], WS_BLWAR, wwa_colors[32] },
	{ wwa_labels[33], WS_HSWAR, wwa_colors[33] },
	{ wwa_labels[34], WS_LEWAR, wwa_colors[34] },
	{ wwa_labels[35], WS_ISWAR, wwa_colors[35] },
	{ wwa_labels[36], WS_HLWAR, wwa_colors[36] },
	{ wwa_labels[37], WS_SNADV, wwa_colors[37] },
	{ wwa_labels[38], WS_LEADV, wwa_colors[38] },
	{ wwa_labels[39], WS_SLADV, wwa_colors[39] },
	{ wwa_labels[40], HW_TOWAR, wwa_colors[0] },
	{ wwa_labels[41], HW_TOWAT, wwa_colors[1] },
	{ wwa_labels[42], HW_STWAR, wwa_colors[2] },
	{ wwa_labels[43], HW_STWAT, wwa_colors[3] },
	{ wwa_labels[44], HW_SWS, wwa_colors[4] },
	{ wwa_labels[45], HW_FFWAR, wwa_colors[5] },
	{ wwa_labels[46], HW_FFWAT, wwa_colors[6] },
	{ wwa_labels[47], HW_TSWAR, wwa_colors[7] },
	{ wwa_labels[48], HW_TSWAT, wwa_colors[8] },
	{ wwa_labels[49], HW_HUWAR, wwa_colors[9] },
	{ wwa_labels[50], HW_HUWAT, wwa_colors[10] },
	{ wwa_labels[51], HW_FLWAR, wwa_colors[11] },
	{ wwa_labels[52], HW_FLWAT, wwa_colors[12] },
	{ wwa_labels[53], HW_WSWAR, wwa_colors[13] },
	{ wwa_labels[54], HW_WSWAT, wwa_colors[14] },
	{ wwa_labels[55], HW_SMWAR, wwa_colors[15] },
	{ wwa_labels[56], HW_WCWAR, wwa_colors[16] },
	{ wwa_labels[57], HW_WCADV, wwa_colors[17] },
	{ wwa_labels[58], HW_EHWAR, wwa_colors[18] },
	{ wwa_labels[59], HW_HEADV, wwa_colors[19] },
	{ wwa_labels[60], HW_HWWAR, wwa_colors[20] },
	{ wwa_labels[61], HW_WIADV, wwa_colors[21] },
	{ wwa_labels[62], HW_DFADV, wwa_colors[22] },
	{ wwa_labels[63], HW_FZWAR, wwa_colors[23] },
	{ wwa_labels[64], HW_LSWAR, wwa_colors[24] },
	{ wwa_labels[65], HW_BSADV, wwa_colors[25] },
	{ wwa_labels[66], HW_RAWAR, wwa_colors[26] },
	{ wwa_labels[67], HW_NUWAR, wwa_colors[27] },
	{ wwa_labels[68], HW_HMWAR, wwa_colors[28] },
	{ wwa_labels[69], HW_VOWAR, wwa_colors[29] },
	{ wwa_labels[70], HW_EAWAR, wwa_colors[30] },
	{ wwa_labels[71], HW_FIWAR, wwa_colors[31] },
	{ wwa_labels[72], WS_BLWAR, wwa_colors[32] },
	{ wwa_labels[73], WS_HSWAR, wwa_colors[33] },
	{ wwa_labels[74], WS_LEWAR, wwa_colors[34] },
	{ wwa_labels[75], WS_ISWAR, wwa_colors[35] },
	{ wwa_labels[76], WS_HLWAR, wwa_colors[36] },
	{ wwa_labels[77], WS_SNADV, wwa_colors[37] },
	{ wwa_labels[78], WS_LEADV, wwa_colors[38] },
	{ wwa_labels[79], WS_SLADV, wwa_colors[39] }

};

#else 
extern nws_wwa_match_code_t wwa[];
#endif


#endif
