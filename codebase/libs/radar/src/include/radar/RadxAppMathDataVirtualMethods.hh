#ifdef RADXAPP_MATH_DATA_BASE
  /**
   * For a particular user operator, process and produce output stored to
   * this object
   * @return true if successful
   * @param[in] keyword  Operator
   * @param[in] p  Processing node object
   */
  virtual bool radxappUserLoopFunction(const std::string &keyword,
				       ProcessingNode &p) = 0;

  /**
   * For a particular unary operation, process and produce output returned
   * as a pointer to MathUserData
   * @return Pointer if successful, NULL otherwise
   * @param[in] p  The unary node
   */
  virtual MathUserData *
  radxappUserLoopFunctionToUserData(const UnaryNode &p) = 0;

  /**
   * Pure virtual
   * Append the derived class' unary operator descriptions to the vector
   * @param[in,out] ops  Vector to append to
   */
  virtual void
  radxappAppendUnaryOperators(std::vector<FunctionDef> &ops) const = 0;
#else
  /**
   * For a particular user operator, process and produce output stored to
   * this object
   * @return true if successful
   * @param[in] keyword  Operator
   * @param[in] p  Processing node object
   */
  virtual bool radxappUserLoopFunction(const std::string &keyword,
				       ProcessingNode &p);

  /**
   * For a particular unary operation, process and produce output returned
   * as a pointer to MathUserData
   * @return Pointer if successful, NULL otherwise
   * @param[in] p  The unary node
   */
  virtual MathUserData *
  radxappUserLoopFunctionToUserData(const UnaryNode &p);

  /**
   * Pure virtual
   * Append the derived class' unary operator descriptions to the vector
   * @param[in,out] ops  Vector to append to
   */
  virtual void
  radxappAppendUnaryOperators(std::vector<FunctionDef> &ops) const;
#endif
