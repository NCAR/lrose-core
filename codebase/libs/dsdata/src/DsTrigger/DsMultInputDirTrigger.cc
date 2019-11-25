/*********************************************************************
 * DsMultInputDirTrigger: Class implementing a DsTrigger which returns new
 *                        files as they appear in any of a list of input
 *                        directories.
 *
 * September 2017
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <sys/types.h>
#include <toolsa/file_io.h>
#include <unistd.h>

#include <dsdata/DsMultInputDirTrigger.hh>

#include <toolsa/InputDir.hh>
#include <toolsa/InputDirRecurse.hh>
#include <toolsa/Path.hh>

using namespace std;



/**********************************************************************
 * Constructors
 */

DsMultInputDirTrigger::DsMultInputDirTrigger() :
  DsTrigger(TYPE_FILE_TRIGGER),
  _objectInitialized(false),
  _endOfData(false),
  _processOldFiles(false),
  _heartbeatFunc(NULL),
  _checkIntervalSecs(1)
{
  // Do nothing
}


/**********************************************************************
 * Destructor
 */

DsMultInputDirTrigger::~DsMultInputDirTrigger()
{
  for (vector< InputDir* >::iterator input_dir = _inputDirs.begin();
       input_dir != _inputDirs.end(); ++input_dir)
  {
    if (*input_dir)
      delete *input_dir;
  } /* endfor - input_dir */
  
}


/**********************************************************************
 * init() - Initialize the object.  The object must be initialized
 *          before it can be used.
 *
 * input_dir: directory to watch for new files.
 *
 *  Returns 0 on success, -1 on error.
 *
 * Use getErrStr() for error message.
 */

int DsMultInputDirTrigger::init(const vector< string > &input_dirs,
                                const string &file_substring,
                                const bool process_old_files,
                                const heartbeat_func_t heartbeat_func,
                                const bool recurse,
                                const string &exclude_substring,
                                const int check_interval_secs)
{
  const string method_name = "DsMultInputDirTrigger::init()";
  
  _clearErrStr();
 

  _processOldFiles = process_old_files;

  // Initialize the InputDir objects

  for (vector< string >::const_iterator input_dir_string = input_dirs.begin();
       input_dir_string != input_dirs.end(); ++input_dir_string)
  {
    // Check to make sure that the input directory exists. If it doesn't exist,
    // we can't use it for triggering

    if (!Path::exists(*input_dir_string))
    {
      cerr << "WARNING: " << method_name << endl;
      cerr << "Input directory doesn't exist: " << *input_dir_string << endl;
      cerr << "Cannot use this directory as a trigger" << endl;

      continue;
    }
    
    InputDir *input_dir;
    
    if (recurse)
      input_dir = new InputDirRecurse();
    else
      input_dir = new InputDir();
  
    input_dir->setDirName(*input_dir_string);
    input_dir->setFileSubstring(file_substring);
    input_dir->setExcludeSubstring(exclude_substring);
    input_dir->setProcessOldFiles(process_old_files);

    _inputDirs.push_back(input_dir);
  } /* endfor - input_dir_string */

  if (_inputDirs.size() == 0)
  {
    _errStr = "ERROR - " + method_name + "\n";
    _errStr += "No input directories for trigger\n";
    
    return -1;
  }
  
  _heartbeatFunc = heartbeat_func;
  _checkIntervalSecs = check_interval_secs;

  _currInputDir = _inputDirs.begin();

  _objectInitialized = true;
  
  return 0;
}


/**********************************************************************
 * next() - Get the next trigger and set the triggerInfo accordingly
 *
 * Returns 0 upon success, -1 upon failure.
 */

int DsMultInputDirTrigger::next()
{
  const string method_name = "DsMultInputDirTrigger::next()";

  assert(_objectInitialized);
  
  // clear out the old stuff

  _clearErrStr();
  _triggerInfo.clear();

  // Check for end of trigger data

  if (endOfData())
      return -1;

  // Wait for the next available input file
  
  char *next_filename = 0;
  
  while ((next_filename = (*_currInputDir)->getNextFilename(1, -1)) == 0)
  {
    if (_heartbeatFunc)
      _heartbeatFunc("Waiting for data");

    // We've processed all of the existing files for the current directory so
    // move to the next directory

    ++_currInputDir;

    // If we've cycled through all of the input directories, either exit if we
    // are in archive mode or loop back around to the first input directory

    if (_currInputDir == _inputDirs.end())
    {
      // We'll assume this is archive mode if there isn't an assigned
      // heartbeat function and we are processing old files.

      if (_heartbeatFunc == 0 && _processOldFiles)
      {
        _endOfData = true;
        return -1;
      }
      else
      {
        if (_heartbeatFunc)
          _heartbeatFunc("Waiting for data");
      }

      // Move to the first input directory
      
      _currInputDir = _inputDirs.begin();

      // Sleep for a little while so we don't overwhelm the CPU
      
      sleep(_checkIntervalSecs);
    }
  }
  
  // Set the trigger info

  struct stat file_stat;
  
  if (ta_stat(next_filename, &file_stat) != 0)
  {
    _errStr = "ERROR - " + method_name + "\n";
    _errStr += string("Unable to stat new data file: ") + next_filename + "\n";
    
    return -1;
  }
  
  _triggerInfo.setFilePath(next_filename);
  _triggerInfo.setIssueTime(file_stat.st_mtime);

  delete [] next_filename;
  
  return 0;
}


/**********************************************************************
 * endOfData() - Check whether we are at the end of the data.
 */

bool DsMultInputDirTrigger::endOfData() const
{
  return _endOfData;
}


/**********************************************************************
 * reset() - Reset to start of data list
 */

void DsMultInputDirTrigger::reset()
{
  assert(_objectInitialized);
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
