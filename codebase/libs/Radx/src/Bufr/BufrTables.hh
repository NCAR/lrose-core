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
// BufrTables.hh
//
// Contains BUFR table information.
//
// Brenda Javornik, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2017
//
///////////////////////////////////////////////////////////////

#ifndef BufrTables_HH
#define BufrTables_HH

#include <stdlib.h>

class BufrTables {

public:

  BufrTables();
  ~BufrTables();

  //  const char **getMasterTableB(int number);
  // int getMasterTableBSize(int number);

  // master table B

  static const char *bufrtabb_2[];
  static const size_t N_bufrtabb_2;

  static const char *bufrtabb_6[];
  static const size_t N_bufrtabb_6;

  static const char *bufrtabb_11[];
  static const size_t N_bufrtabb_11;

  static const char *bufrtabb_12[];
  static const size_t N_bufrtabb_12;

  static const char *bufrtabb_13[];
  static const size_t N_bufrtabb_13;

  static const char *bufrtabb_14[];
  static const size_t N_bufrtabb_14;

  static const char *bufrtabb_15[];
  static const size_t N_bufrtabb_15;

  static const char *bufrtabb_16[];
  static const size_t N_bufrtabb_16;

  // master table D

  static const char *bufrtabd_2[];
  static const size_t N_bufrtabd_2;

  static const char *bufrtabd_6[];
  static const size_t N_bufrtabd_6;

  static const char *bufrtabd_11[];
  static const size_t N_bufrtabd_11;

  static const char *bufrtabd_12[];
  static const size_t N_bufrtabd_12;

  static const char *bufrtabd_13[];
  static const size_t N_bufrtabd_13;

  static const char *bufrtabd_14[];
  static const size_t N_bufrtabd_14;

  static const char *bufrtabd_15[];
  static const size_t N_bufrtabd_15;

  static const char *bufrtabd_16[];
  static const size_t N_bufrtabd_16;

  // local table B

  static const char *localtabb_7_1[];
  static const size_t N_localtabb_7_1;

  static const char *localtabb_41_2[];
  static const size_t N_localtabb_41_2;
  
  static const char *localtabb_85_0[];
  static const size_t N_localtabb_85_0;

  static const char *localtabb_85_1[];
  static const size_t N_localtabb_85_1;

  static const char *localtabb_85_2[];
  static const size_t N_localtabb_85_2;

  static const char *localtabb_85_10[];
  static const size_t N_localtabb_85_10;

  static const char *localtabb_85_12[];
  static const size_t N_localtabb_85_12;

  static const char *localtabb_85_14[];
  static const size_t N_localtabb_85_14;

  static const char *localtabb_247_7[];
  static const size_t N_localtabb_247_7;

  static const char *localtabb_247_8[];
  static const size_t N_localtabb_247_8;

  static const char *localtabb_247_9[];
  static const size_t N_localtabb_247_9;


  static const char *localtabb_255_1[];
  static const size_t N_localtabb_255_1;

  static const char *localtabb_65535_6[];
  static const size_t N_localtabb_65535_6;

  // local table D

  static const char *localtabd_7_1[];
  static const size_t N_localtabd_7_1;

  static const char *localtabd_41_2[];
  static const size_t N_localtabd_41_2;

  static const char *localtabd_85_0[];
  static const size_t N_localtabd_85_0;

  static const char *localtabd_85_1[];
  static const size_t N_localtabd_85_1;

  static const char *localtabd_85_2[];
  static const size_t N_localtabd_85_2;

  static const char *localtabd_85_10[];
  static const size_t N_localtabd_85_10;

  static const char *localtabd_85_12[];
  static const size_t N_localtabd_85_12;

  static const char *localtabd_85_14[];
  static const size_t N_localtabd_85_14;

  static const char *localtabd_247_7[];
  static const size_t N_localtabd_247_7;

  static const char *localtabd_247_8[];
  static const size_t N_localtabd_247_8;

  static const char *localtabd_247_9[];
  static const size_t N_localtabd_247_9;

  static const char *localtabd_255_1[];
  static const size_t N_localtabd_255_1;

  static const char *localtabd_65535_6[];
  static const size_t N_localtabd_65535_6;
  

};
#endif
