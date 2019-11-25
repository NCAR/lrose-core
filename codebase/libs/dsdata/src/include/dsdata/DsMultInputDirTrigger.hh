/************************************************************************
 * DsMultInputDirTrigger: Class implementing a DsTrigger which returns new
 *                        files as they appear in any of a list of input
 *                        directories.
 *
 * September 2017
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef DsMultInputDirTrigger_HH
#define DsMultInputDirTrigger_HH

#include <string>
#include <vector>
#include <cassert>

#include <dsdata/DsTrigger.hh>
#include <toolsa/InputDir.hh>

using namespace std;


class DsMultInputDirTrigger : public DsTrigger
{

public:

  ////////////////////////////
  // Initialization methods //
  ////////////////////////////

  /**********************************************************************
   * Constructors
   */

  DsMultInputDirTrigger();


  /**********************************************************************
   * Destructor
   */

  virtual ~DsMultInputDirTrigger();


  /**********************************************************************
   * init() - Initialize the object.  The object must be initialized
   *          before it can be used.
   *
   * input_dir: directory to watch for new files.
   *
   * file_substring: Only files whose names contain this substring will
   *                 act as triggers.
   *
   * process_old_files: Flag indicating whether to process old files
   *                    in the input directory.
   *
   * recurse: If true, the object will recurse through the input directory
   *          looking for files.  If false, the object will only look in
   *          the specified input directory.
   *
   * exclude_substring: Files whose names contain this substring will not
   *                    act as triggers.
   *
   *  Returns 0 on success, -1 on error.
   *
   * Use getErrStr() for error message.
   */

  int init(const vector< string > &input_dirs,
	   const string &file_substring,
	   const bool process_old_files,
	   const heartbeat_func_t heartbeat_func = 0,
	   const bool recurse = false,
	   const string &exclude_substring = "",
           const int check_interval_secs = 1);
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**********************************************************************
   * next() - Get the next trigger and set the triggerInfo accordingly
   *
   * Returns 0 upon success, -1 upon failure.
   */

  int next();
  

  /**********************************************************************
   * endOfData() - Check whether we are at the end of the data.
   */

  bool endOfData() const;
  

  /**********************************************************************
   * reset() - Reset to start of data list
   */

  void reset();
  

private:

  /////////////////////
  // Private members //
  /////////////////////

  bool _objectInitialized;
 
  bool _endOfData;
 
  bool _processOldFiles;
 
  vector< InputDir* > _inputDirs;
  vector< InputDir* >::iterator _currInputDir;
  
  heartbeat_func_t _heartbeatFunc;

  int _checkIntervalSecs;
  
};

#endif /* DsMultInputDirTrigger_HH */


