/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
//////////////////////////////////////////////////////////////////////////////
// Class for Lucid global constants.
//////////////////////////////////////////////////////////////////////////////
 
#ifndef CONSTANTS_HH
#define CONSTANTS_HH

// GLOBAL DATA object

class Constants {

public:

  static constexpr const char *LUCID_VERSION = "1.0 2025/03/01";
  static constexpr const char *LUCID_COPYRIGHT =
    "(c) 1991-2024,  National Center for Atmospheric Research";

  // return codes
  
  static constexpr int CIDD_SUCCESS = 0;
  static constexpr int CIDD_FAILURE = 1;
  static constexpr int INCOMPLETE = 2;

  // array sizes
  
  static constexpr int MAX_DATA_FIELDS = 1000;
  static constexpr int MAX_SECTS = 128;
  static constexpr int MAX_COLORS = 1024;
  static constexpr int MAX_FRAMES = 1024;
  static constexpr int MAX_ROUTE = 128;
  static constexpr int MAX_COLOR_CELLS = 65536;
  static constexpr int MAX_CACHE_PIXMAPS = 64;

  static constexpr int NUM_GRID_LAYERS = 16;
  static constexpr int NUM_CONT_LAYERS = 16;
  static constexpr int NUM_PRODUCT_DETAIL_THRESHOLDS = 3;
  static constexpr int NUM_CUSTOM_ZOOMS = 3;

  static constexpr int NUM_PARSE_FIELDS = (MAX_ROUTE +2) * 3;
  static constexpr int PARSE_FIELD_SIZE = 1024;
  
  // string max len
  
  static constexpr int MAX_PATH = 1024;
  static constexpr int MAX_TITLE = 1024;
  static constexpr int NAME_LENGTH = 256;
  static constexpr int TITLE_LENGTH = 256;
  static constexpr int LABEL_LENGTH = 256;
  static constexpr int URL_LENGTH = 1024;
  static constexpr int URL_MIN_SIZE = 8;

  // math and physical constants
  
  static constexpr double LUCID_DEG_TO_RAD = 0.017453292519943295; // degrees to radians conversion
  static constexpr double LUCID_DEG_RAD = 57.29577951308232;  // degrees per radian

  static constexpr double LUCID_RADIAN90 = 1.5707963267948966; // radian value for 90 degrees 
  static constexpr double LUCID_RADIAN180 = 3.1415926535897930; // radian value for 180 degrees 
  static constexpr double LUCID_RADIAN270 = 4.7123889803846900; // radian value for 270 degrees 

  static constexpr double LUCID_KM_PER_DEG_AT_EQ = 111.198487;
  static constexpr double LUCID_PSEUDO_RADIUS = 8533.0; // 4/3 earth radius

};

#endif

