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
// Code for program OdimHdf5ToMdv
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
    tt->comment_hdr = tdrpStrDup("OdimHdf5ToMdv");
    tt->comment_text = tdrpStrDup("This program converts ODIM OPERA grids, in HDF5, to MDV/NetCDF.");
    tt++;
    
    // Parameter 'Comment 1'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = COMMENT_TYPE;
    tt->param_name = tdrpStrDup("Comment 1");
    tt->comment_hdr = tdrpStrDup("DEBUGGING AND PROCESS CONTROL");
    tt->comment_text = tdrpStrDup("");
    tt++;
    
    // Parameter 'debug'
    // ctype is '_debug_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = ENUM_TYPE;
    tt->param_name = tdrpStrDup("debug");
    tt->descr = tdrpStrDup("Debug option");
    tt->help = tdrpStrDup("If set, debug messages will be printed appropriately");
    tt->val_offset = (char *) &debug - &_start_;
    tt->enum_def.name = tdrpStrDup("debug_t");
    tt->enum_def.nfields = 4;
    tt->enum_def.fields = (enum_field_t *)
        tdrpMalloc(tt->enum_def.nfields * sizeof(enum_field_t));
      tt->enum_def.fields[0].name = tdrpStrDup("DEBUG_OFF");
      tt->enum_def.fields[0].val = DEBUG_OFF;
      tt->enum_def.fields[1].name = tdrpStrDup("DEBUG_NORM");
      tt->enum_def.fields[1].val = DEBUG_NORM;
      tt->enum_def.fields[2].name = tdrpStrDup("DEBUG_VERBOSE");
      tt->enum_def.fields[2].val = DEBUG_VERBOSE;
      tt->enum_def.fields[3].name = tdrpStrDup("DEBUG_EXTRA");
      tt->enum_def.fields[3].val = DEBUG_EXTRA;
    tt->single_val.e = DEBUG_OFF;
    tt++;
    
    // Parameter 'instance'
    // ctype is 'char*'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRING_TYPE;
    tt->param_name = tdrpStrDup("instance");
    tt->descr = tdrpStrDup("Process instance");
    tt->help = tdrpStrDup("Used for registration with procmap.");
    tt->val_offset = (char *) &instance - &_start_;
    tt->single_val.s = tdrpStrDup("test");
    tt++;
    
    // Parameter 'register_with_procmap'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("register_with_procmap");
    tt->descr = tdrpStrDup("Option to register with procmap while running.");
    tt->help = tdrpStrDup("See 'instance'.");
    tt->val_offset = (char *) &register_with_procmap - &_start_;
    tt->single_val.b = pFALSE;
    tt++;
    
    // Parameter 'reg_interval'
    // ctype is 'int'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = INT_TYPE;
    tt->param_name = tdrpStrDup("reg_interval");
    tt->descr = tdrpStrDup("Registration interval.");
    tt->help = tdrpStrDup("The number of seconds between expected procmap registrations.");
    tt->val_offset = (char *) &reg_interval - &_start_;
    tt->single_val.i = 60;
    tt++;
    
    // Parameter 'Comment 2'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = COMMENT_TYPE;
    tt->param_name = tdrpStrDup("Comment 2");
    tt->comment_hdr = tdrpStrDup("DATA INPUT");
    tt->comment_text = tdrpStrDup("");
    tt++;
    
    // Parameter 'mode'
    // ctype is '_mode_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = ENUM_TYPE;
    tt->param_name = tdrpStrDup("mode");
    tt->descr = tdrpStrDup("Operating mode");
    tt->help = tdrpStrDup("In REALTIME mode, the program waits for a new input file. In ARCHIVE mode, it moves through the data between the start and end times set on the command line. In FILELIST mode, it moves through the list of file names specified on the command line. Paths (in FILELIST mode, at least) MUST contain a day-directory below the data file -- ./data_file.mdv will not work as a file path");
    tt->val_offset = (char *) &mode - &_start_;
    tt->enum_def.name = tdrpStrDup("mode_t");
    tt->enum_def.nfields = 3;
    tt->enum_def.fields = (enum_field_t *)
        tdrpMalloc(tt->enum_def.nfields * sizeof(enum_field_t));
      tt->enum_def.fields[0].name = tdrpStrDup("ARCHIVE");
      tt->enum_def.fields[0].val = ARCHIVE;
      tt->enum_def.fields[1].name = tdrpStrDup("REALTIME");
      tt->enum_def.fields[1].val = REALTIME;
      tt->enum_def.fields[2].name = tdrpStrDup("FILELIST");
      tt->enum_def.fields[2].val = FILELIST;
    tt->single_val.e = ARCHIVE;
    tt++;
    
    // Parameter 'input_dir'
    // ctype is 'char*'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRING_TYPE;
    tt->param_name = tdrpStrDup("input_dir");
    tt->descr = tdrpStrDup("Directory for input data.");
    tt->help = tdrpStrDup("This is used in REALTIME and ARCHIVE modes only. In FILELIST mode, the file paths are specified on the command line.");
    tt->val_offset = (char *) &input_dir - &_start_;
    tt->single_val.s = tdrpStrDup("raw/input");
    tt++;
    
    // Parameter 'use_ldata_info_file'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("use_ldata_info_file");
    tt->descr = tdrpStrDup("Option to use _latest_data_info file as input trigger.");
    tt->help = tdrpStrDup("REALTIME mode only. If true, waits on _latest_data_info file. If false, scans the directory for new file.");
    tt->val_offset = (char *) &use_ldata_info_file - &_start_;
    tt->single_val.b = pFALSE;
    tt++;
    
    // Parameter 'max_realtime_age'
    // ctype is 'int'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = INT_TYPE;
    tt->param_name = tdrpStrDup("max_realtime_age");
    tt->descr = tdrpStrDup("Max valid age of rdata input files in realtime mode (secs)");
    tt->help = tdrpStrDup("This the max valid age for an incoming file. The program will wait for a data file more recent than this age.");
    tt->val_offset = (char *) &max_realtime_age - &_start_;
    tt->has_min = TRUE;
    tt->min_val.i = 1;
    tt->single_val.i = 3600;
    tt++;
    
    // Parameter 'process_latest_file_only'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("process_latest_file_only");
    tt->descr = tdrpStrDup("Option to use only the latest file to have arrived.");
    tt->help = tdrpStrDup("REALTIME mode only. If true, will process only the latest file on startup, if the file has an age less than max_realtime_age. If false, on startup will process all files with age less than max_realtime_age.");
    tt->val_offset = (char *) &process_latest_file_only - &_start_;
    tt->single_val.b = pTRUE;
    tt++;
    
    // Parameter 'file_name_ext'
    // ctype is 'char*'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRING_TYPE;
    tt->param_name = tdrpStrDup("file_name_ext");
    tt->descr = tdrpStrDup("Specify the extension of the files to be read.");
    tt->help = tdrpStrDup("If blank, all extensions will be used.");
    tt->val_offset = (char *) &file_name_ext - &_start_;
    tt->single_val.s = tdrpStrDup("");
    tt++;
    
    // Parameter 'file_name_substr'
    // ctype is 'char*'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRING_TYPE;
    tt->param_name = tdrpStrDup("file_name_substr");
    tt->descr = tdrpStrDup("Specify a seach string for the names of the files to be read.");
    tt->help = tdrpStrDup("If blank, no check will be performed.");
    tt->val_offset = (char *) &file_name_substr - &_start_;
    tt->single_val.s = tdrpStrDup("");
    tt++;
    
    // Parameter 'Comment 3'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = COMMENT_TYPE;
    tt->param_name = tdrpStrDup("Comment 3");
    tt->comment_hdr = tdrpStrDup("OUTPUT FIELDS");
    tt->comment_text = tdrpStrDup("");
    tt++;
    
    // Parameter 'output_fields'
    // ctype is '_output_field_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRUCT_TYPE;
    tt->param_name = tdrpStrDup("output_fields");
    tt->descr = tdrpStrDup("Output fields details.");
    tt->help = tdrpStrDup("These are the fields that will be written to the output files. The quantity strings are found under GROUP 'dataset1/2/3' etc, GROUP 'data1', DATASET 'data'.");
    tt->array_offset = (char *) &_output_fields - &_start_;
    tt->array_n_offset = (char *) &output_fields_n - &_start_;
    tt->is_array = TRUE;
    tt->array_len_fixed = FALSE;
    tt->array_elem_size = sizeof(output_field_t);
    tt->array_n = 2;
    tt->struct_def.name = tdrpStrDup("output_field_t");
    tt->struct_def.nfields = 4;
    tt->struct_def.fields = (struct_field_t *)
        tdrpMalloc(tt->struct_def.nfields * sizeof(struct_field_t));
      tt->struct_def.fields[0].ftype = tdrpStrDup("string");
      tt->struct_def.fields[0].fname = tdrpStrDup("hdf5Name");
      tt->struct_def.fields[0].ptype = STRING_TYPE;
      tt->struct_def.fields[0].rel_offset = 
        (char *) &_output_fields->hdf5Name - (char *) _output_fields;
      tt->struct_def.fields[1].ftype = tdrpStrDup("string");
      tt->struct_def.fields[1].fname = tdrpStrDup("outputName");
      tt->struct_def.fields[1].ptype = STRING_TYPE;
      tt->struct_def.fields[1].rel_offset = 
        (char *) &_output_fields->outputName - (char *) _output_fields;
      tt->struct_def.fields[2].ftype = tdrpStrDup("string");
      tt->struct_def.fields[2].fname = tdrpStrDup("longName");
      tt->struct_def.fields[2].ptype = STRING_TYPE;
      tt->struct_def.fields[2].rel_offset = 
        (char *) &_output_fields->longName - (char *) _output_fields;
      tt->struct_def.fields[3].ftype = tdrpStrDup("encoding_type_t");
      tt->struct_def.fields[3].fname = tdrpStrDup("encoding");
      tt->struct_def.fields[3].ptype = ENUM_TYPE;
      tt->struct_def.fields[3].rel_offset = 
        (char *) &_output_fields->encoding - (char *) _output_fields;
        tt->struct_def.fields[3].enum_def.name = tdrpStrDup("encoding_type_t");
        tt->struct_def.fields[3].enum_def.nfields = 3;
        tt->struct_def.fields[3].enum_def.fields = (enum_field_t *) tdrpMalloc
          (tt->struct_def.fields[3].enum_def.nfields * sizeof(enum_field_t));
        tt->struct_def.fields[3].enum_def.fields[0].name = tdrpStrDup("ENCODING_INT8");
        tt->struct_def.fields[3].enum_def.fields[0].val = ENCODING_INT8;
        tt->struct_def.fields[3].enum_def.fields[1].name = tdrpStrDup("ENCODING_INT16");
        tt->struct_def.fields[3].enum_def.fields[1].val = ENCODING_INT16;
        tt->struct_def.fields[3].enum_def.fields[2].name = tdrpStrDup("ENCODING_FLOAT32");
        tt->struct_def.fields[3].enum_def.fields[2].val = ENCODING_FLOAT32;
    tt->n_struct_vals = 8;
    tt->struct_vals = (tdrpVal_t *)
        tdrpMalloc(tt->n_struct_vals * sizeof(tdrpVal_t));
      tt->struct_vals[0].s = tdrpStrDup("DBZH");
      tt->struct_vals[1].s = tdrpStrDup("DBZ");
      tt->struct_vals[2].s = tdrpStrDup("reflectivity");
      tt->struct_vals[3].e = ENCODING_FLOAT32;
      tt->struct_vals[4].s = tdrpStrDup("QIND");
      tt->struct_vals[5].s = tdrpStrDup("QUALITY");
      tt->struct_vals[6].s = tdrpStrDup("Quality-control");
      tt->struct_vals[7].e = ENCODING_FLOAT32;
    tt++;
    
    // Parameter 'Comment 4'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = COMMENT_TYPE;
    tt->param_name = tdrpStrDup("Comment 4");
    tt->comment_hdr = tdrpStrDup("CENSORING");
    tt->comment_text = tdrpStrDup("");
    tt++;
    
    // Parameter 'apply_censoring'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("apply_censoring");
    tt->descr = tdrpStrDup("Option to censor data fields based on quality values stored in a separate field.");
    tt->help = tdrpStrDup("The quality field indicates the confidence that can be placed in the data field. A quallity value of 0 indicates a definite problem.");
    tt->val_offset = (char *) &apply_censoring - &_start_;
    tt->single_val.b = pTRUE;
    tt++;
    
    // Parameter 'censoring'
    // ctype is '_censoring_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRUCT_TYPE;
    tt->param_name = tdrpStrDup("censoring");
    tt->descr = tdrpStrDup("Censoring data fields using a quality field.");
    tt->help = tdrpStrDup("Specify the fields to be censored, and the name of the quality field to be used. The data field is censored if the quality field les less than the min value or greater than the max value. The names refer to the ");
    tt->array_offset = (char *) &_censoring - &_start_;
    tt->array_n_offset = (char *) &censoring_n - &_start_;
    tt->is_array = TRUE;
    tt->array_len_fixed = FALSE;
    tt->array_elem_size = sizeof(censoring_t);
    tt->array_n = 1;
    tt->struct_def.name = tdrpStrDup("censoring_t");
    tt->struct_def.nfields = 4;
    tt->struct_def.fields = (struct_field_t *)
        tdrpMalloc(tt->struct_def.nfields * sizeof(struct_field_t));
      tt->struct_def.fields[0].ftype = tdrpStrDup("string");
      tt->struct_def.fields[0].fname = tdrpStrDup("dataHdf5Name");
      tt->struct_def.fields[0].ptype = STRING_TYPE;
      tt->struct_def.fields[0].rel_offset = 
        (char *) &_censoring->dataHdf5Name - (char *) _censoring;
      tt->struct_def.fields[1].ftype = tdrpStrDup("string");
      tt->struct_def.fields[1].fname = tdrpStrDup("qualHdf5Name");
      tt->struct_def.fields[1].ptype = STRING_TYPE;
      tt->struct_def.fields[1].rel_offset = 
        (char *) &_censoring->qualHdf5Name - (char *) _censoring;
      tt->struct_def.fields[2].ftype = tdrpStrDup("double");
      tt->struct_def.fields[2].fname = tdrpStrDup("minQuality");
      tt->struct_def.fields[2].ptype = DOUBLE_TYPE;
      tt->struct_def.fields[2].rel_offset = 
        (char *) &_censoring->minQuality - (char *) _censoring;
      tt->struct_def.fields[3].ftype = tdrpStrDup("double");
      tt->struct_def.fields[3].fname = tdrpStrDup("maxQuality");
      tt->struct_def.fields[3].ptype = DOUBLE_TYPE;
      tt->struct_def.fields[3].rel_offset = 
        (char *) &_censoring->maxQuality - (char *) _censoring;
    tt->n_struct_vals = 4;
    tt->struct_vals = (tdrpVal_t *)
        tdrpMalloc(tt->n_struct_vals * sizeof(tdrpVal_t));
      tt->struct_vals[0].s = tdrpStrDup("DBZH");
      tt->struct_vals[1].s = tdrpStrDup("QIND");
      tt->struct_vals[2].d = 0.1;
      tt->struct_vals[3].d = 1;
    tt++;
    
    // Parameter 'Comment 5'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = COMMENT_TYPE;
    tt->param_name = tdrpStrDup("Comment 5");
    tt->comment_hdr = tdrpStrDup("OPTION TO CONVERT OUTPUT GRID TO LAT/LON");
    tt->comment_text = tdrpStrDup("");
    tt++;
    
    // Parameter 'adjust_projection_origin'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("adjust_projection_origin");
    tt->descr = tdrpStrDup("Option to adjust the projection origin.");
    tt->help = tdrpStrDup("If the standard projection origin is used, the echoes appear to be in the wrong location. This option allows you to adjust the origin to get the echoes in the correct place.");
    tt->val_offset = (char *) &adjust_projection_origin - &_start_;
    tt->single_val.b = pTRUE;
    tt++;
    
    // Parameter 'adjusted_projection_origin_lat'
    // ctype is 'double'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = DOUBLE_TYPE;
    tt->param_name = tdrpStrDup("adjusted_projection_origin_lat");
    tt->descr = tdrpStrDup("Adjusted latitude for the projection origin.");
    tt->help = tdrpStrDup("");
    tt->val_offset = (char *) &adjusted_projection_origin_lat - &_start_;
    tt->single_val.d = 54;
    tt++;
    
    // Parameter 'adjusted_projection_origin_lon'
    // ctype is 'double'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = DOUBLE_TYPE;
    tt->param_name = tdrpStrDup("adjusted_projection_origin_lon");
    tt->descr = tdrpStrDup("Adjusted longitude for the projection origin.");
    tt->help = tdrpStrDup("");
    tt->val_offset = (char *) &adjusted_projection_origin_lon - &_start_;
    tt->single_val.d = 9.5;
    tt++;
    
    // Parameter 'convert_output_grid_to_latlon'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("convert_output_grid_to_latlon");
    tt->descr = tdrpStrDup("Option to convert the output grid to lat/lon.");
    tt->help = tdrpStrDup("If true, the Mdvx class will auto-convert to a suitable lat/lon grid. This is performed using a nearest neighbor transformation.");
    tt->val_offset = (char *) &convert_output_grid_to_latlon - &_start_;
    tt->single_val.b = pFALSE;
    tt++;
    
    // Parameter 'Comment 6'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = COMMENT_TYPE;
    tt->param_name = tdrpStrDup("Comment 6");
    tt->comment_hdr = tdrpStrDup("OUTPUT URL");
    tt->comment_text = tdrpStrDup("");
    tt++;
    
    // Parameter 'output_url'
    // ctype is 'char*'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRING_TYPE;
    tt->param_name = tdrpStrDup("output_url");
    tt->descr = tdrpStrDup("URL for writing output files.");
    tt->help = tdrpStrDup("");
    tt->val_offset = (char *) &output_url - &_start_;
    tt->single_val.s = tdrpStrDup("mdvp:://localhost::/tmp/mdv/odim");
    tt++;
    
    // Parameter 'Comment 7'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = COMMENT_TYPE;
    tt->param_name = tdrpStrDup("Comment 7");
    tt->comment_hdr = tdrpStrDup("OUTPUT METADATA STRINGS");
    tt->comment_text = tdrpStrDup("These will be used if the file does not contain suitable information.");
    tt++;
    
    // Parameter 'data_set_name'
    // ctype is 'char*'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRING_TYPE;
    tt->param_name = tdrpStrDup("data_set_name");
    tt->descr = tdrpStrDup("Name of data set.");
    tt->help = tdrpStrDup("Appears in Mdv master header.");
    tt->val_offset = (char *) &data_set_name - &_start_;
    tt->single_val.s = tdrpStrDup("OPERA ODIM");
    tt++;
    
    // Parameter 'data_set_source'
    // ctype is 'char*'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRING_TYPE;
    tt->param_name = tdrpStrDup("data_set_source");
    tt->descr = tdrpStrDup("Source of data.");
    tt->help = tdrpStrDup("Appears in Mdv master header.");
    tt->val_offset = (char *) &data_set_source - &_start_;
    tt->single_val.s = tdrpStrDup("OPERA");
    tt++;
    
    // Parameter 'Comment 8'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = COMMENT_TYPE;
    tt->param_name = tdrpStrDup("Comment 8");
    tt->comment_hdr = tdrpStrDup("Z DIMENSION DETAILS");
    tt->comment_text = tdrpStrDup("Not currently used - waiting for 3-D grids");
    tt++;
    
    // Parameter 'radar_min_z_km'
    // ctype is 'double'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = DOUBLE_TYPE;
    tt->param_name = tdrpStrDup("radar_min_z_km");
    tt->descr = tdrpStrDup("Height of lowest level (km)");
    tt->help = tdrpStrDup("");
    tt->val_offset = (char *) &radar_min_z_km - &_start_;
    tt->single_val.d = 0;
    tt++;
    
    // Parameter 'radar_delta_z_km'
    // ctype is 'double'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = DOUBLE_TYPE;
    tt->param_name = tdrpStrDup("radar_delta_z_km");
    tt->descr = tdrpStrDup("Delta height between levels (km)");
    tt->help = tdrpStrDup("");
    tt->val_offset = (char *) &radar_delta_z_km - &_start_;
    tt->single_val.d = 0.125;
    tt++;
    
    // trailing entry has param_name set to NULL
    
    tt->param_name = NULL;
    
    return;
  
  }
