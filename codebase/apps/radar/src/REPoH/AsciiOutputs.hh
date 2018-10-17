/**
 * @file AsciiOutputs.hh
 * @brief All ascii related outputs, one per url
 * @class AsciiOutputs
 * @brief All ascii related outputs, one per url
 */

#ifndef ASCII_OUTPUTS_HH
#define ASCII_OUTPUTS_HH

#include "AsciiOutput.hh"
#include "Parms.hh"
#include <vector>

class VirtVolParms;

//------------------------------------------------------------------
class AsciiOutputs
{
public:

  /**
   * Constructor
   */
  AsciiOutputs(void);

  /**
   * Destructor
   */
  virtual ~AsciiOutputs(void);
  void clear(void);


  bool initialize(const Parms &parms,
		  const time_t &t);

  AsciiOutput *refToAsciiOutput(const std::string &name, bool suppressWarn);
  
protected:
private:

  // Output ascii stuff, one for unfiltered, one for filtered kernels
  std::vector<AsciiOutput *> _asciiOutput;

  void _clear(void);
  bool _setupAsciiOutput(const RepohParams::Ascii_output_t &p,
			 const VirtVolParms &vparms,
			 const time_t &t);
};

#endif
