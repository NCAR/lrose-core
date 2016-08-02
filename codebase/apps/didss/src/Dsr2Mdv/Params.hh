/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1997
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1997/9/26 14:18:54
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
/////////////////////////////////////////////////////////////
// Params.h: TDRP params object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1998
//
//////////////////////////////////////////////////////////

#ifndef PARAMS_H
#define PARAMS_H

#include <tdrp/tdrp.h>
#include "Dsr2Mdv_tdrp.h"
using namespace std;

class Params {
  
public:

  Dsr2Mdv_tdrp_struct p; // parameter struct

  int OK;
  int Done;

  Params (char *params_file_path,
	  tdrp_override_t *override,
	  char *prog_name,
	  int print_params,
	  int print_short);
  
protected:
  
private:

  TDRPtable *table;         // TDRP parsing table
  static Params* _instance;
  
};

#endif
