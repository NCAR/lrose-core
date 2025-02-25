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
  
  static constexpr int MAX_DATA_FIELDS = 1000;
  static constexpr int MAX_SECTS = 128;
  static constexpr int MAX_COLORS = 1024;
  static constexpr int MAX_FRAMES = 1024;
  static constexpr int MAX_ROUTE = 128;
  static constexpr int MAX_COLOR_CELLS = 65536;

  static constexpr int MAX_PATH = 1024;
  static constexpr int MAX_TITLE = 1024;

  static constexpr int NUM_GRID_LAYERS = 16;
  static constexpr int NUM_CONT_LAYERS = 16;
  static constexpr int NUM_PRODUCT_DETAIL_THRESHOLDS = 3;
  
  static constexpr int NAME_LENGTH = 256;
  static constexpr int LABEL_LENGTH = 256;

  

};

#endif

