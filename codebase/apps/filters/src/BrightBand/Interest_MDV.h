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
// Interest_MDV.h:  Sets up and writes interest MDV file
//
// Jaimi Yee, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// January 1998
//
//////////////////////////////////////////////////////////

#ifndef Interest_MDV_h
#define Interest_MDV_h

#include <Mdv/DsMdvx.hh>

#include "Refl_Interest.h"
#include "Incr_Refl_Interest.h"
#include "Template_Interest.h"
#include "Max_Interest.h"
#include "Texture_Interest.h"
#include "Clump.h"

using namespace std;

class Interest_MDV {
  
public:

  // constructor
       
  Interest_MDV (const string &interest_url,
                DsMdvx *input_data,
                Refl_Interest *refl_interest,
                Incr_Refl_Interest *incr_refl_interest,
		vector< Template_Interest* > &template_interest,
                Max_Interest *max_interest,
                Texture_Interest *texture_interest,
                Clump *clumps,
		const bool debug_flag = false);

  // destructor

  virtual ~Interest_MDV ();

  
protected:
  
private:
  
  //
  // Constants
  //
  static const int MAX_NAME_LEN;
  
  bool _debug;
  
  string _interestUrl;

  void _prepare_data(DsMdvx &output_file,
		     const string &dataset_source,
		     const Mdvx::master_header_t input_master_hdr,
		     Refl_Interest *refl_interest,
                     Incr_Refl_Interest *incr_refl_interest,
		     vector< Template_Interest* > &template_interest,
                     Max_Interest *max_interest,
                     Texture_Interest *texture_interest,
                     Clump *clumps);

};

#endif
