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
//////////////////////////////////////////////////////////
// Params.cc : TDRP parameters
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1998
//
//////////////////////////////////////////////////////////

#include "Params.hh"
#include <toolsa/umisc.h>
#include <toolsa/str.h>
using namespace std;

// Declare static instance of Params

Params::Params (char *params_file_path,
		tdrp_override_t *override,
		char *prog_name,
		int print_params,
		int print_short)

{

  OK = TRUE;
  Done = FALSE;
  
  table = Dsr2Mdv_tdrp_init(&p);

  if (FALSE == TDRP_read(params_file_path, table, &p,
			 override->list)) {
    fprintf(stderr, "ERROR - %s:Parms::Params\n", prog_name);
    fprintf(stderr, "Cannot read params file '%s'\n",
	    params_file_path);
    OK = FALSE;
    Done = TRUE;
    return;
  }
  TDRP_free_override(override);

  if (print_params) {
    TDRP_print_params(table, &p, prog_name, TRUE);
    Done = TRUE;
    return;
  } else if (print_short) {
    TDRP_print_params(table, &p, prog_name, FALSE);
    Done = TRUE;
    return;
  }
  
  // set malloc debug level

  if (p.malloc_debug_level > 0) {
    umalloc_debug(p.malloc_debug_level);
  }

  // if output fields are specified, check that DBZ is in the
  // list

//   if (p.specify_output_fields) {

//     int dbz_found = FALSE;
//     int n_fields_out = p.output_field_names.len;
//     for (int i = 0; i < n_fields_out; i++) {
//       char *fieldName = p.output_field_names.val[i];
//       if (!strcmp(fieldName, "DBZ")) {
// 	dbz_found = TRUE;
// 	break;
//       }
//     }

//     if (!dbz_found) {
//       fprintf(stderr, "ERROR - %s:Params\n", prog_name);
//       fprintf(stderr, "No \"DBZ\" field in the output list.\n");
//       fprintf(stderr, "The DBZ field is required.\n");
//       OK = FALSE;
//       Done = TRUE;
//     }

//   }

  return;

}




