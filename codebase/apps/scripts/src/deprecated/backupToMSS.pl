#!/usr/bin/perl
#
# Script: backup.2.3_beta.pl
# Written By: John Eckhardt
# Dated: 03.28.2003
# Modified: 09.18.2003 
#
# RCS: $Id: backupToMSS.pl,v 1.2 2004/11/07 21:23:01 aoaws Exp $

  my $script_name = "backup.2.3_beta.pl";

  MAIN:
  {

    # !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    # Root directory declaration. This is where all backup working files, logs
    # lists and so forth will be stored. It is vitally important that this be
    # correct for this script to perform correctly.
    #
    my $root_dir = "/hal/juneau/scripts/backup";
    # !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


    # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    # A few package includes, variable declarations/initializations etc, etc.

    # Package Includes
    use diagnostics;
    use File::Data; 
    use File::Recurse;
    use Getopt::Long;
    use POSIX qw(strftime);
    use strict;
    use warnings;

    # Scalar integers in alpha order.  
    my $current_index = -99;		# current command index
    my $length = -99;			#  
    my $level_zero = 0;			# boolean switch  
    my $mask = 0777;			# mask for new files/directories

    # Scalar strings in alpha order.  
    my $base_dir = "";
    my $command_file = "";
    my $current_index_file = "";
    my $current_tarball_file = "";
    my $current_dir = "";
    my $directory = "";
    my $drive = "";
    my $help_flag = 0;
    my $logfile_dir = "";
    my $logfile = "";
    my $no_shuffle = 0;
    my $previous_dir = "";
    my $previous_state_file = "";
    my $response = ""; 
    #my $script_name = "backup.2.2_beta.pl";
    my $tar_tally_dir = "";
    my $tar_tally_file = "";
    
    # Arrays in alpha order.
    my @commands = {};
    my @new_stat = {};  
    my @old_stat = {};
    my @tar_tally = {};
    # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    # Read and verify the arguments, build file paths.
    # Currently supported flags: -h, --help, -z, -noshuffle, -drive.
    # Currently supported arguments: $drive.

    GetOptions ('drive=s' => \$drive, 
                'h' => \$help_flag,  
                'help' => \$help_flag,  
                'noshuffle' => \$no_shuffle, 
                'z+' => \$level_zero); 

    #print "drive = ".$drive."\n"; 
    #print "help = ".$help_flag."\n"; 
    #print "level_zero =".$level_zero."\n"; 

    # Chech for the help flag. 
    if($help_flag)
    { &help; }

    # Verify that a drive has been specified.
    if($drive eq "")
    { die "\n$script_name: You must specify a drive to backup.\n"; }
   
    else
    {
      if($drive =~ /\//)
      { 
	print   "You cannot include any \"/\" in your drive name\n";
        print "If you want to backup /d1, simply enter \"d1\".\n";
	die   "$script_name: incorrect argument.\n"; 
      }  
    }

    # Verify that the drive exists.
    if (! -d "/".$drive)
    {  die "\n$script_name: That disk does not exist.\n"; }
    else
    { print "You have chosen to backup /".$drive."\n"; }

    # Build directory paths.
    $base_dir = $root_dir."/".$drive; 
    $current_dir = $base_dir."/current"; 
    $logfile_dir = $base_dir."/current/logs";
    $previous_dir = $base_dir."/previous"; 
    $tar_tally_dir = $current_dir."/tartallys/";

    # Verify that all directories exist.
    if (! -e $base_dir)
    {
      print "The base directory, ".$base_dir." where all backup related ".
	+   "will be stored, does not exist. In order to continue, it (and ".
	+   "all its associated subdirectories) must be created.\n"; 

      $response = "q";
      while(($response ne 'y') && ($response ne 'n'))
      {
 	  print "Shall I create them now? (y/n)\n";
        $response = <STDIN>;
	chomp($response);
      }

      if($response eq 'n')
      { die "$script_name: Could not create mandatory directory strucure.\n";}
      else
      {
	if(! -d $base_dir)
	{ mkdir $base_dir, $mask or die "can't mkdir $base_dir: $!"; } 
	if(! -d $current_dir)
	{ mkdir $current_dir, $mask or die "can't mkdir $current_dir: $!"; } 
	if(! -d $logfile_dir)
	{ mkdir $logfile_dir, $mask or die "can't mkdir $logfile_dir: $!"; } 
	if(! -d $previous_dir)
	{ mkdir $previous_dir, $mask or die "can't mkdir $previous_dir: $!"; }
	if(! -d $tar_tally_dir)
	{ mkdir $tar_tally_dir, $mask or die "can't mkdir $previous_dir: $!"; }
      }
    }

    # Its possible that shuffling files could eliminate directories. These  
    # next few lines ensure that the directory structure remains correct.
    if(! -d $base_dir)
    { mkdir $base_dir, $mask or die "can't mkdir $base_dir: $!"; }
    if(! -d $current_dir)
    { mkdir $current_dir, $mask or die "can't mkdir $current_dir: $!"; }
    if(! -d $logfile_dir)
    { mkdir $logfile_dir, $mask or die "can't mkdir $logfile_dir: $!"; }
    if(! -d $previous_dir)
    { mkdir $previous_dir, $mask or die "can't mkdir $previous_dir: $!"; }
    if(! -d $tar_tally_dir)
    { mkdir $tar_tally_dir, $mask or die "can't mkdir $previous_dir: $!"; }

    # Build file paths.
    $command_file = $current_dir."/commands.txt";
    $current_index_file = $current_dir."/index.txt";
    $current_tarball_file = $current_dir."/tar_num.txt";
    $logfile = $logfile_dir."/".(strftime "%a_%b_%e_%H_%M_%S_%Y", localtime).
	+ ".txt";
    $previous_state_file = $current_dir."previous_state.txt";

    # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    # ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    # Here follows the section of backup.2.2_beta.pl that is actually 
    # getting some work done.

    # Print a greeting
    print "Greetings, salutations and welcome to $script_name.\n"; 

    # Check the current index.
    $current_index = &current_index($current_index_file); 

    if($current_index >= 0)
    {
      print "$script_name sees that the index from a previous run is ".
	+   "greater than zero, which impies that the previous run did not ".
	+   "complete.\n";  

      if($level_zero)
      {
	print "However, you are currently in the process of running a level".
		+ " zero backup.\n";
 
	$response   = "q";
        while(($response ne 'resume') && ($response ne 'zero'))
        {
	  print "Which course of action do you want to pursue? ";
	  print "(zero/resume)\n";
          $response = <STDIN>;
	  chomp($response);
	}   
	
	if($response eq 'zero')
	{
	  &zero($command_file, 
		+ $current_index_file, 
		+ $current_tarball_file,
		+ $drive,
                + $logfile,
                + $no_shuffle,
                + $root_dir,
		+ $tar_tally_dir);
	}
	else
	{
	  open (LOG, ">$logfile");
          print LOG "Backup_Type: resume\n".
          print LOG "Start_Time: ".(strftime "%a_%b_%e_%H_%M_%S_%Y", 
			+ localtime)."\n";
 	  print "$script_name is resuming the previous run.\n";
          close LOG;
	}
      }

      else
      {
        open (LOG, ">$logfile");
        print LOG "Backup_Type: resume\n".
        print LOG "Start_Time: ".(strftime "%a_%b_%e_%H_%M_%S_%Y", 
                    + localtime)."\n";
        print "$script_name is resuming the previous run.\n"; 
        close LOG;
      }
    }

    elsif($level_zero)
    {
 	# Run a zero-level 
      &zero($command_file, 
	    + $current_index_file,
            + $current_tarball_file,
   	      + $drive,
            + $logfile,
            + $no_shuffle,
            + $root_dir,
  	      + $tar_tally_dir);
    }

    else
    {
      # Run a supplement. 
      &supplement($command_file,
            + $current_index_file,
            + $current_tarball_file,
            + $drive,
            + $logfile,
            + $no_shuffle,
            + $root_dir,
            + $tar_tally_dir);
    }

    # In all cases, call resume().
    &resume($command_file, 
            +	$current_index_file,
    	      + $current_tarball_file, 
	    + $drive,
            +	$logfile, 
            +	$tar_tally_dir); 
  }

  ######### Subroutines #########  ( Alphabettically )

  ##### Returns current index from index file but makes no changes.  ######
  sub current_index
  {
    my $file = $_[0];
    my $index_val = -99;
    my @index = {};
    my $status = 0;

    $status = open(INDEX, $file); 
   
    if($status == 0) 
    {  
      warn "$script_name could not open index.txt ". 
	 + "This could mean this is a first run.\n"; 
      $index_val = -1; 
    } 

    elsif($status == 1)
    {
      @index = <INDEX>;
      close INDEX;
      $index_val = $index[0];
    }

    else
    {
      die "Unexpected file opening problem in &current_index!";
    }

    if($index_val eq "")
    { $index_val = -1; }

    return $index_val;
  }

  ############# Print a help screen  ##########
  sub help
  {
print 
"NAME

        backup.2.2.pl - backup the contents of a drive to the MSS

SYNOPSIS

        backup.2.2.pl [OPTION]... -drive-DRIVE...

DESCRIPTION

        This script makes a backup of the contents of DRIVE and saves the 
        redunant files to the MSS. It is capable of performing both zero-
        level and supplimental backups. By default it also manages two copies 
        of backups for a given drive named current and previous.

        -drive=DRIVE,   is the only mandatory argument to this script. It
                        identifies the disk to be backed-up. This MUST be
                        a fstab drive on the machine running backup.2.2.pl.

        -z,             run a zero level backup, the default is to perform 
                        a supplimental backup.  

        -noshuffle,     do not shuffle the local or remote directory
                        structures. Us this in the event of irrecoverable
                        backup.2.2.pl failure or if you only want to 
                        maintain one copy of backupfiles.

        -h or --help,   prints this screen (more or less).

";
    die "Good Luck\n"; 
  }

  #############  Increment value stored in index file #############
  sub incriment_index
  {
    my $current_index_file = $_[0];
    my $local_length = $_[1];

    my @tmp;
    my $current_index_val = -69;

    # Iterate the index files.
    open(INDEX, "<$current_index_file")
      || warn "Could not open $current_index_file\n";
    @tmp = <INDEX>;
    close INDEX;

    $current_index_val = $tmp[0];

    open(INDEX, ">$current_index_file");
    if($current_index_val < $local_length-1)
    {  $current_index_val++; } 
    else
    {  $current_index_val = -1 } 
    print INDEX $current_index_val;
    close(INDEX);
    #print "File: $current_index_file New Ind: $current_index_val\n";

    return $current_index_val;
  } 


  ####  Create (or ensure) that the mss has the appropriate directories  ####
  sub make_mss_structure
  {
    my $drive = $_[0];
    my $root_dir = $_[1];

    my $cmd = "";
    my $holder_file_path = $root_dir."/.holder";  
    my $user = getlogin || getpwuid($<);
       $user = uc($user);
    my $mms_backup_root = "mss:/".$user."/backup/";

    open(HOLDER, ">$holder_file_path")
      || die " &make_mss_structure could not open the holder file.";
    print HOLDER "This is simply a holder file.";
    close HOLDER;

    # Hold the backup directory.
    $cmd = "msrcp -pe 32767 ".$holder_file_path." ".$mms_backup_root.".holder"; 
    system($cmd);

    # Hold the drive subdirectory.
    $cmd = "msrcp -pe 32767 ".$holder_file_path." ".+
	$mms_backup_root."/".$drive."/.holder"; 
    system($cmd);
  
    # Hold the current and previous sub-subdirecories.
    $cmd = "msrcp -pe 32767 ".$holder_file_path." ".+
	$mms_backup_root."/".$drive."/current/.holder"; 
    system($cmd);

    $cmd = "msrcp -pe 32767 ".$holder_file_path." ".+
	$mms_backup_root."/".$drive."/previous/.holder"; 
    system($cmd);
  } 

  #######  Resume a backup already in progress.  ########
  # This is called for all tpyes of run, zero-level, supplement, interrupted
  # The name impies the functionality of the latest but may have been more
  # aptly named run() because of the function it truely serves. 
  sub resume
  {
    # Grab the filenames from the arguments
    my $command_file = $_[0];
    my $current_index_file = $_[1];
    my $current_tarball_file = $_[2]; 
    my $drive = $_[3];
    my $logfile = $_[4]; 
    my $tar_tally_dir = $_[5]; 

    # Other local variables. 
    my $mms_backup_path = "initialized"; 
    my $cmd = "initialized";
    my $command_file_handle = "";
    my $current_index_val = -1;
    my $current_tarball_name = "initialized";
    my $current_tarball_size = 4500001;
    my $file = "initialized";
    my $length = -1;
    my $logfile_handle = "initialized";
    my $tar_tally_file = ""; 
    my $tarball_number = -69;
    #my $tarball_list_handle = "initialized";
    my $user = getlogin || getpwuid($<);                         
       $user = uc($user);
 
    my @commands = {};
    my @statistics = {}; 
    my @tmp = {};
    
    # Build the backup path.
    $mms_backup_path = "mss:/".$user."/backup/".$drive."/current/";
 
    # Look at the current index value. 
    $current_index_val = &current_index($current_index_file);
 
    # If index is -1, then we are finished.
    if($current_index_val < 0)   
    {
      warn "$script_name saw no commands to run.\n";
      return 0;
    }

    else 
    {
      print "I am opening necesary files and loading their data. This may take".
	+ " some time, please be patient.\n";  

      # Open, read and close the command file
      #$command_file_handle = File::Data->new($command_file);
      #@commands = $command_file_handle->READ;
      open (COMMANDS, "<$command_file");
      @commands = <COMMANDS>; 
      close (COMMANDS);

      # Open the tarball list and logfile
      #$tarball_list_handle = File::Data->new($tar_tally_file);
      #$logfile_handle =  File::Data->new($logfile);
    }

    print "Begining tarball manufacture and delivery to MSS.\n";

    $length = @commands;
    # Check for empty list
    if($length == 0)
    { $current_index_val = -1 } 

    # Get the current tarball number.
    $current_tarball_number = &current_index($current_tarball_file);

    # Define the tally file
    $tar_tally_file = $tar_tally_dir."tarball".$current_tarball_number.".txt";  

    # Make initial logfile entry.
    open (LOGFILE, ">>$logfile");
    print LOGFILE "tarball $current_tarball_number begins at: ".
		+ (strftime "%a_%b_%e_%H_%M_%S_%Y", localtime)."\n";

    # Build current tarball name
    $current_tarball_name = $drive."_tarball_".$current_tarball_number.".tar"; 
  
    # Intialize tarball size variable.
    if(($current_index_val != 0) &&
	+ (-e $current_tarball_name)) 
    {
      # Grab Tarball Statistics.
      @statistics = stat($current_tarball_name);
      $current_tarball_size = $statistics[7];
    }
    else
    { $current_tarball_size = 0; } 

    # Open tally file (in append mode to save existing file if -e).
    open(TARTALLY, ">>$tar_tally_file")
      || die "Could not open $tar_tally_file: $!\n";
    
    # Open log file (in append mode to save existing file if -e).
    open(LOGFILE, ">>$logfile")
      || die "Could not open $logfile: $!\n";

    # Initialize the index variable
    $current_index_val = &current_index($current_index_file); 

    while($current_index_val != -1)
    {
      # Get the file.
      $file = $commands[$current_index_val];
      
      # Chomp the file line and check for its existence.
      chomp($file);
      if((! -e $file) || ($file eq ""))	
      {  
        $current_index_val = &incriment_index($current_index_file, $length);
        next;  
      }
 
      # Grab the files statistics.
      @statistics = stat($file);

      # Check that the file is statable
      if(@statistics eq NULL)
      { 
        print LOGFILE "error: stat failed on $file.\n"; 
        $current_index_val = &incriment_index($current_index_file, $length);
        next;  
      }

      # Add the file the tarball.          
      $cmd = "tar -rpPf ".$current_tarball_name." ".$file;
      #print "$cmd\n";
      system($cmd); 

      # Make tarball tally file entry. 
      print TARTALLY "file: $file\n";
      print TARTALLY "stats: @statistics\n";      

      # Increment current tarball size
      $current_tarball_size = $current_tarball_size + $statistics[7];

      #print "@statistics \n";

      if($current_tarball_size >= 450000000)
      {
        # Logfile entry.
        print LOGFILE "tarball $current_tarball_number stoped at: ".
   		  + (strftime "%a_%b_%e_%H_%M_%S_%Y", localtime)."\n"; 
        
        # Close old tally file. 
        print TARTALLY "stoptime: ".
 			+ (strftime "%a_%b_%e_%H_%M_%S_%Y", localtime)."\n";
        close(TARTALLY);
        
        # Move the tarball to MSS.
        print LOGFILE "tarball $current_tarball_number msrcp begins at: ".
		  + (strftime "%a_%b_%e_%H_%M_%S_%Y", localtime)."\n"; 
	$cmd = "msrcp -pe 32767 ".$current_tarball_name." ".
		+ $mms_backup_path.$current_tarball_name;
        print "$cmd\n";
        system($cmd);
        print LOGFILE "tarball $current_tarball_number msrcp ends at: ".
		  + (strftime "%a_%b_%e_%H_%M_%S_%Y", localtime)."\n"; 

	# Remove the tarball from the local directory.
	$cmd = "rm ".$current_tarball_name;
        #print "$cmd\n";
        system($cmd);

	# Increment tarball number.
 	$current_tarball_number = &incriment_index($current_tarball_file, 
						+ $current_tarball_number+2); 

    	# Build current tarball name
    	$current_tarball_name=$drive."_tarball_".$current_tarball_number.".tar";

        # Open new tally file. 
        $tar_tally_file = $tar_tally_dir."tarball".$current_tarball_number.
			  + ".txt";
        open(TARTALLY, ">>$tar_tally_file")
          || die "Could not open $tar_tally_file: $!\n";
        print TARTALLY "tarball: $current_tarball_number\n";
        print TARTALLY "startime: ".
 			+ (strftime "%a_%b_%e_%H_%M_%S_%Y", localtime)."\n";
 
	# Reset tarball size.
        $current_tarball_size = 0;
        
        # Logfile entry.
        print LOGFILE "tarball $current_tarball_number starts at: ".
		  + (strftime "%a_%b_%e_%H_%M_%S_%Y", localtime)."\n"; 
      }

      # Iterate the index value & file.
      $current_index_val = &incriment_index($current_index_file, $length);	
    }

    # Logfile entry.
    print LOGFILE "tarball $current_tarball_number msrcp at: ".
		+ (strftime "%a_%b_%e_%H_%M_%S_%Y", localtime)."\n";

    # Close tally file
    close(TARTALLY);

    # In the event of a risidual tarball, do the following ....
    if(-e $current_tarball_name)
    {
      # Move the tarball to MSS.
      $cmd = "msrcp -pe 32767 ".$current_tarball_name." ".
	     + $mms_backup_path.$current_tarball_name;
      print "$cmd\n";
      system($cmd);

      # Increment tarball number.
      $current_tarball_number = &incriment_index($current_tarball_file,
                                             + $current_tarball_number+2);

      # Remove the tarball.
      $cmd = "rm ".$current_tarball_name;
      print "$cmd\n";
      system($cmd);
    }

    # Final logfile entry. 
    print LOGFILE "End_Time: ".(strftime "%a_%b_%e_%H_%M_%S_%Y", localtime)."\n";
    close(LOGFILE);

    # Print a farewell.
    print "All finished. Thanks for using $script_name\n";
  }

  ########## Shuffle the local files for this backup. ###########
  sub shuffle_local    
  {
    my $drive = $_[0];
    my $rootdir =  $_[1];
      
    $cmd = "rm -r ".$rootdir."/".$drive."/previous/*";
    print $cmd."\n";
    system($cmd);
    $cmd = "cp -r ".$rootdir."/".$drive."/current/* ".
              +   $rootdir."/".$drive."/previous/";
    print $cmd."\n";
    system($cmd);
    $cmd = "rm -r ".$rootdir."/".$drive."/current/*";
    print $cmd."\n";
    system($cmd);
    $cmd = "mkdir ".$rootdir."/".$drive."/current/logs";
    print $cmd."\n";
    system($cmd);
    $cmd = "mkdir ".$rootdir."/".$drive."/current/tartallys";
    print $cmd."\n";
    system($cmd);
  }

  ########## Shuffle the MSS files for this backup. ###########
  sub shuffle_mss    
  {
    my $drive = $_[0];

    my $cmd = "";

    # Remove contents from the previous directoy.
    $cmd = "msrm \"backup/$drive/previous/*\"";
    print $cmd."\n";
    system($cmd);

    # Move "current" directory's content to "previous"
    $cmd = "msmv \"backup/$drive/current/*\" backup/$drive/previous";
    print $cmd."\n";
    system($cmd);
  }

  ########## Create a supplemental backup. ###########
  sub supplement 
  {
    print "Creating a supplemental backup.\n";

    # File name strings as delivered via arguments to subroutine.
    my $command_file =  $_[0];
    my $current_index_file = $_[1];
    my $current_tarball_file = $_[2];
    my $drive = "/".$_[3];
    my $logfile = $_[4];
    my $no_shuffle = $_[5];
    my $rootdir = $_[6];
    my $tarball_tally_dir = $_[7];

    # Subroutine variable declarations.
    my $cmd = "";
    my $dir = "none";
    my $file = "none";
    my $full_path = "none";
    my $next = 0;
    my $length = -1;
    my $size = 0;
    my $type = ""; 

    my @components;
    my @next_components;
    my %files;
    my @previous_info;
    my @stats_new = {};
    my @stats_old = {};
    my %status_hash;      


    # Open Log File.
    open(LOG, ">".$logfile)
      ||   die "$script_name could not open $logfile\n";
    print LOG "Backup_Type: supplement\n".
    print LOG "Start_Time: ".(strftime "%a_%b_%e_%H_%M_%S_%Y", localtime)."\n";

    # Load the file hash.
    print "Hashing the directory structure, this is time consuming.\n";
    print "Patience.\n";
    print LOG "File_hash_start: ".
                + (strftime "%a_%b_%e_%H_%M_%S_%Y", localtime)."\n";
    %files = Recurse([$drive], {match => '\.'});
    print LOG "File_hash finish: ".
        + (strftime "%a_%b_%e_%H_%M_%S_%Y", localtime)."\n";
       
    # Load the previous file statistics.
    open(TARBALLS, $tarball_tally_file) 
     || warn "Could not open $tarball_tally_file.\n";
    @previous_info = <TARBALLS>;
    close(TARBALLS);

    # Load a hash with this file information. 
    $length = @previous_info; 
    for(my $ii=0; $ii<$length; $ii++)
    {
      # Ignore commented lines.
      if(substr($previous_info[$ii],0,1) eq "#")
      { next; }
 
      @components = split(/ /, $previous_info[$ii]); 
      $type = $components[0];

      if($type eq "tarball:")
      {
	#print "Tarball entry.\n";
	next;
      }
      elsif($type eq "file:")
      {
	#print "File entry.\n";
	$file = $components[1];
  	chomp $file;
        @next_components = split(/ /, $previous_info[$ii+1]);
	$next = $next_components[0];
	shift @next_components;
        chomp $next_components[12];
	#print "File : $file\n";
        #print "Stats : @next_components\n";

	if($next eq "stats:")
 	{
          $status_hash{$file} = 
	   + sprintf("%s %s %s %s %s %s %s %s %s %s %s %s %s",@next_components);
	}
	else
	{
	  $status_hash{$file} = "0 0 0 0 0 0 0 0 0 0 0 0 0"; 
	}
 
      }
      elsif($type eq "stats:")
      {
	#print "Stats entry.\n";
	next;
      }
      else
      {
	die "Unexpected entry in $tarball_tally_file.\n";
      } 

    }

    print LOG "Command file writing start: ".
                + (strftime "%a_%b_%e_%H_%M_%S_%Y", localtime)."\n";

    # Open the necessary files.
    open(COMMAND, ">".$command_file)
     || die "Could not open the command file. \n";
 
    # Iterate through every file.
    foreach $dir (sort keys %files)
    {
      foreach $file (@{ $files{$dir} })
      {
        # Construct the original file's full path and throw it in the file.
        $full_path = sprintf("%s/%s", $dir, $file);
        @stats_old = split(/ /, $status_hash{$full_path});
        @stats_new = stat($full_path);

        print "$status_hash{$full_path}\n";
 	print "File: $full_path\n";
	print "Old: @stats_old\n";
	print "New: @stats_new\n";

        # Make stat comparison and add to command file as necessary. 
	if(($stats_old[7] != $stats_new[7]) || ($stats_old[9] != $stats_new[9]))
	{
          print COMMAND "$full_path\n";
	}
      }
    }
    print LOG "Command file writing end: ".
                + (strftime "%a_%b_%e_%H_%M_%S_%Y", localtime)."\n";
   
    # Close command file
    close COMMAND;

    # Create/reset the index file
    open(INDEX, ">$current_index_file")
      || die "Could not open $current_index_file\n";
    print INDEX "0";
    close INDEX;
 
    # Close the logfile.
    close LOG;
    
    #die "Catch before resume.";
  }

  ########### Create a new, level-zero backup.  ############
  sub zero  # Brrrr. 
  {
    print "Creating a new level-zero backup.\n";
   
    # File name strings as delivered via arguments to subroutine. 
    my $command_file =  $_[0];  
    my $current_index_file = $_[1]; 
    my $current_tarball_file = $_[2]; 
    my $drive = "/".$_[3];
    my $logfile = $_[4];
    my $no_shuffle = $_[5];
    my $rootdir = $_[6];
    my $tarball_tally_file = $_[7]; 

    # Subroutine variable declarations.
    my $cmd = "";
    my $dir = "none";
    my $file = "none";
    my %files;
    my $full_path = "none";

    # Ensure MSS directory structure
    &make_mss_structure($drive, $rootdir);

    # If $no_shuffle, don't shuffle.
    unless($no_shuffle)
    {
      # Perform local file rearrangement
      &shuffle_local($drive, $rootdir); 
    
      # Perform MSS file rearrangement
      &shuffle_mss($drive); 
    }

    # Open Log File.
    open(LOG, ">".$logfile)
      ||   die "$script_name could not open $logfile\n";
    print LOG "Backup_Type: zero\n".
    print LOG "Start_Time: ".(strftime "%a_%b_%e_%H_%M_%S_%Y", localtime)."\n";

    # Load the file hash.
    print "Hashing the directory structure, this is time consuming.\n";
    print "Patience.\n";
    print LOG "File_hash_start: ".
		+ (strftime "%a_%b_%e_%H_%M_%S_%Y", localtime)."\n";
    %files = Recurse([$drive], {match => '\.'});
    print LOG "File_hash finish: ".
	+ (strftime "%a_%b_%e_%H_%M_%S_%Y", localtime)."\n";
    

    print LOG "Command file writing start: ".
		+ (strftime "%a_%b_%e_%H_%M_%S_%Y", localtime)."\n";


    # Open the necessary files.
    open(COMMAND, ">".$command_file)
     || die "Could not open the command file. \n";

    # Iterate through every file. 
    foreach $dir (sort keys %files) 
    { 
      foreach $file (@{ $files{$dir} })
      {          
      	# Construct the original file's full path and throw it in the file.
      	$full_path = sprintf("%s/%s", $dir, $file);
      	print COMMAND "$full_path\n";
      }  
    }
    print LOG "Command file writing end: ".
		+ (strftime "%a_%b_%e_%H_%M_%S_%Y", localtime)."\n";
   
    # Close command file
    close COMMAND;

    # Create/reset the index file
    open(INDEX, ">$current_index_file")
      || die "Could not open $current_index_file\n";
    print INDEX "0";
    close INDEX;
  
    # Create and intialize the tarball number file
    open(NUM, ">$current_tarball_file");
    print NUM "0";
    close NUM;
 
    ## Create the tarball tally file
    #open(TALLY,">$tarball_tally_file"); 
    #print TALLY "\#Here follows a list of tarballs, their files, ".
#		+ "and those files' properties.\n";
    #close TALLY;
 
    # Close the logfile. 
    close LOG;

    # Remove any residual tarballs that might be lurking around.
    $drive = substr($drive,1,length($drive)-1); 
    $cmd = "rm ".$rootdir."/".$drive."/current/*".$drive."*.tar";
    print $cmd."\n";
    system($cmd);
 
  }
