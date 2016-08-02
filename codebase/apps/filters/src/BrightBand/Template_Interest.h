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
// Template_Interest.h:  Interest derived from application of template
//
// Jaimi Yee, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1997
//
//////////////////////////////////////////////////////////

#ifndef Template_Interest_h
#define Template_Interest_h

#include "Interest.h"

using namespace std;

class Template_Interest : public Interest {
  
public:

  // constructor
       
  Template_Interest(vector< double > input_template_points,
		    long band_base_idex,
		    long band_top_idex,
		    double min_refl_in_band,
		    double max_refl_in_band,
		    bool compute,
		    const double max_down,
		    const double max_up,
		    const bool debug_flag = false);

  // destructor
  
  virtual ~Template_Interest();

  // Calculate the interest fields

  bool calcInterestFields(MdvxField *dbz_field,
			  fl32 *max_refl_interest,
			  fl32 *incr_refl_interest);
  
  // Add correlation, slope and shape measure arrays to the MDV file

  bool addFields(DsMdvx &output_file,
		 char *long_corr_name,
		 char *short_corr_name,
		 char *long_slope_name,
		 char *short_slope_name);

protected:
  
  //
  // Constants
  //
  static const double _SLOPE_INTEREST_WEIGHT;
  static const double _BAD_CORR_VALUE;
  static const double _BAD_SLOPE_VALUE;

private:

  bool _computeInterest;
  
  vector< double > _templateValues;

  fl32 *_corr_values;
  fl32 *_slope_values;

  int _band_base;
  int _band_top;
  
  double _maxDownParam;
  double _maxUpParam;
  double _min_refl_in_band;
  double _max_refl_in_band;

  int _compute_interest(fl32 *max_refl_interest,
                        fl32 *incr_refl_interest,
			const double max_down,
			const double max_up);
    
};

#endif
