#! /usr/bin/perl
#
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 2011
# ** University Corporation for Atmospheric Research(UCAR)
# ** National Center for Atmospheric Research(NCAR)
# ** Research Applications Program(RAP)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# ** 2011/10/28 0:2:45
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
#
# This module contains Perl routine(s) for looking up stations from the
# aero_stations family of tables in our MySQL database.
#
# @author  Arnaud Dumont
# --------------------------------------------------------------------
package    StationUtils;
require    Exporter;
require 5.002;
@ISA = qw(Exporter);
@EXPORT = qw(init getStations getStationsLenient getStationId stationInfo_lookup);

use strict;
use DbConnect;
use Data::Dumper;  #Helpful in debuggging and looking at data structures

our $dbName = "weather";
our ($dbTable, $dbHandle, $sql, $sth);
our $debug = 0;
our $verbose = 0;
our $status = 0;

# Put any initialization of a customized StationUtils here
sub init {}


=TESTS
&PrintResults( &getStations( "den kden ord", "ASOS METAR airport", "lat lon airport_id" ) ); 
&PrintResults( &getStations( "den kden ord", "ASOS METAR navaid vor airport", "elevation" ) ); 
&PrintResults( &getStations( "den kden ord", "notam" ) ); 
#print &getStationId( "KDEN", "METAR asos awos SA airport" );

&PrintResults( &getStationId( "CWZG", "METAR ASOS AWOS SA AIRPORT", "" ) ); 
&PrintResults( &getStationId( "WZG", "METAR ASOS AWOS SA AIRPORT", "" ) ); 
StationUtil
&PrintResults( &getStations( "AGGG AT01 AZUH CTRA CWZG CYWH DAMR DATF DIAV DIST DUPE DXLK EDEU EDID EDIE EDKS EFTN EGTI EGUA EGVJ EGWZ EGXS EHFS EHSB EIDB ENEB ENFB ENSV EPQT FAJB FALE FALZ FBDR FBGR FCPD FDBB FDJR FDLM FDMX FDND FDNY FDOT FDPP FDUS FDVV FPPA FUDG FWGL GLRC GMOY GOTY GQNP GQPH GQPV GVPR HERN KBAN KCGX KHAT KHBB KHME KHSS KMQT KQAV KQAZ KQEB KQEL KQER KQEV KQEY KQFS KQFX KQHA KQMS KQPA KQPB KQTS KQTW KQVP KQZ4 LDDV LDKT LEDA LGAZ LIEU LIIH LIIJ LIIZ LIQE LIRO LIVA LKIB LKTT LLJR LOWH LSZD LTFG LUCC LURN LYDU LYLJ LYMB LYOS LYPL LYPZ LYRI LYSA LYSK LYZA LYZD LZKC MASA MGCP MGMT MGTU MKBK MMMS MPPA MTAR MTAR MTEG MTTP MTTP MYEC NSFU NSFU OAAB OABS OBAH OBBQ OBBQ OBSC OBZI OEAO OEAR OEML OEMN OEMU OEOA OEOF OEOH OEOS OEOT OERC OERG OERL OERX OESM OICJ OIGK OIIX OIKO OIMQ OINK OISM OJJR OKBC OKTK OKZK OMAU OMDW OMDZ OMMA OMMS OMSA OMSQ OMSR OSAS OSGN OTMA OTMS OYAG OYAM OYDM OYHJ OYJA OYKD OYMT OYSD OYSM PAAP PABN PAHY PALH PALJ PATO RING RJNS RKSK RPWD SADD SAGS SBRQ SBXG SESG SLCZ SLZO SPEC SPEI TABS TAOA TBBI TMDB UGGN UTEE UTNK UTNR UTSD UTTA UTTR UTTW VABG VANY VONY VQTU VTBA WIRN WMAM YAYT ZUCD ZUDF ZUDS ZUDZ ZUFJ ZUFL ZUGH ZUSN ZUTR ZUWF ZUXJ ZUXY ZUYB ZUZY", "", "id icao_code location_code site_name" ) ); 

&PrintResults( &getStationId( "UTTR", "METAR ASOS AWOS SA AIRPORT *", "" ) ); 

sub PrintResults{
    my %results = @_;
    my ($key, $value);
1
    for( keys %results ) {
        print "\nRecords for $_:";
        for( @{$results{$_}} ) {
            print "\n  Record: ";
            while( ($key, $value) = each %{$_} ) {
                print "$key=>$value, "
            }
        }
    }
    print "\n";
}
=cut

# --------------------------------------------------------------------
# INPUTS:
#   $ids    is a string containing the identifier(s) of the station(s) to return.
#             These may be specified as space-separated single 'words',
#             in any order, and with any capitalization. May be empty.
#   $types  is a string containing the name(s) of the station type(s) to return. 
#             These may be specified as space-separated single 'words',
#             in any order, and with any capitalization. May be empty.
#   $fields is a string containing the name(s) of the station fields to return.
#             These may be specified as space-separated single 'words',
#             in any order. The names must exactly match the desired column names 
#             from the aero_stations table to return in the result. May be null.
#             In all cases, the result will include aero_stations.location_code,
#             aero_stations.icao_code, aero_station.lat, and aero_stations.lon' 
#             as well as site_types.type.
#   One of $types or $ids must be non-null
#
# OUTPUT:
#   a hash of station IDs (both FAA-style and/or ICAO-style) to an array of records,
#   where each record is one which matches the request arguments. Each record is a
#   hash with the values specified in the $fields argument. By default, returns:
#     location  => FAA ID  (aero_stations.location_code) 
#     icao      => ICAO ID (aero_stations.icao_code) 
#     name      => site name (aero_stations.site_name) 
#     lat       => latitude (aero_stations.lat)
#     lon       => longitude (aero_stations.lon)
#     elevation => elevation (aero_stations.elevation)
#     type      => site type string containing space-separated words (site_types.type)
#     priority  => the numbered priority of this station, relative to others. May be 
#   So, to get the latitude of the first match for the Denver International Airport, 
#     you could use &getStations(){'KDEN'}[0]{lat}
#   Returned stations are prioritized depending on the IDs requested:
#     - If the requested ID had 3 letters, the returned station is the one
#       who's type matches the requested type and which matches first:
#          1) the location_code exactly
#          2) the icao_code, after prepending a "K"
#          3) the icao_code, after prepending a "P"
#       Only the first match is returned
#     - If the requested ID had 4 letters, the returned station is the one
#       who's type matches the requested type and which matches first:
#          1) the icao_code exactly
#          2) the location_code, after stripping a leading "K" or "P"
#       Only the first match is returned
# --------------------------------------------------------------------
sub getStations {
    my ( $ids, $types, $fields ) = @_;
    my ( $end, $start );
    my ( @ids, @types, @fields ); 
    my ($statement, %record, $icaoCode, $locCode, $name, $lat, $lon, $elevation, $type, %results );
  
    $start = times();

    print "Connecting to $dbName database\n" if $verbose;
    $dbHandle = &connectDb($dbName);

    # Build up the query string for the ids, if applicable
    my $idConstraints = "";
    @ids = split / /, "\U$ids";
    if( scalar @ids > 0 ) {
        $idConstraints = " ( ";
        for( @ids ) {
            # Add an "OR" continuation for all but the first id
            if( $idConstraints ne " ( " ) {
                $idConstraints = join " ", $idConstraints, "OR";
            }
            # For 3-letter ids, they could be an FAA/IATA id or an ICAO id missing its K or P
            if( length( $_ ) == 3 ) {
            	#For the instance of PAUN (AK) and KAUN/AUN (CA).
                $idConstraints = join " ", $idConstraints, 
                   "( a.location_code=\"$_\" OR ( a.location_code IS NULL AND ( a.icao_code=\"K$_\" OR a.icao_code=\"P$_\" ) ) )";
#                    "( a.location_code=\"$_\" OR ( a.icao_code=\"K$_\" OR a.icao_code=\"P$_\" ) )";
            }
            # for 4-letter ids, they could be an ICAO ID or a FAA/IATA id with an added K or P
            elsif ( length( $_ ) == 4 ) {
            	#print "Length equals 4, $_\n";
                $idConstraints = join " ", $idConstraints, "( a.icao_code=\"$_\"";
                if( $_ =~ /(K|P)(\w{3})/ ) {
                   #For the instance of PAUN (AK) and KAUN/AUN (CA).
                   $idConstraints = join " ", $idConstraints, "OR ( a.icao_code IS NULL and a.location_code=\"$2\" ) )";
#                    $idConstraints = join " ", $idConstraints, "OR a.location_code=\"$1\" )";
                } else {
                    $idConstraints = join " ", $idConstraints, ")";
                }
            }
            # do not support ids of other lengths at this time
            else {
                print "getStations: Currently, station names must have 3 or 4 character: $_ is invalid\n";
            }
        }
        $idConstraints = join " ", $idConstraints, ")";
    }

    # Build up the query string for the types, if applicable
    my $typeConstraints = "";
    @types = split / /, "\U$types";
    if( scalar @types > 0 ) {
        $typeConstraints = " ( ";
        for( @types ) {
            # Add an "OR" continuation for all but the first id
            if( $typeConstraints ne " ( " ) {
                $typeConstraints = join " ", $typeConstraints, "OR";
            }
            # Add the constraint for this type, being lenient to match it anywhere in the type string
            $typeConstraints = join " ", $typeConstraints, "s.type like \"%$_%\"";
        }
        $typeConstraints = join " ", $typeConstraints, ")";
    }


    # Create a logical AND if both ids and types were specified
    my $logicalAND = "";
    if( length $idConstraints > 0 && length $typeConstraints > 0 ) {
        $logicalAND = " AND ";
    }

    # build up the list of desired fields
    my $queryFields = "";
    @fields = split / /, $fields;
    push @fields, 'location_code' if( $fields !~ /location_code/ );
    push @fields, 'icao_code' if( $fields !~ /icao_code/ );
    push @fields, 'lat' if( $fields !~ /lat/ );
    push @fields, 'lon' if( $fields !~ /lon/ );
    push @fields, 'elevation' if( $fields !~ /elevation/ );
    $queryFields = "a." . ( join ", a.", @fields );

    # Execute the query
    my $sql = qq{
      SELECT $queryFields, s.type FROM aero_stations a, site_types s 
      WHERE $idConstraints $logicalAND $typeConstraints AND a.site_type_id = s.id };
    print "getStations: Issuing SQL: $sql\n" if $verbose;
    $statement = $dbHandle->prepare($sql);
    $statement->execute;
    $statement->bind_columns( \( @record{ @{$statement->{NAME_lc} } } ) );
    while( $statement->fetch ) {
        if( $record{location_code} ) {
            # New FAA location. Add.
            if( ! $results{$record{location_code}} ) {
                $results{$record{location_code}} = [];
            }                
            push @{ $results{$record{location_code}} }, { %record };
        }
        if( $record{icao_code} ) {
            # New ICAO location. Add.
            if( ! $results{$record{icao_code}} ) {
                $results{$record{icao_code}} = [];
            }
            push @{ $results{$record{icao_code}} }, { %record };
        }
    }
    $dbHandle->disconnect;

    $end = times();
    printf "getStations: took %.3f secs\n", $end - $start if $verbose;
    #print "Results from getStations: ", Dumper(\%results);
    return %results;
}



# --------------------------------------------------------------------
# Gets the requested stations and "fills in" the hash by putting in a 
# key for any stations that were found with an alternate id, such as
# adding a "K"or a "P" to a 3-letter FAA ID, or removing a "K" or "P"
# from a 4-letter ICAO ID. This ensures that the calling function will 
# be able to find the station in the returned list using the ID that it
# originally used to request it, even if it was found with a different ID.
#
# NOTE: This function will yield many false matches! It is assumed that the
#       results will be filtered by the calling function to find the right one!
# 
# INPUTS:
#   $ids   is a string containing the identifier(s) of the station(s) to return.
#          These may be specified as space-separated single 'words',
#          in any order, and with any capitalization. May be empty.
#   $types is a string containing the name(s) of the station type(s) to return. 
#          These may be specified as space-separated single 'words',
#          in any order, and with any capitalization. May be empty.
#
#   One of $types or $ids must be non-null
#
# OUTPUT:
#   a hash of station IDs (both FAA-style and/or ICAO-style) to an array of records,
#   where each record is one which matches the request arguments. Each record is a
#   hash with the following values:
#     location => FAA ID  (aero_stations.location_code) 
#     icao     => ICAO ID (aero_stations.icao_code) 
#     name     => site name (aero_stations.site_name) 
#     lat      => latitude (aero_stations.lat)
#     lon      => longitude (aero_stations.lon)
#     type     => site type string containing space-separated words (site_types.type)
#     priority => the numbered priority of this station, relative to others. May be 
#   So, to get the latitude of the first match for the Denver International Airport, 
#     you could use &getStations(){'KDEN'}[0]{lat}
#   Returned stations are prioritized depending on the IDs requested:
#     - If the requested ID had 3 letters, the returned station is the one
#       who's type matches the requested type and which matches first:
#          1) the location_code exactly
#          2) the icao_code, after prepending a "K"
#          3) the icao_code, after prepending a "P"
#       Only the first match is returned
#     - If the requested ID had 4 letters, the returned station is the one
#       who's type matches the requested type and which matches first:
#          1) the icao_code exactly
#          2) the location_code, after stripping a leading "K" or "P"
#       Only the first match is returned
# --------------------------------------------------------------------
sub getStationsLenient() {
    my ( $ids, $types, $fields ) = @_;
    my (%results, $id);

    %results = &getStations( $ids, $types, $fields );

    # Loop over each requested id to see if a match was found
    foreach $id (split / /, "\U$ids") {
        if( ! $results{$id} ) {

            # Look to see if there is a record under the "alternate" code
            if( length( $id ) == 3 ) {
                if( $results{"K$id"} ) {
                    $results{$id} = $results{"K$id"};
                } elsif( $results{"P$id"} ) {
                    $results{$id} = $results{"P$id"};
                } else {
                    print "getStationsLenient: Could not find any variant of station: $id \n" if $debug;
                }
            } elsif( length( $id ) == 4 ) {
                if( $id =~ /[KP](\w{3})/ ) {
                    if( $results{$1} ) {
                        $results{$id} = $results{$1};
                    } else {
                        print "GetStationsLenient: Could not find any variant of station: $id \n" if $debug;
                    }
                } else {
                    print "getStationsLenient: Could not find station: $id \n" if $debug;
                }
            } else {
                    print "getStationsLenient: Could not find station: $id \n" if $debug;
            }
        }
    }
    return %results;
}

# --------------------------------------------------------------------
# Gets the id of the unique aero_station for the given string name. The
# name match is lenient (matches K, P, and stripped variants) and the
# returned value is the first one matching the given type(s) in their
# given priority order. May return null, if no match is found.
#
# INPUTS:
#   $name  is a string containing the name (FAA or ICAO) of the station id to return.
#   $types is a string containing the name(s) of the station type(s) to match. 
#   Both inputs are required.
#
# OUTPUT:
#   the numeric ID of the matching station, or undefined, if none was found.
# --------------------------------------------------------------------
sub getStationId() {
    my ( $name, $types ) = @_;
    my ( $type, @types, %results, %record );
    my $finalWildcard = 0;
   

    # See if there is a wildcard as the last type
    @types = split / /, "\U$types";
    #print $types[$#types];
    if( $types[$#types] eq "*" ) {
	pop @types;
	$finalWildcard = 1;
    }

    # Get the matches, which may be many
    %results = &getStations( $name, ($finalWildcard ? "" : $types), 'id' );
 
    

    # Look for an exact name match
    # pick the station with an "earliest" matching type,
    foreach $type ( @types ) {
        # loop over any exact matches
        for( @{$results{$name}} ) {
            %record = %{$_};
            if( $record{type} =~ /$type/ ) {
                print "getStationId: Found exact match for $name: $record{id}-$_ of type $record{type}($type)\n" if $debug;
                return $record{id};
            }
        }
    }

    # Look for a truncated 4->3 letter code match (ICAO->FAA)
    if( $name =~ /^[KP](\w{3})$/ ) {

        # again picking the station with an "earliest" matching type,
	foreach $type ( @types ) {
	    for( @{$results{$1}} ) {
                %record = %{$_};
                if( $record{type} =~ /$type/ ) {
                    print "getStationId: Found ICAO->FAA match for $name: $record{id}-$_ of type $record{type}($type)\n" if $debug;
                    return $record{id};
                }
            }
        }
    }

    # Look for an augmented 3->4 letter code match
    if( length($name) == 3 ) {

        # again picking the station with an "earliest" matching type,
        foreach $type ( @types ) {
            for( @{$results{"K$name"}} ) {
                %record = %{$_};
                if( $record{type} =~ /$type/ ) {
                    print "getStationId: Found FAA->ICAO match for $name: $record{id}-$_ of type $record{type}($type)\n" if $debug;
                    return $record{id};
                }
            }
            for( @{$results{"P$name"}} ) {
                %record = %{$_};
                if( $record{type} =~ /$type/ ) {
                    print "getStationId: Found FAA->ICAO match for $name: $record{id}-$_ of type $record{type}($type)\n" if $debug;
                    return $record{id};
                }
            }
        }
    }

    # Look for a station with any type (if a wildcard type was specified last)
    if( $finalWildcard ) {
	# Look for an exact name match
	for( @{$results{$name}} ) {
	    %record = %{$_};
	    print "getStationId: Found exact match for $name: $record{id}-$_ with wildcard type $record{type}\n" if $debug;
	    return $record{id};
	}
	# Look for a truncated 4->3 letter code match (ICAO->FAA)
	if( $name =~ /^[KP](\w{3})$/ ) {
            for( @{$results{$1}} ) {
                %record = %{$_};
		print "getStationId: Found ICAO->FAA match for $name: $record{id}-$_ with wildcard type $record{type}\n" if $debug;
		return $record{id};
            }
        }
	# Look for an augmented 3->4 letter code match
	if( length($name) == 3 ) {
            for( @{$results{"K$name"}} ) {
                %record = %{$_};
		print "getStationId: Found FAA->ICAO match for $name: $record{id}-$_ with wildcard type $record{type}\n" if $debug;
		return $record{id};
            }
            for( @{$results{"P$name"}} ) {
                %record = %{$_};
		print "getStationId: Found FAA->ICAO match for $name: $record{id}-$_ with wildcard type $record{type}\n" if $debug;
		return $record{id};
            }
        }
    }

    # If we get here, there was no record found. Warn.
    print "getStationId: Could not find a station for name $name!\n";
}


# --------------------------------------------------------------------
# Gets the id of the unique aero_station for the given string name. The
# name match is lenient (matches A-Z and stripped variants) and the
# returned value is the first one matching the given type(s) in their
# given priority order. Returns a "dummy" station, if no match is found.
#
# INPUTS:
#   $name  is a string containing the name (FAA or ICAO) of the station id to return.
#   $types is a string containing the name(s) of the station type(s) to match. 
#   Both inputs are required.
#
# OUTPUT:
#   the numeric ID of the matching station, or 0 for the "dummy" station, if none was found.
# --------------------------------------------------------------------
sub getStationIdByType() {
    my ( $name, $types ) = @_;
    my ( $type, @types, %results, %record );
    my $finalWildcard = 0;
    my (@keys, @values, $key, $value, $size);
    my (@arrayOfTheRecordsHash, $thisRecordArray, $stationType, @arrayOfAllRecords);
    my ($thisArrayElement, %thisArrayzHash);
    my %matches;
    my $hashSize= 0;
    my $curSize = 0;
    my @arr;
    my @matchArray;
    my ($prevPriority, $curPriority, $prevInputType);
    my %ordinalsHash = ();
    my ($uniqueKey, $prevKey);
    my (@finalKey, @finalArray, @finalElements, $finalId, $finalName,$finalKey);
    my ($curArray,@singleArray);
    my $counter;
    my $dummyStationId = 1;
    my ($currentStationName, $currentLocation, $currentIcao);
    my $numberOfInputTypes;
    $size = 0;
    
    # See if there is a wildcard as the last type
    @types = split / /, "\U$types";
    #print $types[$#types];
    if( $types[$#types] eq "*" ) {
	   pop @types;
	   $finalWildcard = 1;
    }
    
    #Determine the number of input types, this will be used later on 
    #to adjust the priorities (to determine which matches to keep).
    $numberOfInputTypes = scalar @types;

    # Get the matches, which may be many  
    %results = &getStations( $name, ($finalWildcard ? "" : $types), 'id' );
    
    #If for some reason, this station does not exist in the database and getStations
    #returns an empty hash, return the "dummy" station and exit.
    $hashSize = scalar keys %results;
    if( $hashSize == 0 ){
    	print "getStationIdByType: getStations() Could not find a station for name $name!\n";
        return $dummyStationId;
    }
    
    
    
    #
    #Find matching types in the stations that were returned from the getStations() call to the prioritized stations that were input to
    #this method.  Higher priority matches are those that match the input types that are early in the listing (i.e. for input types of
    #"METAR ASOS AWOS", a match that is a METAR type is a higher priority match than one that is an AWOS type).
    #
    
    #Obtain the hash containing the ordinal values of the input types. This information will be used in determining the priorities
    #of the matches returned from getStations().
    %ordinalsHash = &getOrdinalValuesForTypes(@types);
    @keys = keys %results;
    #Loop over each input type.
    foreach $type(@types){
    	#Loop over each result returned from getStations().
    	foreach $key (@keys){
    		#loop over each array element that corresponds to the value for this key (i.e.station name).  Each array element is
    		#a hash that represents the station record. This record is comprised of an icao identifier, lat, lon, elevation, type, etc. 
    		@arrayOfTheRecordsHash = $results{$key};
    		#Loop over each array in the results hash from getStations() [i.e. each array in the value].
    		foreach $thisRecordArray (@arrayOfTheRecordsHash){
    			@arrayOfAllRecords = @{$thisRecordArray};
    			#Loop over each key in the records hash to extract the type, which
    			#can then be compared to the input types.
    			foreach $thisArrayElement (@arrayOfAllRecords){
    			   %thisArrayzHash = %{$thisArrayElement};
    			   #print "thisArrayElement hash is each record?? : ", Dumper(\%thisArrayzHash), "\n";
    			   $stationType = $thisArrayzHash{type};
    			   
    			   if( $stationType =~ /$type/){
    			   	   #If a match is found, and the type is a higher priority than the type already in the hash called
    			   	   #'matches', then remove the lower priority match and insert this match into the hash.
    			   	   #When the match is stored, the key is unique by using a combination of the id and
    			   	   #station name and the value is an array of the station name,input type, and id 
    			   	   #(from the aero_stations database table).
    			   	   
    			   	   #If this is the first time a match is found, then assign the prevPriority and curPriority to the first input type.
    			   	   $size = scalar keys %matches;	
    			   	   $curPriority = $ordinalsHash{$type};
    			   	   $currentIcao = $thisArrayzHash{icao};
    			   	   $currentLocation = $thisArrayzHash{location};
    			   	   
    			   	   #Adjust the priority value if this match only has a location code.  We give preference
    			   	   #to a lower priority type match to a station that has an icao code over
    			   	   #a  higher priority type match to a station with only a location code).  
    			   	   #A higher numerical values for priority correspond to lower priority.  
    			   	   if( !defined($currentIcao)){
    			   	   	   #Check for location code
    			   	   	   if( defined($currentLocation)){
    			   	   	   	   #Down-grade the priority.
    			   	   	   	   $curPriority = $curPriority + $numberOfInputTypes;
    			   	   	   } 
    			   	   }
    			   	   
    			   	   #Set the previous priority and input type values if this is the
    			   	   #first time we are making comparisons.
    			   	   if( $size == 0 ){
    			   	      $prevPriority = $curPriority;
    			   	      $prevInputType = $type;
    			   	      $prevKey = $uniqueKey;
                                      #print "+++Initial: previous priority= $prevPriority previous input type= $prevInputType previous key = $prevKey\n";
    			   	   }
    			   	   
    			   	   #print "Match found for $key, id= ", $thisArrayzHash{id}, " and station type $stationType with input type $type\n";
    			   	   #print "Current priority: $curPriority  Current input type: $type Prev priority: $prevPriority Prev Input type: $prevInputType \n";
    			   	   if( $curPriority <= $prevPriority ){
    			   	   	   @matchArray = ();
    			   	   	   #print "Saving the match found for $key, id= ", $thisArrayzHash{id}, " and station type $stationType with input type $type \n";
    			   	   	  #Insert this latest match into the hash and remove the previous one if needed.
    			   	   	   $matchArray[0] = $key;
    			   	       $matchArray[1] = $type;
    			   	       $matchArray[2] = $thisArrayzHash{id} ;
    			   	       $uniqueKey = $thisArrayzHash{id}  . $key ;
    			   	       push @ { $matches{$uniqueKey}} , @matchArray; 
    			   	       
    			   	       #Remove the previous match, if it exists.
    			   	       $curSize = scalar keys %matches;
    			   	       if( $curSize > 1 ){
    			   	       	   #print "Removing ", $matches{$prevKey}, "\n";
    			   	       	   delete ($matches{$prevKey});
    			   	       }
    			   	       
    			   	       $prevPriority = $curPriority;
    			   	       $prevInputType = $type;
    			   	       $prevKey = $uniqueKey;
    			   	   }
    			   }
    			}
    		}
    	}
    }
    
    
    #If more than one match remains in the 'matches' hash, return the best match based on how closely the station name
    #matches the input name  (eg. KDEN AIRPORT' would take be returned rather than 'DEN AIRPORT' if the input name=KDEN ).
    $curSize = scalar keys %matches;
    $hashSize = scalar keys %matches;
    $counter = $curSize;
    if( $curSize > 1 ){
    	#print "Potential problem, there should only be one match left...", Dumper(\%matches);
    	while ($counter > 0 ){
            #Iterate over each entry in the 'matches' hash and return the closest match to the requested station name.
    	    @finalKey = keys %matches;
    	    foreach $finalKey(@finalKey){
    		   #Get all the arrays that correspond to this key 
    		   #in the 'matches' hash.
    		   @finalArray = $matches{$finalKey};
    		
    		   #Retrieve the station name and id.
    		   #The value that corresponds to this key will contain only
    		   #one array. The station name is the zeroth element of this array
    		   #and the id is the second element.
    		   $finalName = ${$finalArray[0]}[0];
    		   $finalId = ${finalArray[0]}[2];
    		
    		   #Check for an exact match.
    		   if($finalName =~ /$name/){
    		      return $finalId;
    	       }
    	    }
    		
    		#Decrement the counter each time we iterate through the 'matches' hash.
    		$counter--;
    	}
    }elsif( $hashSize == 0 ){
    	# If we get here, there was no record found. Return the "dummy" station.
    	#And print a warning in the log file.
        #The dummy station has id=1, icao_code='', location_code='', and site_name "UNKNOWN SITE".
    	print "*WARNING* getStationIdByType: Could not find a station for name $name!\n";
        return $dummyStationId;
    }else{
    	#Only one match in the 'matches' hash. Return the id of this matching station.
    	#Get the key to the matching entry, then get the first array 
    	#(the value is comprised of 1 or more arrays).  Then get the third element (index=2) of that array,  
    	#this is the station's aero_stations id (that was found via the call to getStations() ).
    	#print "Only one match remains, returning $finalId\n";
    	@finalKey = keys %matches;
    	@finalArray = $matches{$finalKey[0]};
    	@finalElements = $finalArray[0];
    	$finalId = ${$finalElements[0]}[2];
    	#print "station id???", Dumper(\${$finalElements[0]}[2]);
    	return $finalId;
    }
}


# --------------------------------------------------------------------
#  airSigmetStationInfo_lookup replaces Navaids::airsigmetId_lookup 
#
#  INPUT:
#     $dbhandle       Data base handle used to connect to the database
#     $stationId      The station referenced in the report.
#
#  OUTPUT:
#     $lat            The latitude (in decimal degrees) of the station referenced in the report.
#     $lon            The longitude (in decimal degrees) of the station referenced in the report.
# --------------------------------------------------------------------
 sub airSigmetStation_lookup {
    my ($dbhandle, $stationId) = @_;
    my ($lat, $lon,$clat, $clon, $count);
    my ($result);
    my $siteTypes;
    my $key;
    my $value;
    my $counter = 0;
    my ($sql,$dbTable, $sth, $rv, $rc);
        
    $count = 0;
    $lat = 0;
    $lon = 0;

    #First priority is navigational aids, followed by airport types, and finally weather station types.
    my $types = "VOR/DME VORTAC VOT VOR FAN NDB NDB/DME UHF/NDB AIRMET/SIGMET AIRPORT HELIPORT MARINE ULTRALIGHT GLIDERPORT METAR TAF NOTAM FT FD SA UA WA WS";
    
    
    #Retrieve the aero_station id for this station/location.  If no station is found, 0 is returned.
    #$result = &getStationIdByType($stationId, $types);
    $result = &getStationId($stationId, $types);
    #if ( $result != 0 ){
    if ( defined($result) ){
       $sql = qq{SELECT lat,lon FROM aero_stations  WHERE id=$result};
       $sth = $dbhandle->prepare($sql);
       $rv = $sth->execute;
       $rc = $sth->bind_columns(\$clat, \$clon );
       
       while ($sth->fetch) {
            $lat = $clat;
            $lon = $clon;
       }

       $rc = $sth->finish if (defined($rc));
    }

    return ($lat, $lon);
    

}




# --------------------------------------------------------------------
#  stationInfo_lookup taken from NavAids.pm and modified to do a station 
#  lookup against the aero tables.  This is useful for sanity-checking returned
#  matches from a database query.  The PIREP decoder uses this to select
#  the station from a list of possible stations is the closest to the
#  reporting station (reference station).  
#  
#  INPUT:
#     $dbhandle       Data base handle used to connect to the database
#     $stationId      The station referenced in the report.
#     $nearIcao       Usually taken from the header, the reporting location/origin of the report.
#
#  OUTPUT:
#     $lat            The latitude (in decimal degrees) of the station referenced in the report.
#     $lon            The longitude (in decimal degrees) of the station referenced in the report. 
#     $elev           The elevation (in meters) of the station referenced in the report.
#
# --------------------------------------------------------------------
 sub stationInfo_lookup {
    my ($dbhandle, $stationId, $nearIcao) = @_;
    my ($clat, $clon, $lat, $lon, $elev, $count, @lats, @lons, @elevs);
    my ($n, $dist, $distMin, $latSave, $lonSave, $elevSave);
    my ($dbTable, $sql, $sth, $rv, $rc);
    my (%results, %records);
    my $siteTypes;

    
    $count = 0;
    $clat = $lat = 39.9999;
    $clon = $lon = -98.9999;
    $elev = 0;
   
    #First, find any station that looks like a possible match to the $stationId, then calculate the distance from each returned station to the reporting station to
    #determine the "best" and most reasonable match.
    my $types = "AIRPORT HELIPORT TACAN VOR/DME VOR VORTAC VOT NDB/DME UHF/NDB NDB FAN MARINE ULTRALIGHT GLIDERPORT METAR TAF NOTAM FT FD SA UA WA WS";
    %results = getStations($stationId, $types );
    my $key;
    my $value;
    my $counter = 0;
    for( keys %results ) {
        print STDERR "\nRecords for $_:" if $verbose;
        $counter++;
        for( @{$results{$_}} ) {
            while( ($key, $value) = each %{$_} ) {
               # print " $key=>$value, ";
                if( $key eq 'lat' ){
                	$lat = $value;
                	push (@lats, $lat);
                }
                if( $key eq'lon'){
                	$lon = $value;
                	push (@lons, $lon);
                }
                if ($key eq'elevation'){
                	$elev = $value;
                	push (@elevs, $elev);
                }
            }
        }
    }
    
  
    #Taken from NavAids.pm stationInfo_lookup.  
    if ($dbhandle) {

        if ($nearIcao eq "KWBC" || $nearIcao eq "KNKA" || $nearIcao eq "KKCI") {
            $clat = 39.9999;
            $clon = -98.9999;
        }else {
           $sql = qq{SELECT aero.lat,aero.lon,aero.elevation FROM aero_stations as aero, site_types as site WHERE aero.icao_code="$nearIcao" AND (aero.site_type_id=site.id AND FIND_IN_SET('ARP',site.type)>0 ) AND aero.lat>10 AND aero.lon<-50};
            #$sql = qq{SELECT aero.lat,aero.lon,aero.elevation FROM aero_stations as aero, site_types as site WHERE aero.icao_code="$nearIcao" AND aero.site_type_id=site.id AND aero.lat>10 AND aero.lon<-50};
            $sth = $dbhandle->prepare($sql);
            $rv = $sth->execute;
            $rc = $sth->bind_columns(undef, \$clat, \$clon, \$elev);
            while ($sth->fetch) {
                $count++;
            }
            $rc = $sth->finish if (defined($rc));

            if ($count > 1) {
                warn "WARNING, ambiguous identifier $nearIcao with more than one ($count) matching entry in $dbTable\n";
            } elsif ($clat == 0 && $clon == 0) {
                warn "WARNING, centroid not found for $nearIcao in $dbTable\n";
                $clat = 39.9999;
                $clon = -98.9999;
            }
        }


        if ($counter == 0) {
            #warn "WARNING, identifier $stationId not found in $dbTable (no lat/lon assigned)\n";
            print STDOUT "WARNING, identifier $stationId not found in $dbTable (no lat/lon assigned)\n" if $verbose;
        } elsif ($counter > 1) {
            $distMin = 2000000000;
            $dist = 0;
            for $n (0..$counter-1) {
                $dist = &NavAids::gc_dist($clat, -1*$clon, $lats[$n], -1*$lons[$n]);
                if ($dist < $distMin) {
                    $latSave = $lats[$n];
                    $lonSave = $lons[$n];
                    $elevSave = $elevs[$n];
                    $distMin = $dist;
                }
            }
            $lat = $latSave;
            $lon = $lonSave;
            $elev = $elevSave;
        } else {
            $lat = $lats[0];
            $lon = $lons[0];
            $elev = $elevs[0];
        }

    } else {
        warn "No database handle supplied, cannot retrieve lat/lon info for $stationId";
    }
    return ($lat, $lon, $elev);

}

# --------------------------------------------------------------------
# getOrdinalValuesForTypes
#   Assigns an ordinal value to each input type.  A lower value corresponds to
#   a higher priority when a match is found to a station type.
#  
#  INPUT:
#     @types          An array of input types.
#
#  OUTPUT:
#     %ordinals       A hash with the input type as key and its corresponding ordinal value as the value.
#
# --------------------------------------------------------------------
sub getOrdinalValuesForTypes{
   my(@types) = @_;
   my ($type, $ordinalValue);
   my %ordinals = ();	
   
   $ordinalValue = 0;
   
   foreach $type(@types){
   	  $ordinals{$type} = $ordinalValue;
      $ordinalValue++;
   }
   
   return %ordinals;

}
1;
