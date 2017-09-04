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
 * @file Algorithm.hh 
 * @brief The algorithm class, controls the filters.
 * @class Algorithm
 * @brief The algorithm class, controls the filters.
 */

#ifndef ALGORITHM_H
#define ALGORITHM_H
#include <FiltAlg/Info.hh>
#include <FiltAlg/FiltAlgParms.hh>
#include <FiltAlg/Data.hh>
#include <FiltAlg/Filter.hh>
#include <FiltAlg/FiltInfo.hh>
#include <toolsa/TaThreadDoubleQue.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxProj.hh>
#include <vector>
#include <deque>

class LineList;
class FiltCreate;

//------------------------------------------------------------------
class Algorithm 
{
public:
  /**
   * Constructor
   * @param[in] p program param settings
   * @param[in] create  pointer to object used to create filters
   */
  Algorithm(const FiltAlgParms &p, const FiltCreate *create);

  /**
   * Destructor
   */
  virtual ~Algorithm(void);

  /**
   * @return true if object well formed
   */
  inline bool ok(void) const { return _ok;}

  /**
   * Update at a particular data time
   * @param[in] t the time of the data
   * @param[in] p program param settings
   *
   * @return true for success
   */
  bool update(const time_t &t, const FiltAlgParms &p);
  
  /**
   * Compute method needed for threading
   * @param[in] i  Information
   */
  static void compute(void *i);

  /**
   * Write out a linelist as SPDB bdry data
   * @param[in] spdbUrl  Where to write
   * @param[in] l  The linelist
   * @param[in] gp  The projection to use to translate to lat/lon from xy
   */
  static bool write_lines(const std::string &spdbUrl, const LineList &l, 
			  const GridProj &gp);


protected:
private:

  /**
   * @class AlgThreads
   * @brief Simple class to instantiate TaThreadDoubleQue by implementing
   * the clone() method.
   */
  class AlgThreads : public TaThreadDoubleQue
  {
  public:
    /**
     * Empty constructor
     */
    inline AlgThreads() : TaThreadDoubleQue() {}
    /**
     * Empty destructor
     */
    inline virtual ~AlgThreads() {}
    /**
     * Clone a thread and return pointer to base class
     * @param[in] index 
     */
    TaThread *clone(const int index);
  };


  bool _ok;             /**< True if object well formed */
  time_t _last_time;    /**< Previous processing time, used for feedback*/
  vector<double> _last_vlevel;    /**< vertical levels, previous */
  vector<Data> _input;  /**< The input data which is read in */
  vector<Data> _output; /**< The output data which is written out */
  vector<Filter *> _filters;  /**< The filters, in order, pointers because
			       *   they are derived classes */
  /**
   * Information retained for each vlevel 
   * during a filter, needed for thread safeness
   */
  vector<FiltInfo> _filtInfo; 
  vector<double> _vlevel;    /**< vertical levels */
  Mdvx::field_header_t _hdr;   /**< MDV information pulled from input data*/
  Mdvx::vlevel_header_t _vhdr; /**< MDV information pulled from input data*/
  MdvxProj _proj;              /**< MDV information pulled from input data*/

  /**
   * Information that can be stored as XML or written out for matlab
   * A pointer because an app can derive off of this class
   */
  Info *_info;

  /**
   * Information that can be read in as XML
   * A pointer because an app can derive off of this class
   */
  Info *_input_info;

  /**
   * Threading object
   */
  AlgThreads _thread;

  /**
   * Create and store a filter using input params
   * @param[in] create  Pointer to object that creates filters
   * @param[in] pm  Main params
   * @param[in] p  The params for this particuilar filter
   * @return true if successful
   */
  bool _create_filter(const FiltCreate *create,
		      const FiltAlgParms &pm,
		      const FiltAlgParams::data_filter_t &p);

  /**
   * Initialize for a trigger time
   * @param[in] t
   * @param[in] p
   * @param[out] dout
   * @param[out] vlevelChange
   *
   * @return true for success
   */
  bool _update_init(const time_t &t, const FiltAlgParms &p, DsMdvx &dout,
		    bool &vlevelChange);

  /**
   * Write output that is found in dout
   * @param[in] p
   * @param[in] dout
   *
   * @return true for success
   */
  bool _do_output(const FiltAlgParms &p, DsMdvx &dout);

  /**
   * Read inputs
   * @param[in] t  Time to read
   * @param[in] p  The parameters
   * @param[out] mhdr  Master header returned
   * @return true if successful
   */
  bool _read(const time_t &t, const FiltAlgParms &p,
	     Mdvx::master_header_t &mhdr);
  /**
   * Read all inputs from a particular URL
   * @param[in] t  Time to read
   * @param[in] p  Params
   * @param[in] i0  Index to inputs in params
   * @param[in] url  The url 
   * @param[out] mhdr  Master header returned
   * @param[in,out] wait_seconds  Time spend waiting
   * @return true if successful
   *
   * @note can wait up to a maximum
   */
  bool _read_1(const time_t &t, const FiltAlgParms &p, const int i0, 
	       const string &url, Mdvx::master_header_t &mhdr,
	       int &wait_seconds);

  /**
   * Read all feedback inputs from a particular URL using _last_time or
   * most recent earlier time
   *
   * @param[in] t  Current time
   * @param[in] p  Params
   * @param[in] i0  Index to inputs in params
   * @param[in] url  The url 
   * @return true if successful
   *
   * @note no waiting
   */
  bool _feedback_read_1(const time_t &t, const FiltAlgParms &p, const int i0,
			const string &url);

  /**
   * Add input (which should be in DsMdvx object) to local state
   * @param[in] input_field  Name of the field in mdv
   * @param[in] name   Name of the data
   * @param[in] p  Params
   * @param[in] d  Object with data
   * @return true if successful
   *
   * @note puts data to _input vector
   */
  bool _add_input(const char *input_field, const char *name,
		  const FiltAlgParms &p, DsMdvx &d);

  /**
   * Add input (which is either in DsMdvx object, or 'fake') to local state
   * @param[in] input_field  Name of the field in mdv
   * @param[in] name   Name of the data
   * @param[in] p  Params
   * @param[in] d  Object with data (if fake=false)
   * @param[in] fake  True to add fake all missing data
   *
   * @return true if successful
   *
   * @note puts data to _input vector
   */
  bool _add_feedback_input(const char *input_field, const char *name,
			   const FiltAlgParms &p, DsMdvx &d,
			   const bool &fake);

  /**
   * Do filtering for one filter
   * @param[in] t  Time
   * @param[in] vlevelChange  True if vertical levels have changed
   * @param[in] vlevel_tolerance  Vertical level error allowance (degrees)
   * @param[in] f The filter
   * @param[in,out] dout  Object to put results into
   * @return true if successful
   *
   * Changes local state
   */
  bool _filter(const time_t &t, bool vlevelChange,
	       const double vlevel_tolerance,
	       Filter *f, DsMdvx &dout);

  /**
   * Create a vector of outputs by applying the filter 
   * @param[in] t  Time
   * @param[in] vlevel_tolerance  Vertical level error allowance (degrees)
   * @param[in] f The filter
   * @param[out] new_out  The outputs that are not the main output
   * @return true if successful
   *
   * The main output is handled internally.
   * The inputs are gotten internally
   */
  bool _filter_1(const time_t &t, const double vlevel_tolerance,
		 Filter *f, vector<Data> &new_out);

  /**
   * Filter when data is 1 dimensional
   * @param[in] f  The filter
   * @param[in] gin  The input data
   * @param[in,out] gout  The output data
   */
  bool _filter_1d(const Filter *f, const Data *gin, Data *gout);

  /**
   * Filter when data is 2 dimensional
   * @param[in] f  The filter
   * @param[in] gin  The input data
   * @param[in,out] gout  The output data
   */
  bool _filter_2d(const Filter *f, const Data *gin, Data *gout);

  /**
   * Add field to a DsMdvx object
   * @param[in] t  Time
   * @param[in] g  Data from which to get a 3d field
   * @param[in,out] dout  Where to put data
   * @return true if successful
   */
  bool _add_field(const time_t &t, const Data &g, DsMdvx &dout) const;


  /**
   * Compute for a vertical level
   * @param[in] gin  Input data for filter
   * @param[in] i  Vlevel index
   * @param[in] f  Filter
   * @param[in]  gout  Output Data object
   */
  void _compute(const Data &gin, const int i, const Filter *f,
		const Data *gout);


  /**
   * Set up persistent FiltInfo data for inputs
   *
   * @param[in] gin  Input data
   * @param[in] i  Vlevel index
   * @param[in] f  Filter
   * @param[in]  gout  Output Data object
   *
   * @return Pointer to the info
   */
  FiltInfo *_setupInfo(const Data &gin, const int i,
		       const Filter *f, const Data *gout);

};

#endif
