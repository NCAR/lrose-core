#ifdef FILTALG_BASE
  /**
   * Pure virtual, Synchronize inputs for a user defined operation.
   * Each user defined operation has it's own particular action,
   *
   * @param[in] userKey  Keyword for the user operation, which the class
   *                     should recognize
   * @param[in] names  Input data names
   *
   * @return true if successful
   */
  virtual bool
  virtVolSynchUserInputs(const std::string &userKey,
			 const std::vector<std::string> &names) = 0;
#else
  /**
   * Pure virtual, Synchronize inputs for a user defined operation.
   * Each user defined operation has it's own particular action,
   *
   * @param[in] userKey  Keyword for the user operation, which the class
   *                     should recognize
   * @param[in] names  Input data names
   *
   * @return true if successful
   */
  virtual bool 
  virtVolSynchUserInputs(const std::string &userKey,
			 const std::vector<std::string> &names);

#endif

