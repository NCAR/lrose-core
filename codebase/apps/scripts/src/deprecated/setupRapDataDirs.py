#!/usr/bin/env python2
#
# NOTE: August 10, 2000 (Terri Betancourt)
#       For autoNowcast projects, the easies way to use this script
#       is via the wrappers;
#
#       - autoNowast_mkdata (for configuring directories on all nowcast hosts)
#       - host_mkdata       (for configuring directories on a single host)
#
#       These wrapper scripts will determine the appropriate control file
#       to use based on host roles specified in $CONTROL_DIR/params/host_env.*
# 
# NOTE: April 11,2000. This script was modified by Sue Dettling.
#       I made the following changes to the script:
#       1) command line option "-p  dataParamDir" was added so
#          that this is no longer hardwired.
#       2) link destinations can now handle envirionment variables
#          in the pathname.
#       3) I added the suffix .py to the script name so that
#          one can take advantage of emacs' nifty Python mode.
#       
def GiveHelp():
 print ""
 print " This is a small program that reads a control file"
 print " and uses it to setup a data directory structure"
 print " according to RAP conventions."
 print ""
 print " A typical command sequence might look like this:"
 print ""
 print " setupRapDataDirs.py -f $DATA_HOME/data_lists/INGEST_HOST.inField "
 print "                     -p $DATA_HOME/params"
 print ""
 print " The control file will look something like this :"
 print ""
 print "########################################################"
 print "#"
 print "# local data directories relative to RAP_DATA_DIR"
 print "#"
 print "fmq          local       _Janitor.fmq"
 print "mdv          local       _fileDist.mdv  _Janitor.mdv"
 print "mdv/ptrec    local       _Janitor.mdv.local"
 print "spdb         local       _Janitor.spdb"
 print "spdb/colide  local"
 print "#"
 print "#linked data directories relative to RAP_DATA_DIR"
 print "#"
 print "mdv/radarPPI     link /nfs/d1/wsmr/mdv/radarPPI      _Janitor.mdv.xmnt"
 print "mdv/radarRadial  link /nfs/d1/wsmr/mdv/radarRadial   _Janitor.mdv.xmnt"
 print "#"
 print "#"
 print "#####################################################"
 print ""
 print " Note that lines starting with a '#' are comment lines (blank"
 print " lines are OK too). The first entry is the name of the directory"
 print " to make. The second entry is either 'local', indicating that the"
 print " directory is local and a mkdir -p $RAP_DATA_DIR/<dirname>"
 print " is to be executed, or 'link' indicating that the directory is a"
 print " link to another directory (usually a cross mount to another machine). "
 print ""
 print " Note that directories with names beginning with '/' are treated"
 print " as absolute rather than being relative to $RAP_DATA_DIR"
 print ""
 print " In the case of a link, the third entry is where the"
 print " directory should be linked to. In this case, instead of"
 print " executing a mkdir, this script executes an ln -s (make soft link)."
 print ""
 print " Subsequent entries are the names of files that are to be"
 print " copied into the directory once it has been made. For a file"
 print " named :"
 print ""
 print " X+blah"
 print ""
 print " The script will copy the source file named :"
 print ""
 print " <dataParamDir>/X+blah"
 print ""
 print " to :"
 print ""
 print " <DestinationDir>/X"
 print ""
 print " So that the line" 
 print ""
 print " spdb  local _Janitor+spdb"
 print ""
 print " Makes the directory $RAP_DATA_DIR/spdb and then"
 print " copies the file <dataParamDir>/_Janitor+spdb to"
 print " $RAP_DATA_DIR/spdb/_Janitor"
 print ""
 print ""
 print " Command line options: "
 print ""
 print " -f [input control file name] (required)"
 print " -v for verbose. This may actually mask meaningful errors in verbage."
 print " -h for this message"
 print " -c for clean, will execute rm -rf $RAP_DATA_DIR/*"
 print "    before proceeding. USE CAUTIOUSLY!!!"
 print " -p dataParamDir (required)"
 print ""
 return
#
#
import sys
import string
import os
#
# ParseArgBool returns 1 if the Flag is present
# in the command line, else 0
#
def ParseArgBool(Flag):

 for i in range(1,len(sys.argv)):
  if (sys.argv[i] == Flag):
   return 1
 return 0

#
# ParseArg returns the value after the
# key in the command line, so if
# the command line contained "-f file.dat" and
# flag was set to "-f" then "file.dat" would be
# returned. Returns "" if no flag found.
# 
#
def ParseArg(Flag):

 for i in range(1,len(sys.argv)-1):
  if (sys.argv[i] == Flag):
   return sys.argv[i+1]

 return ""

#####################################################
#
# Function to strip the plus out of a string.
# _Janitor+mdv.data becomes _Janitor
#
def ChopAtPlus(String):

 for i in range(0,len(String)):
  if (String[i] == "+"):
   return String[0:i]
 return String

####################################################
##
# Function to process a line from the control file.
#
def ProcessLine(Ln,dataParamDir, verbose):
 LineElements = string.split(Ln)
 NumElements = len(LineElements)

#
# We need at least the directory name and
# the link / local indicator on each line.
# So return if we do not have at least two elements.
#
 if (NumElements < 2):
  return

 DirSpec = LineElements[0]
#
# If the specified directory does not start with a "/" then
# it is relative to $RAP_DATA_DIR
#
 DirSpec = os.path.expandvars(DirSpec)
 if ((DirSpec[0] != "/")):
  DirName = "$RAP_DATA_DIR/" + DirSpec
 else :
  DirName = DirSpec
 print " Dir path: ", DirName
 GotType = 0
 DirType = LineElements[1] #Should be either 'local' or 'link'
#
#
#
 if (DirType == "local"):
  GotType = 1
  isLink = 0
  FileListStart = 2

 if (DirType == "link"):
  GotType = 1
  isLink = 1
  FileListStart = 3
  linkDest = os.path.expandvars(LineElements[2])

 if (GotType == 0):
  print "Cannot recognise directory type ",DirType
  sys.exit(1);

 if (verbose == 1):
  print "Directory ", DirName, "\tlink ", isLink

 if (isLink == 0):
  Command = "\mkdir -p " + DirName
  if (verbose == 1):
   print "Executing : ", Command
  os.system( Command )

 if (isLink == 1):
#
# Make the target dir
#
  Command = "\mkdir -p " + linkDest
  ret = os.system( Command )
  print "\n\n ", Command, "ret= " , ret, "\n"
  
#
# Delete any prior links. The -f flag was added to
# get silent operation - no error messages.
#
  Command = "/bin/rm -f " + DirName
  os.system( Command )

#
# Make the link dir just so all recursive directories get created
# Then delete the base directory so the link can be put into place
# The -f flag was added to get silent operation - no error messages.
#
  Command = "\mkdir -p " + DirName
  os.system( Command )
  Command = "/bin/rmdir " + DirName
  os.system( Command )

#
# Make the link
#
  Command = "cd; \ln -s "  + linkDest + " " + DirName
  if (verbose == 1):
   print "Executing : ", Command
  os.system( Command )

 if (NumElements > FileListStart):
  if (verbose == 1):
   print DirName," has ",NumElements-FileListStart, " file(s) to be copied in"
  for i in range(FileListStart, NumElements):
   DestFile =  DirName + "/" + ChopAtPlus(LineElements[i])
   SourceFile = dataParamDir +  LineElements[i]
   if (verbose == 1):
    print "Copying ", SourceFile, " to ", DestFile
   Command = "\cp -r " + SourceFile + " " + DestFile
   os.system( Command )
  if (verbose == 1):
   print ""

#
#
#######################################################
#
#
# Main module starts.
#
#
#
# Parse command line arguments.
# 
#
needHelp = ParseArgBool("-h")
if (needHelp == 1):
 GiveHelp()
 sys.exit(1)
#

#
ControlFile = ParseArg("-f")
if (ControlFile == ""):
 print "I need a control file. Use -h for help."
 sys.exit(1)


dataParamDir = ParseArg("-p")
if (dataParamDir == ""):
 print "I need a dataParamDir.Use -h for help."
 sys.exit(1)
else:
 if dataParamDir[-1] != '/':
  dataParamDir = dataParamDir + "/"
 
#
verbose = ParseArgBool("-v")
#
cleanStart = ParseArgBool("-c")
#
# Get the control file name, and open
# it.
#
try: 
 file  = open(ControlFile,"r")
except (IOError):
 print "Cannot open control file ",ControlFile
 print "I cannot cope."
 sys.exit(1)
#
if (verbose == 1):
 print sys.argv[0], " using control file ", ControlFile
 print sys.argv[0], " dataParamDir ", dataParamDir
#
#
# Delete existing tree, if desired.
#
if (cleanStart == 1):
 if (verbose == 1):
  print "Removing files."
  Command = "/bin/rm -vrf $RAP_DATA_DIR/*"
 else :
  Command = "/bin/rm -rf $RAP_DATA_DIR/*"
 os.system( Command )
 if (verbose == 1):
  print "Done removing files."
#
# Loop through the lines in the file.
# If they do not start with a "#", then
# hand them off to the ProcessLine routine.
#
while 1:
 ln = file.readline()
 if (ln):
  if (ln[0] != "#"):
   ProcessLine(ln, dataParamDir,verbose)
 else:
  break

file.close()

print "Normal termination."

sys.exit(0)
