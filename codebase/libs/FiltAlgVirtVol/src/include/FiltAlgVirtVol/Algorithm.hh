/**
 * @file Algorithm.hh 
 * @brief The algorithm class, contains and controls the filters.
 * @class Algorithm
 * @brief The algorithm class, contains and controls the filters.
 */

#ifndef ALGORITHM_H
#define ALGORITHM_H
#include <FiltAlgVirtVol/FiltAlgParms.hh>
#include <FiltAlgVirtVol/AlgorithmParms.hh>
#include <rapmath/MathParser.hh>
#include <rapmath/MathData.hh>
#include <toolsa/TaThreadDoubleQue.hh>

class VolumeData;

//------------------------------------------------------------------
class Algorithm 
{
public:
  /**
   * Constructor just to print out operators
   */
  Algorithm(const MathData &data, const VolumeData &vdata);

  /**
   * Constructor
   * @param[in] p  Algorithm parameter settings
   */
  Algorithm(const FiltAlgParms &p, const MathData &data,
	    const VolumeData &vdata);

  /**
   * Destructor
   */
  virtual ~Algorithm(void);

  /**
   * @return true if object well formed
   */
  inline bool ok(void) const { return _ok;}

  /**
   * Update for a particular input volume
   *
   * @param[in] P   Algorithm parameters
   * @param[in] input  One sweep of input data
   * @param[in] extra optional additional input, can be NULL, depends on app,
   *                  set at app level.
   * @return true for success
   */
  bool update(const AlgorithmParms &P, VolumeData *input);


  /**
   * @return number of inputs to the algorthm
   */
  inline size_t numInputs(void) const {return _inputs.size();}

  /**
   * @return reference to i'th algorithm input name
   * @param[in] i  Index
   */
  inline const std::string &ithInputRef(int i) const {return _inputs[i];}

  /**
   * @return number of outputs from the algorthm
   */
  inline size_t numOutputs(void) const {return _outputs.size();}

  /**
   * @return reference to i'th algorithm output name
   * @param[in] i  Index
   */
  inline const std::string &ithOutputRef(int i) const {return _outputs[i];}

  /**
   * @return true if input string is one of the algorithm inputs
   * @param[in] name
   */
  bool isInput(const std::string &name) const;

  /**
   * @return true if input string is one of the algorithm outputs
   * @param[in] name
   */
  bool isOutput(const std::string &name) const;

  /**
   * Print debug
   */
  void printOperators(void) const;

  /**
   * Compute method used in threaded algorithms
   * @param[in] ti  Pointer to AlgInfo
   */
  static void compute(void *ti);


protected:

private:

  bool _ok;             /**< True if object well formed */
  MathParser _p;        /**< The math handler, executes all filter steps */

  /**
   * Names of all inputs to the algorithm from outside (names as known by the
   * algorithm class)
   */
  std::vector<std::string> _inputs;

  /**
   * Names of all outputs from the algorithm to the outside (names known by
   * the algorithm class)
   */
  std::vector<std::string> _outputs;

  /**
   * @class AlgThreads
   * @brief Simple class to instantiate TaThreadDoubleQue by implementing
   * the clone() method.
   */
  class AlgThreads : public TaThreadDoubleQue
  {
  public:
    /**
     * Empty constructor
     */
    inline AlgThreads() : TaThreadDoubleQue() {}
    /**
     * Empty destructor
     */
    inline virtual ~AlgThreads() {}
    /**
     * Clone a thread and return pointer to base class
     * @param[in] index 
     */
    TaThread *clone(int index);
  };

  /**
   * @class AlgInfo
   * @brief Information passed to the Algorithm threaded compute virtual method
   */
  class AlgInfo
  {
  public:
    /**
     * Constructor, args match one to one with members
     */
    inline AlgInfo(int index, const Algorithm *alg, VolumeData *volume,
		   TaThreadQue *thread) :
      _alg(alg), _volume(volume), _index(index), _thread(thread) {}

    /**
     * Destructor
     */
    inline virtual ~AlgInfo(void) {}

    const Algorithm *_alg;  /**< Pointer to the Algorithm object */
    VolumeData *_volume;    /**< Pointer to the data used by the Algorithm */
    int _index;             /**< Index into the volume */
    TaThreadQue *_thread;   /**< Threading pointer, used for lock/unlock */

  protected:
  private:
  };

};

#endif
