#!/bin/sh --
#
# zombie_watcher.sh
#
# Shell script to look for zombies in the process table, search for
# their parents and output the results to STDOUT. By default, this
# process runs forever, checking then sleeping, then checking again.
#
# This is intentionally written in Bourne Shell to be as lightweight
# as possible.
# 
#=====================================================================
#
############### Defaults #######################
#
# Set defaults (season to taste)
#
Tmp_dir=$HOME/tmp
Sleep_secs=60
Lines_from_top=15
Exit_success=1
Exit_failure=0

# Set a default output file. This is reset in the loop below
# if Use_stdout is FALSE

Today=`date -u '+%Y%m%d'`
Out_file=$ERRORS_LOG_DIR/$Today/zombie_watcher.log

# Set command line defaults

Use_stdout=TRUE
Run_once=FALSE

###########  Command line parsing ################

# Parse command line

for var in $@ ; do
    case $var in
        -h) echo "Usage: $0 [-h] [-l] [-x]"
            echo "Purpose: To check the process table for zombies and"
	    echo "         print them and their parent processes."
	    echo " "
	    echo "  -h    : Print this usage message"
	    echo "  -l    : Print log messages to log file: \$ERRORS_LOG_DIR/$Today/zombie_watcher.log"
	    echo "          Default is to print to STDOUT"
	    echo "  -x    : Run check once and exit."
	    echo "          Default is loop forever sleeping $Sleep_secs secs between checks"
	    exit $Exit_failure
	    ;;
        -l) Use_stdout=FALSE
	    ;;
	-x) Run_once=TRUE
	    ;;
    esac
done

############### Define functions #######################
#
# define function write text message to screen,
# status file, and log file
#

function LogAdd
{
    if test $Use_stdout = TRUE ; then
        echo $1
    else 
	echo $1 >> $Out_file
    fi
}

################# End of functions #####################


############### Main #######################
#
# Set environment, this is very important for ps
#

PS_PERSONALITY=BSD
export PS_PERSONALITY

#
# Define the output columns of the ps alx command

ps_header_file=$Tmp_dir/ps_header.tmp
if test -e $ps_header_file ; then
    /bin/rm -f $ps_header_file
fi

ps alx | head -1 > $ps_header_file
col1=`awk '{print $1}' $ps_header_file`
col2=`awk '{print $2}' $ps_header_file`
col3=`awk '{print $3}' $ps_header_file`
col4=`awk '{print $4}' $ps_header_file`

ppid_col=-1
if test $col3 = PPID ; then
    ppid_col=3
fi
if test $col4 = PPID ; then
    ppid_col=4
fi

#
# Start looping
#

Done=FALSE
while [ $Done = FALSE ] ; do

    # Set the name of the output file (in case we have crossed over
    # the date boundary)

    if test $Use_stdout = FALSE ; then
	Today=`date -u '+%Y%m%d'`
	Out_file=$ERRORS_LOG_DIR/$Today/zombie_watcher.log
    fi

    # Print header to outfile

    Now=`date -u`
    LogAdd "======= $Now ========"

    # Print top information
    
    top_file=$Tmp_dir/top.list
    if test -e $top_file ; then
	/bin/rm -f $top_file
    fi

    top -n 1 -b | head -$Lines_from_top > $top_file

    LogAdd "Current state..."
    if test $Use_stdout = FALSE ; then
	cat $top_file >> $Out_file
    else
	cat $top_file
    fi
    
    # Look for zombies in the process list. Be careful not to find
    # the egrep itself.
    
    zombie_file=$Tmp_dir/zombie.list
    if test -e $zombie_file ; then
	rm -f $zombie_file
    fi

    ps alx | egrep defunct | egrep --invert-match "egrep defunct" > $zombie_file

    # Were there any zombies?

    found_zombies=FALSE
    nlines=`cat $zombie_file | wc -l`
    if test $nlines -ge 1 ; then
	found_zombies=TRUE
    fi

    # Cat the zombies to the output

    LogAdd " "

    if test $found_zombies = FALSE ; then
	LogAdd "List of zombies... NONE"
    else
	LogAdd "List of zombies..."
	if test $Use_stdout = FALSE ; then
	    cat $ps_header_file >> $Out_file
	    cat $zombie_file >> $Out_file
	else
	    cat $ps_header_file
	    cat $zombie_file
	fi

	# Parse the zombie file to look for the parent processes

	case $ppid_col in
	    3) ppids=`awk '{print $3}' $zombie_file` ;;
	    4) ppids=`awk '{print $4}' $zombie_file` ;;
	esac

	# Look for the parents. Need to be careful to only match the PPID
	# at the beginning of the ps output so get an exact match

	for ppid in $ppids ; do
	    LogAdd "ParentID: $ppid..."
	    line=`ps afx | egrep "^[ ]+${ppid}[ ]"`
	    LogAdd "$line"
	done
    fi

    # Sleep
    
    if test $Run_once = TRUE ; then
	Done=TRUE
    fi

    if test $Done = FALSE ; then
	sleep $Sleep_secs
    fi

done # Done

exit $Exit_success
