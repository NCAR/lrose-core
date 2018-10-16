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
   * Lots of methods to load particular expected things from the args vector.
   * This can be expanded as needed.
   */
  const MathLoopData *loadData(std::vector<ProcessingNode *> &args,
			       int index) const;
  bool loadDataValueValue(std::vector<ProcessingNode *> &args,
			  const MathLoopData **field, double &v0,
			  double &v1) const;
  bool loadMultiData(std::vector<ProcessingNode *> &args,
		     std::vector<const MathLoopData *> &fields) const;
  bool loadDataData(std::vector<ProcessingNode *> &args,
		    const MathLoopData **data0,
		    const MathLoopData **data1) const;
  bool loadDataValue(std::vector<ProcessingNode *> &args,
		     const MathLoopData **field, double &v) const;
  bool loadDataAndPairs(std::vector<ProcessingNode *> &args,
		       const MathLoopData **field,
		       std::vector<std::pair<double,double> > &pairs) const;
  bool
  loadDataDataAndPairs(std::vector<ProcessingNode *> &args,
		       const MathLoopData **field,
		       const MathLoopData **field2,
		       std::vector<std::pair<double,double> > &pairs) const;
  bool loadValueAndMultiData(std::vector<ProcessingNode *> &args,
			     double &value,
			     std::vector<const MathLoopData *> &fields) const;
  bool 
  loadDataAndFourNumbers(std::vector<ProcessingNode *> &args,
			 const MathLoopData **data,
			 double &n0, double &n1, double &n2,
			 double &n3) const;
  bool 
  loadDataAndThreeNumbers(std::vector<ProcessingNode *> &args,
			  const MathLoopData **data,
			  double &n0, double &n1, double &n2) const;
  bool 
  loadDataAndFiveNumbers(std::vector<ProcessingNode *> &args,
			 const MathLoopData **data,
			 double &n0, double &n1, double &n2,
			 double &n3, double &n4) const;
  bool 
  loadNumberAndDataNumberPairs(std::vector<ProcessingNode *> &args,
			       double &n,
			       std::vector<const MathLoopData *> &data,
			       std::vector<double> &numbers) const;
  bool loadDataAndUserData(std::vector<ProcessingNode *> &args,
			   const MathLoopData **data,
			   MathUserData **udata);
  bool loadDataAndTwoUserDatas(std::vector<ProcessingNode *> &args,
			       const MathLoopData **data,
			       MathUserData **udata1,
			       MathUserData **udata2);
  bool loadDataAndUserDataAndValue(std::vector<ProcessingNode *> &args,
				   const MathLoopData **data,
				   MathUserData **udata,
				   double &value);
  bool loadNameAndUserDataAndValue(std::vector<ProcessingNode *> &args,
				   std::string &name,
				   MathUserData **udata,
				   double &value);
  bool
  loadDataDataValuesAndPairs(std::vector<ProcessingNode *> &args,
			     int numValues,
			     const MathLoopData **field,
			     const MathLoopData **field2,
			     std::vector<double> &values,
		       std::vector<std::pair<double,double> > &pairs) const;
  MathUserData *userData(std::vector<ProcessingNode *> &args);

  bool loadMultiDataAndMultiValues(std::vector<ProcessingNode *> &args,
				   int numData,
				   std::vector<const MathLoopData *> &data,
				   int numValues,
				   std::vector<double> &values) const;
  std::string getDataName(std::vector<ProcessingNode *> &args, int index) const;

protected:
private:

};

#endif
