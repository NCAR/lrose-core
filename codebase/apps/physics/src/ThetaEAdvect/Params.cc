/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR                                                         */
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
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED 'AS IS' AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
////////////////////////////////////////////
// Params.cc
//
// TDRP C++ code file for class 'Params'.
//
// Code for program ThetaEAdvect
//
// This file has been automatically
// generated by TDRP, do not modify.
//
/////////////////////////////////////////////

/**
 *
 * @file Params.cc
 *
 * @class Params
 *
 * This class is automatically generated by the Table
 * Driven Runtime Parameters (TDRP) system
 *
 * @note Source is automatically generated from
 *       paramdef file at compile time, do not modify
 *       since modifications will be overwritten.
 *
 *
 * @author Automatically generated
 *
 */
#include "Params.hh"
#include <cstring>

  ////////////////////////////////////////////
  // Default constructor
  //

  Params::Params()

  {

    // zero out table

    memset(_table, 0, sizeof(_table));

    // zero out members

    memset(&_start_, 0, &_end_ - &_start_);

    // class name

    _className = "Params";

    // initialize table

    _init();

    // set members

    tdrpTable2User(_table, &_start_);

    _exitDeferred = false;

  }

  ////////////////////////////////////////////
  // Copy constructor
  //

  Params::Params(const Params& source)

  {

    // sync the source object

    source.sync();

    // zero out table

    memset(_table, 0, sizeof(_table));

    // zero out members

    memset(&_start_, 0, &_end_ - &_start_);

    // class name

    _className = "Params";

    // copy table

    tdrpCopyTable((TDRPtable *) source._table, _table);

    // set members

    tdrpTable2User(_table, &_start_);

    _exitDeferred = false;

  }

  ////////////////////////////////////////////
  // Destructor
  //

  Params::~Params()

  {

    // free up

    freeAll();

  }

  ////////////////////////////////////////////
  // Assignment
  //

  void Params::operator=(const Params& other)

  {

    // sync the other object

    other.sync();

    // free up any existing memory

    freeAll();

    // zero out table

    memset(_table, 0, sizeof(_table));

    // zero out members

    memset(&_start_, 0, &_end_ - &_start_);

    // copy table

    tdrpCopyTable((TDRPtable *) other._table, _table);

    // set members

    tdrpTable2User(_table, &_start_);

    _exitDeferred = other._exitDeferred;

  }

  ////////////////////////////////////////////
  // loadFromArgs()
  //
  // Loads up TDRP using the command line args.
  //
  // Check usage() for command line actions associated with
  // this function.
  //
  //   argc, argv: command line args
  //
  //   char **override_list: A null-terminated list of overrides
  //     to the parameter file.
  //     An override string has exactly the format of an entry
  //     in the parameter file itself.
  //
  //   char **params_path_p:
  //     If this is non-NULL, it is set to point to the path
  //     of the params file used.
  //
  //   bool defer_exit: normally, if the command args contain a 
  //      print or check request, this function will call exit().
  //      If defer_exit is set, such an exit is deferred and the
  //      private member _exitDeferred is set.
  //      Use exidDeferred() to test this flag.
  //
  //  Returns 0 on success, -1 on failure.
  //

  int Params::loadFromArgs(int argc, char **argv,
                           char **override_list,
                           char **params_path_p,
                           bool defer_exit)
  {
    int exit_deferred;
    if (_tdrpLoadFromArgs(argc, argv,
                          _table, &_start_,
                          override_list, params_path_p,
                          _className,
                          defer_exit, &exit_deferred)) {
      return (-1);
    } else {
      if (exit_deferred) {
        _exitDeferred = true;
      }
      return (0);
    }
  }

  ////////////////////////////////////////////
  // loadApplyArgs()
  //
  // Loads up TDRP using the params path passed in, and applies
  // the command line args for printing and checking.
  //
  // Check usage() for command line actions associated with
  // this function.
  //
  //   const char *param_file_path: the parameter file to be read in
  //
  //   argc, argv: command line args
  //
  //   char **override_list: A null-terminated list of overrides
  //     to the parameter file.
  //     An override string has exactly the format of an entry
  //     in the parameter file itself.
  //
  //   bool defer_exit: normally, if the command args contain a 
  //      print or check request, this function will call exit().
  //      If defer_exit is set, such an exit is deferred and the
  //      private member _exitDeferred is set.
  //      Use exidDeferred() to test this flag.
  //
  //  Returns 0 on success, -1 on failure.
  //

  int Params::loadApplyArgs(const char *params_path,
                            int argc, char **argv,
                            char **override_list,
                            bool defer_exit)
  {
    int exit_deferred;
    if (tdrpLoadApplyArgs(params_path, argc, argv,
                          _table, &_start_,
                          override_list,
                          _className,
                          defer_exit, &exit_deferred)) {
      return (-1);
    } else {
      if (exit_deferred) {
        _exitDeferred = true;
      }
      return (0);
    }
  }

  ////////////////////////////////////////////
  // isArgValid()
  // 
  // Check if a command line arg is a valid TDRP arg.
  //

  bool Params::isArgValid(const char *arg)
  {
    return (tdrpIsArgValid(arg));
  }

  ////////////////////////////////////////////
  // isArgValid()
  // 
  // Check if a command line arg is a valid TDRP arg.
  // return number of args consumed.
  //

  int Params::isArgValidN(const char *arg)
  {
    return (tdrpIsArgValidN(arg));
  }

  ////////////////////////////////////////////
  // load()
  //
  // Loads up TDRP for a given class.
  //
  // This version of load gives the programmer the option to load
  // up more than one class for a single application. It is a
  // lower-level routine than loadFromArgs, and hence more
  // flexible, but the programmer must do more work.
  //
  //   const char *param_file_path: the parameter file to be read in.
  //
  //   char **override_list: A null-terminated list of overrides
  //     to the parameter file.
  //     An override string has exactly the format of an entry
  //     in the parameter file itself.
  //
  //   expand_env: flag to control environment variable
  //               expansion during tokenization.
  //               If TRUE, environment expansion is set on.
  //               If FALSE, environment expansion is set off.
  //
  //  Returns 0 on success, -1 on failure.
  //

  int Params::load(const char *param_file_path,
                   char **override_list,
                   int expand_env, int debug)
  {
    if (tdrpLoad(param_file_path,
                 _table, &_start_,
                 override_list,
                 expand_env, debug)) {
      return (-1);
    } else {
      return (0);
    }
  }

  ////////////////////////////////////////////
  // loadFromBuf()
  //
  // Loads up TDRP for a given class.
  //
  // This version of load gives the programmer the option to
  // load up more than one module for a single application,
  // using buffers which have been read from a specified source.
  //
  //   const char *param_source_str: a string which describes the
  //     source of the parameter information. It is used for
  //     error reporting only.
  //
  //   char **override_list: A null-terminated list of overrides
  //     to the parameter file.
  //     An override string has exactly the format of an entry
  //     in the parameter file itself.
  //
  //   const char *inbuf: the input buffer
  //
  //   int inlen: length of the input buffer
  //
  //   int start_line_num: the line number in the source which
  //     corresponds to the start of the buffer.
  //
  //   expand_env: flag to control environment variable
  //               expansion during tokenization.
  //               If TRUE, environment expansion is set on.
  //               If FALSE, environment expansion is set off.
  //
  //  Returns 0 on success, -1 on failure.
  //

  int Params::loadFromBuf(const char *param_source_str,
                          char **override_list,
                          const char *inbuf, int inlen,
                          int start_line_num,
                          int expand_env, int debug)
  {
    if (tdrpLoadFromBuf(param_source_str,
                        _table, &_start_,
                        override_list,
                        inbuf, inlen, start_line_num,
                        expand_env, debug)) {
      return (-1);
    } else {
      return (0);
    }
  }

  ////////////////////////////////////////////
  // loadDefaults()
  //
  // Loads up default params for a given class.
  //
  // See load() for more detailed info.
  //
  //  Returns 0 on success, -1 on failure.
  //

  int Params::loadDefaults(int expand_env)
  {
    if (tdrpLoad(NULL,
                 _table, &_start_,
                 NULL, expand_env, FALSE)) {
      return (-1);
    } else {
      return (0);
    }
  }

  ////////////////////////////////////////////
  // sync()
  //
  // Syncs the user struct data back into the parameter table,
  // in preparation for printing.
  //
  // This function alters the table in a consistent manner.
  // Therefore it can be regarded as const.
  //

  void Params::sync(void) const
  {
    tdrpUser2Table(_table, (char *) &_start_);
  }

  ////////////////////////////////////////////
  // print()
  // 
  // Print params file
  //
  // The modes supported are:
  //
  //   PRINT_SHORT:   main comments only, no help or descriptions
  //                  structs and arrays on a single line
  //   PRINT_NORM:    short + descriptions and help
  //   PRINT_LONG:    norm  + arrays and structs expanded
  //   PRINT_VERBOSE: long  + private params included
  //

  void Params::print(FILE *out, tdrp_print_mode_t mode)
  {
    tdrpPrint(out, _table, _className, mode);
  }

  ////////////////////////////////////////////
  // checkAllSet()
  //
  // Return TRUE if all set, FALSE if not.
  //
  // If out is non-NULL, prints out warning messages for those
  // parameters which are not set.
  //

  int Params::checkAllSet(FILE *out)
  {
    return (tdrpCheckAllSet(out, _table, &_start_));
  }

  //////////////////////////////////////////////////////////////
  // checkIsSet()
  //
  // Return TRUE if parameter is set, FALSE if not.
  //
  //

  int Params::checkIsSet(const char *paramName)
  {
    return (tdrpCheckIsSet(paramName, _table, &_start_));
  }

  ////////////////////////////////////////////
  // freeAll()
  //
  // Frees up all TDRP dynamic memory.
  //

  void Params::freeAll(void)
  {
    tdrpFreeAll(_table, &_start_);
  }

  ////////////////////////////////////////////
  // usage()
  //
  // Prints out usage message for TDRP args as passed
  // in to loadFromArgs().
  //

  void Params::usage(ostream &out)
  {
    out << "TDRP args: [options as below]\n"
        << "   [ -params/--params path ] specify params file path\n"
        << "   [ -check_params/--check_params] check which params are not set\n"
        << "   [ -print_params/--print_params [mode]] print parameters\n"
        << "     using following modes, default mode is 'norm'\n"
        << "       short:   main comments only, no help or descr\n"
        << "                structs and arrays on a single line\n"
        << "       norm:    short + descriptions and help\n"
        << "       long:    norm  + arrays and structs expanded\n"
        << "       verbose: long  + private params included\n"
        << "       short_expand:   short with env vars expanded\n"
        << "       norm_expand:    norm with env vars expanded\n"
        << "       long_expand:    long with env vars expanded\n"
        << "       verbose_expand: verbose with env vars expanded\n"
        << "   [ -tdrp_debug] debugging prints for tdrp\n"
        << "   [ -tdrp_usage] print this usage\n";
  }

  ////////////////////////////////////////////
  // arrayRealloc()
  //
  // Realloc 1D array.
  //
  // If size is increased, the values from the last array 
  // entry is copied into the new space.
  //
  // Returns 0 on success, -1 on error.
  //

  int Params::arrayRealloc(const char *param_name, int new_array_n)
  {
    if (tdrpArrayRealloc(_table, &_start_,
                         param_name, new_array_n)) {
      return (-1);
    } else {
      return (0);
    }
  }

  ////////////////////////////////////////////
  // array2DRealloc()
  //
  // Realloc 2D array.
  //
  // If size is increased, the values from the last array 
  // entry is copied into the new space.
  //
  // Returns 0 on success, -1 on error.
  //

  int Params::array2DRealloc(const char *param_name,
                             int new_array_n1,
                             int new_array_n2)
  {
    if (tdrpArray2DRealloc(_table, &_start_, param_name,
                           new_array_n1, new_array_n2)) {
      return (-1);
    } else {
      return (0);
    }
  }

  ////////////////////////////////////////////
  // _init()
  //
  // Class table initialization function.
  //
  //

  void Params::_init()

  {

    TDRPtable *tt = _table;

    // Parameter 'Comment 0'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = COMMENT_TYPE;
    tt->param_name = tdrpStrDup("Comment 0");
    tt->comment_hdr = tdrpStrDup("DEBUGGING PARAMETERS");
    tt->comment_text = tdrpStrDup("Parameters controlling debug outputs.");
    tt++;
    
    // Parameter 'debug'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("debug");
    tt->descr = tdrpStrDup("debug flag");
    tt->help = tdrpStrDup("Debug flag.");
    tt->val_offset = (char *) &debug - &_start_;
    tt->single_val.b = pFALSE;
    tt++;
    
    // Parameter 'Comment 1'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = COMMENT_TYPE;
    tt->param_name = tdrpStrDup("Comment 1");
    tt->comment_hdr = tdrpStrDup("PROCESS PARAMETERS");
    tt->comment_text = tdrpStrDup("");
    tt++;
    
    // Parameter 'instance'
    // ctype is 'char*'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRING_TYPE;
    tt->param_name = tdrpStrDup("instance");
    tt->descr = tdrpStrDup("Process instance");
    tt->help = tdrpStrDup("Used for registration with procmap.");
    tt->val_offset = (char *) &instance - &_start_;
    tt->single_val.s = tdrpStrDup("Test");
    tt++;
    
    // Parameter 'Comment 2'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = COMMENT_TYPE;
    tt->param_name = tdrpStrDup("Comment 2");
    tt->comment_hdr = tdrpStrDup("PROCESS I/O PARAMETERS");
    tt->comment_text = tdrpStrDup("Parameters describing the input and output locations.");
    tt++;
    
    // Parameter 'trigger_mode'
    // ctype is '_trigger_mode_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = ENUM_TYPE;
    tt->param_name = tdrpStrDup("trigger_mode");
    tt->descr = tdrpStrDup("Input triggering mode");
    tt->help = tdrpStrDup("In LATEST_DATA mode, the program waits for new data from the MDV location specified by the latest_data_trigger parameter.\nIn TIME_LIST mode, the program operates on archive data as specified in the time_list_trigger parameter.\nIn INTERVAL mode, the program operates on archive data as specified in the interval_trigger parameter.");
    tt->val_offset = (char *) &trigger_mode - &_start_;
    tt->enum_def.name = tdrpStrDup("trigger_mode_t");
    tt->enum_def.nfields = 3;
    tt->enum_def.fields = (enum_field_t *)
        tdrpMalloc(tt->enum_def.nfields * sizeof(enum_field_t));
      tt->enum_def.fields[0].name = tdrpStrDup("LATEST_DATA");
      tt->enum_def.fields[0].val = LATEST_DATA;
      tt->enum_def.fields[1].name = tdrpStrDup("TIME_LIST");
      tt->enum_def.fields[1].val = TIME_LIST;
      tt->enum_def.fields[2].name = tdrpStrDup("INTERVAL");
      tt->enum_def.fields[2].val = INTERVAL;
    tt->single_val.e = LATEST_DATA;
    tt++;
    
    // Parameter 'latest_data_trigger'
    // ctype is 'char*'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRING_TYPE;
    tt->param_name = tdrpStrDup("latest_data_trigger");
    tt->descr = tdrpStrDup("URL to use when using a LATEST_DATA trigger");
    tt->help = tdrpStrDup("");
    tt->val_offset = (char *) &latest_data_trigger - &_start_;
    tt->single_val.s = tdrpStrDup("mdvp:://localhost::./mdv/ruc");
    tt++;
    
    // Parameter 'time_list_trigger'
    // ctype is '_time_list_trigger_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRUCT_TYPE;
    tt->param_name = tdrpStrDup("time_list_trigger");
    tt->descr = tdrpStrDup("Trigger information used when using the TIME_LIST trigger");
    tt->help = tdrpStrDup("url specifies the URL to use to get the available data times.\nstart_time specifies the archive start time in any format recognized by the DateTime class.\nend_time specifies the archive end time in any format recognized by the DateTime class.");
    tt->val_offset = (char *) &time_list_trigger - &_start_;
    tt->struct_def.name = tdrpStrDup("time_list_trigger_t");
    tt->struct_def.nfields = 3;
    tt->struct_def.fields = (struct_field_t *)
        tdrpMalloc(tt->struct_def.nfields * sizeof(struct_field_t));
      tt->struct_def.fields[0].ftype = tdrpStrDup("string");
      tt->struct_def.fields[0].fname = tdrpStrDup("url");
      tt->struct_def.fields[0].ptype = STRING_TYPE;
      tt->struct_def.fields[0].rel_offset = 
        (char *) &time_list_trigger.url - (char *) &time_list_trigger;
      tt->struct_def.fields[1].ftype = tdrpStrDup("string");
      tt->struct_def.fields[1].fname = tdrpStrDup("start_time");
      tt->struct_def.fields[1].ptype = STRING_TYPE;
      tt->struct_def.fields[1].rel_offset = 
        (char *) &time_list_trigger.start_time - (char *) &time_list_trigger;
      tt->struct_def.fields[2].ftype = tdrpStrDup("string");
      tt->struct_def.fields[2].fname = tdrpStrDup("end_time");
      tt->struct_def.fields[2].ptype = STRING_TYPE;
      tt->struct_def.fields[2].rel_offset = 
        (char *) &time_list_trigger.end_time - (char *) &time_list_trigger;
    tt->n_struct_vals = 3;
    tt->struct_vals = (tdrpVal_t *)
        tdrpMalloc(tt->n_struct_vals * sizeof(tdrpVal_t));
      tt->struct_vals[0].s = tdrpStrDup("mdvp:://localhost::./mdv/ruc");
      tt->struct_vals[1].s = tdrpStrDup("2001/1/1 00:00:00");
      tt->struct_vals[2].s = tdrpStrDup("2002/1/2 00:00:00");
    tt++;
    
    // Parameter 'interval_trigger'
    // ctype is '_interval_trigger_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRUCT_TYPE;
    tt->param_name = tdrpStrDup("interval_trigger");
    tt->descr = tdrpStrDup("Trigger information used when using the INTERVAL trigger");
    tt->help = tdrpStrDup("start_time specifies the archive start time in any format recognized by the DateTime class.\nend_time specifies the archive end time in any format recognized by the DateTime class.\ninterval gives the number of seconds between each trigger time.");
    tt->val_offset = (char *) &interval_trigger - &_start_;
    tt->struct_def.name = tdrpStrDup("interval_trigger_t");
    tt->struct_def.nfields = 3;
    tt->struct_def.fields = (struct_field_t *)
        tdrpMalloc(tt->struct_def.nfields * sizeof(struct_field_t));
      tt->struct_def.fields[0].ftype = tdrpStrDup("string");
      tt->struct_def.fields[0].fname = tdrpStrDup("start_time");
      tt->struct_def.fields[0].ptype = STRING_TYPE;
      tt->struct_def.fields[0].rel_offset = 
        (char *) &interval_trigger.start_time - (char *) &interval_trigger;
      tt->struct_def.fields[1].ftype = tdrpStrDup("string");
      tt->struct_def.fields[1].fname = tdrpStrDup("end_time");
      tt->struct_def.fields[1].ptype = STRING_TYPE;
      tt->struct_def.fields[1].rel_offset = 
        (char *) &interval_trigger.end_time - (char *) &interval_trigger;
      tt->struct_def.fields[2].ftype = tdrpStrDup("long");
      tt->struct_def.fields[2].fname = tdrpStrDup("interval");
      tt->struct_def.fields[2].ptype = LONG_TYPE;
      tt->struct_def.fields[2].rel_offset = 
        (char *) &interval_trigger.interval - (char *) &interval_trigger;
    tt->n_struct_vals = 3;
    tt->struct_vals = (tdrpVal_t *)
        tdrpMalloc(tt->n_struct_vals * sizeof(tdrpVal_t));
      tt->struct_vals[0].s = tdrpStrDup("2001/1/1 00:00:00");
      tt->struct_vals[1].s = tdrpStrDup("2001/1/2 00:00:00");
      tt->struct_vals[2].l = 3600;
    tt++;
    
    // Parameter 'mixing_ratio_field_info'
    // ctype is '_input_info_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRUCT_TYPE;
    tt->param_name = tdrpStrDup("mixing_ratio_field_info");
    tt->descr = tdrpStrDup("Information about the input mixing ratio field");
    tt->help = tdrpStrDup("Uses field_name to identify field in MDV file unless field_name is \"\" in which case it uses field_num to identify the input field.\nNote that this data is expected to be in g/kg.");
    tt->val_offset = (char *) &mixing_ratio_field_info - &_start_;
    tt->struct_def.name = tdrpStrDup("input_info_t");
    tt->struct_def.nfields = 3;
    tt->struct_def.fields = (struct_field_t *)
        tdrpMalloc(tt->struct_def.nfields * sizeof(struct_field_t));
      tt->struct_def.fields[0].ftype = tdrpStrDup("string");
      tt->struct_def.fields[0].fname = tdrpStrDup("url");
      tt->struct_def.fields[0].ptype = STRING_TYPE;
      tt->struct_def.fields[0].rel_offset = 
        (char *) &mixing_ratio_field_info.url - (char *) &mixing_ratio_field_info;
      tt->struct_def.fields[1].ftype = tdrpStrDup("string");
      tt->struct_def.fields[1].fname = tdrpStrDup("field_name");
      tt->struct_def.fields[1].ptype = STRING_TYPE;
      tt->struct_def.fields[1].rel_offset = 
        (char *) &mixing_ratio_field_info.field_name - (char *) &mixing_ratio_field_info;
      tt->struct_def.fields[2].ftype = tdrpStrDup("long");
      tt->struct_def.fields[2].fname = tdrpStrDup("field_num");
      tt->struct_def.fields[2].ptype = LONG_TYPE;
      tt->struct_def.fields[2].rel_offset = 
        (char *) &mixing_ratio_field_info.field_num - (char *) &mixing_ratio_field_info;
    tt->n_struct_vals = 3;
    tt->struct_vals = (tdrpVal_t *)
        tdrpMalloc(tt->n_struct_vals * sizeof(tdrpVal_t));
      tt->struct_vals[0].s = tdrpStrDup("mdvp:://localhost::./mdv/ruc");
      tt->struct_vals[1].s = tdrpStrDup("mr");
      tt->struct_vals[2].l = 0;
    tt++;
    
    // Parameter 'temp_field_info'
    // ctype is '_input_info_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRUCT_TYPE;
    tt->param_name = tdrpStrDup("temp_field_info");
    tt->descr = tdrpStrDup("Information about the input temperature field");
    tt->help = tdrpStrDup("Uses field_name to identify field in MDV file unless field_name is \"\" in which case it uses field_num to identify the input field.\nNote that this data is expected to be in Kelvin.");
    tt->val_offset = (char *) &temp_field_info - &_start_;
    tt->struct_def.name = tdrpStrDup("input_info_t");
    tt->struct_def.nfields = 3;
    tt->struct_def.fields = (struct_field_t *)
        tdrpMalloc(tt->struct_def.nfields * sizeof(struct_field_t));
      tt->struct_def.fields[0].ftype = tdrpStrDup("string");
      tt->struct_def.fields[0].fname = tdrpStrDup("url");
      tt->struct_def.fields[0].ptype = STRING_TYPE;
      tt->struct_def.fields[0].rel_offset = 
        (char *) &temp_field_info.url - (char *) &temp_field_info;
      tt->struct_def.fields[1].ftype = tdrpStrDup("string");
      tt->struct_def.fields[1].fname = tdrpStrDup("field_name");
      tt->struct_def.fields[1].ptype = STRING_TYPE;
      tt->struct_def.fields[1].rel_offset = 
        (char *) &temp_field_info.field_name - (char *) &temp_field_info;
      tt->struct_def.fields[2].ftype = tdrpStrDup("long");
      tt->struct_def.fields[2].fname = tdrpStrDup("field_num");
      tt->struct_def.fields[2].ptype = LONG_TYPE;
      tt->struct_def.fields[2].rel_offset = 
        (char *) &temp_field_info.field_num - (char *) &temp_field_info;
    tt->n_struct_vals = 3;
    tt->struct_vals = (tdrpVal_t *)
        tdrpMalloc(tt->n_struct_vals * sizeof(tdrpVal_t));
      tt->struct_vals[0].s = tdrpStrDup("mdvp:://localhost::./mdv/ruc");
      tt->struct_vals[1].s = tdrpStrDup("TMP");
      tt->struct_vals[2].l = 0;
    tt++;
    
    // Parameter 'pressure_field_info'
    // ctype is '_input_info_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRUCT_TYPE;
    tt->param_name = tdrpStrDup("pressure_field_info");
    tt->descr = tdrpStrDup("Information about the input pressure field");
    tt->help = tdrpStrDup("Uses field_name to identify field in MDV file unless field_name is \"\" in which case it uses field_num to identify the input field.\nNote that this data is expected to be in mb.");
    tt->val_offset = (char *) &pressure_field_info - &_start_;
    tt->struct_def.name = tdrpStrDup("input_info_t");
    tt->struct_def.nfields = 3;
    tt->struct_def.fields = (struct_field_t *)
        tdrpMalloc(tt->struct_def.nfields * sizeof(struct_field_t));
      tt->struct_def.fields[0].ftype = tdrpStrDup("string");
      tt->struct_def.fields[0].fname = tdrpStrDup("url");
      tt->struct_def.fields[0].ptype = STRING_TYPE;
      tt->struct_def.fields[0].rel_offset = 
        (char *) &pressure_field_info.url - (char *) &pressure_field_info;
      tt->struct_def.fields[1].ftype = tdrpStrDup("string");
      tt->struct_def.fields[1].fname = tdrpStrDup("field_name");
      tt->struct_def.fields[1].ptype = STRING_TYPE;
      tt->struct_def.fields[1].rel_offset = 
        (char *) &pressure_field_info.field_name - (char *) &pressure_field_info;
      tt->struct_def.fields[2].ftype = tdrpStrDup("long");
      tt->struct_def.fields[2].fname = tdrpStrDup("field_num");
      tt->struct_def.fields[2].ptype = LONG_TYPE;
      tt->struct_def.fields[2].rel_offset = 
        (char *) &pressure_field_info.field_num - (char *) &pressure_field_info;
    tt->n_struct_vals = 3;
    tt->struct_vals = (tdrpVal_t *)
        tdrpMalloc(tt->n_struct_vals * sizeof(tdrpVal_t));
      tt->struct_vals[0].s = tdrpStrDup("mdvp:://localhost::./mdv/ruc");
      tt->struct_vals[1].s = tdrpStrDup("PRS");
      tt->struct_vals[2].l = 0;
    tt++;
    
    // Parameter 'u_field_info'
    // ctype is '_input_info_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRUCT_TYPE;
    tt->param_name = tdrpStrDup("u_field_info");
    tt->descr = tdrpStrDup("Information about the input U field");
    tt->help = tdrpStrDup("Uses field_name to identify field in MDV file unless field_name is \"\" in which case it uses field_num to identify the input field.\nNote that this data is expected to be in m/s.");
    tt->val_offset = (char *) &u_field_info - &_start_;
    tt->struct_def.name = tdrpStrDup("input_info_t");
    tt->struct_def.nfields = 3;
    tt->struct_def.fields = (struct_field_t *)
        tdrpMalloc(tt->struct_def.nfields * sizeof(struct_field_t));
      tt->struct_def.fields[0].ftype = tdrpStrDup("string");
      tt->struct_def.fields[0].fname = tdrpStrDup("url");
      tt->struct_def.fields[0].ptype = STRING_TYPE;
      tt->struct_def.fields[0].rel_offset = 
        (char *) &u_field_info.url - (char *) &u_field_info;
      tt->struct_def.fields[1].ftype = tdrpStrDup("string");
      tt->struct_def.fields[1].fname = tdrpStrDup("field_name");
      tt->struct_def.fields[1].ptype = STRING_TYPE;
      tt->struct_def.fields[1].rel_offset = 
        (char *) &u_field_info.field_name - (char *) &u_field_info;
      tt->struct_def.fields[2].ftype = tdrpStrDup("long");
      tt->struct_def.fields[2].fname = tdrpStrDup("field_num");
      tt->struct_def.fields[2].ptype = LONG_TYPE;
      tt->struct_def.fields[2].rel_offset = 
        (char *) &u_field_info.field_num - (char *) &u_field_info;
    tt->n_struct_vals = 3;
    tt->struct_vals = (tdrpVal_t *)
        tdrpMalloc(tt->n_struct_vals * sizeof(tdrpVal_t));
      tt->struct_vals[0].s = tdrpStrDup("mdvp:://localhost::./mdv/ruc");
      tt->struct_vals[1].s = tdrpStrDup("UGRD");
      tt->struct_vals[2].l = 0;
    tt++;
    
    // Parameter 'v_field_info'
    // ctype is '_input_info_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRUCT_TYPE;
    tt->param_name = tdrpStrDup("v_field_info");
    tt->descr = tdrpStrDup("Information about the input V field");
    tt->help = tdrpStrDup("Uses field_name to identify field in MDV file unless field_name is \"\" in which case it uses field_num to identify the input field.\nNote that this data is expected to be in m/s.");
    tt->val_offset = (char *) &v_field_info - &_start_;
    tt->struct_def.name = tdrpStrDup("input_info_t");
    tt->struct_def.nfields = 3;
    tt->struct_def.fields = (struct_field_t *)
        tdrpMalloc(tt->struct_def.nfields * sizeof(struct_field_t));
      tt->struct_def.fields[0].ftype = tdrpStrDup("string");
      tt->struct_def.fields[0].fname = tdrpStrDup("url");
      tt->struct_def.fields[0].ptype = STRING_TYPE;
      tt->struct_def.fields[0].rel_offset = 
        (char *) &v_field_info.url - (char *) &v_field_info;
      tt->struct_def.fields[1].ftype = tdrpStrDup("string");
      tt->struct_def.fields[1].fname = tdrpStrDup("field_name");
      tt->struct_def.fields[1].ptype = STRING_TYPE;
      tt->struct_def.fields[1].rel_offset = 
        (char *) &v_field_info.field_name - (char *) &v_field_info;
      tt->struct_def.fields[2].ftype = tdrpStrDup("long");
      tt->struct_def.fields[2].fname = tdrpStrDup("field_num");
      tt->struct_def.fields[2].ptype = LONG_TYPE;
      tt->struct_def.fields[2].rel_offset = 
        (char *) &v_field_info.field_num - (char *) &v_field_info;
    tt->n_struct_vals = 3;
    tt->struct_vals = (tdrpVal_t *)
        tdrpMalloc(tt->n_struct_vals * sizeof(tdrpVal_t));
      tt->struct_vals[0].s = tdrpStrDup("mdvp:://localhost::./mdv/ruc");
      tt->struct_vals[1].s = tdrpStrDup("VGRD");
      tt->struct_vals[2].l = 0;
    tt++;
    
    // Parameter 'pressure_limits'
    // ctype is '_pressure_limits_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRUCT_TYPE;
    tt->param_name = tdrpStrDup("pressure_limits");
    tt->descr = tdrpStrDup("The minimum and maximum pressure levels to process");
    tt->help = tdrpStrDup("");
    tt->val_offset = (char *) &pressure_limits - &_start_;
    tt->struct_def.name = tdrpStrDup("pressure_limits_t");
    tt->struct_def.nfields = 2;
    tt->struct_def.fields = (struct_field_t *)
        tdrpMalloc(tt->struct_def.nfields * sizeof(struct_field_t));
      tt->struct_def.fields[0].ftype = tdrpStrDup("double");
      tt->struct_def.fields[0].fname = tdrpStrDup("lower_level");
      tt->struct_def.fields[0].ptype = DOUBLE_TYPE;
      tt->struct_def.fields[0].rel_offset = 
        (char *) &pressure_limits.lower_level - (char *) &pressure_limits;
      tt->struct_def.fields[1].ftype = tdrpStrDup("double");
      tt->struct_def.fields[1].fname = tdrpStrDup("upper_level");
      tt->struct_def.fields[1].ptype = DOUBLE_TYPE;
      tt->struct_def.fields[1].rel_offset = 
        (char *) &pressure_limits.upper_level - (char *) &pressure_limits;
    tt->n_struct_vals = 2;
    tt->struct_vals = (tdrpVal_t *)
        tdrpMalloc(tt->n_struct_vals * sizeof(tdrpVal_t));
      tt->struct_vals[0].d = 875;
      tt->struct_vals[1].d = 675;
    tt++;
    
    // Parameter 'max_input_valid_secs'
    // ctype is 'long'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = LONG_TYPE;
    tt->param_name = tdrpStrDup("max_input_valid_secs");
    tt->descr = tdrpStrDup("Maximum input valid age in seconds");
    tt->help = tdrpStrDup("Once the algorithm is triggered, it will not use any input data older than this value.");
    tt->val_offset = (char *) &max_input_valid_secs - &_start_;
    tt->single_val.l = 1800;
    tt++;
    
    // Parameter 'is_forecast_data'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("is_forecast_data");
    tt->descr = tdrpStrDup("Flag to read forecast directory structure mdv data");
    tt->help = tdrpStrDup("");
    tt->val_offset = (char *) &is_forecast_data - &_start_;
    tt->single_val.b = pFALSE;
    tt++;
    
    // Parameter 'write_as_forecast'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("write_as_forecast");
    tt->descr = tdrpStrDup("Flag to write data in forecast directory structure");
    tt->help = tdrpStrDup("");
    tt->val_offset = (char *) &write_as_forecast - &_start_;
    tt->single_val.b = pFALSE;
    tt++;
    
    // Parameter 'output_url'
    // ctype is 'char*'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRING_TYPE;
    tt->param_name = tdrpStrDup("output_url");
    tt->descr = tdrpStrDup("Output URL");
    tt->help = tdrpStrDup("");
    tt->val_offset = (char *) &output_url - &_start_;
    tt->single_val.s = tdrpStrDup("mdvp:://localhost::./mdv/output");
    tt++;
    
    // Parameter 'Comment 3'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = COMMENT_TYPE;
    tt->param_name = tdrpStrDup("Comment 3");
    tt->comment_hdr = tdrpStrDup("ALGORITHM PARAMETERS");
    tt->comment_text = tdrpStrDup("");
    tt++;
    
    // trailing entry has param_name set to NULL
    
    tt->param_name = NULL;
    
    return;
  
  }
