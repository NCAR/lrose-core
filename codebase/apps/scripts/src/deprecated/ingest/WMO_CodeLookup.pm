package WMO_CodeLookup;
@ISA = qw(Exporter);
@EXPORT=qw(lookupCldCvgLink lookupCldCvgTitle lookupWxLink lookupWxTitle lookupCldTypeLink lookupCldTypeTitle lookupSigmetHazardLinkUS lookupSigmetHazardTitleUS lookupSigmetPhenomenonLink lookupSigmetPhenomenonTitle);

use strict;
use warnings;

sub lookupCldCvgLink()
{
    my( $pkgName, $cvg ) = @_;
    my $link = "http://codes.wmo.int/bufr4/codeflag/0-20-008/31";

    if( defined($cvg) ) {
    if ( $cvg eq "SKC" || $cvg eq "CLEAR" || $cvg eq "CAVOK") {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-008/0";
    } elsif ( $cvg eq "FEW" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-008/1";
    } elsif ( $cvg eq "SCATTERED" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-008/2";
    } elsif ( $cvg eq "BROKEN" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-008/3";
    } elsif ( $cvg eq "OVERCAST" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-008/4";
    } elsif ( $cvg eq "SCATTERED-BROKEN" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-008/6";
    } elsif ( $cvg eq "BROKEN-OVERCAST" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-008/7";
    } elsif ( $cvg eq "ISOLATED" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-008/8";
    } elsif ( $cvg eq "ISOLATED-EMBEDDED" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-008/9";
    } elsif ( $cvg eq "OCCASIONAL" || $cvg eq "OCNL" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-008/10";
    } elsif ( $cvg eq "OCCASIONAL-EMBEDDED" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-008/11";
    } elsif ( $cvg eq "FREQUENT" || $cvg eq "FRQ" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-008/12";
    } elsif ( $cvg eq "DENSE" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-008/13";
    } elsif ( $cvg eq "LAYERS" || $cvg eq "LYR" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-008/14";
    } elsif ( $cvg eq "OBSCURED" || $cvg eq "OBSC" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-008/15";
    } elsif ( $cvg eq "EMBEDDED" || $cvg eq "EMBD") {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-008/16";
    } elsif ( $cvg eq "FREQUENT-EMBEDDED" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-008/17";
    }
    }

    $link;
}

sub lookupCldCvgTitle()
{
    my( $pkgName, $cvg ) = @_;
    my $title = "Missing value";

    if( defined($cvg) ) {
    if ( $cvg eq "SKC" || $cvg eq "CLEAR" || $cvg eq "CAVOK") {
        $title = "Sky clear";
    } elsif ( $cvg eq "FEW" ) {
        $title = "Few";
    } elsif ( $cvg eq "SCATTERED" ) {
        $title = "Scattered";
    } elsif ( $cvg eq "BROKEN" ) {
        $title = "Broken";
    } elsif ( $cvg eq "OVERCAST" ) {
        $title = "Overcast";
    } elsif ( $cvg eq "SCATTERED-BROKEN" ) {
        $title = "Scattered/broken";
    } elsif ( $cvg eq "BROKEN-OVERCAST" ) {
        $title = "Broken/overcast";
    } elsif ( $cvg eq "ISOLATED" ) {
        $title = "Isolated";
    } elsif ( $cvg eq "ISOLATED-EMBEDDED" ) {
        $title = "Isolated embedded";
    } elsif ( $cvg eq "OCCASIONAL" || $cvg eq "OCNL" ) {
        $title = "Occasional";
    } elsif ( $cvg eq "OCCASIONAL-EMBEDDED" ) {
        $title = "Occasiona embedded";
    } elsif ( $cvg eq "FREQUENT" || $cvg eq "FRQ" ) {
        $title = "Frequent";
    } elsif ( $cvg eq "DENSE" ) {
        $title = "Dense";
    } elsif ( $cvg eq "LAYERS" || $cvg eq "LYR" ) {
        $title = "Layers";
    } elsif ( $cvg eq "OBSCURED" || $cvg eq "OBSC" ) {
        $title = "Obscured (OBSC)";
    } elsif ( $cvg eq "EMBEDDED" || $cvg eq "EMBD") {
        $title = "Embedded (EMBD)";
    } elsif ( $cvg eq "FREQUENT-EMBEDDED" ) {
        $title = "Frequent embedded";
    }
    }
    $title;
}

sub lookupWxLink()
{
    my( $pkgName, $wx ) = @_;
    my $link = "Unknown";
    my $title = "Unknown";

    if( defined($wx) ) {
    if ( $wx eq "+DS" ) {
        $link = "http://codes.wmo.int/306/4678/+DS";
        $title = "Heavy duststorm";
    } elsif ( $wx eq "+DZ" ) {
        $link = "http://codes.wmo.int/306/4678/+DZ";
        $title = "Heavy precipitation of drizzle";
    } elsif ( $wx eq "+FZDZ" ) {
        $link = "http://codes.wmo.int/306/4678/+FZDZ";
        $title = "Heavy precipitation of freezing drizzle";
    } elsif ( $wx eq "+FZRA" ) {
        $link = "http://codes.wmo.int/306/4678/+FZRA";
        $title = "Heavy precipitation of freezng rain";
    } elsif ( $wx eq "+PL" ) {
        $link = "http://codes.wmo.int/306/4678/+PL";
        $title = "Heavy precipitation of ice pellets";
    } elsif ( $wx eq "+RA" ) {
        $link = "http://codes.wmo.int/306/4678/+RA";
        $title = "Heavy precipitation of rain";
    } elsif ( $wx eq "+SG" ) {
        $link = "http://codes.wmo.int/306/4678/+SG";
        $title = "Heavy precipitation of snow grains";
    } elsif ( $wx eq "+SHRA" ) {
        $link = "http://codes.wmo.int/306/4678/+SHRA";
        $title = "Heavy showery precipitation of rain";
    } elsif ( $wx eq "+SHSN" ) {
        $link = "http://codes.wmo.int/306/4678/+SHSN";
        $title = "Heavy showery precipitation of snow";
    } elsif ( $wx eq "+SN" ) {
        $link = "http://codes.wmo.int/306/4678/+SN";
        $title = "Heavy precipitation of snow";
    } elsif ( $wx eq "+SS" ) {
        $link = "http://codes.wmo.int/306/4678/+SS";
        $title = "Heavy sandstorm";
    } elsif ( $wx eq "+TSRA" ) {
        $link = "http://codes.wmo.int/306/4678/+TSRA";
        $title = "Thunderstorm with heavy precipitation of rain in the vicinity";
    } elsif ( $wx eq "+UP" ) {
        $link = "http://codes.wmo.int/306/4678/+UP";
        $title = "Heavy unidentified precipitation";
    } elsif ( $wx eq "-DS" ) {
        $link = "http://codes.wmo.int/306/4678/-DS";
        $title = "Light duststorm";
    } elsif ( $wx eq "-DZ" ) {
        $link = "http://codes.wmo.int/306/4678/-DZ";
        $title = "Light precipitation of drizzle";
    } elsif ( $wx eq "-FZDZ" ) {
        $link = "http://codes.wmo.int/306/4678/-FZDZ";
        $title = "Light precipitation of freezing drizzle";
    } elsif ( $wx eq "-FZRA" ) {
        $link = "http://codes.wmo.int/306/4678/-FZRA";
        $title = "Light precipitation of freezng rain";
    } elsif ( $wx eq "-PL" ) {
        $link = "http://codes.wmo.int/306/4678/-PL";
        $title = "Light precipitation of ice pellets";
    } elsif ( $wx eq "-RA" ) {
        $link = "http://codes.wmo.int/306/4678/-RA";
        $title = "Light precipitation of rain";
    } elsif ( $wx eq "-SG" ) {
        $link = "http://codes.wmo.int/306/4678/-SG";
        $title = "Light precipitation of snow grains";
    } elsif ( $wx eq "-SHRA" ) {
        $link = "http://codes.wmo.int/306/4678/-SHRA";
        $title = "Light showery precipitation of rain";
    } elsif ( $wx eq "-SHSN" ) {
        $link = "http://codes.wmo.int/306/4678/-SHSN";
        $title = "Light showery precipitation of snow";
    } elsif ( $wx eq "-SN" ) {
        $link = "http://codes.wmo.int/306/4678/-SN";
        $title = "Light precipitation of snow";
    } elsif ( $wx eq "-SS" ) {
        $link = "http://codes.wmo.int/306/4678/-SS";
        $title = "Light sandstorm";
    } elsif ( $wx eq "-TSRA" ) {
        $link = "http://codes.wmo.int/306/4678/-TSRA";
        $title = "Thunderstorm with light precipitation of rain in the vicinity";
    } elsif ( $wx eq "-UP" ) {
        $link = "http://codes.wmo.int/306/4678/-UP";
        $title = "Light unidentified precipitation";
    } elsif ( $wx eq "UP" ) {
        $link = "http://codes.wmo.int/306/4678/UP";
        $title = "Unidentified precipitation";
    } elsif ( $wx eq "BLDU" ) {
        $link = "http://codes.wmo.int/306/4678/BLDU";
        $title = "Blowing dust";
    } elsif ( $wx eq "BLSA" ) {
        $link = "http://codes.wmo.int/306/4678/BLSA";
        $title = "Blowing sand";
    } elsif ( $wx eq "BLSN" ) {
        $link = "http://codes.wmo.int/306/4678/BLSN";
        $title = "Blowing snow";
    } elsif ( $wx eq "BR" ) {
        $link = "http://codes.wmo.int/306/4678/BR";
        $title = "Mist";
    } elsif ( $wx eq "DS" ) {
        $link = "http://codes.wmo.int/306/4678/DS";
        $title = "Duststorm";
    } elsif ( $wx eq "DZ" ) {
        $link = "http://codes.wmo.int/306/4678/DZ";
        $title = "Drizzle";
    } elsif ( $wx eq "FC" ) {
        $link = "http://codes.wmo.int/306/4678/FC";
        $title = "Funnel cloud(s) (tornado or water-spout)";
    } elsif ( $wx eq "FG" ) {
        $link = "http://codes.wmo.int/306/4678/FG";
        $title = "Fog";
    } elsif ( $wx eq "FU" ) {
        $link = "http://codes.wmo.int/306/4678/FU";
        $title = "Smoke";
    } elsif ( $wx eq "HZ" ) {
        $link = "http://codes.wmo.int/306/4678/HZ";
        $title = "Haze";
    } elsif ( $wx eq "MIFG" ) {
        $link = "http://codes.wmo.int/306/4678/MIFG";
        $title = "Shallow fog";
    } elsif ( $wx eq "FZDZ" ) {
        $link = "http://codes.wmo.int/306/4678/FZDZ";
        $title = "Precipitation of freezing drizzle";
    } elsif ( $wx eq "FZRA" ) {
        $link = "http://codes.wmo.int/306/4678/FZRA";
        $title = "Precipitation of freezng rain";
    } elsif ( $wx eq "PL" ) {
        $link = "http://codes.wmo.int/306/4678/PL";
        $title = "Precipitation of ice pellets";
    } elsif ( $wx eq "PO" ) {
        $link = "http://codes.wmo.int/306/4678/PO";
        $title = "Dust/sand whirls (dust devils)";
    } elsif ( $wx eq "PRFG" ) {
        $link = "http://codes.wmo.int/306/4678/PRFG";
        $title = "Partial fog (covering part of the aerodrome)";
    } elsif ( $wx eq "RA" ) {
        $link = "http://codes.wmo.int/306/4678/RA";
        $title = "RAIN";
    } elsif ( $wx eq "SA" ) {
        $link = "http://codes.wmo.int/306/4678/SA";
        $title = "Sand";
    } elsif ( $wx eq "SG" ) {
        $link = "http://codes.wmo.int/306/4678/SG";
        $title = "Precipitation of snow grains";
    } elsif ( $wx eq "SHRA" ) {
        $link = "http://codes.wmo.int/306/4678/SHRA";
        $title = "Showery precipitation of rain";
    } elsif ( $wx eq "SHSN" ) {
        $link = "http://codes.wmo.int/306/4678/SHSN";
        $title = "Showery precipitation of snow";
    } elsif ( $wx eq "SN" ) {
        $link = "http://codes.wmo.int/306/4678/SN";
        $title = "Snow";
    } elsif ( $wx eq "SQ" ) {
        $link = "http://codes.wmo.int/306/4678/SQ";
        $title = "Squalls";
    } elsif ( $wx eq "SS" ) {
        $link = "http://codes.wmo.int/306/4678/SS";
        $title = "Sandstorm";
    } elsif ( $wx eq "TS" ) {
        $link = "http://codes.wmo.int/306/4678/TS";
        $title = "Thunderstorm";
    } elsif ( $wx eq "TSRA" ) {
        $link = "http://codes.wmo.int/306/4678/TSRA";
        $title = "Thunderstorm with precipitation of rain in the vicinity";
    } elsif ( $wx eq "UP" ) {
        $link = "http://codes.wmo.int/306/4678/UP";
        $title = "Unidentified precipitation";
    } elsif ( $wx eq "VA" ) {
        $link = "http://codes.wmo.int/306/4678/VA";
        $title = "Volcanic ash";
    } elsif ( $wx eq "VCBLDU" ) {
        $link = "http://codes.wmo.int/306/4678/VCBLDU";
        $title = "Blowing dust in the vicinity";
    } elsif ( $wx eq "VCBLSA" ) {
        $link = "http://codes.wmo.int/306/4678/VCBLSA";
        $title = "Blowing sand in the vicinity";
    } elsif ( $wx eq "VCBLSN" ) {
        $link = "http://codes.wmo.int/306/4678/VCBLSN";
        $title = "Blowing snow in the vicinity";
    } elsif ( $wx eq "VCDS" ) {
        $link = "http://codes.wmo.int/306/4678/VCDS";
        $title = "Duststorm in the vicinity";
    } elsif ( $wx eq "VCFC" ) {
        $link = "http://codes.wmo.int/306/4678/VCFC";
        $title = "Funnel cloud(s) (tornado or water-spout) in the vicinity";
    } elsif ( $wx eq "VCFG" ) {
        $link = "http://codes.wmo.int/306/4678/VCFG";
        $title = "Fog in the vicinity";
    } elsif ( $wx eq "VCPO" ) {
        $link = "http://codes.wmo.int/306/4678/VCPO";
        $title = "Dust/sand whirls (dust devils) in the vicinity";
    } elsif ( $wx eq "VCSH" ) {
        $link = "http://codes.wmo.int/306/4678/VCSH";
        $title = "Shower(s) in the vicinity";
    } elsif ( $wx eq "VCSS" ) {
        $link = "http://codes.wmo.int/306/4678/VCSS";
        $title = "Sandstorm in the vicinity";
    } elsif ( $wx eq "VCTS" ) {
        $link = "http://codes.wmo.int/306/4678/VCTS";
        $title = "Thunderstorm in the vicinity";
    } elsif ( $wx eq "VCVA" ) {
        $link = "http://codes.wmo.int/306/4678/VCVA";
        $title = "Volcanic ash in the vicinity";
    }
    }

    $link;
}

sub lookupWxTitle()
{
    my( $pkgName, $wx ) = @_;
    my $link = "Unknown";
    my $title = "Unknown";

    if( defined($wx) ) {
    if ( $wx eq "+DS" ) {
        $link = "http://codes.wmo.int/306/4678/+DS";
        $title = "Heavy duststorm";
    } elsif ( $wx eq "+DZ" ) {
        $link = "http://codes.wmo.int/306/4678/+DZ";
        $title = "Heavy precipitation of drizzle";
    } elsif ( $wx eq "+FZDZ" ) {
        $link = "http://codes.wmo.int/306/4678/+FZDZ";
        $title = "Heavy precipitation of freezing drizzle";
    } elsif ( $wx eq "+FZRA" ) {
        $link = "http://codes.wmo.int/306/4678/+FZRA";
        $title = "Heavy precipitation of freezng rain";
    } elsif ( $wx eq "+PL" ) {
        $link = "http://codes.wmo.int/306/4678/+PL";
        $title = "Heavy precipitation of ice pellets";
    } elsif ( $wx eq "+RA" ) {
        $link = "http://codes.wmo.int/306/4678/+RA";
        $title = "Heavy precipitation of rain";
    } elsif ( $wx eq "+SG" ) {
        $link = "http://codes.wmo.int/306/4678/+SG";
        $title = "Heavy precipitation of snow grains";
    } elsif ( $wx eq "+SHRA" ) {
        $link = "http://codes.wmo.int/306/4678/+SHRA";
        $title = "Heavy showery precipitation of rain";
    } elsif ( $wx eq "+SHSN" ) {
        $link = "http://codes.wmo.int/306/4678/+SHSN";
        $title = "Heavy showery precipitation of snow";
    } elsif ( $wx eq "+SN" ) {
        $link = "http://codes.wmo.int/306/4678/+SN";
        $title = "Heavy precipitation of snow";
    } elsif ( $wx eq "+SS" ) {
        $link = "http://codes.wmo.int/306/4678/+SS";
        $title = "Heavy sandstorm";
    } elsif ( $wx eq "+TSRA" ) {
        $link = "http://codes.wmo.int/306/4678/+TSRA";
        $title = "Thunderstorm with heavy precipitation of rain in the vicinity";
    } elsif ( $wx eq "+UP" ) {
        $link = "http://codes.wmo.int/306/4678/+UP";
        $title = "Heavy unidentified precipitation";
    } elsif ( $wx eq "-DS" ) {
        $link = "http://codes.wmo.int/306/4678/-DS";
        $title = "Light duststorm";
    } elsif ( $wx eq "-DZ" ) {
        $link = "http://codes.wmo.int/306/4678/-DZ";
        $title = "Light precipitation of drizzle";
    } elsif ( $wx eq "-FZDZ" ) {
        $link = "http://codes.wmo.int/306/4678/-FZDZ";
        $title = "Light precipitation of freezing drizzle";
    } elsif ( $wx eq "-FZRA" ) {
        $link = "http://codes.wmo.int/306/4678/-FZRA";
        $title = "Light precipitation of freezng rain";
    } elsif ( $wx eq "-PL" ) {
        $link = "http://codes.wmo.int/306/4678/-PL";
        $title = "Light precipitation of ice pellets";
    } elsif ( $wx eq "-RA" ) {
        $link = "http://codes.wmo.int/306/4678/-RA";
        $title = "Light precipitation of rain";
    } elsif ( $wx eq "-SG" ) {
        $link = "http://codes.wmo.int/306/4678/-SG";
        $title = "Light precipitation of snow grains";
    } elsif ( $wx eq "-SHRA" ) {
        $link = "http://codes.wmo.int/306/4678/-SHRA";
        $title = "Light showery precipitation of rain";
    } elsif ( $wx eq "-SHSN" ) {
        $link = "http://codes.wmo.int/306/4678/-SHSN";
        $title = "Light showery precipitation of snow";
    } elsif ( $wx eq "-SN" ) {
        $link = "http://codes.wmo.int/306/4678/-SN";
        $title = "Light precipitation of snow";
    } elsif ( $wx eq "-SS" ) {
        $link = "http://codes.wmo.int/306/4678/-SS";
        $title = "Light sandstorm";
    } elsif ( $wx eq "-TSRA" ) {
        $link = "http://codes.wmo.int/306/4678/-TSRA";
        $title = "Thunderstorm with light precipitation of rain in the vicinity";
    } elsif ( $wx eq "-UP" ) {
        $link = "http://codes.wmo.int/306/4678/-UP";
        $title = "Light unidentified precipitation";
    } elsif ( $wx eq "UP" ) {
        $link = "http://codes.wmo.int/306/4678/UP";
        $title = "Unidentified precipitation";
    } elsif ( $wx eq "BLDU" ) {
        $link = "http://codes.wmo.int/306/4678/BLDU";
        $title = "Blowing dust";
    } elsif ( $wx eq "BLSA" ) {
        $link = "http://codes.wmo.int/306/4678/BLSA";
        $title = "Blowing sand";
    } elsif ( $wx eq "BLSN" ) {
        $link = "http://codes.wmo.int/306/4678/BLSN";
        $title = "Blowing snow";
    } elsif ( $wx eq "BR" ) {
        $link = "http://codes.wmo.int/306/4678/BR";
        $title = "Mist";
    } elsif ( $wx eq "DS" ) {
        $link = "http://codes.wmo.int/306/4678/DS";
        $title = "Duststorm";
    } elsif ( $wx eq "DZ" ) {
        $link = "http://codes.wmo.int/306/4678/DZ";
        $title = "Drizzle";
    } elsif ( $wx eq "FC" ) {
        $link = "http://codes.wmo.int/306/4678/FC";
        $title = "Funnel cloud(s) (tornado or water-spout)";
    } elsif ( $wx eq "FG" ) {
        $link = "http://codes.wmo.int/306/4678/FG";
        $title = "Fog";
    } elsif ( $wx eq "FU" ) {
        $link = "http://codes.wmo.int/306/4678/FU";
        $title = "Smoke";
    } elsif ( $wx eq "HZ" ) {
        $link = "http://codes.wmo.int/306/4678/HZ";
        $title = "Haze";
    } elsif ( $wx eq "MIFG" ) {
        $link = "http://codes.wmo.int/306/4678/MIFG";
        $title = "Shallow fog";
    } elsif ( $wx eq "FZDZ" ) {
        $link = "http://codes.wmo.int/306/4678/FZDZ";
        $title = "Precipitation of freezing drizzle";
    } elsif ( $wx eq "FZRA" ) {
        $link = "http://codes.wmo.int/306/4678/FZRA";
        $title = "Precipitation of freezng rain";
    } elsif ( $wx eq "PL" ) {
        $link = "http://codes.wmo.int/306/4678/PL";
        $title = "Precipitation of ice pellets";
    } elsif ( $wx eq "PO" ) {
        $link = "http://codes.wmo.int/306/4678/PO";
        $title = "Dust/sand whirls (dust devils)";
    } elsif ( $wx eq "PRFG" ) {
        $link = "http://codes.wmo.int/306/4678/PRFG";
        $title = "Partial fog (covering part of the aerodrome)";
    } elsif ( $wx eq "RA" ) {
        $link = "http://codes.wmo.int/306/4678/RA";
        $title = "RAIN";
    } elsif ( $wx eq "SA" ) {
        $link = "http://codes.wmo.int/306/4678/SA";
        $title = "Sand";
    } elsif ( $wx eq "SG" ) {
        $link = "http://codes.wmo.int/306/4678/SG";
        $title = "Precipitation of snow grains";
    } elsif ( $wx eq "SHRA" ) {
        $link = "http://codes.wmo.int/306/4678/SHRA";
        $title = "Showery precipitation of rain";
    } elsif ( $wx eq "SHSN" ) {
        $link = "http://codes.wmo.int/306/4678/SHSN";
        $title = "Showery precipitation of snow";
    } elsif ( $wx eq "SN" ) {
        $link = "http://codes.wmo.int/306/4678/SN";
        $title = "Snow";
    } elsif ( $wx eq "SQ" ) {
        $link = "http://codes.wmo.int/306/4678/SQ";
        $title = "Squalls";
    } elsif ( $wx eq "SS" ) {
        $link = "http://codes.wmo.int/306/4678/SS";
        $title = "Sandstorm";
    } elsif ( $wx eq "TS" ) {
        $link = "http://codes.wmo.int/306/4678/TS";
        $title = "Thunderstorm";
    } elsif ( $wx eq "TSRA" ) {
        $link = "http://codes.wmo.int/306/4678/TSRA";
        $title = "Thunderstorm with precipitation of rain in the vicinity";
    } elsif ( $wx eq "UP" ) {
        $link = "http://codes.wmo.int/306/4678/UP";
        $title = "Unidentified precipitation";
    } elsif ( $wx eq "VA" ) {
        $link = "http://codes.wmo.int/306/4678/VA";
        $title = "Volcanic ash";
    } elsif ( $wx eq "VCBLDU" ) {
        $link = "http://codes.wmo.int/306/4678/VCBLDU";
        $title = "Blowing dust in the vicinity";
    } elsif ( $wx eq "VCBLSA" ) {
        $link = "http://codes.wmo.int/306/4678/VCBLSA";
        $title = "Blowing sand in the vicinity";
    } elsif ( $wx eq "VCBLSN" ) {
        $link = "http://codes.wmo.int/306/4678/VCBLSN";
        $title = "Blowing snow in the vicinity";
    } elsif ( $wx eq "VCDS" ) {
        $link = "http://codes.wmo.int/306/4678/VCDS";
        $title = "Duststorm in the vicinity";
    } elsif ( $wx eq "VCFC" ) {
        $link = "http://codes.wmo.int/306/4678/VCFC";
        $title = "Funnel cloud(s) (tornado or water-spout) in the vicinity";
    } elsif ( $wx eq "VCFG" ) {
        $link = "http://codes.wmo.int/306/4678/VCFG";
        $title = "Fog in the vicinity";
    } elsif ( $wx eq "VCPO" ) {
        $link = "http://codes.wmo.int/306/4678/VCPO";
        $title = "Dust/sand whirls (dust devils) in the vicinity";
    } elsif ( $wx eq "VCSH" ) {
        $link = "http://codes.wmo.int/306/4678/VCSH";
        $title = "Shower(s) in the vicinity";
    } elsif ( $wx eq "VCSS" ) {
        $link = "http://codes.wmo.int/306/4678/VCSS";
        $title = "Sandstorm in the vicinity";
    } elsif ( $wx eq "VCTS" ) {
        $link = "http://codes.wmo.int/306/4678/VCTS";
        $title = "Thunderstorm in the vicinity";
    } elsif ( $wx eq "VCVA" ) {
        $link = "http://codes.wmo.int/306/4678/VCVA";
        $title = "Volcanic ash in the vicinity";
    }
    }

    $title;
}

sub lookupCldTypeLink()
{
    my( $pkgName, $cloudType ) = @_;
    my $link = "Unknown";
    my $title = "Unknown";

    if( defined($cloudType) ) {
    if ( $cloudType eq "CI" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/0";
        $title = "Cirrus (Ci)";
    } elsif ( $cloudType eq "CC" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/1";
        $title = "Cirrocumulus (Cc)";
    } elsif ( $cloudType eq "CS" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/2";
        $title = "Cirrostratus (Cs)";
    } elsif ( $cloudType eq "AC" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/3";
        $title = "Altocumulus (Ac)";
    } elsif ( $cloudType eq "AS" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/4";
        $title = "Altostratus (As)";
    } elsif ( $cloudType eq "NS" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/5";
        $title = "Nimbostratus (Ns)";
    } elsif ( $cloudType eq "SC" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/6";
        $title = "Stratocumulus (Sc)";
    } elsif ( $cloudType eq "ST" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/7";
        $title = "Stratus (St)";
    } elsif ( $cloudType eq "CU" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/8";
        $title = "Cumulus (Cu)";
    } elsif ( $cloudType eq "CB" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/9";
        $title = "Cumulonimbus (Cb)";
    } elsif ( $cloudType eq "NOCH" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/10";
        $title = "No CH clouds";
    } elsif ( $cloudType eq "CF" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/11";
        $title = "Cirrus fibratus, sometimes uncinus, not progressively invading the sky";
    } elsif ( $cloudType eq "CS" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/12";
        $title = "Cirrus spissatus, in patches or entangled sheaves, which usually do not increase and sometimes seem to be the remains of the upper part of a Cumulonimbus; or Cirrus castellanus or floccus";
    } elsif ( $cloudType eq "CSC" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/13";
        $title = "Cirrus spissatus cumulonimbogenitus";
    } elsif ( $cloudType eq "CU" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/14";
        $title = "Cirrus uncinus or fibratus, or both, progressively invading the sky; they generally thicken as a whole";
    } elsif ( $cloudType eq "CI/CS" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/15";
        $title = "Cirrus (often in bands) and Cirrostratus, or Cirrostratus alone, progressively invading the sky; they generally thicken as a whole, but the continuous veil does not reach 45 degrees above the horizon";
    } elsif ( $cloudType eq "CI/CS/CI" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/16";
        $title = "Cirrus (often in bands) and Cirrostratus, or Cirrostratus alone, progressively Invading the sky; they generally thicken as a whole; the continuous veil extends more than 45 degrees above the horizon, without the sky being totally covered";
    } elsif ( $cloudType eq "CI" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/17";
        $title = "Cirrostratus covering the whole sky";
    } elsif ( $cloudType eq "CS" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/18";
        $title = "Cirrostratus not progressively invading the sky and not entirely covering it";
    } elsif ( $cloudType eq "CC/CH" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/19";
        $title = "Cirrocumulus alone, or Cirrocumulus predominant among the CH clouds";
    } elsif ( $cloudType eq "NOCM" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/20";
        $title = "No CM clouds";
    } elsif ( $cloudType eq "AT" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/21";
        $title = "Altostratus translucidus";
    } elsif ( $cloudType eq "AO/NS" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/22";
        $title = "Altostratus opacus or Nimbostratus";
    } elsif ( $cloudType eq "AT1" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/23";
        $title = "Altocumulus translucidus at a single level";
    } elsif ( $cloudType eq "PATCH" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/24";
        $title = "Patches (often lenticular) of Altocumulus translucidus, continually changing and occurring at one or more levels";
    }
    }

    $link;
}

sub lookupCldTypeTitle()
{
    my( $pkgName, $cloudType ) = @_;
    my $link = "Unknown";
    my $title = "Unknown";

    if( defined($cloudType) ) {
    if ( $cloudType eq "CI" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/0";
        $title = "Cirrus (Ci)";
    } elsif ( $cloudType eq "CC" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/1";
        $title = "Cirrocumulus (Cc)";
    } elsif ( $cloudType eq "CS" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/2";
        $title = "Cirrostratus (Cs)";
    } elsif ( $cloudType eq "AC" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/3";
        $title = "Altocumulus (Ac)";
    } elsif ( $cloudType eq "AS" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/4";
        $title = "Altostratus (As)";
    } elsif ( $cloudType eq "NS" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/5";
        $title = "Nimbostratus (Ns)";
    } elsif ( $cloudType eq "SC" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/6";
        $title = "Stratocumulus (Sc)";
    } elsif ( $cloudType eq "ST" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/7";
        $title = "Stratus (St)";
    } elsif ( $cloudType eq "CU" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/8";
        $title = "Cumulus (Cu)";
    } elsif ( $cloudType eq "CB" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/9";
        $title = "Cumulonimbus (Cb)";
    } elsif ( $cloudType eq "NOCH" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/10";
        $title = "No CH clouds";
    } elsif ( $cloudType eq "CF" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/11";
        $title = "Cirrus fibratus, sometimes uncinus, not progressively invading the sky";
    } elsif ( $cloudType eq "CS" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/12";
        $title = "Cirrus spissatus, in patches or entangled sheaves, which usually do not increase and sometimes seem to be the remains of the upper part of a Cumulonimbus; or Cirrus castellanus or floccus";
    } elsif ( $cloudType eq "CSC" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/13";
        $title = "Cirrus spissatus cumulonimbogenitus";
    } elsif ( $cloudType eq "CU" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/14";
        $title = "Cirrus uncinus or fibratus, or both, progressively invading the sky; they generally thicken as a whole";
    } elsif ( $cloudType eq "CI/CS" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/15";
        $title = "Cirrus (often in bands) and Cirrostratus, or Cirrostratus alone, progressively invading the sky; they generally thicken as a whole, but the continuous veil does not reach 45 degrees above the horizon";
    } elsif ( $cloudType eq "CI/CS/CI" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/16";
        $title = "Cirrus (often in bands) and Cirrostratus, or Cirrostratus alone, progressively Invading the sky; they generally thicken as a whole; the continuous veil extends more than 45 degrees above the horizon, without the sky being totally covered";
    } elsif ( $cloudType eq "CI" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/17";
        $title = "Cirrostratus covering the whole sky";
    } elsif ( $cloudType eq "CS" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/18";
        $title = "Cirrostratus not progressively invading the sky and not entirely covering it";
    } elsif ( $cloudType eq "CC/CH" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/19";
        $title = "Cirrocumulus alone, or Cirrocumulus predominant among the CH clouds";
    } elsif ( $cloudType eq "NOCM" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/20";
        $title = "No CM clouds";
    } elsif ( $cloudType eq "AT" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/21";
        $title = "Altostratus translucidus";
    } elsif ( $cloudType eq "AO/NS" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/22";
        $title = "Altostratus opacus or Nimbostratus";
    } elsif ( $cloudType eq "AT1" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/23";
        $title = "Altocumulus translucidus at a single level";
    } elsif ( $cloudType eq "PATCH" ) {
        $link = "http://codes.wmo.int/bufr4/codeflag/0-20-012/24";
        $title = "Patches (often lenticular) of Altocumulus translucidus, continually changing and occurring at one or more levels";
    }
    }
    $title;
}

# These are the hazard codes that make it through the decoder:
#   ASH
#   CONVECTIVE
#   ICE
#   IFR
#   MTN OBSCN
#   TURB
# 
sub lookupSigmetHazardLinkUS()
{
    my( $pkgName, $hazard, $severity ) = @_;
    my $link = "Unknown";
    my $title = "Unknown";

    if( defined($hazard) ) {
        if ( $hazard eq "ASH" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/VA";
            $title = "Volcanic ash";
        } elsif ( $hazard eq "CONVECTIVE" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/FRQ_TS";
            $title = "Frequent thunderstorm";
        } elsif ( $hazard eq "ICE" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/SEV_ICE";
            $title = "Severe airframe icing";
        } elsif ( $hazard eq "IFR" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/Unknown";
            $title = "Unknown";
        } elsif ( $hazard eq "MTN OBSCN" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/Unknown";
            $title = "Unknown";
        } elsif ( $hazard eq "TURB" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/SEV_TURB";
            $title = "Severe turbulence";
        }
    }

    $link;
}

sub lookupSigmetHazardTitleUS()
{
    my( $pkgName, $hazard, $severity ) = @_;
    my $link = "Unknown";
    my $title = "Unknown";

    if( defined($hazard) ) {
        if ( $hazard eq "ASH" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/VA";
            $title = "Volcanic ash";
        } elsif ( $hazard eq "CONVECTIVE" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/FRQ_TS";
            $title = "Frequent thunderstorm";
        } elsif ( $hazard eq "ICE" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/SEV_ICE";
            $title = "Severe airframe icing";
        } elsif ( $hazard eq "IFR" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/Unknown";
            $title = "Unknown";
        } elsif ( $hazard eq "MTN OBSCN" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/Unknown";
            $title = "Unknown";
        } elsif ( $hazard eq "TURB" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/SEV_TURB";
            $title = "Severe turbulence";
        }
    }
    $title;
}

sub lookupSigmetPhenomenonLink()
{
    my( $pkgName, $phenomenon ) = @_;
    my $link = "Unknown";
    my $title = "Unknown";

    if( defined($phenomenon) ) {
        if ( $phenomenon eq "EMBD TS" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/EMBD_TS";
            $title = "Embedded thunderstorm";
        } elsif ( $phenomenon eq "EMBD TSGR" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/EMBD_TSGR";
            $title = "Embedded thunderstorm with hail";
        } elsif ( $phenomenon eq "FRQ TS" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/FRQ_TS";
            $title = "Frequent thunderstorm";
        } elsif ( $phenomenon eq "FRQ TSGR" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/FRQ_TSGR";
            $title = "Frequent thunderstorm with hail";
        } elsif ( $phenomenon eq "HVY DS" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/HVY_DS";
            $title = "Heavy dust storm";
        } elsif ( $phenomenon eq "HVY SS" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/HVY_SS";
            $title = "Heavy sand storm";
        } elsif ( $phenomenon eq "OBSC TC" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/OBSC_TC";
            $title = "Obscured thunderstorm";
        } elsif ( $phenomenon eq "OBSC TCGR" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/OBSC_TCGR";
            $title = "Obscured thunderstorm with hail";
        } elsif ( $phenomenon eq "RDOACT CLD" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/RDOACT_CLD";
            $title = "Radioactive cloud";
        } elsif ( $phenomenon eq "SEV ICE" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/SEV_ICE";
            $title = "Severe airframe icing";
        } elsif ( $phenomenon eq "SEV ICE FZRA" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/SEV_ICE_FZRA";
            $title = "Severe airframe icing from freezing rain";
        } elsif ( $phenomenon eq "SEV MTW" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/SEV_MTW";
            $title = "Severe mountain wave";
        } elsif ( $phenomenon eq "SEV TURB" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/SEV_TURB";
            $title = "Severe turbulence";
        } elsif ( $phenomenon eq "SQL TS" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/SQL_TS";
            $title = "Squall line";
        } elsif ( $phenomenon eq "SQL TSGR" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/SQL_TSGR";
            $title = "Squall line with hail";
        } elsif ( $phenomenon eq "TC" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/TC";
            $title = "Tropical cyclone";
        } elsif ( $phenomenon eq "VA" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/VA";
            $title = "Volcanic ash";
        }
    }

    $link;
}

sub lookupSigmetPhenomenonTitle()
{
    my( $pkgName, $phenomenon ) = @_;
    my $link = "Unknown";
    my $title = "Unknown";

    if( defined($phenomenon) ) {
        if ( $phenomenon eq "EMBD TS" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/EMBD_TS";
            $title = "Embedded thunderstorm";
        } elsif ( $phenomenon eq "EMBD TSGR" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/EMBD_TSGR";
            $title = "Embedded thunderstorm with hail";
        } elsif ( $phenomenon eq "FRQ TS" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/FRQ_TS";
            $title = "Frequent thunderstorm";
        } elsif ( $phenomenon eq "FRQ TSGR" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/FRQ_TSGR";
            $title = "Frequent thunderstorm with hail";
        } elsif ( $phenomenon eq "HVY DS" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/HVY_DS";
            $title = "Heavy dust storm";
        } elsif ( $phenomenon eq "HVY SS" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/HVY_SS";
            $title = "Heavy sand storm";
        } elsif ( $phenomenon eq "OBSC TC" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/OBSC_TC";
            $title = "Obscured thunderstorm";
        } elsif ( $phenomenon eq "OBSC TCGR" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/OBSC_TCGR";
            $title = "Obscured thunderstorm with hail";
        } elsif ( $phenomenon eq "RDOACT CLD" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/RDOACT_CLD";
            $title = "Radioactive cloud";
        } elsif ( $phenomenon eq "SEV ICE" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/SEV_ICE";
            $title = "Severe airframe icing";
        } elsif ( $phenomenon eq "SEV ICE FZRA" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/SEV_ICE_FZRA";
            $title = "Severe airframe icing from freezing rain";
        } elsif ( $phenomenon eq "SEV MTW" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/SEV_MTW";
            $title = "Severe mountain wave";
        } elsif ( $phenomenon eq "SEV TURB" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/SEV_TURB";
            $title = "Severe turbulence";
        } elsif ( $phenomenon eq "SQL TS" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/SQL_TS";
            $title = "Squall line";
        } elsif ( $phenomenon eq "SQL TSGR" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/SQL_TSGR";
            $title = "Squall line with hail";
        } elsif ( $phenomenon eq "TC" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/TC";
            $title = "Tropical cyclone";
        } elsif ( $phenomenon eq "VA" ) {
            $link = "http://codes.wmo.int/49-2/SigWxPhenomena/VA";
            $title = "Volcanic ash";
        }
    }
    $title;
}

1;
# --------------------------------------------------------------------
