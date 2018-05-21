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
#ifdef FILTER_BASE

  /**
   * Debug printing of summary of filter in derived class
   */
  virtual void filter_print(void) const = 0;

  /**
   * Debug printing of summary of filter in derived class
   * @param[in] vlevel Vertical level (degrees) which is printed out also
   */
  virtual void filter_print(const double vlevel) const = 0;

   /**
    * virtual function to inialize an output object.
    * @param[in] inp  Input data sometimes used to build output 
    * @param[in] f  Filter parameters for this particular instance of the filter
    * @param[out] g output data to initialize
    *
    * Initalizes the field name, the data type, and whether it is written
    * to disk or not
    */
   virtual void initialize_output(const Data &inp,
				  const FiltAlgParams::data_filter_t f,
				  Data &g) = 0;

  /**
   * virtual function to apply the filter,
   *
   * @param[in] inp Inputs
   * @param[in,out] out Outputs
   * @return true if successful
   */
   virtual bool filter(const FiltInfoInput &inp, FiltInfoOutput &out) const = 0;

  /**
   * virtual function to create all the extra data for the particular filter
   * which are in addition to the main input, and do any other initialization
   * steps.
   *
   * @param[in] t  Time
   * @param[in] input  Vector of 3d grids that are initial app inputs
   * @param[in] output  Vector of 3d grids that are upstream filtering outputs
   * @return true if successful
   *
   * @note the main input is handled elsewhere
   */
  virtual bool create_inputs(const time_t &t, const vector<Data> &input,
			     const vector<Data> &output) = 0;

  /**
   * Virtual function to create extra information as needed and store to
   * the FiltInfo as 'extra' information
   */
  virtual void create_extra(FiltInfo &info) const = 0;

  /**
   * virtual function to store all the filter outputs 
   * @param[in] o  main output to store
   * #param[in] extra  Other optional outputs to store AND FREE
   * @param[out] info  Object to put information into if the filter supports it
   * @param[out] output  returned output which has been modified to contain
   *                     output from this filter
   * @return true if successful
   */
   virtual bool store_outputs(const Data &o, Info *info,
			      vector<FiltInfo> &extra,
			      vector<Data> &output)=0;

  /**
   * virtual function 
   * Create or verify an info object if this filter writes info.
   * On input current info object exists, which can be destroyed and
   * overwritten by this method.
   *
   * @param[in,out] info  Pointer to the Info object
   */
  virtual void set_info(Info **info) const = 0;

  /**
   * virtual function 
   * Create or verify an info object if this filter reads info.
   * On input current info object exists, which can be destroyed and
   * overwritten by this method.
   *
   * @param[in,out] info  Pointer to the Info object
   */
  virtual void set_input_info(Info **info) const = 0;

  /**
   * virtual function 
   * Take action when the vertical levels have changed.
   */
  virtual void vertical_level_change(void);

#else

  /**
   * Debug printing of summary of filter in derived class
   */
  virtual void filter_print(void) const;

  /**
   * Debug printing of summary of filter in derived class
   * @param[in] vlevel Vertical level (degrees) which is printed out also
   */
  virtual void filter_print(const double vlevel) const;

   /**
    * virtual function to inialize an output object.
    * @param[in] inp  Input data sometimes used to build output 
    * @param[in] f  Filter parameters for this particular instance of the filter
    * @param[out] g output data to initialize
    *
    * Initalizes the field name, the data type, and whether it is written
    * to disk or not
    */
  virtual void initialize_output(const Data &inp,
				 const FiltAlgParams::data_filter_t f,
				 Data &g);

  /**
   * virtual function to apply the filter
   *
   * @param[in] inp Inputs
   * @param[in,out] out  Outputs
   * @return true if successful
   */
   virtual bool filter(const FiltInfoInput &inp, FiltInfoOutput &out) const;

  /**
   * virtual function to create all the extra data for the particular filter
   * which are in addition to the main input, and do any other initialization
   * steps.
   *
   * @param[in] t  Time
   * @param[in] input  Vector of 3d grids that are initial app inputs
   * @param[in] output  Vector of 3d grids that are upstream filtering outputs
   * @return true if successful
   *
   * @note the main input is handled elsewhere
   */
  virtual bool create_inputs(const time_t &t, const vector<Data> &input,
			     const vector<Data> &output);

  /**
   * Virtual function to create extra information as needed and store to
   * the FiltInfo as 'extra' information
   */
  virtual void create_extra(FiltInfo &info) const;

  /**
   * virtual function to store all the filter outputs 
   * @param[in] o  main output to store
   * #param[in] extra  Other optional outputs to store AND FREE
   * @param[out] info  Object to put information into if the filter supports it
   * @param[out] output  returned output which has been modified to contain
   *                     output from this filter
   * @return true if successful
   */
   virtual bool store_outputs(const Data &o, Info *info,
			      vector<FiltInfo> &extra,
			      vector<Data> &output);

  /**
   * virtual function 
   * Create or verify an info object if this filter writes info.
   * On input current info object exists, which can be destroyed and
   * overwritten by this method.
   *
   * @param[in,out] info  Pointer to the Info object
   */
  virtual void set_info(Info **info) const;

  /**
   * virtual function 
   * Create or verify an info object if this filter reads info.
   * On input current info object exists, which can be destroyed and
   * overwritten by this method.
   *
   * @param[in,out] info  Pointer to the Info object
   */
   virtual void set_input_info(Info **info) const;

  /**
   * virtual function 
   * Take action when the vertical levels have changed.
   */
  void vertical_level_change(void);

#endif
