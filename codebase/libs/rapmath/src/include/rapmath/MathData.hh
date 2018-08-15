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

protected:
private:

};

#endif
