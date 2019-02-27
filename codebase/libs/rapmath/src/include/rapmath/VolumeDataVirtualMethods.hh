#ifdef VOLUME_DATA_BASE

  /**
   * @return number of processing nodes within this object for either
   * 2d or 1d looping
   *
   *
   * @param[in] twoD  true for 2d data loop, false for 1d data loop.
   *
   */
  virtual int numProcessingNodes(bool twoD) const = 0;

  /**
   * Store inputs into the VolumeData state, transfer ownership of the pointer
   * to the VolumeData Object.
   *
   * @param[in] name  Name of MathUserData that is going in
   * @param[in] v  Pointer to MathUserData that becomes owned by the object
   */
  virtual bool storeMathUserData(const std::string &name, MathUserData *v)= 0;


  /**
   * Initialization when starting action within a volume loop.
   * Depending on the volume content this can be within a loop over 2d
   * slices, or a loop over 1d slices 
   *
   * @param[in] index  Index into the data for looping
   * @param[in] twoD  true for 2d data loop, false for 1d data loop.
   *
   * @return newly create MathData pointer for this loop action,
   *         which is owned by the caller and freed by the caller
   */
  virtual MathData * initializeProcessingNode(int index, bool twoD) const = 0;


  /**
   * Synchronize inputs for a user defined operation.
   * Each user defined operation has it's own particular action,
   * which is handled by the VolumeData derived class
   *
   * @param[in] userKey  Keyword for the user operation, which the derived class
   *                     should recognize
   * @param[in] names  Input data names
   *
   * @return true if successful
   */
  virtual bool
  synchUserDefinedInputs(const std::string &userKey,
  			 const std::vector<std::string> &names) = 0;


  /**
   * Process at a node assumed to be a Unary node (user defined)
   *
   * @return MathUserData pointer created by the unary function,
   *         which is newly created and owned by the calling routine,
   *         or return NULL
   *
   * @param[in] p  The unary node
   */
  virtual MathUserData *processUserVolumeFunction(const UnaryNode &p) = 0;

  /**
   * Add MathData from a height to the volume
   * @param[in] zIndex  Index to height
   * @param[in] s  Sweep MathData
   */
  virtual void addNew(int zIndex, const MathData *s) = 0;


  /**
   * @return the user defined unary operators as pairs with
   * first=name, second=description
   */
  virtual std::vector<FunctionDef> userUnaryOperators(void) const = 0;

#else
#ifdef FILTALG_BASE

  /**
   * @return number of processing nodes within this object for either
   * 2d or 1d looping
   *
   *
   * @param[in] twoD  true for 2d data loop, false for 1d data loop.
   *
   */
  virtual int numProcessingNodes(bool twoD) const;

  /**
   * Store inputs into the VolumeData state, transfer ownership of the pointer
   * to the VolumeData Object.
   *
   * @param[in] name  Name of MathUserData that is going in
   * @param[in] v  Pointer to MathUserData that becomes owned by the object
   */
  virtual bool storeMathUserData(const std::string &name, MathUserData *v) = 0;


  /**
   * Initialization when starting action within a volume loop.
   * Depending on the volume content this can be within a loop over 2d
   * slices, or a loop over 1d slices 
   *
   * @param[in] index  Index into the data for looping
   * @param[in] twoD  true for 2d data loop, false for 1d data loop.
   *
   * @return newly create MathData pointer for this loop action,
   *         which is owned by the caller and freed by the caller
   */
  virtual MathData * initializeProcessingNode(int index, bool twoD) const = 0;


  /**
   * Synchronize inputs for a user defined operation.
   * Each user defined operation has it's own particular action,
   * which is handled by the VolumeData derived class
   *
   * @param[in] userKey  Keyword for the user operation, which the derived class
   *                     should recognize
   * @param[in] names  Input data names
   *
   * @return true if successful
   */
  virtual bool
  synchUserDefinedInputs(const std::string &userKey,
  			 const std::vector<std::string> &names);


  /**
   * Process at a node assumed to be a Unary node (user defined)
   *
   * @return MathUserData pointer created by the unary function,
   *         which is newly created and owned by the calling routine,
   *         or return NULL
   *
   * @param[in] p  The unary node
   */
  virtual MathUserData *processUserVolumeFunction(const UnaryNode &p) = 0;

  /**
   * Add MathData from a height to the volume
   * @param[in] zIndex  Index to height
   * @param[in] s  Sweep MathData
   */
  virtual void addNew(int zIndex, const MathData *s) = 0;


  /**
   * @return the user defined unary operators as pairs with
   * first=name, second=description
   */
  virtual std::vector<FunctionDef> userUnaryOperators(void) const = 0;

#else
#ifdef FILTALG_DERIVED

  /**
   * Store inputs into the VolumeData state, transfer ownership of the pointer
   * to the VolumeData Object.
   *
   * @param[in] name  Name of MathUserData that is going in
   * @param[in] v  Pointer to MathUserData that becomes owned by the object
   */
  virtual bool storeMathUserData(const std::string &name, MathUserData *v);

  /**
   * Initialization when starting action within a volume loop.
   * Depending on the volume content this can be within a loop over 2d
   * slices, or a loop over 1d slices 
   *
   * @param[in] index  Index into the data for looping
   * @param[in] twoD  true for 2d data loop, false for 1d data loop.
   *
   * @return newly create MathData pointer for this loop action,
   *         which is owned by the caller and freed by the caller
   */
  virtual MathData * initializeProcessingNode(int index, bool twoD) const;


  /**
   * Process at a node assumed to be a Unary node (user defined)
   *
   * @return MathUserData pointer created by the unary function,
   *         which is newly created and owned by the calling routine,
   *         or return NULL
   *
   * @param[in] p  The unary node
   */
  virtual MathUserData *processUserVolumeFunction(const UnaryNode &p);

  /**
   * Add MathData from a height to the volume
   * @param[in] zIndex  Index to height
   * @param[in] s  Sweep MathData
   */
  virtual void addNew(int zIndex, const MathData *s);

  /**
   * @return the user defined unary operators as pairs with
   * first=name, second=description
   */
  virtual std::vector<FunctionDef> userUnaryOperators(void) const;

#else

  /**
   * @return number of processing nodes within this object for either
   * 2d or 1d looping
   *
   *
   * @param[in] twoD  true for 2d data loop, false for 1d data loop.
   *
   */
  virtual int numProcessingNodes(bool twoD) const;

  /**
   * Store inputs into the VolumeData state, transfer ownership of the pointer
   * to the VolumeData Object.
   *
   * @param[in] name  Name of MathUserData that is going in
   * @param[in] v  Pointer to MathUserData that becomes owned by the object
   */
  virtual bool storeMathUserData(const std::string &name, MathUserData *v);

  /**
   * Initialization when starting action within a volume loop.
   * Depending on the volume content this can be within a loop over 2d
   * slices, or a loop over 1d slices 
   *
   * @param[in] index  Index into the data for looping
   * @param[in] twoD  true for 2d data loop, false for 1d data loop.
   *
   * @return newly create MathData pointer for this loop action,
   *         which is owned by the caller and freed by the caller
   */
  virtual MathData * initializeProcessingNode(int index, bool twoD) const;


  /**
   * Synchronize inputs for a user defined operation.
   * Each user defined operation has it's own particular action,
   * which is handled by the VolumeData derived class
   *
   * @param[in] userKey  Keyword for the user operation, which the derived class
   *                     should recognize
   * @param[in] names  Input data names
   *
   * @return true if successful
   */
  virtual bool
  synchUserDefinedInputs(const std::string &userKey,
  			 const std::vector<std::string> &names);


  /**
   * Process at a node assumed to be a Unary node (user defined)
   *
   * @return MathUserData pointer created by the unary function,
   *         which is newly created and owned by the calling routine,
   *         or return NULL
   *
   * @param[in] p  The unary node
   */
  virtual MathUserData *processUserVolumeFunction(const UnaryNode &p);

  /**
   * Add MathData from a height to the volume
   * @param[in] zIndex  Index to height
   * @param[in] s  Sweep MathData
   */
  virtual void addNew(int zIndex, const MathData *s);

  /**
   * @return the user defined unary operators as pairs with
   * first=name, second=description
   */
  virtual std::vector<FunctionDef> userUnaryOperators(void) const;

#endif
#endif
#endif
