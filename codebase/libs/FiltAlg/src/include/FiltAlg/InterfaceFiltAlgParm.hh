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
/**
 * @file InterfaceFiltAlgParm.hh
 * @brief static functions for param files and standard application actions.
 * @class InterfaceFiltAlgParm
 * @brief static functions for param files and standard application actions.
 */
# ifndef    INTERFACE_FILT_ALG_PARM_HH
# define    INTERFACE_FILT_ALG_PARM_HH
#include <FiltAlg/FiltAlgParams.hh>

//------------------------------------------------------------------
class InterfaceFiltAlgParm
{
public:

  /**
   * Default constructor
   */
  InterfaceFiltAlgParm(void);
  
  /**
   * Destructor
   */
  virtual ~InterfaceFiltAlgParm(void);

  /**
   * default driver initialization steps for apps.
   * @param[in] app_name  Name of the app
   * @param[in] p  base class params used by all FiltAlg apps
   * @param[in] cleanup a cleanup function to call on various exit signals.
   * @return true if init was successful
   */
  static bool driver_init(const string &app_name, const FiltAlgParams &p,
			  void cleanup(int));

  /**
   * Process informs any monitoring software that it is terminating.
   */
  static void finish(void);

  /**
   * default triggering method. Waits till new data triggers a return.
   *
   * @param[in] p params that are set on entry
   * @param[out] t   time that was triggered.
   * @return true if a time was triggered, false for no more triggering.
   */
  static bool driver_trigger(const FiltAlgParams &p, time_t &t);

  /**
   * Load in params for the standard app
   *
   * @param[out] p main parameter object
   *
   * @note app typically calls ParmFiltAlgAppInit(T &app_params, int argc, char **argv)
   * template function (see ParmFiltAlgApp.hh) prior to calling this.
   */
  static bool load_params(FiltAlgParams &p);

  /**
   * Set state about param files from command line args.
   * @param[in] argc number of command line args
   * @param[in] argv the args
   * @return true if these args agree with expected TDRP values
   * specifically -params <parmfile>   and -print_params
   *
   * @note called from within the ParmFiltAlgAppInit template function
   */
  static bool parm_init(int argc, char **argv);

  /**
   *  finish up parameter file actions after loading in all params
   *
   * @note called from within the ParmFiltAlgAppFinish template function
   */
  static void parm_finish(void);

  /**
   *  return name of the path for the param file specified by command line args
   *
   * @note app typically calls ParmFiltAlgAppInit(T &app_params, int argc, char **argv)
   * template function (see ParmFiltAlgApp.hh) prior to calling this.
   */
  static const char *parm_path(void);

  /**
   *  return true if user wants to load params as specified by command line args
   *
   * @note app typically calls ParmFiltAlgAppInit(T &app_params, int argc, char **argv)
   * template function (see ParmFiltAlgApp.hh) prior to calling this.
   */
  static bool is_parm_load(void);

  /**
   *  return true if user wants to print params as specified by command line args
   *
   * @note app typically calls ParmFiltAlgAppInit(T &app_params, int argc, char **argv)
   * template function (see ParmFiltAlgApp.hh) prior to calling this.
   */
  static bool is_parm_print(void);

  /**
   * create a generic example of the FiltAlgParams params, write to fname
   * @param[out] fname
   * @return true if successful
   */
  static bool create_example_params(const string &fname);

  /**
   * parse standard commnd line args, return range of times if present
   * in the args.
   *
   * The range of times [t0,t1] is found in command line arguments
   *
   * @param[in] argc command argument size
   * @param[in] argv command arguments
   * @param[out] t0  Earliest time (if found)
   * @param[out] t1  Latest time (if found)
   *
   * @return true if found the times in the command line args
   *
   * @note will be looking for '-interval yyyymmddhhmmss yyyymmddhhmmss'
   */
  static bool get_archive_cmdarg_range(int argc, char **argv, time_t &t0,
				       time_t &t1);

  /**
   *  @return most recent time. if realtime, return now, else return upper
   * limit of archive times
   */
  static time_t most_recent_time(void);

  /**
   *  @return oldest time. if realtime, return now, else return lower
   * limit of archive times
   */
  static time_t oldest_time(void);

  /**
   * set the main triggering log message to on or off
   * @param[in] stat 
   */
  static void set_triggering_print(const bool stat);


  /**
   * return true if command args indicated params should be printed
   * @note app typically calls ParmFiltAlgAppInit(T &app_params, int argc, char **argv)
   * template function (see ParmFiltAlgApp.hh) prior to calling this.
   */
  static bool param_type_is_print(void);

  /**
   * return true if command args indicated params should be printed and loaded
   * @note app typically calls ParmFiltAlgAppInit(T &app_params, int argc, char **argv)
   * template function (see ParmFiltAlgApp.hh) prior to calling this.
   */
  static bool param_type_is_print_and_load(void);

protected:
private:  

};

# endif
