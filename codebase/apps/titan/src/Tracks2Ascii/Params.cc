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
// Code for program Tracks2Ascii
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
    tt->comment_hdr = tdrpStrDup("Program name: Tracks2Ascii");
    tt->comment_text = tdrpStrDup("Tracks2Ascii prints out storm and track data to stdout in columm ASCII format, suitable for use by spread-sheet, data-base or similar application. Storm properties are seleted using the TRACK_ENTRY option, while aggregate track propertes are selected using the COMPLETE_TRACK option.");
    tt++;
    
    // Parameter 'Comment 1'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = COMMENT_TYPE;
    tt->param_name = tdrpStrDup("Comment 1");
    tt->comment_hdr = tdrpStrDup("DEBUG AND PROCESS CONTROL");
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
    tt->enum_def.nfields = 3;
    tt->enum_def.fields = (enum_field_t *)
        tdrpMalloc(tt->enum_def.nfields * sizeof(enum_field_t));
      tt->enum_def.fields[0].name = tdrpStrDup("DEBUG_OFF");
      tt->enum_def.fields[0].val = DEBUG_OFF;
      tt->enum_def.fields[1].name = tdrpStrDup("DEBUG_NORM");
      tt->enum_def.fields[1].val = DEBUG_NORM;
      tt->enum_def.fields[2].name = tdrpStrDup("DEBUG_VERBOSE");
      tt->enum_def.fields[2].val = DEBUG_VERBOSE;
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
    tt->single_val.s = tdrpStrDup("Test");
    tt++;
    
    // Parameter 'Comment 2'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = COMMENT_TYPE;
    tt->param_name = tdrpStrDup("Comment 2");
    tt->comment_hdr = tdrpStrDup("DATA INPUT");
    tt->comment_text = tdrpStrDup("");
    tt++;
    
    // Parameter 'input_dir'
    // ctype is 'char*'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRING_TYPE;
    tt->param_name = tdrpStrDup("input_dir");
    tt->descr = tdrpStrDup("Directory path for input data.");
    tt->help = tdrpStrDup("This is used if start and end times are specified on the command line. It is ignored if you specify a file list using -f on the command line.");
    tt->val_offset = (char *) &input_dir - &_start_;
    tt->single_val.s = tdrpStrDup("$(RAP_DATA_DIR)/titan/storms");
    tt++;
    
    // Parameter 'target_entity'
    // ctype is '_target_entity_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = ENUM_TYPE;
    tt->param_name = tdrpStrDup("target_entity");
    tt->descr = tdrpStrDup("Entity for which data is sought.");
    tt->help = tdrpStrDup("COMPLETE_TRACK - properties for the whole track. Where more than one part exists at a time the properties are aggregated from the parts. TRACK_ENTRY - instantaneous storm and track properties for each part of the tracks at regular intervals. INITIAL_PROPS - properties at the start of the track, used for selecting storms similar to seeded cases in weather mod activities.");
    tt->val_offset = (char *) &target_entity - &_start_;
    tt->enum_def.name = tdrpStrDup("target_entity_t");
    tt->enum_def.nfields = 3;
    tt->enum_def.fields = (enum_field_t *)
        tdrpMalloc(tt->enum_def.nfields * sizeof(enum_field_t));
      tt->enum_def.fields[0].name = tdrpStrDup("COMPLETE_TRACK");
      tt->enum_def.fields[0].val = COMPLETE_TRACK;
      tt->enum_def.fields[1].name = tdrpStrDup("TRACK_ENTRY");
      tt->enum_def.fields[1].val = TRACK_ENTRY;
      tt->enum_def.fields[2].name = tdrpStrDup("INITIAL_PROPS");
      tt->enum_def.fields[2].val = INITIAL_PROPS;
    tt->single_val.e = COMPLETE_TRACK;
    tt++;
    
    // Parameter 'use_complex_tracks'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("use_complex_tracks");
    tt->descr = tdrpStrDup("Option to process complex tracks.");
    tt->help = tdrpStrDup("If set, tracks with mergers and splits will be processed.");
    tt->val_offset = (char *) &use_complex_tracks - &_start_;
    tt->single_val.b = pTRUE;
    tt++;
    
    // Parameter 'use_simple_tracks'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("use_simple_tracks");
    tt->descr = tdrpStrDup("Option to process simple tracks.");
    tt->help = tdrpStrDup("If set, tracks without mergers and splits will be processed.");
    tt->val_offset = (char *) &use_simple_tracks - &_start_;
    tt->single_val.b = pTRUE;
    tt++;
    
    // Parameter 'count_only'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("count_only");
    tt->descr = tdrpStrDup("Option to only count storms to get total number.");
    tt->help = tdrpStrDup("Suppresses normal print output.");
    tt->val_offset = (char *) &count_only - &_start_;
    tt->single_val.b = pFALSE;
    tt++;
    
    // Parameter 'sample_interval'
    // ctype is 'int'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = INT_TYPE;
    tt->param_name = tdrpStrDup("sample_interval");
    tt->descr = tdrpStrDup("Sampling interval (secs).");
    tt->help = tdrpStrDup("TRACK_ENTRY entity only. The track entry properties are printed out at this interval. If set to -1, all entries are printed.");
    tt->val_offset = (char *) &sample_interval - &_start_;
    tt->single_val.i = 1800;
    tt++;
    
    // Parameter 'scan_interval'
    // ctype is 'int'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = INT_TYPE;
    tt->param_name = tdrpStrDup("scan_interval");
    tt->descr = tdrpStrDup("Volume scan interval (secs).");
    tt->help = tdrpStrDup("Used in conjunction with sample_interval to determine whether to print the entry for a given scan. It is a temporal search region. If no entries lie within this interval on either side of the search time, no analysis is done for this time.");
    tt->val_offset = (char *) &scan_interval - &_start_;
    tt->single_val.i = 360;
    tt++;
    
    // Parameter 'min_duration'
    // ctype is 'int'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = INT_TYPE;
    tt->param_name = tdrpStrDup("min_duration");
    tt->descr = tdrpStrDup("Minimum track duration (secs).");
    tt->help = tdrpStrDup("Only tracks which exceed this duration are processed.");
    tt->val_offset = (char *) &min_duration - &_start_;
    tt->single_val.i = 900;
    tt++;
    
    // Parameter 'use_box_limits'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("use_box_limits");
    tt->descr = tdrpStrDup("Option to limit analysis to a bounding box.");
    tt->help = tdrpStrDup("If set, only tracks which pass through the box will be processed.");
    tt->val_offset = (char *) &use_box_limits - &_start_;
    tt->single_val.b = pFALSE;
    tt++;
    
    // Parameter 'box'
    // ctype is '_grid'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRUCT_TYPE;
    tt->param_name = tdrpStrDup("box");
    tt->descr = tdrpStrDup("Box parameters");
    tt->help = tdrpStrDup("The parameters of the bounding box - see 'use_box_limits'. The size limits are in km relative to the grid origin. min_percent is the minimum percentage of the tracks which must lie inside the box. min_nscans is the minimum number of scans for which storms must lie in the box. If either of these conditions is true, the track is accepted for analysis.");
    tt->val_offset = (char *) &box - &_start_;
    tt->struct_def.name = tdrpStrDup("grid");
    tt->struct_def.nfields = 6;
    tt->struct_def.fields = (struct_field_t *)
        tdrpMalloc(tt->struct_def.nfields * sizeof(struct_field_t));
      tt->struct_def.fields[0].ftype = tdrpStrDup("double");
      tt->struct_def.fields[0].fname = tdrpStrDup("min_x");
      tt->struct_def.fields[0].ptype = DOUBLE_TYPE;
      tt->struct_def.fields[0].rel_offset = 
        (char *) &box.min_x - (char *) &box;
      tt->struct_def.fields[1].ftype = tdrpStrDup("double");
      tt->struct_def.fields[1].fname = tdrpStrDup("min_y");
      tt->struct_def.fields[1].ptype = DOUBLE_TYPE;
      tt->struct_def.fields[1].rel_offset = 
        (char *) &box.min_y - (char *) &box;
      tt->struct_def.fields[2].ftype = tdrpStrDup("double");
      tt->struct_def.fields[2].fname = tdrpStrDup("max_x");
      tt->struct_def.fields[2].ptype = DOUBLE_TYPE;
      tt->struct_def.fields[2].rel_offset = 
        (char *) &box.max_x - (char *) &box;
      tt->struct_def.fields[3].ftype = tdrpStrDup("double");
      tt->struct_def.fields[3].fname = tdrpStrDup("max_y");
      tt->struct_def.fields[3].ptype = DOUBLE_TYPE;
      tt->struct_def.fields[3].rel_offset = 
        (char *) &box.max_y - (char *) &box;
      tt->struct_def.fields[4].ftype = tdrpStrDup("double");
      tt->struct_def.fields[4].fname = tdrpStrDup("min_percent");
      tt->struct_def.fields[4].ptype = DOUBLE_TYPE;
      tt->struct_def.fields[4].rel_offset = 
        (char *) &box.min_percent - (char *) &box;
      tt->struct_def.fields[5].ftype = tdrpStrDup("int");
      tt->struct_def.fields[5].fname = tdrpStrDup("min_nscans");
      tt->struct_def.fields[5].ptype = INT_TYPE;
      tt->struct_def.fields[5].rel_offset = 
        (char *) &box.min_nscans - (char *) &box;
    tt->n_struct_vals = 6;
    tt->struct_vals = (tdrpVal_t *)
        tdrpMalloc(tt->n_struct_vals * sizeof(tdrpVal_t));
      tt->struct_vals[0].d = 0;
      tt->struct_vals[1].d = 0;
      tt->struct_vals[2].d = 0;
      tt->struct_vals[3].d = 0;
      tt->struct_vals[4].d = 0;
      tt->struct_vals[5].i = 0;
    tt++;
    
    // Parameter 'check_too_close'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("check_too_close");
    tt->descr = tdrpStrDup("Option to reject tracks too close to radar.");
    tt->help = tdrpStrDup("This allows rejection of tracks with tops missing because it is too close to the radar.");
    tt->val_offset = (char *) &check_too_close - &_start_;
    tt->single_val.b = pFALSE;
    tt++;
    
    // Parameter 'max_nscans_too_close'
    // ctype is 'int'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = INT_TYPE;
    tt->param_name = tdrpStrDup("max_nscans_too_close");
    tt->descr = tdrpStrDup("Max nscans too close to radar - tops missing.");
    tt->help = tdrpStrDup("Max number of scans per track allowed with missing tops.");
    tt->val_offset = (char *) &max_nscans_too_close - &_start_;
    tt->single_val.i = 5;
    tt++;
    
    // Parameter 'check_too_far'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("check_too_far");
    tt->descr = tdrpStrDup("Option to reject tracks at max range.");
    tt->help = tdrpStrDup("This allows rejection of tracks too far from the radar - data missing because part of the storm is out of range.");
    tt->val_offset = (char *) &check_too_far - &_start_;
    tt->single_val.b = pFALSE;
    tt++;
    
    // Parameter 'max_nscans_too_far'
    // ctype is 'int'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = INT_TYPE;
    tt->param_name = tdrpStrDup("max_nscans_too_far");
    tt->descr = tdrpStrDup("Max nscans too far.");
    tt->help = tdrpStrDup("Max number of scans per track allowed at max range.");
    tt->val_offset = (char *) &max_nscans_too_far - &_start_;
    tt->single_val.i = 5;
    tt++;
    
    // Parameter 'check_vol_at_start'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("check_vol_at_start");
    tt->descr = tdrpStrDup("Option to check vol at start of track.");
    tt->help = tdrpStrDup("This allows rejection of tracks which existed at radar startup.");
    tt->val_offset = (char *) &check_vol_at_start - &_start_;
    tt->single_val.b = pFALSE;
    tt++;
    
    // Parameter 'max_vol_at_start'
    // ctype is 'double'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = DOUBLE_TYPE;
    tt->param_name = tdrpStrDup("max_vol_at_start");
    tt->descr = tdrpStrDup("Max vol at start of sampling (km2).");
    tt->help = tdrpStrDup("Tracks with starting vol in excess of this value are rejected.");
    tt->val_offset = (char *) &max_vol_at_start - &_start_;
    tt->single_val.d = 5;
    tt++;
    
    // Parameter 'check_vol_at_end'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("check_vol_at_end");
    tt->descr = tdrpStrDup("Option to check vol at end of track.");
    tt->help = tdrpStrDup("This allows rejection of tracks which existed at radar shutdown.");
    tt->val_offset = (char *) &check_vol_at_end - &_start_;
    tt->single_val.b = pFALSE;
    tt++;
    
    // Parameter 'max_vol_at_end'
    // ctype is 'double'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = DOUBLE_TYPE;
    tt->param_name = tdrpStrDup("max_vol_at_end");
    tt->descr = tdrpStrDup("Max vol at end of sampling (km2).");
    tt->help = tdrpStrDup("Tracks with ending vol in excess of this value are rejected.");
    tt->val_offset = (char *) &max_vol_at_end - &_start_;
    tt->single_val.d = 5;
    tt++;
    
    // Parameter 'use_case_tracks_file'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("use_case_tracks_file");
    tt->descr = tdrpStrDup("Option to only print tracks specified in a case_tracks file.");
    tt->help = tdrpStrDup("The option only applies if target_entity is COMPLETE_TRACK. Cloud seeding cases are stored in a case_tracks file. If this option is TRUE, only those tracks in the file will be printed.");
    tt->val_offset = (char *) &use_case_tracks_file - &_start_;
    tt->single_val.b = pFALSE;
    tt++;
    
    // Parameter 'case_tracks_file_path'
    // ctype is 'char*'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = STRING_TYPE;
    tt->param_name = tdrpStrDup("case_tracks_file_path");
    tt->descr = tdrpStrDup("File path for seed/no-seed cases.");
    tt->help = tdrpStrDup("This file indicates the time and track numbers for each seed and no-seed case. See use_case_tracks_file.");
    tt->val_offset = (char *) &case_tracks_file_path - &_start_;
    tt->single_val.s = tdrpStrDup("case_tracks.txt");
    tt++;
    
    // Parameter 'Comment 3'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = COMMENT_TYPE;
    tt->param_name = tdrpStrDup("Comment 3");
    tt->comment_hdr = tdrpStrDup("OUTPUT DETAILS");
    tt->comment_text = tdrpStrDup("");
    tt++;
    
    // Parameter 'print_level_properties'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("print_level_properties");
    tt->descr = tdrpStrDup("Option to add level properties to storm properties.");
    tt->help = tdrpStrDup("If true, extra columns are added to include the storms properties at each level. This option only applies to the TRACK_ENTRY type target.");
    tt->val_offset = (char *) &print_level_properties - &_start_;
    tt->single_val.b = pFALSE;
    tt++;
    
    // Parameter 'print_polygons'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("print_polygons");
    tt->descr = tdrpStrDup("Option to print storm polygons.");
    tt->help = tdrpStrDup("TRACK_ENTRY only. If set the storm polygons are printed out for each track entry.");
    tt->val_offset = (char *) &print_polygons - &_start_;
    tt->single_val.b = pFALSE;
    tt++;
    
    // Parameter 'initial_props_nscans'
    // ctype is 'int'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = INT_TYPE;
    tt->param_name = tdrpStrDup("initial_props_nscans");
    tt->descr = tdrpStrDup("Number of scans used to compute initial props.");
    tt->help = tdrpStrDup("");
    tt->val_offset = (char *) &initial_props_nscans - &_start_;
    tt->single_val.i = 5;
    tt++;
    
    // Parameter 'refl_histogram_only'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("refl_histogram_only");
    tt->descr = tdrpStrDup("Option to only print out the reflectivity histogram.");
    tt->help = tdrpStrDup("Only applicable if target_entity = TRACK_ENTRY. If this is set, the reflectivity histogram is written out after the date, time, track number and location. If refl_histogram_vol is true, you get the histogram for the volume. If it is false, you get the histogram for the area.");
    tt->val_offset = (char *) &refl_histogram_only - &_start_;
    tt->single_val.b = pFALSE;
    tt++;
    
    // Parameter 'refl_histogram_vol'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("refl_histogram_vol");
    tt->descr = tdrpStrDup("Select volume for reflectivity histograms.");
    tt->help = tdrpStrDup("If refl_histogram_only is true, and this is true, print out the reflectivity histogram for the volume. If it is false, print out the histogram for area.");
    tt->val_offset = (char *) &refl_histogram_vol - &_start_;
    tt->single_val.b = pTRUE;
    tt++;
    
    // Parameter 'specify_complex_track_num'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("specify_complex_track_num");
    tt->descr = tdrpStrDup("Option to print out data from only a single complex track.");
    tt->help = tdrpStrDup("");
    tt->val_offset = (char *) &specify_complex_track_num - &_start_;
    tt->single_val.b = pFALSE;
    tt++;
    
    // Parameter 'complex_track_num'
    // ctype is 'int'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = INT_TYPE;
    tt->param_name = tdrpStrDup("complex_track_num");
    tt->descr = tdrpStrDup("Complex track number.");
    tt->help = tdrpStrDup("See specify_complex_track_num.");
    tt->val_offset = (char *) &complex_track_num - &_start_;
    tt->single_val.i = 1;
    tt++;
    
    // Parameter 'specify_simple_track_num'
    // ctype is 'tdrp_bool_t'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = BOOL_TYPE;
    tt->param_name = tdrpStrDup("specify_simple_track_num");
    tt->descr = tdrpStrDup("Option to print out data from only a single simple track.");
    tt->help = tdrpStrDup("");
    tt->val_offset = (char *) &specify_simple_track_num - &_start_;
    tt->single_val.b = pFALSE;
    tt++;
    
    // Parameter 'simple_track_num'
    // ctype is 'int'
    
    memset(tt, 0, sizeof(TDRPtable));
    tt->ptype = INT_TYPE;
    tt->param_name = tdrpStrDup("simple_track_num");
    tt->descr = tdrpStrDup("Simple track number.");
    tt->help = tdrpStrDup("See specify_simple_track_num.");
    tt->val_offset = (char *) &simple_track_num - &_start_;
    tt->single_val.i = 1;
    tt++;
    
    // trailing entry has param_name set to NULL
    
    tt->param_name = NULL;
    
    return;
  
  }
