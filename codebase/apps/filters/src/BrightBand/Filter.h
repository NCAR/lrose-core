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
//////////////////////////////////////////////////////////
// Filter.h:  Data with brightband filtered out
//
// Jaimi Yee, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// January 1998
//
//////////////////////////////////////////////////////////

#ifndef Filter_h
#define Filter_h

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>

class Filter {
  
public:

  // constructor
       
  Filter (const string &output_url,
	  DsMdvx *input_data,
          MdvxField *dbz_field,
          int num_clumps,
          fl32 *clump_values,
          int *template_num,
          int *hts,
          int band_base1,
          int band_top1,
          int band_base2,
          int band_top2,
          int band_base3,
          int band_top3,
	  const bool debug_flag,
          Mdvx::encoding_type_t output_encoding,
          Mdvx::compression_type_t output_compression);

  // destructor

  virtual ~Filter ();

  // data members
 
  int OK;

protected:
  

private:

  bool _debug;
  
  DsMdvx *_input_data;
  MdvxField *_dbz_field;
  Mdvx::field_header_t _dbzFieldHdr;
  
  string _file_path;
  string _dir_path;
     
};

#endif
