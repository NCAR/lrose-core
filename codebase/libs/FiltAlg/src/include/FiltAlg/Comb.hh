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
 * @file Comb.hh 
 * @brief Handles Combining multiple inputs
 * @class Comb
 * @brief Handles Combining multiple inputs
 * 
 */

#ifndef COMB_H
#define COMB_H
#include <vector>
#include <FiltAlg/FiltAlgParams.hh>
#include <FiltAlg/CombineData.hh>
#include <FiltAlg/Data.hh>

class AngleCombiner;

//------------------------------------------------------------------
class Comb 
{
public:

  /**
   * @struct DataWithConf_t
   * @brief describe one data type when it includes an associated
   *        confidence
   */
  typedef struct
  {
    std::string name; /**< Name */
    bool is_input;    /**< True if data is input to app */
    std::string conf_name; /**< Confidence data name */
    bool conf_is_input;   /**< True if confidence data is input to app*/
    double weight; /**< Weight for this data in combining */
  } DataWithConf_t;

  /**
   * @struct DataWithoutConf_t
   * @brief describe one data type when it does not include an
   *        associated confidence
   */
  typedef struct
  {
    std::string name; /**< Name */
    bool is_input;    /**< True if data is input to app */
    double weight; /**< Weight for this data in combining */
  } DataWithoutConf_t;

  /**
   * Empty constructor
   */
  Comb(void);

  /**
   * Constructor using params
   * @param[in] p  Params
   * @param[in] index  Index to a particular list of combine objects in params
   *
   * Sets all the _data but no pointers to any actual data
   */
  Comb(const FiltAlgParams &p, const int index);

  /**
   * Constructor for data with confidence
   * @param[in] p  The description of the inputs (except main one)
   * @param[in] mainConf Name of main confidence input
   * @param[in] mainConfIsInput True if main confidence is app input
   * @param[in] vlevel_tolerance  Vertical level tolerance
   *
   * Sets all the _data and _conf but no pointers to any actual data
   */
  Comb(const std::vector<DataWithConf_t> &p, const std::string &mainConf,
       const bool mainConfIsInput, const double vlevel_tolerance);


  /**
   * Constructor for data without confidence
   * @param[in] p  The description of the inputs (except main one)
   * @param[in] vlevel_tolerance  Vertical level tolerance
   *
   * Sets all the _data but no pointers to any actual data
   */
  Comb(const std::vector<DataWithoutConf_t> &p, const double vlevel_tolerance);

  /**
   * Destructor
   */
  virtual ~Comb(void);

  /**
   * @return true if well formed
   */
  inline bool ok(void) const {return _ok;}

  /**
   * Set pointers for the data using input vectors
   * @param[in] input Input Data
   * @param[in] output Output Data
   *
   * @return true if all state pointers have been set
   */
  bool create_inputs(const vector<Data> &input, const vector<Data> &output);

  /**
   * Check if all local data types equal input
   * @param[in] type
   * @return true if all data agrees with this type
   */
  bool check_data(const Data::Data_t type) const;

  /**
   * set a slice to maximum of what it is on input and local state
   * (if 2d grid it is done at each gridpoint, otherwise single point)
   * @param[in] vlevel Vertical level (degrees)
   * @param[in,out] o  Slice to modify
   *
   * @return true if was able to do this false if inconstancies or wrong types
   */
  bool max(const double vlevel, VlevelSlice &o) const;

  /**
   * set a slice to average of what it is on input and local state
   * (if 2d grid it is done at each gridpoint, otherwise single point)
   * @param[in] vlevel Vertical level (degrees)
   * @param[in] orientation  True if quantities getting averaged are orientation
   *                         angles
   * @param[in,out] o  Slice to modify
   *
   * @return true if was able to do this false if inconstancies or wrong types
   */
  bool average(const double vlevel, const bool orientation,
	       VlevelSlice &o) const;

  /**
   * set a slice to product of what it is on input and local state
   * (if 2d grid it is done at each gridpoint, otherwise single point)
   * @param[in] vlevel Vertical level (degrees)
   * @param[in,out] o  Slice to modify
   *
   * @return true if was able to do this false if inconstancies or wrong types
   */
  bool product(const double vlevel, VlevelSlice &o) const;

  /**
   * Set a slice to the weighted sum of what it is on input and local state
   * (if 2d grid it is done at each gridpoint, otherwise single point)
   * @param[in] vlevel Vertical level (degrees)
   * @param[in] w0  Weight to give input data
   * @param[in] norm True to normalize the result, false to not do so
   * @param[in] orientation  True if quantities getting averaged are orientation
   *                         angles
   * @param[in,out] o  Slice to modify
   *
   * @return true if was able to do this false if inconstancies or wrong types
   * 
   * @note Local state at input vlevel is given the local weight in weighted
   *       sum
   */
  bool weighted_sum(const double vlevel, const double w0, const bool norm,
		    const bool orientation, VlevelSlice &o) const;

  /**
   * Set a slice to the weighted sum of confidence data in local state
   * (if 2d grid it is done at each gridpoint, otherwise single point)
   * @param[in] vlevel Vertical level (degrees)
   * @param[in] w0  Weight to give main confidence data
   * @param[in] norm True to normalize the result, false to not do so
   * @param[in,out] o  Slice to set
   *
   * @return true if was able to do this false if inconstancies or wrong types
   * 
   * @note Local state at input vlevel is given the local weight in weighted
   *       sum
   */
  bool weighted_confidence(const double vlevel, const double w0,
			   const bool norm, VlevelSlice &o) const;

  /**
   * One dimensional data adjust v to the max of input and local state
   * @param[in,out] v  Value
   * @return true if successful
   */
  bool max(double &v) const;

  /**
   * One dimensional data adjust o to the average of input and local state
   * @param[in] orientation  True if quantities getting averaged are orientation
   *                         angles
   * @param[in,out] o  Value
   * @return true if successful
   */
  bool average(const bool orientation, double &o) const;

  /**
   * One dimensional data adjust o to the product of input and local state
   * @param[in,out] o  Value
   * @return true if successful
   */
  bool product(double &o) const;

  /**
   * One dimensional data adjust o to the weighted sum of what it is on
   * input and local state
   * @param[in] w0  Weight to give input data
   * @param[in] norm True to normalize the result, false to not do so
   * @param[in] orientation  True if quantities getting averaged are orientation
   *                         angles
   * @param[in,out] o  Value
   *
   * @return true if was able to do this false if inconstancies or wrong types
   * 
   * @note Local value is given the local weight in weighted sum
   */
  bool weighted_sum(const double w0, const bool norm, const bool orientation, 
		    double &o) const;
  

  /**
   * @return true if the input string is one of the named data elements
   * @param[in] name  Name to check for
   */
  bool hasNamedData(const std::string &name) const;

  /**
   * @return Pointer to named Data, NULL for none found.
   *
   * @param[in] name  Name to match
   */
  const Data *dataPointer(const std::string &name) const;

protected:
private:

  bool _ok;                  /**< True for well formed */
  double _vlevel_tolerance;  /**< Allowed error in vlevels */
  bool _has_confidence;      /**< True if _data includes confidence */
  vector<CombineData> _data; /**< The data to combine */

  CombineData _mainConf;     /**< Confidence data for main input (or not used)*/


  /**
   * Build combine data, without setting pointers, and put to state
   * @param[in] n  Length of ff array
   * @param[in] ff  Array of combine params to use
   */
  void _build(const int n, const FiltAlgParams::combine_t *ff);

  void _average(const double vlevel, VlevelSlice &o) const;
  void _orientation_average(const double vlevel, VlevelSlice &o) const;
  bool _weighted_sum(const double vlevel, const double w0,
		     const bool norm, VlevelSlice &o) const;
  void _weighted_orientation_sum(const double vlevel, const double w0,
			       const bool norm, VlevelSlice &o) const;
  void _orientation_combine(AngleCombiner &A, const double vlevel,
			    VlevelSlice &o) const;
  std::vector<double> _weights(const double w0) const;
  bool _mainConfidence1d(double &conf) const;
  bool _mainConfidenceGrid(double &conf) const;
  bool _get1dDataConf(const int i, double &vi, double &ci, bool &isbad) const;
  bool _get1dMainDataConf(const VlevelSlice &o, double &vi,
			  double &ci) const;
  bool _get2dDataConf(const int i, const double vlevel,
		      const int x, const int y,
		      double &vi, double &ci) const;
  bool _get2dMainDataConf(const VlevelSlice &o, const double vlevel,
			  const int x, const int y,
			  double &vi, double &ci) const;

};

#endif
