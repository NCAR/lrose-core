#ifdef MATH_USER_DATA_BASE

  /**
   * @return true if able to return a floating value 
   *
   * @param[out] v   Value
   */
  virtual bool getFloat(double &v) const = 0;

#else

  /**
   * @return true if able to return a floating value 
   *
   * @param[out] v   Value
   */
  virtual bool getFloat(double &v) const;

#endif
