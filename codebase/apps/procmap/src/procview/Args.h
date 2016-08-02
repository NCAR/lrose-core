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
/////////////////////////////////////////////////////////////
// Args.h: Command line object - Singleton
/////////////////////////////////////////////////////////////

#ifndef ARGS_H
#define ARGS_H

#include <stdio.h>
#include <tdrp/tdrp.h>
#include "procview_tdrp.h"

class Args {
  
public:

  static Args *Inst (int argc, char **argv);
  static Args *Inst ();
  
  char *progName;
  char *paramsFilePath;
  
  // procmapHost has its own pointer since this may be
  // changed from the TDRP table value

  char *procmapHost;

  int OK;
  int checkParams;
  int printParams;
  int printFull;

  procview_tdrp_struct params;  // parameter struct
  
  static void setHostCallback ( void *clientData, char *hostName );
  static void selectHostCallback ( void *clientData, char *hostName );
  
protected:
  
  Args (int argc, char **argv);

private:

  static Args *_instance;
  TDRPtable *_table; // TDRP parsing table

  void usage(FILE *out);
  
};

#endif
