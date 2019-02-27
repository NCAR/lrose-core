#ifdef NODE_BASE

  /**
   * Debug print the 'parsed' node content
   */
  virtual void printParsed(void) const = 0;

  /**
   * Debug print the 'parsed' node content with a \n
   */
  virtual void printParsedCr(void) const = 0;

  /**
   * Do the action at this node, processing loop
   * @param[in,out] data
   * @return true for success
   */
  virtual bool process(MathData *data) const = 0;

  /**
   * Do the action at this node, processing loop
   * @param[in,out] data
   * @return pointer to user data
   */
  virtual MathUserData *processToUserDefined(MathData *data) const = 0;

  /**
   * Do the action at this node, entire volume
   * @param[in,out] data
   * @return pointer to user data
   */
  virtual MathUserData *processVol(VolumeData *data) const = 0;

  /**
   * Do the computation at a node in a loop
   * @param[in] data
   * @param[in] ind Index into data
   * @param[out] v  Value returned
   * @return true if value computed and stored to v
   */
  virtual bool compute(const MathData *data, int ind, double &v) const=0;

  /**
   * Append the output field names to the vector
   *
   * @param[in,out] names  Vector appended to
   */
  virtual void outputFields(std::vector<std::string> &names) const=0;

  /**
   * Append the input field names to the vector
   *
   * @param[in,out] names  Vector appended to
   */
  virtual void inputFields(std::vector<std::string> &names) const=0;

  /**
   * Free pointers and clean up state, done only when completely finished,
   * not part of destructor
   */
  virtual void cleanup(void) = 0;

#else

  /**
   * Debug print the 'parsed' node content
   */
  virtual void printParsed(void) const;

  /**
   * Debug print the 'parsed' node content with a \n
   */
  virtual void printParsedCr(void) const;

  /**
   * Do the action at this node, processing loop
   * @param[in,out] data
   * @return true for success
   */
  virtual bool process(MathData *data) const;

  /**
   * Do the action at this node, processing loop, result is user defined data
   * @param[in,out] data
   * @return pointer to user data
   */
  virtual MathUserData *processToUserDefined(MathData *data) const;

  /**
   * Do the action at this node, entire volume
   * @param[in,out] data
   * @return pointer to user data
   */
  virtual MathUserData *processVol(VolumeData *data) const;

  /**
   * Do the computation at a node in a loop
   * @param[in] data
   * @param[in] ind Index into data
   * @param[out] v  Value returned
   * @return true if value computed and stored to v
   */
  virtual bool compute(const MathData *data,
		       int ind, double &v) const;

  /**
   * Append the output field names to the vector
   *
   * @param[in,out] names  Vector appended to
   */
  virtual void outputFields(std::vector<std::string> &names) const;

  /**
   * Append the input field names to the vector
   *
   * @param[in,out] names  Vector appended to
   */
  virtual void inputFields(std::vector<std::string> &names) const;

  /**
   * Free pointers and clean up state, done only when completely finished,
   * not part of destructor
   */
  virtual void cleanup(void);

#endif
