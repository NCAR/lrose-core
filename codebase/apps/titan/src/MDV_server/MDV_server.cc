/*********************************************************************
 * MDV_SERVER.C
 *
 * Provides a data service for MDV format data.
 * 
 * Frank Hage, RAP, NCAR, Boulder, CO, USA
 *
 * Updated by Mike Dixon, Dec 1991
 */

#define MAIN
#include "MDV_server.hh"
#undef MAIN

#include <toolsa/pmu.h>
using namespace std;

/********************************************************************
 * MAIN: Process arguments, initialize and begin application
 */

int main(int argc, char **argv)

{
  
  char *params_file_path = NULL;

  int check_params;
  int print_params;

  path_parts_t progname_parts;
  tdrp_override_t override;

  /*
   * set signal traps
   */

  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGQUIT, tidy_and_exit);
  PORTsignal(SIGPIPE, SIG_IGN);
  
  /*
   * allocate global structure and other memory, initialize
   */
  
  Glob = (global_t *) umalloc(sizeof(global_t));
  memset(Glob, 0, sizeof(global_t));
  
  /*
   * set program name
   */
  
  uparse_path(argv[0], &progname_parts);
  Glob->prog_name = (char *)
    umalloc ((ui32) (strlen(progname_parts.base) + 1));
  strcpy(Glob->prog_name, progname_parts.base);
  
  /*
   * display copyright
   */
  
  ucopyright(Glob->prog_name);

  /*
   * parse the command line
   */

  parse_args(argc, argv,
             &check_params, &print_params,
             &override,
             &params_file_path);
  
  /*
   * load up parameters
   */
  
  Glob->table = MDV_server_tdrp_init(&Glob->params);

  if (FALSE == TDRP_read(params_file_path,
                         Glob->table,
                         &Glob->params,
                         override.list)) {
    fprintf(stderr, "ERROR - %s:main\n", Glob->prog_name);
    fprintf(stderr, "Cannot read params file '%s'\n",
            params_file_path);
    tidy_and_exit(-1);
  }
  TDRP_free_override(&override);

  if (check_params) {
    TDRP_check_is_set(Glob->table, &Glob->params);
    exit(0);
  }
  
  if (print_params) {
    TDRP_print_params(Glob->table, &Glob->params, Glob->prog_name, TRUE);
    exit(0);
  }
  
  set_derived_params();
  
  /*
   * initialize server and process registration
   */

  register_server_init();

  PMU_auto_init(Glob->prog_name, Glob->params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  /*
   * set debugging for file searches
   */

  file_search_debug(Glob->params.debug);
  
  /*
   * put process in background
   */
  
  if (!Glob->params.debug) {
    udaemonize();
  }
  
  /*
   * Do the work
   */
  
  operate();
  
  return(0);
  
}

