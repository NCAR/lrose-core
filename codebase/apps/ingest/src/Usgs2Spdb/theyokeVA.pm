#!/usr/local/bin/perl
#
# Id: theyoke 33 2006-07-25 14:53:36Z mackers 
# http://www.mackers.com/projects/theyoke/
#
# TheYoke is an ultra-simple RSS aggregrator designed for use on the UNIX command line.
#
# Modified to use CAP (Common Alerting Protocol) feeds, Output to a log file,
# and call an application to save the data to Spdb
#
# Jason Craig November 2006
#
# Installation
#   1. Run with ./theyoke to create the configuration files and directories.
#   2. Add your RSS feed URLs to "$RAP_DATA_DIR/.theyoke/feeds".
#   3. Every subsequent execution will get new items in the list of feeds.
#

use strict;
use XML::RSS;
use XML::Simple;
use URI;
use LWP::UserAgent;
use Digest::MD5 qw(md5_base64);
use Encode qw(encode_utf8);
use Getopt::Long;
use Data::Dumper;
push @INC, $ENV{'HOME'} . "/rap/bin";
require 'xml_cap.pl';

my($USAGE) = "Usage: $0: [[--debug]|[-d]]+ [--test] [--description] [--logdata] [--stdout] [[--version] [-V]] [[--columns=int] [-c=int]] [--numfeeds=number] [--onlyfeeds=regexp] [--reversetime] [--application=app] [--appparams=params] [feedurl]...\n";
my $version = "1.23-baka";
my $config_dir = $ENV{'RAP_DATA_DIR'} . "/spdb/Usgs/.theyoke/";
my $feeds_dir = $config_dir . ".feeds/";
my $feeds_file = $config_dir . "feeds";
my $log_dir = $ENV{'RAP_DATA_DIR'} . "/spdb/Usgs/";
my $agent = "TheYoke/$version (+http://www.mackers.com/projects/theyoke/) ";
my @OUTPUT;
my @feed_urls;
my (%OPTIONS);
my $exit_val = 0;

Getopt::Long::Configure("bundling", "no_ignore_case", "no_auto_abbrev", "no_getopt_compat", "require_order");
GetOptions(\%OPTIONS, 'debug|d+', 'test', 'description', 'logdata', 'stdout', 'version|V+', 'columns|c=i', 'numfeeds=i', 'onlyfeeds=s', 'reversetime', 'application=s', 'appparams=s') || die $USAGE;



if ($OPTIONS{'version'}) {
    print "$0 version $version\n";
    exit(0);
}

### Check for files and dirs. If none, create ###
unless (-d $config_dir || $#ARGV >= 0) {
    mkdir ($config_dir) || die ("Couldn't create directory $config_dir");
}
unless (-d $feeds_dir || $#ARGV >= 0) {
    mkdir ($feeds_dir) || die ("Couldn't create directory $feeds_dir");
}
unless (-f $feeds_file || $#ARGV >= 0) {
    if (open(FEEDS, ">> $feeds_file")) {
	# TODO print feeds file content comments
	print FEEDS "";
	close FEEDS;
    }
    exit(0);
}

### Read feeds file ###
if ($#ARGV >= 0) {
    foreach (@ARGV) {
	my $u1 = URI->new($_);
	push(@feed_urls, $u1);
    }
} else {
    if (open(FEEDS, "< $feeds_file")) {
	while (<FEEDS>) {
	    next if (/^#/);
		     next unless (/\w/);
		     my $u1 = URI->new($_);
		     push (@feed_urls, $u1);
		 }
	} else {
	    print STDERR "theyoke: could not open $feeds_file\n";
	    exit(-1);
	}
    }

    if (scalar(@feed_urls) == 0) {
	print STDERR "theyoke: no feeds found. please enter some URLs in $feeds_file (or as command line argument)\n";
    }

### Create new files if necessary
    foreach my $feed_url (@feed_urls) {
	$feed_url = $feed_url->as_string;
	my $file_path = $feeds_dir . &get_checksum($feed_url);
	unless (-f $file_path || $#ARGV >= 0) {
	    print STDERR "theyoke: adding feed for $feed_url\n";
	    if (open (FEED, "> $file_path")) {
		print FEED "$feed_url\n0\nno_digest_yet\nno_title_yet\nno_etag_yet\n";
		close FEED;
	    } else {
		print STDERR "theyoke: couldn't write to $feed_url\n";
	    }
	}
    }
### Open the log file for the data
    if($OPTIONS{'logdata'}) {
	
	my @timeData = localtime(time);
	my $day = $timeData[3];
	my $month= $timeData[4] + 1;
	my $year = $timeData[5] + 1900;

	if($day < 10) {
	    $day = "0" . $day;
	}
	if($month < 10) {
	    $month = "0" . $month;
	}
	open(LOG, ">>${log_dir}theyokeVA.${year}${month}${day}.log") || die "Can't Data Log File: ${log_dir}theyokeVA.${year}${month}${day}.log ($!)\n";

    }
### Create the user agent ###
    my $ua = LWP::UserAgent->new(
				 env_proxy => 1,
				 keep_alive => 0,
				 timeout => 30,
				 agent => $agent,
				 );

### For each feed file ###
    my $count = 0;
    my $dont_have_content = 1;
    print STDERR "theyoke: Syndicating first $OPTIONS{'numfeeds'} feeds.\n" if ($OPTIONS{'debug'} && defined($OPTIONS{'numfeeds'}));
    foreach my $feed_url (@feed_urls) {
	last if (defined($OPTIONS{'numfeeds'}) && $count++ == $OPTIONS{'numfeeds'});

	if ($OPTIONS{'onlyfeeds'} && $feed_url !~ /$OPTIONS{'onlyfeeds'}/) {
	    print STDERR "theyoke: Skipping... not in /$OPTIONS{'onlyfeeds'}/\n" if ($OPTIONS{'debug'});
	    next;
	}
	@OUTPUT = ();

	### get the filename
	my $file_digest = &get_checksum($feed_url);
	my $file_digest_path = $feeds_dir . $file_digest;

	#### open the file
	my $previous_content_digest = "no_digest_yet";
	my $last_title = "no_title_yet";
	my $last_mod = 0;
	my $etag = "no_etag_yet";
	if ($#ARGV < 0) {
	    if (open(FEED, "< $file_digest_path")) {
		# 1st line: url
		my $this_url = <FEED>;
		# 2nd line: last modified system time
		$last_mod = <FEED>;
		chomp($last_mod);
		# 3rd line: previous checksum for whole body
		$previous_content_digest = <FEED>;
		chomp($previous_content_digest);
		# 4th line: previous checksum for last known item
		$last_title = <FEED>;
		chomp($last_title);
		# 5th line: etag
		$etag = <FEED>;
		chomp($etag);
		close FEED;
		unless (($previous_content_digest ne "") && ($last_title ne "") && ($etag ne "")) {
		    print STDERR "theyoke: $file_digest_path is corrupt or you're using a new version of theyoke. will regenerate next time...\n";
		    unlink $file_digest_path;
		    next;
		}
	    } else {
		die ("theyoke: couldn't open $file_digest_path");
	    }
	}

	### send request to see if not modified
	$| = 1;
	print STDERR "theyoke: Getting \"$feed_url\" - " if ($OPTIONS{'debug'});
	my $head = HTTP::Headers->new;
	$head->if_modified_since($last_mod);
	$head->push_header("If-None-Match" => $etag);
	my $req = HTTP::Request->new("GET", $feed_url, $head);
	print STDERR $req->as_string if ($OPTIONS{'debug'} > 1);
	my $resp = $ua->request($req);
	my $content;
	if ($resp->code == 304) {
	    print STDERR " got a 304, skipping\n" if ($OPTIONS{'debug'});
	    $| = 0;
	    next;
	} elsif ($resp->is_success) {
	    print STDERR " got " . $resp->code . "\n" if ($OPTIONS{'debug'});
	    $content = $resp->content();
	    $| = 0;
	    $content =~ s/’//g;   # This character showed up in a message and breaks everything
            # The following removes HTML tags, we only want XMl tags
	    if(index($content, "<content:encoded>") >= 0) {
		my $start = index($content, "<content:encoded>");
		my $end = index($content, "</content:encoded>", $start) + 18;
		while($start >= 0) {
		    $content = substr($content, 0, $start) . substr($content, $end);
		    $start = index($content, "<content:encoded>", $end);
		    $end = index($content, "</content:encoded>", $start) + 18;
		}
	    }
#	    print index($content, "<content:encoded>");
#	    print index($content, "</content:encoded>");
#	    $content =~ s#<content:encoded>.*?</content:encoded>#matched#g;      # removes everything between encoded tags
#	    $content =~ s/<br>//g;
#	    $content =~ s/<b>//g;
#	    $content =~ s/</b>//g;
#	    $content =~ s/</font>//g;
#	    $content =~ s/<font color='#[0-9]{6}'>//g;
	} else {
	    print STDERR "theyoke: \"$feed_url\": got " . $resp->code . ", skipping\n";
	    $| = 0;
	    next;
	}

	### skip if checksums match (i.e. head lied - no new content)
	my $new_last_title = "";
	my $new_content_digest = &get_checksum($content);
	if ($new_content_digest eq $previous_content_digest) {
	    print STDERR "theyoke: checksums match, skipping\n" if ($OPTIONS{'debug'});
	} else {

	    ### new content - parse the rss
	    my $newtitle = 0;
	    my $xml = new XML::Simple;
#	    my $rss = new XML::RSS;
	    my $rss = 0;
	    my $rss_title = "";
	    my $rss_description = "";
	    {
		# XML::RSS seems to always through a DIE
		#local $SIG{__DIE__} = sub { print STDERR "theyoke: RSS parser error on \"$feed_url\".\n"; };
		eval {
#		    $rss->parse($content);
		    $rss = $xml->XMLin($content, forcearray => ['items']);
		};
		if ($@) {
		    print STDERR "theyoke: RSS parser error on \"$feed_url\": $@\n";
		    next;
		}
	    }
#	    $rss_title = $rss->channel('title');
#	    $rss_description = $rss->channel('description');
	    $rss_title = $rss->{'channel'}->{'title'};
	    $rss_description = $rss->{'channel'}->{'description'};

	    ### check for no items
	    if (@{$rss->{'channel'}->{'item'}} == 0) {
		print STDERR "theyoke: no RSS items found in \"$feed_url\". bad RSS?\n";
		next;
	    }

	    ### check for no title
	    if ($rss_title eq "") {
		print STDERR "theyoke: no channel title found for \"$feed_url\". bad RSS?\n";
		next;
	    }

	    ### look for new items
	    foreach my $item (@{$rss->{'channel'}->{'item'}}) {
		my $this_description = $item->{'description'};
		my $this_title = $item->{'title'};
		my $this_link = $item->{'link'};
		my $wassname = "";
		if ($this_title ne "") {
		    $wassname = $this_title;
		} else {
		    print STDERR "theyoke: no message title found for item:\n";
		    print STDERR Dumper($item);
		    next;
		}
		my $this_wassname_digest = &get_checksum($wassname);
		if ($this_wassname_digest ne $last_title) {
		    # aha! new content

		    ## Detect if this is a RSS feed of CAP messages
		    if(index($rss_description, "CAP Alerts") >= 0 || 
		       index($rss_description, "Cap Alerts") >= 0 || 
		       index($rss_description, "CAP Messages") >= 0 ||
		       index($rss_description, "Cap Messages") >= 0) {

			### send request for associated CAP message
			$| = 1;
			print STDERR "theyoke: Getting \"$this_link\" - " if ($OPTIONS{'debug'});
			my $req2 = HTTP::Request->new("GET", $this_link, $head);
			print STDERR $req2->as_string if ($OPTIONS{'debug'} > 1);
			my $resp2 = $ua->request($req2);
			my $content2;
			if ($resp2->code == 304) {
			    print STDERR " got a 304, skipping\n" if ($OPTIONS{'debug'});
			    $| = 0;
			    next;
			} elsif ($resp2->is_success) {
			    print STDERR " got " . $resp2->code . "\n" if ($OPTIONS{'debug'});
			    $content2 = $resp2->content();
			    $| = 0;
			} else {
			    print STDERR "theyoke: \"$this_link\": got " . $resp2->code . ", skipping\n";
			    $| = 0;
			    next;
			}
			
			my $cap = 0;
			{
			    eval {
				$cap = xml_cap::read($content2);
			    };
			    if ($@) {
				print STDERR "theyoke: XML parser error on \"$this_link\": $@\n";
				next;
			    }
			    eval {
				xml_cap::verify($cap);
			      };
			    if ($@) {
				print STDERR "theyoke: CAP parser error on \"$this_link\": $@\n";
				next;
			    }
			}
			foreach my $cap_item (@{$cap->{'cap:info'}}) {
			    my $this_info = "";
			    my @args = 0;
			    
			    if($cap_item->{'cap:event'} eq "Earthquake") {
				my ($sent, $title, $id, $sender, $version, $magnitude, $magnitudeType, $time, $lat, $lon, $depth, $Herror, $Verror, $stations, $phases, $distance, $RMSerror, $azimuthal) = xml_cap::parse_Earthquake($cap);
				
				push(@args, ("-Earthquake", $sent, $title, $id, $sender, $version, $magnitude, $magnitudeType, $time, $lat, $lon, $depth, $Herror, $Verror, $stations, $phases, $distance, $RMSerror, $azimuthal));
				$this_info = "-Earthquake \"$sent\" \"$title\" \"$id\" \"$sender\" \"$version\" \"$magnitude\" \"$magnitudeType\" \"$time\" \"$lat\" \"$lon\" \"$depth\" \"$Herror\" \"$Verror\" \"$stations\" \"$phases\" \"$distance\" \"$RMSerror\" \"$azimuthal\"";
				
			    } else {
				if (index($cap_item->{'cap:event'}, "Volcanic") >= 0 || index($cap_item->{'cap:event'}, "Seismicity") >= 0) {
				    
				    my ($sent, $title, $sender, $lat, $lon, $alt, $code, $time, $id) = xml_cap::parse_VA($cap);
				    
				    push(@args, ("-Volcano", $sent, $title, $sender, $lat, $lon, $alt, $code, $time, $id));
				    $this_info = "-Volcano \"$sent\" \"$title\" \"$sender\" \"$lat\" \"$lon\" \"$alt\" \"$code\" \"$time\" \"$id\"";
				    
				} else {
				    print STDERR "Unknown event: " . $cap_item->{'cap:event'} . "\n";
				    next;
				}
			    }
			    if($OPTIONS{'application'}) {
				if($OPTIONS{'appparams'}) {
				    system(($OPTIONS{'application'}, "-params", $OPTIONS{'appparams'}, @args)) == 0
					or die "system call ", $OPTIONS{'application'}, " failed: $?";
				} else {
				    system($OPTIONS{'application'}, @args) == 0
					or die "system call ", $OPTIONS{'application'}, " failed: $?";
				}
			    }
			    
			    $this_info =~ s/,//g;
			    $this_info =~ s/\" \"/, /g;
			    $this_info =~ s/\"//g;
			    push(@OUTPUT, "$this_info\n");
			    
			}
			
			$dont_have_content = 0;
			# save latest title
			if ($new_last_title eq "") {
			    $new_last_title = $this_wassname_digest;
			}
		    } else {
			# Non CAP Messages, Must parse RSS feed for data
			my $this_info = "";
			my @args = 0;

			if(index($rss_title, "Global Disaster Alert and Coordination System") >= 0) {

			    my ($sent, $title, $sender, $lat, $lon, $alt, $code, $time, $id) = parse_GDACS($item);
			    
			    push(@args, ("-Volcano", $sent, $title, $sender, $lat, $lon, $alt, $code, $time, $id));
			    $this_info = "-Volcano \"$sent\" \"$title\" \"$sender\" \"$lat\" \"$lon\" \"$alt\" \"$code\" \"$time\" \"$id\"";
			    
			} else {
			  if(index($rss_title, "RSOE HAVARIA - Volcano Monitoring") >= 0) {
			    my ($sent, $title, $sender, $lat, $lon, $alt, $code, $time, $id) = parse_RSOE($item);

			    push(@args, ("-Volcano", $sent, $title, $sender, $lat, $lon, $alt, $code, $time, $id));
			    $this_info = "-Volcano \"$sent\" \"$title\" \"$sender\" \"$lat\" \"$lon\" \"$alt\" \"$code\" \"$time\" \"$id\"";
			    
			  } else {
			    print STDERR "Unknown RSS Feed, unable to parse. RSS Title: " . $rss_title . "\n";
			    next;
			  }
		        }
			
			if($OPTIONS{'application'}) {
			    if($OPTIONS{'appparams'}) {
				system(($OPTIONS{'application'}, "-params", $OPTIONS{'appparams'}, @args)) == 0
				    or die "system call ", $OPTIONS{'application'}, " failed: $?";
			    } else {
				system($OPTIONS{'application'}, @args) == 0
				    or die "system call ", $OPTIONS{'application'}, " failed: $?";
			    }
			}
			$this_info =~ s/,//g;
			$this_info =~ s/\" \"/, /g;
			$this_info =~ s/\"//g;
			push(@OUTPUT, "$this_info\n");
			    
		    }
		    # save latest title
		    if ($new_last_title eq "") {
			$new_last_title = $this_wassname_digest;
		    }
		} else {
		    last;
		}
	    }

	    # check for badness
	    if ($new_content_digest eq "") {
		print STDERR "theyoke: empty badness for new_content_digest on $feed_url\n";
		next;
	    }
	}

	# check for changed rss file but not changed headings
	if ($new_last_title eq "") {
	    if ($OPTIONS{'debug'}) {
		if ($new_content_digest ne $previous_content_digest) {
		    print STDERR "theyoke: checksums don't match, but ";
		} else {
		    print STDERR "theyoke: ";
		}
		print STDERR "no new headlines from $feed_url\n";
	    }
	    $new_last_title = $last_title;
	}

	if ($#OUTPUT >= 0 && $OPTIONS{'logdata'}) {
	    if ($OPTIONS{'reversetime'}) {
		print LOG reverse @OUTPUT;
	    } else {
		print LOG @OUTPUT;
	    }
	}
	if ($#OUTPUT >= 0 && $OPTIONS{'stdout'}) {
	    if ($OPTIONS{'reversetime'}) {
		print reverse @OUTPUT;
	    } else {
		print @OUTPUT;
	    }
	}

	### save checksum
	if ($ARGV < 1 && !$OPTIONS{'test'}) {
	    if (open(FEED, "> $file_digest_path")) {
		# url
		print FEED $feed_url . "\n";
		# last mod
		if ($resp->last_modified) {
		    print FEED $resp->last_modified . "\n";
		} else {
		    print FEED $resp->date . "\n";
		}
		# content checksum
		print FEED $new_content_digest . "\n";
		# title checksum
		print FEED $new_last_title . "\n";
		# etag
		if ($resp->header("ETag")) {
		    print FEED $resp->header("ETag") . "\n";
		} else {
		    print FEED "no_etag\n";
		}
		close FEED;
	    } else {
		die ("Couldn't write to $file_digest_path");
	    }
	}
    }

    $exit_val = 2 if $dont_have_content;

    exit($exit_val);

    sub get_checksum {
	my $tent = md5_base64(encode_utf8($_[0]));
	$tent =~ s/\W/_/g;
	print STDERR $_[0] . " encoding as $tent\n" if ($OPTIONS{'debug'} > 1);
	return $tent;
    }

sub parse_GDACS
{
  my $ref = @_[0];

  my $sent = "-9999.0";
  my $sender = "UNKNOWN";
  my $title = "UNKNOWN";
  my $color_code = "UNKNOWN";
  my $effective = "-9999.0";
  my $lat = "-9999.0";
  my $lon = "-9999.0";
  my $alt = "-9999.0";
  my $id = "-9999";

  $title = $ref->{'VolcanoName'} . ", " . $ref->{'gdas:country'};
  $sent = $ref->{'pubDate'};
  $effective = $ref->{'asgard:date'};
  $sender = "Global Disaster Alert and Coordination System";
  $color_code = $ref->{'gdas:alertLevel'};
  $lat = $ref->{'geo:lat'};
  $lon = $ref->{'geo:long'};
  $id = $ref->{'asgard:ID'};

  $sent = parse_GDACS_time($sent);
  $effective = parse_GDACS_time($effective);

  return $sent, $title, $sender, $lat, $lon, $alt, $color_code, $effective, $id;
}

sub parse_RSOE
{
  my $ref = @_[0];

  my $sent = "-9999.0";
  my $sender = "UNKNOWN";
  my $title = "UNKNOWN";
  my $color_code = "UNKNOWN";
  my $effective = "-9999.0";
  my $lat = "-9999.0";
  my $lon = "-9999.0";
  my $alt = "-9999.0";
  my $id = "-9999";

  my $start = index($ref->{'title'}, "Volcano ")+8;
  my $end = index($ref->{'title'}, "  ", $start);
  $title = substr($ref->{'title'}, $start, $end - $start);
  $sent = $ref->{'pubDate'};
  $sender = "Southwest Volcano Research Centre";

  $start = index($ref->{'description'}, "Status: ")+8;
  $end = index($ref->{'description'}, ",", $start);
  $color_code = substr($ref->{'description'}, $start, $end - $start);
  $color_code =~ s/  / /g;

  if(index($color_code, "Alert Level") >= 0) {
    if(index($color_code, "Alert Level 0") >= 0) {
      $color_code = "Green";
    } else {
      if(index($color_code, "Alert Level 1") >= 0) {
	$color_code = "Yellow";
      } else {
	if(index($color_code, "Alert Level 2") >= 0 || index($color_code, "Alert Level 3") >= 0 || index($color_code, "Alert Level 4") >= 0) {
	  $color_code = "Orange";
        } else {
	  if(index($color_code, "Alert Level 5") >= 0) {
	    $color_code = "Red";
	  }
        }
      }
    }
  }

  if(index($color_code, "  ") >= 0){
      $color_code = substr($color_code, 0, index($color_code, "  "));
  }


  $end = index($ref->{'georss:where'}->{'gml:Point'}->{'gml:pos'}, " ");
  $lat = substr($ref->{'georss:where'}->{'gml:Point'}->{'gml:pos'}, 0, $end);
  $lon = substr($ref->{'georss:where'}->{'gml:Point'}->{'gml:pos'}, $end+1);
  $start = index($ref->{'link'}, "void=")+5;
  $end = index($ref->{'link'}, "&", $start);
  $id = substr($ref->{'link'}, $start, $end - $start);

  $sent = parse_GDACS_time($sent);
  $effective = $sent;

  return $sent, $title, $sender, $lat, $lon, $alt, $color_code, $effective, $id;
}

sub parse_GDACS_time()
{
  my $time = @_[0];
  my %months = 0;
  keys(%months) = 12;
  $months{'Jan'} = '01';
  $months{'Feb'} = '02';
  $months{'Mar'} = '03';
  $months{'Apr'} = '04';
  $months{'May'} = '05';
  $months{'Jun'} = '06';
  $months{'Jul'} = '07';
  $months{'Aug'} = '08';
  $months{'Sep'} = '09';
  $months{'Oct'} = '10';
  $months{'Nov'} = '11';
  $months{'Dec'} = '12';
  
  my @time_arr  = split(/ /, $time);

  my $day = $time_arr[1];
  my $month_txt = $time_arr[2];
  my $year = $time_arr[3];
  my $hour_min = $time_arr[4];
  my $time_zone = $time_arr[5];

  my $month = $months{$month_txt};

  if($day < 10) {
      $day = "0" . $day;
  }

  my $zone_diff = "+00";
  if(index($time_zone, '+') >= 0) {
    $zone_diff =  substr($time_zone, index($time_zone, '+')+1);
    if($zone_diff >= 10) {
	$zone_diff = "+" . $zone_diff;
    } else {
	$zone_diff = "+0" . $zone_diff;
    }
  } else {
      if(index($time_zone, '-') >= 0) {
	  $zone_diff =  substr($time_zone, index($time_zone, '+'));
	  if($zone_diff >= 10) {
	      $zone_diff = "-" . $zone_diff;
	  } else {
	      $zone_diff = "-0" . $zone_diff;
	  }
      }
  }


  return $year . "-" . $month . "-" . $day . "T" . $hour_min . ":00" . $zone_diff . ":00";
}
