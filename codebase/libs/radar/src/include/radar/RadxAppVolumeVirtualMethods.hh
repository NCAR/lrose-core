#ifdef FILTALG_BASE

  /**
   * Pure virtual, each user operator may or may not need to synchronize
   * its input arguments.  
   * @return true if this operator needs to synch
   * @param[in] userKey The operator
   */
  virtual bool needToSynch(const std::string &userKey) const = 0;

  /**
   * Pure virtual
   * @return true if input operator has input named input
   * @param[in] userKey  the operator
   * @param[in] name  Input name to check
   * @param[in] suppressWarn   True to now warn if failure
   */
  virtual bool hasData(const std::string &userKey, const std::string &name,
		       bool suppressWarn) = 0;

#else

  /**
   * Each user operator may or may not need to synchronize
   * its input arguments.  
   * @return true if this operator needs to synch
   * @param[in] userKey The operator
   */
  virtual bool needToSynch(const std::string &userKey) const;

  /**
   * @return true if input operator has input named input
   * @param[in] userKey  the operator
   * @param[in] name  Input name to check
   * @param[in] suppressWarn   True to now warn if failure
   */
  virtual bool hasData(const std::string &userKey, const std::string &name,
		       bool suppressWarn);

#endif
