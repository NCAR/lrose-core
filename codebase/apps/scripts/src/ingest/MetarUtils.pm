#! /usr/bin/perl
#
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
# ** Copyright UCAR (c) 2013
# ** University Corporation for Atmospheric Research(UCAR)
# ** National Center for Atmospheric Research(NCAR)
# ** Research Applications Program(RAP)
# ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
# ** 2013/09/28 
# *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
#
# This module contains Perl routine(s) for decoding Metar reports (ie the 
# parseMetar.pl decoder). This is also used by the metarCsv2Mysql.pl script, which may need
# to decode remarks that it appends from an older METAR. 
# 
# --------------------------------------------------------------------
package    MetarUtils;
require    Exporter;
require 5.002;
@ISA = qw(Exporter);
@EXPORT = qw(decodeMETARremarks);

use strict;

our $debug = 1;
our $verbose = 1;
our $status = 0;

# --------------------------------------------------------------------
#  decodeMETARremarks taken from metar2mysql.  This routine is used
#  by the parseMetar.pl (decoder) and metarCsv2Mysql.pl scripts to
#  decode the remarks section of a METAR report. 
#  
#  INPUT:
#     %metarVars      Hash that represents a METAR report.
#     $remark         The string representation of the remark.
#
#  OUTPUT:
#     $metarVars      The updated hash that represents a METAR report.
#
# --------------------------------------------------------------------

sub decodeMETARremarks {

    #($_) = @_;
    my (%metarVars) = @_;
    my (
           $AUTOindicator, $maintIndicator,
           $SLPNO, $PWINO, $PNO, $FZRANO, $TSNO, $CIGNO, $PRESFR, $PRESRR, $RVRNO,
           $FROPA, $BINOVC, $ACFTMSHP, $NOSPECI, $FIRST, $LAST, $NEXT, $AURBO, $TEMPO,
           $temp, $dewp, $Tmax, $Tmin, $Tmax24, $Tmin24, $slp, $ptend, $ptendChar,
           $precip, $precip6hr, $precip24hr, $reportHr,
           $snowIncrement, $snowDepth, $snowWater,
           $lightning, $tornado, $hail, $hailSize,
           $visTower, $visSfc,
           $sunShine, $cloudLow, $cloudMed, $cloudHi,
           $ceilLow, $ceilHi, $wndPeakDir, $wndPeakSpd, $wndPeakHr, $wndPeakMn,
           $wxString, $wxCaught, $wxPossible, $extraWeather, $i, @WX
          );

    $_ = $metarVars{"remarks"};
    s/\n+//;

       #...------------...Allowed strings for present weather...-------

       @WX = ('VCBLSN', 'VCBLSA', 'VCBLDU',                              # vicinity and blowing
       'VCPO', 'VCSS', 'VCDS', 'VCFG', 'VCSH', 'VCTS',            # vicinity
       'DRDU', 'DRSA', 'DRSN',                                    # low and drifting
       'BLDU', 'BLSA', 'BLPY', 'BLSN',                            # blowing
       'MIFG', 'PRFG', 'BCFG',                                    # shallow, partial, and patches
       '+SHRA', '-SHRA', 'SHRA', '+SHSN', '-SHSN', 'SHSN',        # showers of various stuff
       '+SHPE', '+SHPL', '-SHPE', '-SHPL', 'SHPE', 'SHPL', 'SHGR', 'SHGS',
       '+TSRA', '-TSRA', 'TSRA', '+TSSN', '-TSSN', 'TSSN',        # thunderstorms plus various stuff
       '+TSPE', '+TSPL', '-TSPE', '-TSPL', 'TSPE', 'TSPL', 'TSGR', 'TSGS',
       '+FZRA', '-FZRA', 'FZRA', '+FZDZ', '-FZDZ', 'FZDZ', 'FZFG',# freezing stuff
       '+FC', 'FC', 'SQ', 'VA',                                   # nasty stuff that kills/maims
       '+SS', 'SS', '+DS', 'DS', 'PO',                            # nuisance stuff obstructs vision
       'SA', 'DU', 'FU', 'HZ', 'BR', 'FG',                        # same as previous
       'TS', 'GS', 'GR',                                          # pretty cool stuff for chasers
       'IC', '+PE', '+PL', '-PE', '-PL', 'PE', 'PL',              # wintertime goodies
       '+SG', '-SG', 'SG', '+SN', '-SN', 'SN',                    # more winter fare
       '+RA', '-RA', 'RA', '+DZ', '-DZ', 'DZ',                    # drab precip types
       'UP'                                                       # as dreaded as Darth Vader
      );


    #...------------...Have no idea what 6-9 slash 3digs are...
    s/ ?[6-9]\/[0-9]{3}//g;

    #...------------...Best to get rid of simple contents first...

    $AUTOindicator = $1 if ( s/ ?(A01|A01A|A02|A02A|AO1|AO1A|AO2|AO2A|AOA|AWOS)// ) ;
    $maintIndicator = 1 if ( s/ ?\$// ) ;

    $NOSPECI  = 1 if( s/ ?NOSPECI// );
    $TSNO     = 1 if( s/ ?TSNO// );
    $FZRANO   = 1 if( s/ ?FZRANO// );
    $PWINO    = 1 if( s/ ?PWINO// );

    $metarVars{"qcField"} += 4 if $AUTOindicator;
    $metarVars{"qcField"} += 8 if $maintIndicator;
    # $metarVars{"qcField"} += 16 if ($NOSPECI && $metarVars{"qcField"} < 16);
    $metarVars{"qcField"} += 32 if $TSNO;
    $metarVars{"qcField"} += 64 if $FZRANO;
    $metarVars{"qcField"} += 128 if $PWINO;

    $CIGNO    = 1 if( s/ ?CIGNO// );
    $SLPNO    = 1 if( s/ ?SLPNO// );
    $PNO      = 1 if( s/ ?PNO// );
    $PRESFR   = 1 if( s/ ?PRESFR\/?// );
    $PRESRR   = 1 if( s/ ?PRESRR\/?// );
    $RVRNO    = 1 if( s/ ?RVRNO// );
    $FROPA    = 1 if( s/ ?FROPA// );
    $BINOVC   = 1 if( s/ ?BINOVC// );
    $ACFTMSHP = 1 if( s/ ?\(?ACFT\s?MSHP\)?// );
    $FIRST    = 1 if( s/ ?FIRST// );
    $LAST     = 1 if( s/ ?LAST// );
    $NEXT     = 1 if( s/ ?NEXT( \d{4,6}Z)?// );
    $AURBO    = 1 if( s/ ?AURBO// );
    $TEMPO    = 1 if( s/ ?TEMPO// );

    #...------------...Decode temp/dewp with tenths of a degree...

    if ( s/ T(O|0|1)(\d{3})(O|0|1)?(\d{3})?// ) {
        if($1 == 0 || $1 eq "O") {
            $temp = 0.1 * $2;
        } elsif ($1 == 1) {
            $temp = -0.1 * $2;
        }
        if( defined( $3 ) && ($3 == 0 || $3 eq "O") ) {
            $dewp = 0.1 * $4 ;
        } elsif( defined( $3 ) && $3 == 1 ) {
            $dewp = -0.1 * $4 ;
        }
        if ( $temp && defined($metarVars{"temp"}) && ($temp >= $metarVars{"temp"} - 1) && ($temp <= $metarVars{"temp"} + 1) ) {
            $metarVars{"temp"} = $temp;
        }
        if ( $dewp && defined($metarVars{"dewp"}) && ($dewp >= $metarVars{"dewp"} - 1) && ($dewp <= $metarVars{"dewp"} + 1) ) {
            $metarVars{"dewp"} = $dewp;
        }
    }

    #...------------...Peak wind info...

    if( s/ PK WND ([0-3][0-9][0|5|6])([0|1]?[0-9][0-9])\/([0-2][0-9])?([0-5][0-9])// ) {
        $wndPeakDir = $1*1;
        $wndPeakSpd = $2*1;
        $wndPeakHr = $3*1 if ( defined($3) );
        $wndPeakMn = $4*1;
    }

    #...------------...Max/Min temp in 6 hours, then 24 hours...

    if ( s# 1(0|1|/)(\d{3}|///)## ) {
        $Tmax = $2 / 10 if( $2 ne "///" );
        $Tmax *= -1.0 if( $1 == 1 );
        $metarVars{"maxT"} = $Tmax;
    }
    if ( s# 2(0|1|/)(\d{3}|///)## ) {
        $Tmin = $2 / 10 if( $2 ne "///" );
        $Tmin *= -1.0 if( $1 == 1 );
        $metarVars{"minT"} = $Tmin;
    }
    if ( s# 4(0|1|/)(\d{3}|///)(0|1|/)(\d{3}|///)## ) {
        $Tmax24 = $2 / 10 if( $2 ne "///" );
        $Tmax24 *= -1.0 if( $1 == 1 && $Tmax24);
        $Tmin24 = $4 / 10 if( $4 ne "///" );
        $Tmin24 *= -1.0 if( $3 == 1 && $Tmin24);
        $metarVars{"maxT24"} = $Tmax24 if ($Tmax24);
        $metarVars{"minT24"} = $Tmin24 if ($Tmin24);
    }

    #...------------...Pressure tendency...

    if ( s# 5(0|1|2|3|4|5|6|7|8)(\d{3}/?|///)## ) {
        $ptend = $2 / 10 if( $2 ne "///" );
        $ptendChar = $1;
        if ($ptendChar <= 3) {
            $metarVars{"presTend"} = $ptend;
        } elsif ($ptendChar >= 5) {
            $metarVars{"presTend"} = -1.0 * $ptend;
        }
    }

    #...------------...Sea-level pressure...

    if ( s# SLP ?(\d{3})## ) {
        if( $1 >= 550 ) {
            $slp = $1 / 10. + 900.;
        } else {
            $slp =  $1 / 10. + 1000.;
        }
        $metarVars{"slp"} = $slp;
    }

    #...------------...Precip since last METAR...

    if ( s/ P(\d{4})// ) {
        $precip = $1 / 100;
        $precip = 0.005 if ($precip == 0);
        $metarVars{"precip"} = $precip;
    }

    #...------------...Precip past 3 or 6 hours...
    if ( s/ 6(\d{4})// ) {
        $precip6hr = $1 / 100;
        $precip6hr = 0.005 if ($precip6hr == 0);
        if ($precip6hr >= 0) {
            ($reportHr) = $metarVars{"reportTime"} =~ /\d{4}-\d{2}-\d{2} ([0-2][0-9]):\d{2}:\d{2}/;
            $reportHr *= 1;
            if ( ($reportHr % 6) == 0) {
                $metarVars{"pcp6hr"} = $precip6hr;
            } elsif ( ($reportHr % 3) == 0) {
                $metarVars{"pcp3hr"} = $precip6hr;
            } else {
                $metarVars{"precip"} = $precip if (! defined($metarVars{"precip"}));
            }
        }
    }

    #...------------...Precip past 24 hours...

    if ( s/ 7(\d{4})// ) {
        $precip24hr = $1 / 100 if ( $1 ne "////" );
        $precip24hr = 0.005 if ($precip24hr == 0);
        $metarVars{"pcp24hr"} = $precip24hr;
    }

    #...------------...Snow info...

    if ( s/ 4\/(\d{3})// ) {
        $snowDepth = $1;
    }
    if ( s/ 933(\d{2,3})// ) {
        $snowWater = $1/10;
    }

    if ( s/ SNINCR (\d{1,3})\/(\d{1,3})// ) {
        $snowIncrement = $1;
        $snowDepth = $2 unless ($snowDepth);
    }
    $metarVars{"snow"} = $snowDepth if ($snowDepth > 0);

    #...------------...Lightning, tornado, and hail info...

    if ( s/ (OCNL|FRQ|CNS)? LTG\s?(CG|IC|CC|CA)\s?(DSNT|AP|VCY STN|VCNTY STN)?\s?(NE|NW|SE|SW|N|S|E|W)?// ) {
        $lightning = 1;
    } elsif ( s/ (LIG)// ) {
        $lightning = 1;
    } elsif ( s/ (CB|TCU)\s?(DSNT|AP|VCY STN|VCNTY STN)?\s?(NE|NW|SE|SW|N|S|E|W)?// ) {
        $lightning = 0;
    }

    if ( s/ (TORNADO\w{0,2}|WATERSPOUT|FUNNEL CLOUD)// ) { 
        $tornado = 1;
        print STDERR ("\t\t !!!! TORNADO !!!!\t\t", $metarVars{"rawOb"}, "\n\n") if $verbose;
    }

    $hail = 1 if ( s/ GS// ) ;
    if ( s/ GR M1\/4// ) {
        $hail = 1 ;
        $hailSize = 1 / 8 ;
    } elsif ( s/ GR (\d{1,3}) (\d{1,2})\/(\d{1,2})// ) {
        $hail = 1 ;
        $hailSize = $1 + ( $2 / $3 ) ;
    } elsif ( s/ GR (\d{1,2})\/(\d{1,2})// ) {
        $hail = 1 ;
        $hailSize = ( $1 / $2 ) ;
    } elsif ( s/ GR (\d{1,3})// ) {
        $hail = 1 ;
        $hailSize = $1 ;
    }

    #...------------...Tower or surface visibility...

    if ( s/ TWR (VIS|VSBY) (\d{1,3}) (\d{1,2})\/(\d{1,2})// ) {
        $visTower = $2 + ( $3 / $4 );
    } elsif ( s/ TWR (VIS|VSBY) (\d{1,2})\/(\d{1,2})// ) {
        $visTower = ( $2 / $3 );
    } elsif ( s/ TWR (VIS|VSBY) (\d{1,3})// ) {
        $visTower = $2;
    }

    if ( s/ SFC (VIS|VSBY) (\d{1,3}) (\d{1,2})\/(\d{1,2})// ) {
        $visSfc = $2 + ( $3 / $4 );
    } elsif ( s/ SFC (VIS|VSBY) (\d{1,2})\/(\d{1,2})// ) {
        $visSfc = ( $2 / $3 );
    } elsif ( s/ SFC (VIS|VSBY) (\d{1,3})// ) {
        $visSfc = $2;
    }

    #...------------...Duration of sunshine...

    if( s/ 98(\d{1,3})// ) {
        $sunShine = $1;
    }

    #...------------...Low, middle, and high clouds...

    if( s# 8/(\d|/)(\d|/)(\d|/)/?## ) {
        $cloudLow = $1*1;
        $cloudMed = $2*1;
        $cloudHi  = $3*1;
    }

    #...------------...Variable Ceiling info...

    if ( s/ CIG ([0-2]\d{2})V([0-2]\d{2})// ) {
        $ceilLow = $1*1;
        $ceilHi  = $2*1;
    }

    #...------------...Extra Weather info...

    if ( s/ WEA:? ?(-?\+?[A-Z]+) ?// ) {
        $extraWeather = $1 if (defined($1));
        if ( $extraWeather && !($metarVars{"wxString"}) ) {

            $extraWeather =~ s/TDS//;        # Eliminate false DS
            $extraWeather =~ s/BLU\+?//;     # Eliminate false BL
            $extraWeather =~ s/FCST//;       # Eliminate false FC
            $extraWeather =~ s/GRN//;        # Eliminate false GR
            $extraWeather =~ s/RERA//;       # Eliminate false RA

            print "Extra weather found: $extraWeather\n" if ($extraWeather && $verbose);

            for ( $i=0; $i<3; $i++ ) {
                $wxCaught = 0;
                foreach $wxPossible (@WX) {
                    if ( $extraWeather =~ s/ ?(\Q$wxPossible\E) ?// ) {
                    	#Note:escape the $wxPossible with \Q...\E otherwise the + in the +SHRA, etc.
                    	#will be mistakenly interpreted as a regex quantifier.

                        $wxString .= "$1 ";
                        $wxCaught = 1;
                        last;
                    }
                }
                last unless $wxCaught;
                $wxString =~ s/\s{1,}/ /g;              # collapse multiple spaces to a single space.
            }
            $wxString =~ s/ $//;
            $metarVars{"wxString"} = $wxString if $wxString;
        }
    }

    #print STDOUT "\tLeft over remarks (between colons):$_:\n" if ($verbose && $_ && $_ ne " ");
    return (%metarVars);

}
1;
