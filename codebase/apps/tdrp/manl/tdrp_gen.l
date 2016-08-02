.\" *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
.\" ** Copyright UCAR (c) 1992 - 2010 */
.\" ** University Corporation for Atmospheric Research(UCAR)
.\" ** National Center for Atmospheric Research(NCAR)
.\" ** Research Applications Laboratory(RAL)
.\" ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
.\" ** 2010/10/7 23:11:39
.\" *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
.\" @@(#)tdrp_gen.1 1.0 97/06/25; from Dixon/Caron
.TH TDRP_GEN 3L "25 June 1997"
.SH NAME
tdrp_gen \- table-driven runtime parameter generation
.SH SYNOPSIS
.B tdrp_gen
[
.I <module-name>
]
[
.I [-f <paramdef-file>]
]
.SH DESCRIPTION
tdrp_gen uses a parameter definition file, which describes the 
runtime parameters to be read into the application, to generate the 
files <mod>_tdrp.h and <mod>_tdrp.c. <mod>_tdrp.h defines a structure
in which the parameters are stored. <mod>_tdrp.c defines a table which
is used at runtime to read in the actual parameter file and store the 
values into the structure.  
.SH OPTIONS
.TP
.B \<module-name>
module name: all externals prepended with this name
.br
.ne 8
.TP
.B \-f
paramdef file: parameter definition file (default = 
'paramdef.<module-name>')
.br
.ne 8
.SH PARAMETER TYPES
.nf
     +  boolean - Can be set to true or false.
     +  long - Can be set to any valid signed long value.
     +  double - Can be set to any valid signed double value.
     +  char * - Can be set to any valid string.
     +  enum - Can be set to any specified enum type.
     +  struct - Can be any  combination of the above types.
     +  arrays - Can be any  combination of the above types, 
        including struct

.SH PARAMETER DEFINITION SYNTAX
.nf
	paramdef <parameter type> {
	    <field definition>;
	    <field definition>;
	    ....
	    } <parameter_name> [<array declaration>] ;

	<parameter type> = boolean | long | double | char * | 
		enum <type name> | struct <type name>

     	<field definition> = 
       		p_descr = "<description string>"	|
       		p_help = "<help string>"		|
       		p_private = <boolean>			|
       		p_default = { <default value(s)> }	| 
       		p_min = { <minimum long/double value> } |
       		p_max = { <maximum long/double value> } |
       		p_options = { <enum values list> }	|
       		p_field_type = { <field type list> }	|
       		p_field_name = { <field title list> }

	<array_declaration> = []  | [<max_array_elements>]

.SH PARAMETER DEFINITION RULES
.p
If the description or help strings are not set, they are left 
as null pointers.

The private field defaults to FALSE. If set to TRUE, the value
of this field is always equal to its default.

The default value must match the parameter type.

The min and max values can only be set for types long and double.

The first time a parameter of type "enum <type name>" is encountered is 
the defining instance of that enum type.  The p_options definition 
defines the possible enumerated values.  Subsequent parameters may be
of type "enum <type name>", but may not contain a p_options definition.

The first time a parameter of type "struct <type name>" is encountered
is the defining instance of that struct type. The p_field_type and
p_field_name definitions define the fields of that structure.
Subsequent parameters may be of type "struct <type name>", but may not 
contain a p_field_type or p_field_name definition.

The default values of a structure may be defined by specifying a list
of values, one for each field.  

Arrays are always dynamic. The <max_array_elements> is used only to
bound the array. The size of the actual array is determined by the
parameter file at runtime.  Since array elements come into existence 
by giving them values, arrays never have default values.

Structures may not contain arrays, nor other structures. Structures may
contain an enum type if the enum type has already been defined.

.SH FILES
.nf
	These are output files:
		<mod>_tdrp.h
		<mod>_tdrp.c
	


.SH SEE ALSO
.f
.BR tdrp(3),
.nf
Examine <mod>_tdrp.h.
.SH AUTHOR(S)
.nf
Mike Dixon (John Caron, John Yunker) NCAR/RAP

.SH BUGS
.p
yacc/lex parsing has limited error reporting.

