/**
 * @file MathData.hh 
 * @brief Container for the data within a VolumeData loop for your app
 * 
 * @class MathData
 * @brief Container for the data within a VolumeData loop for your app
 *
 * A pure virtual base class
 */

#ifndef MATH_DATA_H
#define MATH_DATA_H

class ProcessingNode;
class MathLoopData;
class MathUserData;
class UnaryNode;
class VolumeData;
class FunctionDef;

#include <vector>
#include <string>

class MathParser;

class MathData
{
public:
  /**
   * Empty constructor
   */
  inline MathData(void) {}

  /**
   * Destructor
   */
  inline virtual ~MathData(void) {}

#define MATH_DATA_BASE
  #include <rapmath/MathDataVirtualMethods.hh>
#undef MATH_DATA_BASE

  /**
   * Expect a single arg which is userData.
   * Extract that content and return.
   *
   * @param[in] args  Unary function argument list
   * @return pointer for success or NULL for failure
   */
  MathUserData *userData(std::vector<ProcessingNode *> &args);

  /**
   * Load index'th arg as MathLoopData by extracting it from the args
   *
   * @param[in] args  Unary function argument list
   * @param[in] index into args
   * @return pointer for success or NULL for failure
   */
  const MathLoopData *loadData(std::vector<ProcessingNode *> &args,
			       int index) const;

  /**
   * Argument list expected to be (data, value, value)
   * Extract that content and return.
   *
   * @param[in] args  Unary function argument list
   * @param[out] field  The data
   * @param[out] v0  The first value
   * @param[out] v1  The second value
   * @return True for success, false for failure
   */
  bool loadDataValueValue(std::vector<ProcessingNode *> &args,
			  const MathLoopData **field, double &v0,
			  double &v1) const;
  /**
   * Argument list expected to be (data, data, ...)
   * Extract that content and return.
   *
   * @param[in] args  Unary function argument list
   * @param[out] fields  The data fields that were in the args list
   * @return True for success, false for failure
   */
  bool loadMultiData(std::vector<ProcessingNode *> &args,
		     std::vector<const MathLoopData *> &fields) const;

  /**
   * Argument list expected to be (data, data)
   * Extract that content and return.
   *
   * @param[in] args  Unary function argument list
   * @param[out] data0  
   * @param[out] data1  
   * @return True for success, false for failure
   */
  bool loadDataData(std::vector<ProcessingNode *> &args,
		    const MathLoopData **data0,
		    const MathLoopData **data1) const;
  /**
   * Argument list expected to be (data, value)
   * Extract that content and return.
   *
   * @param[in] args  Unary function argument list
   * @param[out] field  data
   * @param[out] v  Value
   * @return True for success, false for failure
   */
  bool loadDataValue(std::vector<ProcessingNode *> &args,
		     const MathLoopData **field, double &v) const;
  /**
   * Argument list expected to be (data, v00, v01, v10, v11, v20, v21,...)
   * Extract that content and return.
   *
   * @param[in] args  Unary function argument list
   * @param[out] field  data
   * @param[out] pairs  pairs of double values
   * @return True for success, false for failure
   */
  bool loadDataAndPairs(std::vector<ProcessingNode *> &args,
		       const MathLoopData **field,
		       std::vector<std::pair<double,double> > &pairs) const;
  /**
   * Argument list expected to be (data, data2, v00,v01, v10,v11, v20,v21,...)
   * Extract that content and return.
   *
   * @param[in] args  Unary function argument list
   * @param[out] field  data
   * @param[out] field2  data2
   * @param[out] pairs  pairs of double values
   * @return True for success, false for failure
   */
  bool
  loadDataDataAndPairs(std::vector<ProcessingNode *> &args,
		       const MathLoopData **field,
		       const MathLoopData **field2,
		       std::vector<std::pair<double,double> > &pairs) const;
  /**
   * Argument list expected to be (value, data, data, ...)
   * Extract that content and return.
   *
   * @param[in] args  Unary function argument list
   * @param[out] value  
   * @param[out] fields  data pointers
   * @return True for success, false for failure
   */
  bool loadValueAndMultiData(std::vector<ProcessingNode *> &args, double &value,
			     std::vector<const MathLoopData *> &fields) const;
  /**
   * Argument list expected to be (data, n0, n1, n2, n3)
   * Extract that content and return.
   *
   * @param[in] args  Unary function argument list
   * @param[out] data
   * @param[out] n0
   * @param[out] n1
   * @param[out] n2
   * @param[out] n3
   * @return True for success, false for failure
   */
  bool 
  loadDataAndFourNumbers(std::vector<ProcessingNode *> &args,
			 const MathLoopData **data, double &n0, double &n1,
			 double &n2, double &n3) const;
  /**
   * Argument list expected to be (data, n0, n1, n2)
   * Extract that content and return.
   *
   * @param[in] args  Unary function argument list
   * @param[out] data
   * @param[out] n0
   * @param[out] n1
   * @param[out] n2
   * @return True for success, false for failure
   */
  bool 
  loadDataAndThreeNumbers(std::vector<ProcessingNode *> &args,
			  const MathLoopData **data, double &n0, double &n1,
			  double &n2) const;
  /**
   * Argument list expected to be (data, n0, n1, n2, n3, n4)
   * Extract that content and return.
   *
   * @param[in] args  Unary function argument list
   * @param[out] data
   * @param[out] n0
   * @param[out] n1
   * @param[out] n2
   * @param[out] n3
   * @param[out] n4
   * @return True for success, false for failure
   */
  bool
  loadDataAndFiveNumbers(std::vector<ProcessingNode *> &args,
			 const MathLoopData **data, double &n0, double &n1,
			 double &n2, double &n3, double &n4) const;
  /**
   * Arg list expected to be (number, data, value, data, value, ...)
   * Extract that content and return.
   *
   * @param[in] args  Unary function argument list
   * @param[out] n  Number
   * @param[out] data  The data fields from the arg list
   * @param[out] numbers  The value  fields from the arg list
   * @return True for success, false for failure
   */
  bool 
  loadNumberAndDataNumberPairs(std::vector<ProcessingNode *> &args, double &n,
			       std::vector<const MathLoopData *> &data,
			       std::vector<double> &numbers) const;
  /**
   * Arg list expected to be (data, userData).
   * Extract that content and return.
   *
   * @param[in] args  Unary function argument list
   * @param[out] n  Number
   * @param[out] data  The data field from the arg list
   * @param[out] udata  The user data field from the arg list
   * @return True for success, false for failure
   */
  bool loadDataAndUserData(std::vector<ProcessingNode *> &args,
			   const MathLoopData **data, MathUserData **udata);
  /**
   * Arg list expected to be (data, userData, userData2).
   * Extract that content and return.
   *
   * @param[in] args  Unary function argument list
   * @param[out] n  Number
   * @param[out] data  The data field from the arg list
   * @param[out] udata1  The 1st user data field from the arg list
   * @param[out] udata2  The 2nd user data field from the arg list
   * @return True for success, false for failure
   */
  bool loadDataAndTwoUserDatas(std::vector<ProcessingNode *> &args,
			       const MathLoopData **data,
			       MathUserData **udata1, MathUserData **udata2);
  /**
   * Arg list expected to be (data, userData, value)
   * Extract that content and return.
   *
   * @param[in] args  Unary function argument list
   * @param[out] n  Number
   * @param[out] data  The data field from the arg list
   * @param[out] udata  The 1st user data field from the arg list
   * @param[out] value The value
   * @return True for success, false for failure
   */
  bool loadDataAndUserDataAndValue(std::vector<ProcessingNode *> &args,
				   const MathLoopData **data,
				   MathUserData **udata,  double &value);
  /**
   * Arg list expected to be (name, userData, value)
   * Extract that content and return.
   *
   * @param[in] args  Unary function argument list
   * @param[out] name  string name
   * @param[out] udata  The 1st user data field from the arg list
   * @param[out] value The value
   * @return True for success, false for failure
   */
  bool loadNameAndUserDataAndValue(std::vector<ProcessingNode *> &args,
				   std::string &name,
				   MathUserData **udata,
				   double &value);
  /**
   * Arg list expected to be (data0, data1, value0, value1, .., value_<n>,
   *                          v00, v01, v10, v11, ...)
   * Extract that content and return.
   *
   * @param[in] args  Unary function argument list
   * @param[in] numValues  Expected length of value list
   * @param[out] field The first data field from the arg list
   * @param[out] field2 The second data field from the arg list
   * @param[out] values  The values, should be numValues in length
   * @param[out] pairs  remaining pairs of doubles
   * @return True for success, false for failure
   */
  bool
  loadDataDataValuesAndPairs(std::vector<ProcessingNode *> &args, int numValues,
			     const MathLoopData **field,
			     const MathLoopData **field2,
			     std::vector<double> &values,
		       std::vector<std::pair<double,double> > &pairs) const;


  /**
   * Arg list expected to be (data0,data1,data_<n>, value0,value1,..,value_<m>
   * where <n> and <m> are fixed

   * Extract that content and return.
   *
   * @param[in] args  Unary function argument list
   * @param[in] numData  Expected length of data list
   * @param[out] data  The data pointers
   * @param[in] numValues  Expected length of value list
   * @param[out] values  The values
   * @return True for success, false for failure
   */
  bool loadMultiDataAndMultiValues(std::vector<ProcessingNode *> &args,
				   int numData,
				   std::vector<const MathLoopData *> &data,
				   int numValues,
				   std::vector<double> &values) const;


  /**
   * @return the string data name in a particular arg position
   *
   * @param[in] args  Unary function argument list
   * @param[in] index  Index into arg list
   * @return string name
   */
  std::string getDataName(std::vector<ProcessingNode *> &args, int index) const;

protected:
private:

};

#endif
