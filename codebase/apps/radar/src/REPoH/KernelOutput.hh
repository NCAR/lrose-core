/**
 * @file KernelOutput.hh
 * @brief Container for kernel output information per vlevel
 *
 * @class KernelOutput
 * @brief Container for kernel output information per vlevel
 */

#ifndef KERNEL_OUTPUT_H
#define KERNEL_OUTPUT_H

#include "Kernels.hh"
#include <rapmath/MathUserData.hh>
#include <vector>
#include <string>

class KernelOutput : public MathUserData
{
public:
  /**
   * @param[in] name   Name of variable
   * @param[in] numVlevelIndex   Number of vlevels
   * @param[in] filtered  True for filtered kernels, false for unfiltered
   * @param[in] outside  True for points outside the cloud, false for inside
   * @param[in] url  Top URL path - subpaths get created per vlevel
   */
  KernelOutput(const std::string &name, int numVlevelIndex, bool filtered,
	       bool outside, const std::string &url);

  virtual ~KernelOutput(void);

  #include <rapmath/MathUserDataVirtualMethods.hh>

  /**
   * @return true if input name = local name
   * @param[in] name
   */
  inline bool nameMatch(const std::string &name) const {return _name == name;}

  /**
   * Replace local Kernels object at a vlevel index with input
   * @param[in] vlevelIndex
   * @param[in] k  Input Kernels
   */
  void storeKernel(int vlevelIndex, const Kernels &k);

  /**
   * Write SPDB Genpoly output from local state
   *
   * @param[in] t  Time
   * @param[in] proj  Projection
   */
  void output(const time_t &t, const MdvxProj &proj) const;
  
private:

  std::string _name;  /**< Variable name */
  bool _filtered;     /**< True for filtered kernels */
  bool _outside;      /**< True for outside the cloud points */
  int _numVlevel;     /**< Number of vertical levels */

  std::vector<std::string> _url;  /**< One URL per vlevel */
  std::vector<Kernels> _kernels;  /**< One Kernels per vlevel */
};

#endif
