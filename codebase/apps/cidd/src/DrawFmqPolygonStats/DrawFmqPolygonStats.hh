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
 *
 * @file DrawFmqPolygonStats.hh
 *
 * @class DrawFmqPolygonStats
 *
 * DrawFmqPolygonStats program object.
 *  
 * @date 12/1/2008
 *
 */

#ifndef DrawFmqPolygonStats_HH
#define DrawFmqPolygonStats_HH

#include <string>

#include <Fmq/DrawQueue.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <rapformats/GenPolyStats.hh>

#include "Args.hh"
#include "Input.hh"
#include "MdvHandler.hh"
#include "Params.hh"

using namespace std;


/** 
 * @class DrawFmqPolygonStats
 */

class DrawFmqPolygonStats
{
 public:

  ////////////////////
  // Public members //
  ////////////////////

  /**
   * @brief Flag indicating whether the program status is currently okay.
   */

  bool okay;


  ////////////////////
  // Public methods //
  ////////////////////

  //////////////////////////////
  // Constructors/Destructors //
  //////////////////////////////

  /**
   * @brief Destructor
   */

  virtual ~DrawFmqPolygonStats(void);
  

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv List of command line arguments.
   *
   * @return Returns a pointer to the program instance.
   */

  static DrawFmqPolygonStats *Inst(int argc, char **argv);

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @return Returns a pointer to the program instance.
   */

  static DrawFmqPolygonStats *Inst();
  

  /**
   * @brief Initialize the local data.
   *
   * @return Returns true if the initialization was successful,
   *         false otherwise.
   */

  bool init();
  

  /////////////////////
  // Running methods //
  /////////////////////

  /**
   * @brief Run the program.
   */

  void run();
  

 private:

  ///////////////////////
  // Private constants //
  ///////////////////////

  /**
   * @brief Value to use to flag missing values for the calculated statistics.
   */

  static const double MISSING_VALUE;
  

  /////////////////////
  // Private members //
  /////////////////////

  /**
   * @brief Singleton instance pointer.
   */

  static DrawFmqPolygonStats *_instance;
  
  /**
   * @brief Program name.
   */

  char *_progName;

  /**
   * @brief Command line arguments.
   */

  Args *_args;

  /**
   * @brief Parameter file paramters.
   */

  Params *_params;
  
  /**
   * @brief Input handler.
   */

  Input *_input;

  /**
   * @brief MDV input handler.
   */

  MdvHandler _mdvHandler;
  
  /**
   * @brief Optional histogram output file.
   */

  FILE *_histogramFile;
  

  /////////////////////
  // Private methods //
  /////////////////////

  /**
   * @brief Constructor
   *
   * @param[in] argc Number of command line arguments.
   * @param[in] argv List of command line arguments.
   *
   * @note Private because this is a singleton object.
   */

  DrawFmqPolygonStats(int argc, char **argv);
  

  bool _addDbzZdrStatistics(const int vlevel_num,
			    unsigned char *grid_array,
			    unsigned char *dropsize_grid_array,
			    GenPolyStats &polygon);
  

  bool _addDiscreteFieldStatistics(const MdvxField &field,
				   const int vlevel_num,
				   unsigned char *grid_array,
				   GenPolyStats &polygon);
  
  void _addDiscreteFieldStatsToPolygon(const string &field_name,
				       const string &field_units,
				       const vector< double > &values,
				       GenPolyStats &polygon) const;
  

  /**
   * @brief Add the statistics for the indicated field to the polygon.
   *
   * @param[in] field MDV field to use for statistics.
   * @param[in] is_log Flag indicating whether this is a DBZ field.
   * @param[in] grid_array Array representing the gridded polygon.  The
   *                       non-zero values in this array indicated grid
   *                       squares that fall within the defined polygon.
   * @param[in,out] polygon The polygon.
   *
   * @return Returns true on success, false on failure.
   */

  bool _addFieldStatistics(const MdvxField &field,
			   const int vlevel_num,
			   const bool is_log,
			   unsigned char *grid_array,
			   GenPolyStats &polygon);
  

  /**
   * @brief Calculate the statistics for the given field and add them to
   *        the polygon.
   *
   * @param[in] field_name Name of field.
   * @param[in] field_units Field units.
   * @param[in] values_linear The field values.
   * @param[in] values_log The field values in log space.
   * @param[in] is_log Flag indicating whether this is a logarithmic field.
   * @param[in,out] polygon The associated polygon.
   *
   * @return Returns the calculated field mean.
   */

  double _addFieldStatsToPolygon(const string &field_name,
				 const string &field_units,
				 const vector< double > &values_linear,
				 const vector< double > &values_log,
				 const bool is_log,
				 GenPolyStats &polygon) const;
  

  inline void _addFieldValue(const string &field_name,
			     const string &field_units,
			     const string &stat_name,
			     const double stat_value,
			     GenPolyStats &polygon) const
  {
    polygon.addField(stat_name + " " + field_name,
		     stat_value, field_units);
  }


  bool _addPolygonStatistics(GenPolyStats &polygon,
			     const int scan_time_offset,
			     const int vlevel_num,
			     const double vlevel_ht,
			     const MdvxPjg &proj,
			     const double radar_lat,
			     const double radar_lon,
			     const double radar_alt_km,
			     const int scan_mode,
			     unsigned char *grid_array,
			     const long num_pts_filled) const;
  

  /**
   * @brief Add the statistics to the given polygon.
   *
   * @param[in,out] polygon The polygon.
   * @param[in] vlevel_ht The vertical level height in native units.
   *
   * @return Returns true on success, false on failure.
   */

  bool _addStatistics(GenPolyStats &polygon,
		      const double vlevel_ht);
  

  /**
   * @brief Calculate the statistics for the given ZDR field and add them to
   *        the polygon.
   *
   * @param[in] field_name Name of field.
   * @param[in] field_units Field units.
   * @param[in] values_linear The field values.
   * @param[in] values_log The field values in log space.
   * @param[in] zh_values_linear The Zh field values.
   * @param[in] zv_values_linear The Zv field values.
   * @param[in,out] polygon The associated polygon.
   *
   * @return Returns the calculated field mean.
   */

  double _addZdrFieldStatsToPolygon(const string &field_name,
				    const string &field_units,
				    const vector< double > &values_linear,
				    const vector< double > &values_log,
				    const vector< double > &zh_values_linear,
				    const vector< double > &zv_values_linear,
				    GenPolyStats &polygon) const;
  

  bool _applyThreshold(const MdvxField &thresh_field,
		       const int vlevel_num,
		       const Params::thresh_compare_t comparison,
		       const double compare_value,
		       unsigned char *grid_array,
		       const size_t grid_size,
		       long &num_pts_filled) const;
  
  inline static double _calcD0Zdr(const double zdr_value)
  {
    return 0.466 + (1.22 * zdr_value) -
      (0.523 * pow(zdr_value, 2)) +
      (0.128 * pow(zdr_value, 3));
  }
  
  inline static double _calcD0Zh(const double zh_value)
  {
    return 0.427 + (0.0166 * zh_value) -
      (0.000765 * pow(zh_value, 2)) +
      (0.0000208 * pow(zh_value, 3));
  }
  
  inline static double _calcDmaxZdr(const double zdr_value)
  {
    return 0.963 + (3.49 * zdr_value) -
      (0.842 * pow(zdr_value, 2)) +
      (0.0853 * pow(zdr_value, 3));
  }
  
  inline static double _calcDmaxZh(const double zh_value)
  {
    return 0.844 + (0.0171 * zh_value) -
      (0.000278 * pow(zh_value, 2)) +
      (0.0000351 * pow(zh_value, 3));
  }
  
  static map< double, int > _calcHistogram(const GenPolyStats &polygon,
					   const string &field_name,
					   const vector< double > &values,
					   FILE *histogram_file);

  static double _calcMaximum(const vector< double > &values);
  
  static double _calcMean(const vector< double > &values);
  
  static double _calcMeanZdr(const vector< double > &zh_values,
			     const vector< double > &zv_values);
  
  static double _calcStdDev(const vector< double > &values);
  
  static double _calcMedian(const vector< double > &values);
  
  static double _calcMinimum(const vector< double > &values);
  
  static vector< double > _calcMode(const map< double, int > &histogram);

  /**
   * @brief Create the polygon for the given message.
   *
   * @param[in] input_prod The message to process.
   *
   * @return Returns true on success, false on failure.
   */

  bool _createPolygon(const Human_Drawn_Data_t &input_prod);
  

  bool _getDropsizePolygonGrid(const int vlevel_num,
			       unsigned char *grid_array,
			       const size_t grid_size,
			       long &num_pts_filled) const;
  
  bool _getPolygonGrid(const MdvxPjg &proj,
		       const int vlevel_num,
		       const GenPolyStats &polygon,
		       unsigned char *grid_array,
		       long &num_pts_filled) const;
  

  inline static double _linearToLog(const double linear_value)
  {
    return 10.0 * log10(linear_value);
  }
  
  inline static double _logToLinear(const double log_value)
  {
    return pow(10.0, log_value / 10.0);
  }
  
  /**
   * @brief Process the lastest input message.
   *
   * @param[in] input_prod The latest input message.
   *
   * @return Returns true on success, false on failure.
   */

  bool _processInput(const Human_Drawn_Data_t &input_prod);
  
  /**
   * @brief Write the histogram information to the histogram file.
   *
   * @param[in] histogram_file     Pointer to the histogram output file.
   * @param[in] polygon            Polygon information.
   * @param[in] field_name         Name of the field.
   * @param[in] histogram          Histogram values.
   */

  static void _writeHistogramFile(FILE *histogram_file,
				  const GenPolyStats &polygon,
				  const string &field_name,
				  const map< double, int > &histogram);


};


#endif
