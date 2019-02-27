#ifdef MATH_DATA_BASE

  /**
   * @return number of data points in the local object
   */
  virtual int numData(void) const = 0;

  /**
    * Actions to take when one VolumeData loop action is complete.
    *
    * @param[in] index  Index into the VolumeData for looping
    *
    * Called by the object that was created in initializeProcessingNode()
   */
  virtual void finishProcessingNode(int index, VolumeData *vol) = 0;

  /**
   * Called within the loop to set up input data, and output data
   *
   * @param[in] output  Output data name
   * @param[in] inputs  Input data names
   *
   * @return true if successful synching
   */
  virtual bool synchInputsAndOutputs(const std::string &output,
				     const std::vector<std::string> &inputs)=0;
  /**
   * @return pointer to the MathLoopData associated with a name
   *         or NULL for none.  The pointer is owned by this object, should
   *         not be freed by calling routine
   *
   * @param[in] name
   */
  virtual MathLoopData *dataPtr(const std::string &name) = 0;

  /**
   * @return pointer to the MathLoopData associated with a name
   *         or NULL for none.  The pointer is owned by this object, should
   *         not be freed by calling routine
   *
   * @param[in] name
   */
  virtual const MathLoopData *dataPtrConst(const std::string &name) const = 0;

  /**
   * @return pointer to the MathUserData associated with a name
   *         or NULL for none.  The pointer is owned by this object, should
   *         not be freed by calling routine
   *
   * @param[in] name
   */
  virtual
  const MathUserData *userDataPtrConst(const std::string &name) const = 0;
  virtual  MathUserData *userDataPtr(const std::string &name) = 0;


  /**
   * Store inputs into the VolumeData state, transfer ownership of the pointer
   * to the VolumeData Object.
   *
   * @param[in] name  Name of MathUserData that is going in
   * @param[in] v  Pointer to MathUserData that becomes owned by the object
   */
  virtual bool storeMathUserData(const std::string &name, MathUserData *v) = 0;

  /**
   * Perform a 2d or 1d smoothing filter
   */
  virtual bool smooth(MathLoopData *out,
		      std::vector<ProcessingNode *> &args) const = 0;

  /**
   * Perform a 2d or 1d smoothing filter on data first convert to linear,
   * and back to dB when done
   */
  virtual bool smoothDBZ(MathLoopData *out,
			 std::vector<ProcessingNode *> &args) const = 0;

  /**
   * Perform a 2d or 1d standard deviation filter
   */
  virtual bool stddev(MathLoopData *out,
		      std::vector<ProcessingNode *> &args) const = 0;

  /**
   * Perform a 2d or 1d fuzzy remapping
   */
  virtual bool fuzzy(MathLoopData *out,
		      std::vector<ProcessingNode *> &args) const = 0;

  /**
   * Perform a 2d or 1d averaging of multiple input data
   */
  virtual bool average(MathLoopData *out,
		       std::vector<ProcessingNode *> &args) const = 0;

  /**
   * Perform a 2d or 1d maximum of multiple input data
   */
  virtual bool max(MathLoopData *out,
		   std::vector<ProcessingNode *> &args) const = 0;

  virtual bool max_expand(MathLoopData *out,
			  std::vector<ProcessingNode *> &args) const = 0;

  virtual bool weighted_average(MathLoopData *out,
				std::vector<ProcessingNode *> &args) const = 0;

  virtual bool mask(MathLoopData *out,
		    std::vector<ProcessingNode *> &args) const = 0;
  virtual bool
  mask_missing_to_missing(MathLoopData *out,
			  std::vector<ProcessingNode *> &args) const = 0;

  virtual bool trapezoid(MathLoopData *out,
			 std::vector<ProcessingNode *> &args) const = 0;

  virtual bool s_remap(MathLoopData *out,
		       std::vector<ProcessingNode *> &args) const = 0;

  virtual bool median(MathLoopData *out,
		      std::vector<ProcessingNode *> &args) const = 0;

  virtual bool weighted_angle_average(MathLoopData *out,
		      std::vector<ProcessingNode *> &args) const = 0;

  virtual bool expand_angles_laterally(MathLoopData *out, 
		      std::vector<ProcessingNode *> &args) const = 0;

  virtual bool clump(MathLoopData *out, 
		     std::vector<ProcessingNode *> &args) const = 0;
/**
   * 
   * Process at a node assumed to be a user defined operation
   *
   * @return true if successful and local state modified appropriately
   */
  virtual bool processUserLoopFunction(ProcessingNode &p) = 0;


  /**
   * Process at a node assumed to be a Unary node (user defined)
   *
   * @return MathUserData pointer created by the unary function,
   *         which is newly created and owned by the calling routine,
   *         or return NULL
   *
   * @param[in] p  The unary node
   */
  virtual MathUserData *
  processUserLoopFunctionToUserData(const UnaryNode &p) = 0;

  /**
   * Synchronize inputs for a user defined operation.
   *
   * Each user defined operation has it's own particular result
   * which is handled by the MathData derived class
   *
   * @param[in] userKey  Keyword for the user operation, which the derived class
   *                     should recognize
   * @param[in] names  Input data names
   *
   * @return true if successful
   *
   * This method is to allow each app to do whatever it needs to do to set up
   * the user defined operations.  It is in addition to the method
   * synchInputsAndOutputs to do whatever else is needed.
   */
  virtual bool
  synchUserDefinedInputs(const std::string &userKey,
			 const std::vector<std::string> &names) =0;

  /**
   * @return the user defined unary operators as pairs with
   * first=name, second=description
   */
  virtual  std::vector<FunctionDef> userUnaryOperators(void) const = 0;


#else
#ifdef FILTALG_BASE

  /**
   * @return number of data points in the local object
   */
  virtual int numData(void) const;

  /**
    * Actions to take when one VolumeData loop action is complete.
    *
    * @param[in] index  Index into the VolumeData for looping
    *
    * Called by the object that was created in initializeProcessingNode()
   */
  virtual void finishProcessingNode(int index, VolumeData *vol);

  /**
   * @return pointer to the MathLoopData associated with a name
   *         or NULL for none.  The pointer is owned by this object, should
   *         not be freed by calling routine
   *
   * @param[in] name
   */
  virtual MathLoopData *dataPtr(const std::string &name);

  /**
   * @return pointer to the MathLoopData associated with a name
   *         or NULL for none.  The pointer is owned by this object, should
   *         not be freed by calling routine
   *
   * @param[in] name
   */
  virtual const MathLoopData *dataPtrConst(const std::string &name) const;

  /**
   * @return pointer to the MathUserData associated with a name
   *         or NULL for none.  The pointer is owned by this object, should
   *         not be freed by calling routine
   *
   * @param[in] name
   */
  virtual
  const MathUserData *userDataPtrConst(const std::string &name) const;
  virtual  MathUserData *userDataPtr(const std::string &name);

  /**
   * Store inputs into the VolumeData state, transfer ownership of the pointer
   * to the VolumeData Object.
   *
   * @param[in] name  Name of MathUserData that is going in
   * @param[in] v  Pointer to MathUserData that becomes owned by the object
   */
  virtual bool storeMathUserData(const std::string &name, MathUserData *v);

  /**
   * Perform a 2d or 1d smoothing filter
   */
  virtual bool smooth(MathLoopData *out,
		      std::vector<ProcessingNode *> &args) const;

  /**
   * Perform a 2d or 1d smoothing filter on data first convert to linear,
   * and back to dB when done
   */
  virtual bool smoothDBZ(MathLoopData *out,
			 std::vector<ProcessingNode *> &args) const;

  /**
   * Perform a 2d or 1d standard deviation filter
   */
  virtual bool stddev(MathLoopData *out,
		      std::vector<ProcessingNode *> &args) const;

  /**
   * Perform a 2d or 1d fuzzy remapping
   */
  virtual bool fuzzy(MathLoopData *out,
		      std::vector<ProcessingNode *> &args) const;

  /**
   * Perform a 2d or 1d averaging of multiple input data
   */
  virtual bool average(MathLoopData *out,
		       std::vector<ProcessingNode *> &args) const;

  /**
   * Perform a 2d or 1d maximum of multiple input data
   */
  virtual bool max(MathLoopData *out,
		   std::vector<ProcessingNode *> &args) const;

  virtual bool max_expand(MathLoopData *out,
			 std::vector<ProcessingNode *> &args) const;

  virtual bool weighted_average(MathLoopData *out,
		       std::vector<ProcessingNode *> &args) const;

  virtual bool mask(MathLoopData *out,
		    std::vector<ProcessingNode *> &args) const;
  virtual bool
  mask_missing_to_missing(MathLoopData *out,
			  std::vector<ProcessingNode *> &args) const;
  virtual bool trapezoid(MathLoopData *out,
			 std::vector<ProcessingNode *> &args) const;

  virtual bool s_remap(MathLoopData *out,
		       std::vector<ProcessingNode *> &args) const;

  virtual bool median(MathLoopData *out,
		      std::vector<ProcessingNode *> &args) const;

  virtual bool weighted_angle_average(MathLoopData *out,
		      std::vector<ProcessingNode *> &args) const;

  virtual bool expand_angles_laterally(MathLoopData *out, 
		      std::vector<ProcessingNode *> &args) const;

  virtual bool clump(MathLoopData *out, 
		     std::vector<ProcessingNode *> &args) const;

  /**
   * 
   * Process at a node assumed to be a user defined operation
   *
   * @return true if successful and local state modified appropriately
   */
  virtual bool processUserLoopFunction(ProcessingNode &p) = 0;


  /**
   * Process at a node assumed to be a Unary node (user defined)
   *
   * @return MathUserData pointer created by the unary function,
   *         which is newly created and owned by the calling routine,
   *         or return NULL
   *
   * @param[in] p  The unary node
   */
  virtual MathUserData *
  processUserLoopFunctionToUserData(const UnaryNode &p) = 0;

  /**
   * Called within the loop to set up input data, and output data
   *
   * @param[in] output  Output data name
   * @param[in] inputs  Input data names
   *
   * @return true if successful synching
   */
  virtual bool synchInputsAndOutputs(const std::string &output,
				     const std::vector<std::string> &inputs)=0;

  /**
   * Synchronize inputs for a user defined operation.
   *
   * Each user defined operation has it's own particular result
   * which is handled by the MathData derived class
   *
   * @param[in] userKey  Keyword for the user operation, which the derived class
   *                     should recognize
   * @param[in] names  Input data names
   *
   * @return true if successful
   *
   * This method is to allow each app to do whatever it needs to do to set up
   * the user defined operations.  It is in addition to the method
   * synchInputsAndOutputs to do whatever else is needed.
   */
  virtual bool synchUserDefinedInputs(const std::string &userKey,
				      const std::vector<std::string> &names) =0;
  /**
   * @return the user defined unary operators as pairs with
   * first=name, second=description
   */
  virtual std::vector<FunctionDef> userUnaryOperators(void) const = 0;

#else
#ifdef FILTALG_DERIVED

  /**
   * 
   * Process at a node assumed to be a user defined operation
   *
   * @return true if successful and local state modified appropriately
   */
  virtual bool processUserLoopFunction(ProcessingNode &p);


  /**
   * Called within the loop to set up input data, and output data
   *
   * @param[in] output  Output data name
   * @param[in] inputs  Input data names
   *
   * @return true if successful synching
   */
  virtual bool synchInputsAndOutputs(const std::string &output,
				     const std::vector<std::string> &inputs);

  /**
   * Process at a node assumed to be a Unary node (user defined)
   *
   * @return MathUserData pointer created by the unary function,
   *         which is newly created and owned by the calling routine,
   *         or return NULL
   *
   * @param[in] p  The unary node
   */

  virtual MathUserData *processUserLoopFunctionToUserData(const UnaryNode &p);

  /**
   * Synchronize inputs for a user defined operation.
   *
   * Each user defined operation has it's own particular result
   * which is handled by the MathData derived class
   *
   * @param[in] userKey  Keyword for the user operation, which the derived class
   *                     should recognize
   * @param[in] names  Input data names
   *
   * @return true if successful
   *
   * This method is to allow each app to do whatever it needs to do to set up
   * the user defined operations.  It is in addition to the method
   * synchInputsAndOutputs to do whatever else is needed.
   */
  virtual bool synchUserDefinedInputs(const std::string &userKey,
				      const std::vector<std::string> &names);
  /**
   * @return the user defined unary operators as pairs with
   * first=name, second=description
   */
  virtual std::vector<FunctionDef> userUnaryOperators(void) const;


#else
#ifdef RADX_BASE

  /**
   * @return number of data points in the local object
   */
  virtual int numData(void) const;

  /**
    * Actions to take when one VolumeData loop action is complete.
    *
    * @param[in] index  Index into the VolumeData for looping
    *
    * Called by the object that was created in initializeProcessingNode()
   */
  virtual void finishProcessingNode(int index, VolumeData *vol);

  /**
   * Called within the loop to set up input data, and output data
   *
   * @param[in] output  Output data name
   * @param[in] inputs  Input data names
   *
   * @return true if successful synching
   */
  virtual bool synchInputsAndOutputs(const std::string &output,
				     const std::vector<std::string> &inputs);
  /**
   * @return pointer to the MathLoopData associated with a name
   *         or NULL for none.  The pointer is owned by this object, should
   *         not be freed by calling routine
   *
   * @param[in] name
   */
  virtual MathLoopData *dataPtr(const std::string &name);

  /**
   * @return pointer to the MathLoopData associated with a name
   *         or NULL for none.  The pointer is owned by this object, should
   *         not be freed by calling routine
   *
   * @param[in] name
   */
  virtual const MathLoopData *dataPtrConst(const std::string &name) const;


  /**
   * @return pointer to the MathUserData associated with a name
   *         or NULL for none.  The pointer is owned by this object, should
   *         not be freed by calling routine
   *
   * @param[in] name
   */
  virtual
  const MathUserData *userDataPtrConst(const std::string &name) const;
  virtual  MathUserData *userDataPtr(const std::string &name);

  /**
   * Store inputs into the VolumeData state, transfer ownership of the pointer
   * to the VolumeData Object.
   *
   * @param[in] name  Name of MathUserData that is going in
   * @param[in] v  Pointer to MathUserData that becomes owned by the object
   */
  virtual bool storeMathUserData(const std::string &name, MathUserData *v);

  /**
   * Process at a node assumed to be a Unary node (user defined)
   *
   * @return MathUserData pointer created by the unary function,
   *         which is newly created and owned by the calling routine,
   *         or return NULL
   *
   * @param[in] p  The unary node
   */
  virtual MathUserData *processUserLoopFunctionToUserData(const UnaryNode &p);

  /**
   * Synchronize inputs for a user defined operation.
   *
   * Each user defined operation has it's own particular result
   * which is handled by the MathData derived class
   *
   * @param[in] userKey  Keyword for the user operation, which the derived class
   *                     should recognize
   * @param[in] names  Input data names
   *
   * @return true if successful
   *
   * This method is to allow each app to do whatever it needs to do to set up
   * the user defined operations.  It is in addition to the method
   * synchInputsAndOutputs to do whatever else is needed.
   */
  virtual bool synchUserDefinedInputs(const std::string &userKey,
				      const std::vector<std::string> &names);

  /**
   * Perform a 2d or 1d smoothing filter
   */
  virtual bool smooth(MathLoopData *out,
		      std::vector<ProcessingNode *> &args) const ;

  /**
   * Perform a 2d or 1d smoothing filter on data first convert to linear,
   * and back to dB when done
   */
  virtual bool smoothDBZ(MathLoopData *out,
			 std::vector<ProcessingNode *> &args) const ;

  /**
   * Perform a 2d or 1d standard deviation filter
   */
  virtual bool stddev(MathLoopData *out,
		      std::vector<ProcessingNode *> &args) const ;

  /**
   * Perform a 2d or 1d fuzzy remapping
   */
  virtual bool fuzzy(MathLoopData *out,
		      std::vector<ProcessingNode *> &args) const ;

  /**
   * Perform a 2d or 1d averaging of multiple input data
   */
  virtual bool average(MathLoopData *out,
		       std::vector<ProcessingNode *> &args) const ;

  /**
   * Perform a 2d or 1d maximum of multiple input data
   */
  virtual bool max(MathLoopData *out,
		   std::vector<ProcessingNode *> &args) const ;

  virtual bool max_expand(MathLoopData *out,
			 std::vector<ProcessingNode *> &args) const ;

  virtual bool weighted_average(MathLoopData *out,
		       std::vector<ProcessingNode *> &args) const ;

  virtual bool mask(MathLoopData *out,
		    std::vector<ProcessingNode *> &args) const ;
  virtual bool
  mask_missing_to_missing(MathLoopData *out,
			  std::vector<ProcessingNode *> &args) const ;
  virtual bool trapezoid(MathLoopData *out,
			 std::vector<ProcessingNode *> &args) const ;

  virtual bool s_remap(MathLoopData *out,
		       std::vector<ProcessingNode *> &args) const ;

  virtual bool median(MathLoopData *out,
		      std::vector<ProcessingNode *> &args) const ;

  virtual bool weighted_angle_average(MathLoopData *out,
		      std::vector<ProcessingNode *> &args) const ;

  virtual bool expand_angles_laterally(MathLoopData *out, 
		      std::vector<ProcessingNode *> &args) const ;

  virtual bool clump(MathLoopData *out, 
		     std::vector<ProcessingNode *> &args) const ;
  /**
   * 
   * Process at a node assumed to be a user defined operation
   *
   * @return true if successful and local state modified appropriately
   */
  virtual bool processUserLoopFunction(ProcessingNode &p) = 0;

  /**
   * @return the user defined unary operators as pairs with
   * first=name, second=description
   */
  virtual
  std::vector<FunctionDef> userUnaryOperators(void) const = 0;


#else
#ifdef RADX_DERIVED

  /**
   * 
   * Process at a node assumed to be a user defined operation
   *
   * @return true if successful and local state modified appropriately
   */
  virtual bool processUserLoopFunction(ProcessingNode &p);

  /**
   * @return the user defined unary operators as pairs with
   * first=name, second=description
   */
  virtual
  std::vector<FunctionDef> userUnaryOperators(void) const;

#else

  /**
   * @return number of data points in the local object
   */
  virtual int numData(void) const;

  /**
    * Actions to take when one VolumeData loop action is complete.
    *
    * @param[in] index  Index into the VolumeData for looping
    *
    * Called by the object that was created in initializeProcessingNode()
   */
  virtual void finishProcessingNode(int index, VolumeData *vol);

  /**
   * Called within the loop to set up input data, and output data
   *
   * @param[in] output  Output data name
   * @param[in] inputs  Input data names
   *
   * @return true if successful synching
   */
  virtual bool synchInputsAndOutputs(const std::string &output,
				     const std::vector<std::string> &inputs);
  /**
   * @return pointer to the MathLoopData associated with a name
   *         or NULL for none.  The pointer is owned by this object, should
   *         not be freed by calling routine
   *
   * @param[in] name
   */
  virtual MathLoopData *dataPtr(const std::string &name);

  /**
   * @return pointer to the MathLoopData associated with a name
   *         or NULL for none.  The pointer is owned by this object, should
   *         not be freed by calling routine
   *
   * @param[in] name
   */
  virtual const MathLoopData *dataPtrConst(const std::string &name) const;


  /**
   * @return pointer to the MathUserData associated with a name
   *         or NULL for none.  The pointer is owned by this object, should
   *         not be freed by calling routine
   *
   * @param[in] name
   */
  virtual
  const MathUserData *userDataPtrConst(const std::string &name) const;
  virtual  MathUserData *userDataPtr(const std::string &name);

  /**
   * Store inputs into the VolumeData state, transfer ownership of the pointer
   * to the VolumeData Object.
   *
   * @param[in] name  Name of MathUserData that is going in
   * @param[in] v  Pointer to MathUserData that becomes owned by the object
   */
  virtual bool storeMathUserData(const std::string &name, MathUserData *v);

  /**
   * Perform a 2d or 1d smoothing filter
   */
  virtual bool smooth(MathLoopData *out,
		      std::vector<ProcessingNode *> &args) const;

  /**
   * Perform a 2d or 1d smoothing filter on data first convert to linear,
   * and back to dB when done
   */
  virtual bool smoothDBZ(MathLoopData *out,
			 std::vector<ProcessingNode *> &args) const;

  /**
   * Perform a 2d or 1d standard deviation filter
   */
  virtual bool stddev(MathLoopData *out,
		      std::vector<ProcessingNode *> &args) const;

  /**
   * Perform a 2d or 1d fuzzy remapping
   */
  virtual bool fuzzy(MathLoopData *out,
		      std::vector<ProcessingNode *> &args) const;

  /**
   * Perform a 2d or 1d averaging of multiple input data
   */
  virtual bool average(MathLoopData *out,
		       std::vector<ProcessingNode *> &args) const;

  /**
   * Perform a 2d or 1d maximum of multiple input data
   */
  virtual bool max(MathLoopData *out,
		   std::vector<ProcessingNode *> &args) const;

  virtual bool max_expand(MathLoopData *out,
			 std::vector<ProcessingNode *> &args) const;

  virtual bool weighted_average(MathLoopData *out,
		       std::vector<ProcessingNode *> &args) const;

  virtual bool mask(MathLoopData *out,
		    std::vector<ProcessingNode *> &args) const;
  virtual bool
  mask_missing_to_missing(MathLoopData *out,
			  std::vector<ProcessingNode *> &args) const;
  virtual bool trapezoid(MathLoopData *out,
			 std::vector<ProcessingNode *> &args) const;

  virtual bool s_remap(MathLoopData *out,
		       std::vector<ProcessingNode *> &args) const;

  virtual bool median(MathLoopData *out,
		      std::vector<ProcessingNode *> &args) const;

  virtual bool weighted_angle_average(MathLoopData *out,
		      std::vector<ProcessingNode *> &args) const;

  virtual bool expand_angles_laterally(MathLoopData *out, 
		      std::vector<ProcessingNode *> &args) const;

  virtual bool clump(MathLoopData *out, 
		     std::vector<ProcessingNode *> &args) const;
  /**
   * 
   * Process at a node assumed to be a user defined operation
   *
   * @return true if successful and local state modified appropriately
   */
  virtual bool processUserLoopFunction(ProcessingNode &p);


  /**
   * Process at a node assumed to be a Unary node (user defined)
   *
   * @return MathUserData pointer created by the unary function,
   *         which is newly created and owned by the calling routine,
   *         or return NULL
   *
   * @param[in] p  The unary node
   */
  virtual MathUserData *processUserLoopFunctionToUserData(const UnaryNode &p);

  /**
   * Synchronize inputs for a user defined operation.
   *
   * Each user defined operation has it's own particular result
   * which is handled by the MathData derived class
   *
   * @param[in] userKey  Keyword for the user operation, which the derived class
   *                     should recognize
   * @param[in] names  Input data names
   *
   * @return true if successful
   *
   * This method is to allow each app to do whatever it needs to do to set up
   * the user defined operations.  It is in addition to the method
   * synchInputsAndOutputs to do whatever else is needed.
   */
  virtual bool synchUserDefinedInputs(const std::string &userKey,
				      const std::vector<std::string> &names);

  /**
   * @return the user defined unary operators as pairs with
   * first=name, second=description
   */
  virtual
  std::vector<FunctionDef> userUnaryOperators(void) const;


#endif
#endif
#endif
#endif
#endif
