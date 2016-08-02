package xml_cap;
# Package based on xml::Simple that handles 
# CAP (Comman Alerting Protocol) XML messages
#
# Pass a file or string to read() which will return a reference to a hash of the xml
# Then pass the reference to verify() to verify we have a valid CAP format
# Can then call print(), to print the message or:
# parse_VA() parses and returns data from a USGS Volcanic Alert message
# parse_Earthquake() parses and returns data from a USGS Earthquake message
#
#  Jason Craig November 2006

use XML::Simple;
use Data::Dumper;
BEGIN {
 # create object
 $xml = new XML::Simple;
}

sub read
{
  $file_or_string = @_[0];

  @listItems = ['cap:codes', 'cap:info', 'cap:category', 'cap:eventCode', 'cap:parameter', 'cap:resource', 'cap:area', 'cap:polygon', 'cap:circle', 'cap:geocode', 'codes', 'info', 'category', 'eventCode', 'parameter', 'resource', 'area', 'polygon', 'circle', 'geocode'];
  $data = $xml->XMLin($file_or_string, forcearray => @listItems);

  if(defined $data->{'identifier'} && !defined $data->{'cap:identifier'}) {
    $ref = convert_hash_to_caphash($data);
    $data = $ref;
  }

  return $data;
}

sub verify
{
  my $ref = @_[0];
  if (ref($ref) ne 'HASH') {
    die "print_CAP_XML Error: Must pass reference to Cap Xml hash. Received: ${ref}\n";
  }
  %data = %$ref;

  if(!defined $data->{'xmlns:cap'}) {
    die "No cap version identifier found \n";
  }
  if($data->{'xmlns:cap'} ne "http://www.incident.com/cap/1.0")
  {
    die "Unrecognized cap version: ", $data->{'xmlns:cap'}, "\n";
  }
  if( !defined $data->{'cap:identifier'}) {
    die "Cannot find cap identifier tag\n";
  }
  if( !defined $data->{'cap:sender'}) {
    die "Cannot find cap sender tag\n";
  }
  if( !defined $data->{'cap:sent'}) {
    die "Cannot find cap sent tag\n";
  }
  if( !defined $data->{'cap:status'}) {
    die "Cannot find cap status tag\n";
  }
  if( !defined $data->{'cap:scope'}) {
    die "Cannot find cap scope tag\n";
  }
  if( !defined $data->{'cap:msgType'}) {
    die "Cannot find cap msgType tag\n";
  }
  if( !defined $data->{'cap:info'}) {
    die "Cannot find cap info tag\n";
  }
  foreach $info (@{$data->{'cap:info'}})
  {
    if( !defined $info->{'cap:event'}) {
      die "Cannot find cap event tag\n";
    }
    if( !defined $info->{'cap:urgency'}) {
      die "Cannot find cap urgency tag\n";
    }
    if( !defined $info->{'cap:severity'}) {
      die "Cannot find cap severity tag\n";
    }
    if( !defined $info->{'cap:certainty'}) {
      die "Cannot find cap certainty tag\n";
    }
    if(defined $info->{'cap:resource'}) {
      foreach $resource (@{$info->{'cap:resource'}})
      {
        if( !defined $resource->{'cap:resourceDesc'}) {
	  die "Cannot find cap resourceDesc tag\n";
        }
      }
    }
    if(defined $info->{'cap:area'}) {
      foreach $area (@{$info->{'cap:area'}})
      {
        if( !defined $area->{'cap:areaDesc'}) {
	  die "Cannot find cap areaDesc tag\n";
        }
      }
    }
  }
}


sub convert_hash_to_caphash
{
  my $ref = @_[0];
  if (ref($ref) ne 'HASH') {
    die "convert_hash_to_cap Error: Must pass reference to Cap Xml hash. Received: ${ref}\n";
    return 1;
  }
  %data = %$ref;

  my %new_data = ();
  if($data->{'xmlns'} eq "http://www.incident.com/cap/1.0" || $data->{'xmlns:cap'} eq "http://www.incident.com/cap/1.0")
  {
    %new_data = ('xmlns:cap', "http://www.incident.com/cap/1.0");
    while (($key, $value) = each(%data)) {
      if($key eq 'xmlns') {
	next;
      }
      $key = 'cap:' . $key;

      if (ref($value) eq "ARRAY") {
	my @array = ();
	foreach $val (@{$value}) {
	  if (ref($val) eq "HASH") {
	    my %new_data2 = ();
	    while (($key2, $value2) = each(%$val)) {
	      $key2 = 'cap:' . $key2;

	      if (ref($value2) eq "ARRAY") {
	        my @array2 = ();
		foreach $val2 (@{$value2}) {
		  if (ref($val2) eq "HASH") {
		    my %new_data3 = ();
		    while (($key3, $value3) = each(%$val2)) {
		      $key3 = 'cap:' . $key3;

		      if (ref($value3) eq "ARRAY") {
		        my @array3 = ();
			foreach $val3 (@{$value3}) {
			  if (ref($val3) eq "HASH") {
			    my %new_data4 = ();
			    while (($key4, $value4) = each(%$val3)) {
			      $key4 = 'cap:' . $key4;

			      if (ref($value4) eq "ARRAY") {
			        die "convert_hash_to_cap Error: Cannot convert more than 4 layers deep\n";
			      } else {
			        $new_data4{$key4} = $value4;
			      }
			    }
		            push(@array3, \%new_data4);
			  } else {
			    push(@array3, $val3);
			  }
			}
		        $new_data3{$key3} = \@array3;
		      } else {
		        $new_data3{$key3} = $value3;
		      }
		    }
		    push(@array2, \%new_data3);
	          } else {
		    push(@array2, $val2);
	          }
  	        }
	        $new_data2{$key2} = \@array2;
	      } else {
	        $new_data2{$key2} = $value2;
	      }
	    }
	    push(@array, \%new_data2);
  	  } else {
	    push(@array, $value);
	  }
	}
	$new_data{$key} = \@array;
      } else {
	$new_data{$key} = $value;
      }
    }
  }
  return \%new_data;
}


sub print
{
  my $ref = @_[0];
  if (ref($ref) ne 'HASH') {
    die "print_CAP_XML Error: Must pass reference to Cap Xml hash. Received: ${ref}\n";
    return 1;
  }
  %data = %$ref;

  if($data->{'xmlns:cap'} eq "http://www.incident.com/cap/1.0")
  {
    print "CAP Version 1.0:\n";
    print "\tMessage ID: ", $data->{'cap:identifier'}, "\n";
    print "\tSender ID: ", $data->{'cap:sender'}, "\n";
    print "\tSent Date: ", $data->{'cap:sent'}, "\n";
    print "\tStatus: ", $data->{'cap:status'}, "\n";
    print "\tScope: ", $data->{'cap:scope'}, "\n";
    print "\tType: ", $data->{'cap:msgType'}, "\n";
    print "\tPassword: ", $data->{'cap:password'}, "\n" if defined $data->{'cap:password'};
    print "\tOperator/Device ID: ", $data->{'cap:source'}, "\n" if defined $data->{'cap:source'};
    print "\tRestriction: ", $data->{'cap:restriction'}, "\n" if defined $data->{'cap:restriction'};
    print "\tAddresses: ", $data->{'cap:addresses'}, "\n" if defined $data->{'cap:addresses'};
    if(defined $data->{'cap:codes'}) {
      foreach $code (@{$data->{'cap:codes'}}) {
	  print "\tHandling Code: ", $code, "\n";
      }
    }
    print "\tNote: ", $data->{'cap:note'}, "\n" if defined $data->{'cap:note'};
    print "\tReference ID: ", $data->{'cap:references'}, "\n" if defined $data->{'cap:references'};
    print "\tIncident IDs: ", $data->{'cap:incidents'}, "\n" if defined $data->{'cap:incidents'};
    
    foreach $info (@{$data->{'cap:info'}})
    {
      print "\tInfo:\n";
      print "\t\tEvent Type: ", $info->{'cap:event'}, "\n";
      print "\t\tUrgency: ", $info->{'cap:urgency'}, "\n";
      print "\t\tSeverity: ", $info->{'cap:severity'}, "\n";
      print "\t\tCertainty: ", $info->{'cap:certainty'}, "\n";
      if(defined $info->{'cap:category'}) {
	foreach $category (@{$info->{'cap:category'}}) {
	    print "\t\tEvent Category: ", $category, "\n";
	}
      }
      print "\t\tLanguage: ", $info->{'cap:language'}, "\n" if defined $info->{'cap:language'};
      print "\t\tAudience: ", $info->{'cap:audience'}, "\n" if defined $info->{'cap:audience'};
      if(defined $info->{'cap:eventCode'}) {
	foreach $eventCode (@{$info->{'cap:eventCode'}}) {
	  print "\t\tTargeting Code: ", $eventCode, "\n";
        }
      }
      print "\t\tEffective Date: ", $info->{'cap:effective'}, "\n" if defined $info->{'cap:effective'};
      print "\t\tOnset Date: ", $info->{'cap:onset'}, "\n" if defined $info->{'cap:onset'};
      print "\t\tExpiration Date: ", $info->{'cap:expires'}, "\n" if defined $info->{'cap:expires'};
      print "\t\tSender Name: ", $info->{'cap:senderName'}, "\n" if defined $info->{'cap:senderName'};
      print "\t\tHeadline: ", $info->{'cap:headline'}, "\n" if defined $info->{'cap:headline'};
      print "\t\tEvent Description: ", $info->{'cap:description'}, "\n" if defined $info->{'cap:description'};
      print "\t\tInstructions: ", $info->{'cap:instruction'}, "\n" if defined $info->{'cap:instruction'};
      print "\t\tInformation URL: ", $info->{'cap:web'}, "\n" if defined $info->{'cap:web'};
      print "\t\tContact Info: ", $info->{'cap:contact'}, "\n" if defined $info->{'cap:contact'};
      if(defined $info->{'cap:parameter'}) {
	foreach $parameter (@{$info->{'cap:parameter'}}) {
	  print "\t\tParameter: ", $parameter, "\n";
        }
      }
      if(defined $info->{'cap:resource'}) {
	foreach $resource (@{$info->{'cap:resource'}}) {
	  print "\t\tResource:\n";
	  print "\t\t\tDescription: ", $resource->{'cap:description'}, "\n" if defined $resource->{'cap:description'};
	  print "\t\t\tMIME Type: ", $resource->{'cap:mimeType'}, "\n" if defined $resource->{'cap:mimeType'};
	  print "\t\t\tSize: ", $resource->{'cap:size'}, "\n" if defined $resource->{'cap:size'};
	  print "\t\t\tURI: ", $resource->{'cap:uri'}, "\n" if defined $resource->{'cap:uri'};
	  print "\t\t\tDigest: ", $resource->{'cap:digest'}, "\n" if defined $resource->{'cap:digest'};
        }
      }

      foreach $area (@{$info->{'cap:area'}})
      {
	print "\t\tArea:\n";
	print "\t\t\tArea Desc: ", $area->{'cap:areaDesc'}, "\n";
	if(defined $area->{'cap:polygon'}) {
	  foreach $polygon (@{$area->{'cap:polygon'}}) {
	    print "\t\t\tArea Polygon: ", $polygon, "\n";
          }
        }
	if(defined $area->{'cap:circle'}) {
	  foreach $circle (@{$area->{'cap:circle'}}) {
	    print "\t\t\tArea Point-and-Radius: ", $circle, "\n";
          }
        }
	if(defined $area->{'cap:geocode'}) {
	  foreach $geocode (@{$area->{'cap:geocode'}}) {
	    print "\t\t\tGeographic Code: ", $geocode, "\n";
          }
        }
	print "\t\t\tAltitude: ", $area->{'cap:altitude'}, "\n" if defined $area->{'cap:altitude'};
	print "\t\t\tCeiling: ", $area->{'cap:ceiling'}, "\n" if defined $area->{'cap:ceiling'};
      }
    }
  } else {
    if(defined $area->{'xmlns:cap'}) {
      die "Unrecognized cap version: ", $data->{'xmlns:cap'}, "\n";
    } else {
      die "Cannot find xmlns:cap tag, not a Cap File\n";	
    }
  }
  return 0;
}

sub parse_VA
{
  my $ref = @_[0];
  %data = %$ref;

  $sent = "-9999.0";
  $sender = "UNKNOWN";
  $title = "UNKNOWN";
  $color_code = "UNKNOWN";
  $effective = "-9999.0";
  $lat = "-9999.0";
  $lon = "-9999.0";
  $alt = "-9999.0";
  $id = "-9999";

  $sent = $data->{'cap:sent'};
  foreach $info (@{$data->{'cap:info'}}) {
    $effective = $info->{'cap:effective'};
    $sender = $info->{'cap:senderName'};
    foreach $parameter (@{$info->{'cap:parameter'}}) {
      if(index($parameter, 'Color Code=') >= 0) {
	$color_code = substr($parameter, index($parameter, 'Color Code=')+11);
      }
    }
    foreach $area (@{$info->{'cap:area'}}) {
	$end = index($area->{'cap:areaDesc'}, "(")-1;
	$title = substr($area->{'cap:areaDesc'}, 0, $end);

	$start = index($area->{'cap:areaDesc'}, ", ")+2;
	$end = index($area->{'cap:areaDesc'}, " ", $start);
	$lat = substr($area->{'cap:areaDesc'}, $start, $end - $start);

	$start = $end + 1;
	$end = index($area->{'cap:areaDesc'}, ", ", $end);
	$lon = substr($area->{'cap:areaDesc'}, $start, $end - $start);

	$start = rindex($area->{'cap:areaDesc'}, "(")+1;
	$end = rindex($area->{'cap:areaDesc'}, ")")-2;
	$alt = substr($area->{'cap:areaDesc'}, $start, $end - $start);

	foreach $code (@{$area->{'cap:geocode'}}) {
	  $start = index($code, "CAVW=")+5;
	  $end = index($code, "-", $start);
	  $id = substr($code, $start, $end - $start);
	  $start = $end+1;
	  $id = $id . substr($code, $start);
        }
      }
  }
  if(index($lat, "°N") >= 0) {
      $lat = substr($lat, 0, index($lat, "°N"));
  } else {
    if(index($lat, "°S") >= 0) {
      $lat = "-" . substr($lat, 0, index($lat, "°S"));
    } else {
      if(index($lat, "N") >= 0) {
	$lat = substr($lat, 0, index($lat, "N"));
      } else {
	if(index($lat, "S") >= 0) {
	  $lat = "-" . substr($lat, 0, index($lat, "S"));
        } else {
	  die "Error parsing latitude: $lat\n";
        }
      }
    }
  }
  if(index($lon, "°E") >= 0) {
      $lon = substr($lon, 0, index($lon, "°E"));
  } else {
    if(index($lon, "°W") >= 0) {
      $lon = "-" . substr($lon, 0, index($lon, "°W"));
    } else {
      if(index($lon, "E") >= 0) {
	$lon = substr($lon, 0, index($lon, "E"));
      } else {
	if(index($lon, "W") >= 0) {
	  $lon = "-" . substr($lon, 0, index($lon, "W"));
        } else {
	  print STDERR "Error parsing lonitude: $lon\n";
        }
      }
    }
  }
  return $sent, $title, $sender, $lat, $lon, $alt, $color_code, $effective, $id;
}

sub parse_Earthquake
{
  my $ref = @_[0];
  %data = %$ref;

  $sent = "-9999.0";
  $title = "UNKNOWN";
  $id = "-9999";
  $sender = "UNKNOWN";
  $version = "-9999";
  $magnitude = "-9999.0";
  $magnitudeType = "z";
  $time = "-9999.0";
  $lat = "-9999.0";
  $lon = "-9999.0";
  $depth = "-9999.0";
  $Herror = "-9999.0";
  $Verror = "-9999.0";
  $stations = "-9999";
  $phases = "-9999";
  $distance = "-9999.0";
  $RMSerror = "-9999.0";
  $azimuthal = "-9999.0";

  $sent = $data->{'cap:sent'};
  foreach $info (@{$data->{'cap:info'}}) {
    $sender = $info->{'cap:senderName'};

    foreach $parameter (@{$info->{'cap:parameter'}}) {
      if(index($parameter, 'EventTime=') >= 0) {
	$time = substr($parameter, index($parameter, 'EventTime=')+10);
      }
      if(index($parameter, 'EventIDKey=') >= 0) {
	$id = substr($parameter, index($parameter, 'EventIDKey=')+11);
      }
      if(index($parameter, 'Version=') >= 0) {
	$version = substr($parameter, index($parameter, 'Version=')+8);
      }
      if(index($parameter, 'Magnitude=') >= 0) {
	$magnitude = substr($parameter, index($parameter, 'Magnitude=')+10);
      }
      if(index($parameter, 'MagnitudeType=') >= 0) {
	$magnitudeType = substr($parameter, index($parameter, 'MagnitudeType=')+14);
      }
      if(index($parameter, 'Depth=') >= 0) {
	$start = index($parameter, 'Depth=')+6;
	$end = index($parameter, 'km')-1;
	$depth = substr($parameter, $start, $end - $start);
      }
      if(index($parameter, 'HorizontalError=') >= 0) {
	$start = index($parameter, 'HorizontalError=')+16;
	$end = index($parameter, 'km')-1;
	$Herror = substr($parameter, $start, $end - $start);
      }
      if(index($parameter, 'VerticalError=') >= 0) {
	$start = index($parameter, 'VerticalError=')+14;
	$end = index($parameter, 'km')-1;
	$Verror = substr($parameter, $start, $end - $start);
      }
      if(index($parameter, 'NumStations=') >= 0) {
	$stations = substr($parameter, index($parameter, 'NumStations=')+12);
      }
      if(index($parameter, 'NumPhases=') >= 0) {
	$phases = substr($parameter, index($parameter, 'NumPhases=')+10);
      }
      if(index($parameter, 'MinDistance=') >= 0) {
	$start = index($parameter, 'MinDistance=')+12;
	$end = index($parameter, 'km')-1;
	$distance = substr($parameter, $start, $end - $start);
      }
      if(index($parameter, 'RMSTimeError=') >= 0) {
	$start = index($parameter, 'RMSTimeError=')+13;
	$end = index($parameter, 'seconds')-1;
	$RMSerror = substr($parameter, $start, $end - $start);
      }
      if(index($parameter, 'AzimuthalGap=') >= 0) {
	$start = index($parameter, 'AzimuthalGap=')+13;
	$end = index($parameter, 'degrees')-1;
	$azimuthal = substr($parameter, $start, $end - $start);
      }
    }
    $start = index($info->{'cap:description'}, "near") +5;
    $end = rindex($info->{'cap:description'}, "UTC");
    $end = rindex($info->{'cap:description'}, "at", $end) -1;
    $title = substr($info->{'cap:description'}, $start, $end - $start);

    foreach $area (@{$info->{'cap:area'}}) {
      foreach $circle (@{$area->{'cap:circle'}}) {
	  $end = index($circle, ",")-1;
	  $lat = substr($circle, 0, $end);
	  $start = $end +2;
	  $end = index($circle, " ", $start);
	  $lon = substr($circle, $start, $end - $start);
      }
    }
  }
  return $sent, $title, $id, $sender, $version, $magnitude, $magnitudeType, $time, $lat, $lon, $depth, $Herror, $Verror, $stations, $phases, $distance, $RMSerror, $azimuthal;
}


return 1;
END { }
