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
 * @file Filter.hh 
 * @brief base class for filters
 * @class Filter
 * @brief base class for filters
 *
 * @mainpage  The FiltAlg design
 *
 * FiltAlg is a library that supports apps that Filter input Data to produce
 * output Data.  One can think of a Filter as an action applied to Data to
 * produce filtered Data.  The library has a set of filters that each
 * take a different input or inputs to produce an output or outputs.
 * The design allows an app to add filters with Data input and output which
 * are specific to the app but are not general enough to be part of the 
 * library.  The filter choices and brief meaning is as follows:
 *
 * <TABLE BORDER=1 ALIGN="LEFT" cellspacing=2 cellpadding=3 style=T1>
 * <TR ALIGN="LEFT" VALIGN="TOP">
 *       <TH>  Filter    </TH> 
 *       <TH>  Class     </TH>
 *       <TH>  Action  </TH></TR>
 * <TR ALIGN="LEFT" VALIGN="TOP">
 *       <TD> ELLIP </TD>
 *       <TD> Filt2d </TD>
 *       <TD> Input is VlevelData where each slice is a Grid2dW.  The filter
 *            does 2 dimensional averaging within a box around each
 *            point </TD></TR>
 * <TR ALIGN="LEFT" VALIGN="TOP">
 *       <TD> DILATE </TD>
 *       <TD> Filt2d </TD>
 *       <TD> Input is VlevelData where each slice is a Grid2dW.  The filter
 *            is a 2 dimensional maximum within a box around each point
 *            </TD></TR>
 * <TR ALIGN="LEFT" VALIGN="TOP">
 *       <TD> MEDIAN </TD>
 *       <TD> FiltMedian     </TD>
 *       <TD> Input is VlevelData where each slice is a Grid2dW.  The filter
 *            is a 2 dimensional median within a box around each point
 *            </TD></TR>
 * <TR ALIGN="LEFT" VALIGN="TOP">
 *       <TD> SDEV </TD>
 *       <TD> Filt2d </TD>
 *       <TD> Input is VlevelData where each slice is a Grid2dW.  The filter
 *            is a 2 dimensional standard deviation within a box around
 *            each point   </TD></TR>
 * <TR ALIGN="LEFT" VALIGN="TOP">
 *       <TD> TEXTURE_X </TD>
 *       <TD> Filt2d  </TD>
 *       <TD> Input is VlevelData where each slice is a Grid2dW.  The filter
 *            is a 2 dimensional texture filter within a box around
 *            each point   </TD></TR>
 * <TR ALIGN="LEFT" VALIGN="TOP">
 *       <TD> TEXTURE_Y </TD>
 *       <TD> Filt2d     </TD>
 *       <TD> Input is VlevelData where each slice is a Grid2dW.  The filter
 *            is a 2 dimensional texture filter within a box around
 *            each point   </TD></TR>
 * <TR ALIGN="LEFT" VALIGN="TOP">
 *       <TD> MEDIAN_NO_OVERLAP </TD>
 *       <TD> FiltMedianNoOverlap     </TD>
 *       <TD> Input is VlevelData where each slice is a Grid2dW.  The filter
 *            is a 2 dimensional median within a box around each point, with
 *            non-overlapping boxes (one computation per box, then move over
 *            one full box and repeat)
 *            </TD></TR>
 * <TR ALIGN="LEFT" VALIGN="TOP">
 *       <TD> SDEV_NO_OVERLAP </TD>
 *       <TD> Filt2dNoOverlap </TD>
 *       <TD> Input is VlevelData where each slice is a Grid2dW.  The filter
 *            is a 2 dimensional standard deviation within a box around
 *            each point, with non-overlapping boxes (one computation per box,
 *            then move over one full box and repeat)   </TD></TR>
 * <TR ALIGN="LEFT" VALIGN="TOP">
 *       <TD> REMAP </TD>
 *       <TD> FiltRemap  </TD>
 *       <TD> Input is any kind of Data. The output is the same kind of Data,
 *            with each data value transformed through a linear remapping
 *            </TD></TR>
 * <TR ALIGN="LEFT" VALIGN="TOP">
 *       <TD> REPLACE </TD>
 *       <TD> FiltReplace  </TD>
 *       <TD> Input is VlevelData where each slice is a Grid2dW, and any
 *            number of additional inputs of the same kind of Data.
 *            The output is the same as the input 
 *            Data, except that the value at all points where the additional
 *            inputs meet logical conditions is set to a fixed value.
 *            </TD></TR>
 * <TR ALIGN="LEFT" VALIGN="TOP">
 *       <TD> MAX </TD>
 *       <TD> FiltCombine </TD>
 *       <TD> Each of any number of Data inputs must be the same type.
 *            The single output is the same kind of Data, with each output
 *            data value set to the maximum  of all input data values
 *            </TD></TR>
 * <TR ALIGN="LEFT" VALIGN="TOP">
 *       <TD> AVERAGE </TD>
 *       <TD> FiltCombine </TD>
 *       <TD> Each of any number of Data inputs must be the same type.
 *            The single output is the same kind of Data, with each output
 *            data value set to the average of all input data values
 *            </TD></TR>
 * <TR ALIGN="LEFT" VALIGN="TOP">
 *       <TD> PRODUCT </TD>
 *       <TD> FiltCombine </TD>
 *       <TD> Each of any number of Data inputs must be the same type.
 *            The single output is the same kind of Data, with each output
 *            data value set to the product of all input data values
 *            </TD></TR>
 * <TR ALIGN="LEFT" VALIGN="TOP">
 *       <TD> FULL_MEAN </TD>
 *       <TD> FiltScalar </TD>
 *       <TD> Input is VlevelData where each slice is a Grid2dW.  The filter
 *            Computes the mean over the each 2 dimensional slice (Grid2dw)
 *            and outputs VlevelData where each level has the Data1d mean value
 *            </TD></TR>
 * <TR ALIGN="LEFT" VALIGN="TOP">
 *       <TD> FULL_SDEV </TD>
 *       <TD> FiltScalar </TD>
 *       <TD> Input is VlevelData where each slice is a Grid2dW.  The filter
 *            Computes the standard deviation over the each 2 dimensional
 *            slice (Grid2dw), and outputs
 *            VlevelData where each level has the Data1d standard deviation
 *            </TD></TR>
 * <TR ALIGN="LEFT" VALIGN="TOP">
 *       <TD> FULL_MEDIAN </TD>
 *       <TD> FiltScalar  </TD>
 *       <TD> Input is VlevelData where each slice is a Grid2dW.  The filter
 *            Computes the median over the each 2 dimensional
 *            slice (Grid2dw), and outputs
 *            VlevelData where each level has the Data1d median
 *            </TD></TR>
 * <TR ALIGN="LEFT" VALIGN="TOP">
 *       <TD> VERT_AVERAGE </TD>
 *       <TD> FiltVertComb  </TD>
*       <TD> Input is VlevelData where each slice is a Data1d.  The filter
 *            averages the single values from each slice into a single Data1d
 *            output value
 *            </TD></TR>
 * <TR ALIGN="LEFT" VALIGN="TOP">
 *       <TD> VERT_MAX </TD>
 *       <TD> FiltVertComb     </TD>
 *       <TD> Input is VlevelData where each slice is a Data1d.  The filter
 *            takes the maximum of the single values from each slice which is
 *            then the single Data1d output value
 * <TR ALIGN="LEFT" VALIGN="TOP">
 *       <TD> VERT_PRODUCT </TD>
 *       <TD> FiltVertComb     </TD>
 *       <TD> Input is VlevelData where each slice is a Data1d.  The filter
 *            takes the product of the single values from each slice which is
 *            then the single Data1d output value
 * <TR ALIGN="LEFT" VALIGN="TOP">
 *       <TD> WEIGHTED_SUM </TD>
 *       <TD> FiltCombine  </TD>
 *       <TD> Each of any number of Data inputs must be the same type.
 *            The single output is the same kind of Data, with each output
 *            data value set to the weighted sum of all input data values
 *            </TD></TR>
 * <TR ALIGN="LEFT" VALIGN="TOP">
 *       <TD> NORM_WEIGHTED_SUM </TD>
 *       <TD> FiltCombine     </TD>
 *       <TD> Each of any number of Data inputs must be the same type.
 *            The single output is the same kind of Data, with each output
 *            data value set to the weighted sum of all input data values,
 *            normalized by the sum of weights.
 *            </TD></TR>
 * <TR ALIGN="LEFT" VALIGN="TOP">
 *       <TD> MASK </TD>
 *       <TD> FiltMask     </TD>
 *       <TD> Two inputs each are VlevelData where each slice is a Grid2dW.
 *            The main input is the data to be filtered, the other input is
 *            masking data.  The filter masks out main data where ever the
 *            mask data value is in a range of values.  </TD></TR>
 * <TR ALIGN="LEFT" VALIGN="TOP">
 *       <TD> RESCALE </TD>
 *       <TD> FiltRescale  </TD>
 *       <TD> Input is any kind of Data.  The filter rescales the data by
 *            multiplying by a scale and adding an offset.  The output is the
 *            same kind of data.  </TD></TR>
 * <TR ALIGN="LEFT" VALIGN="TOP">
 *       <TD> DB2LINEAR </TD>
 *       <TD> FiltDB     </TD>
 *       <TD> Input is VlevelData where each slice is a Grid2dW.  The filter
 *            Computes the linear value at each input point, assuming the
 *            input data is dB 
 *            </TD></TR>
 * <TR ALIGN="LEFT" VALIGN="TOP">
 *       <TD> LINEAR2DB </TD>
 *       <TD> FiltDB     </TD>
 *       <TD> Input is VlevelData where each slice is a Grid2dW.  The filter
 *            Computes the dB value at each input point, assuming the
 *            input data is linear
 *            </TD></TR>
 * <TR ALIGN="LEFT" VALIGN="TOP">
 *       <TD> PASSTHROUGH </TD>
 *       <TD> FiltPassThrough     </TD>
 *       <TD> Input is any Data, the output is the same Data  </TD></TR>
 * <TR ALIGN="LEFT" VALIGN="TOP">
 *       <TD> APPFILTER </TD>
 *       <TD> Depends on filter     </TD>
 *       <TD> Depends on filter   </TD></TR>
 * </TABLE>
 * <P><BR><BR><BR><BR><BR><BR></P>
 * <P><BR><BR><BR><BR><BR><BR></P>
 * <P ALIGN="LEFT">
 * The way in which the various Filter actions can be configured is desribed
 * <a href="FiltAlgDesign.html">here</a>. </P>
 */

#ifndef FILTER_H
#define FILTER_H
#include "Params.hh"
#include <Radx/RayxData.hh>
#include <Radx/RadxRay.hh>
#include <radar/RadxApp.hh>

//------------------------------------------------------------------
class Filter
{
public:
  /**
   * Constructor
   * @param[in] f  Filter type
   * @param[in] p  Params
   */
  Filter(const Params::data_filter_t f, const Params &p);

  /**
   * Destructor
   */
  virtual ~Filter(void);

  /**
   * @return true if object well formed
   */
  inline bool ok(void) const {return _ok;}

  #define FILTER_BASE
  #include "FilterVirtualFunctions.hh"
  #undef FILTER_BASE

  /**
   * Print out filter and its input and output
   */
  void printInputOutput(void) const;

  /**
   * @return  A string name for the input filter type
   * @param[in] f  Filter type
   */
  static string filter_string(const Params::data_filter_t f);

  /**
   * @return the number of filter param sets configured for currently for
   * the input filter type in the input configuration.
   * @param[in] f  Filter type
   * @param[in] p  A particular configuration
   * 
   * @note the number is the length of an array in p
   */
  static int max_elem_for_filter(const Params::data_filter_t f,
				 const Params &p);

protected:

  bool _ok;                        /**<  true if object well formed */
  Params::data_filter_t _f; /**< params for this particular filter */
  bool _doPrintInputOutput; /**< True to actually print inputs/outputs */

private:
};

#endif
