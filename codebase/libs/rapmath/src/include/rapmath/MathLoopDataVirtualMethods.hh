#ifdef MATH_LOOP_DATA_BASE

  /**
   * @return number of data points
   */
  virtual int numData(void) const = 0;

  /**
   * @return true if input name matches input
   * @param[in] n Name
   */
  virtual bool nameMatch(const std::string &n) const = 0;

  /**
   * Get data value at index
   *
   * @param[in] ipt   Index
   * @param[out] v  Value
   *
   * @return true if successful
   */
  virtual bool getVal(int ipt, double &v) const = 0;

  /**
   * Set value at index to input value
   *
   * @param[in] ipt   Index
   * @param[in] v  Value
   */
  virtual void setVal(int ipt, double v) = 0;

  /**
   * Set value at index to missing
   *
   * @param[in] ipt   Index
   */
  virtual void setMissing(int ipt) = 0;

  /**
   * @return the missing value
   */
  virtual double getMissingValue(void) const = 0;

  /**
   * Return a cloned pointer of the object downcast to the base class
   */
  virtual MathLoopData *clone(void) const = 0;

  /**
   * Debug print
   */
  virtual void print(void) const = 0;

  /**
   * Set all data points to a single value
   * @param[in] v  The value
   */
  virtual void setAllToValue(double v) = 0;

#else

  /**
   * @return number of data points
   */
  virtual int numData(void) const;

  /**
   * @return true if input name matches input
   * @param[in] n Name
   */
  virtual bool nameMatch(const std::string &n) const;

  /**
   * Get data value at index
   *
   * @param[in] ipt   Index
   * @param[out] v  Value
   *
   * @return true if successful
   */
  virtual bool getVal(int ipt, double &v) const;

  /**
   * Set value at index to input value
   *
   * @param[in] ipt   Index
   * @param[in] v  Value
   */
  virtual void setVal(int ipt, double v);

  /**
   * Set value at index to missing
   *
   * @param[in] ipt   Index
   */
  virtual void setMissing(int ipt);

  /**
   * @return the missing value
   */
  virtual double getMissingValue(void) const;

  /**
   * Return a cloned pointer of the object downcast to the base class
   */
  virtual MathLoopData *clone(void) const;

  /**
   * Debug print
   */
  virtual void print(void) const;

  /**
   * Set all data points to a single value
   * @param[in] v  The value
   */
  virtual void setAllToValue(double v);

#endif
