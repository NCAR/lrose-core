// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
#include <string>
#include <toolsa/umisc.h>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>
#include <toolsa/MemBuf.hh>
#include <toolsa/DateTime.hh>
#include <euclid/TypeGrid.hh>
#include <Mdv/mdv/mdv_grid.h>
#include <Mdv/mdv/MdvFile.hh>
#include <Mdv/mdv/MdvField.hh>
#include <Mdv/mdv/MdvVlevel.hh>
#include <Mdv/mdv/MdvChunk.hh>
#include <Mdv/mdv/MdvRead.hh>
using namespace std;

const time_t MdvFile::NEVER = DateTime::NEVER;

int MdvFile::test = 0;

MdvFile::MdvFile()
{
   init();
}

MdvFile::MdvFile( const char* top ) 
{
   init();
   topDir = top;
}

MdvFile::MdvFile( const MdvFile &mdv )
{
   copy( mdv );
}

MdvFile::~MdvFile()
{
   clearData();
}

MdvFile& 
MdvFile::operator= ( const MdvFile &source )
{
   copy( source );
   return *this;
}

void 
MdvFile::copy( const MdvFile &source )
{
   topDir    = source.topDir;
   masterHdr = source.masterHdr;
   vlevels   = source.vlevels;
   chunks    = source.chunks;
   setFields( (vector< MdvField* > &)source.fields );
   _isDebug   = source._isDebug;
   _isVerbose = source._isVerbose;
}

void
MdvFile::init()
{
   _isDebug = false;
   _isVerbose = false;

   LDATA_init_handle( &ldataIndex, "MdvFile::MdvFile", 0 );
}

void
MdvFile::setFields( vector< MdvField* > &sourceFields )
{
   string name;
   vector< MdvField* >::iterator f;

   //
   // Get rid of the old stuff
   //
   clearData();

   //
   // Create new fields from the incoming fields
   //
   for( f=sourceFields.begin(); f != sourceFields.end(); f++ ) {
      name = (*f)->getName();
      addField( name, *((*f)->getGrid()) );
   }
}

void
MdvFile::setFields( vector< pair< string*, Grid* >* >& sourceFields )
{
   string  name;
   Grid   *grid;
   vector< pair< string*, Grid* >* >::iterator f;

   //
   // Get rid of the old stuff
   //
   clearData();

   //
   // Create new fields from the incoming fields
   //
   for( f=sourceFields.begin(); f != sourceFields.end(); f++ ) {
      name = *((*f)->first);
      grid = (*f)->second;
      addField( name, *grid );
   }
}

MdvField*
MdvFile::addField( const string& name, const Grid& grid )
{
   MdvField *field = new MdvField( name, grid );
   fields.push_back( field );

   //
   // Check for max grid dimensions
   //
   checkGeometry( grid );
   masterHdr.setNumFields( fields.size() );

   return( field );
}

void
MdvFile::clearData()
{
   vector< MdvField* >::iterator f;

   //
   // Clear out the old data fields and related geometry info
   // in the master header
   //
   for( f=fields.begin(); f != fields.end(); f++ ) {
      delete (*f);
   }

   fields.erase( fields.begin(), fields.end() );
   masterHdr.clearGeometry();
}

MdvField*
MdvFile::getField( const char* fieldName )
{
   vector< MdvField* >::iterator f;

   //
   // Find a specified field by name
   //
   for( f=fields.begin(); f != fields.end(); f++ ) {
      if ( !strcmp( fieldName, (*f)->getName() ) )
         return (*f);
   }

   return NULL;
}

MdvField*
MdvFile::getField( const int fieldNum )
{
   int i;
   vector< MdvField* >::iterator f;

   //
   // Find a specified field by name
   //
   for( i=0,f=fields.begin(); f != fields.end(); i++,f++ ) {
      if ( i == fieldNum )
         return (*f);
   }

   return NULL;
}

void
MdvFile::checkGeometry( const Grid &grid )
{
   bool   differ = false;
   size_t nfields=fields.size(); 
   size_t nx, ny, nz, dim;
   size_t nxMax, nyMax, nzMax, dimMax;

   //
   // Degenerate case
   //
   if ( !grid.isGeometryKnown() ) {
      return;
   }

   //
   // Get the grid geometry
   //
   nx  = grid.getNx();
   ny  = grid.getNy();
   nz  = grid.getNz();
   dim = grid.getDimension();

   if ( nfields == 1 ) {
      //
      // This is the first field, set the max grid dimensions
      //
      nxMax  = nx;
      nyMax  = ny;
      nzMax  = nz;
      dimMax = dim;
      differ = false;
   }
   else {
      //
      // Compare this grid against current maximums
      //
      masterHdr.getGeometry( &nxMax, &nyMax, &nzMax, &dimMax );
      if ( nx != nxMax  ||  ny != nyMax ||
           nz != nzMax  ||  dim != dimMax ) {
         differ = true;
         if ( nx > nxMax )
            nxMax = nx;
         if ( ny > nyMax )
            nyMax = ny;
         if ( nz > nzMax )
            nzMax = nz;
         if ( dim > dimMax )
            dimMax = dim;
      }

   }

   //
   // Update the master header, if necessary
   //
   if ( nfields == 1  ||  differ ) {
      masterHdr.setGeometry( nxMax, nyMax, nzMax, dimMax, differ );
   }
}

int
MdvFile::setVlevel( int index, const MDV_vlevel_header_t & vlevel )
{
   if (index >= (int) fields.size()) {
      return -1;
   }

   // Initialize the vlevels if none have been added yet.
   if (vlevels.size() == 0) {
      MDV_vlevel_header_t initVlevel;
      MEM_zero(initVlevel);
      MdvVlevel mdvvlevel(initVlevel);
      for (size_t i = 0; i < fields.size(); i++) {
         vlevels.push_back( mdvvlevel );
      }
   }

   vlevels[index] = vlevel;

   // This does NOT set the num levels, it sets the master header flag
   //   vlevel_included to one.
   // 
   masterHdr.setNumLevels(fields.size());

   return 0;
}

void
MdvFile::setChunks( vector< MdvChunk* > &newChunks )
{
   MdvChunk            *chunk;
   vector< MdvChunk* >::iterator c;

   //
   // Get rid of references to the the old stuff
   //
   chunks.erase( chunks.begin(), chunks.end() );

   //
   // Store new vlevel references
   //
   for( c=newChunks.begin(); c != newChunks.end(); c++ ) {
      chunk = *c;
      addChunk( *chunk );                          
   }
}

void
MdvFile::addChunk( MdvChunk &chunk )
{
   chunks.push_back( &chunk );
   masterHdr.setNumChunks( chunks.size() );
}

////////////////////////////////////////////////////////////////////////////////
// MDV DATA OUTPUT
////////////////////////////////////////////////////////////////////////////////

int
MdvFile::write( time_t timeStamp, int encodeType /* = MDV_PLANE_RLE8 */ )
{
   FILE                 *outputFile = NULL;

   masterHdr.setTime( timeStamp );

   //
   // Make sure we can open the output file
   //
   outputFile = getOutputFile( timeStamp );
   if( outputFile == NULL )
      return( MDV_FAILURE );
   else if (write(outputFile, encodeType) != MDV_SUCCESS)
      return MDV_FAILURE;

   //
   // Move the temporary file (Bet you didn't even know there was one!)
   //   to some other place.
   //
   if ( rename( tmpOutputPath, outputPath.getPath().c_str() )) {
      return( MDV_FAILURE );
   }

   //
   // Write out an index file 
   //
   if ( LDATA_info_write( &ldataIndex, (char*)topDir.c_str(), timeStamp,
                          "mdv", NULL, NULL, 0, NULL) ) {
      return ( MDV_FAILURE );
   }   

   return( MDV_SUCCESS );
}

int
MdvFile::write( FILE * outputFile, int encodeType /* = MDV_PLANE_RLE8 */ )
{
   //
   // This function is modeled after MDV_write_all() but does not
   // use the MDV handle which assumes that it "owns" the headers and data
   // and frees them up as it sees fit.  Instead, the headers and data
   // are owned by the relevant classes.
   //
   int                   outputVolSize, nextOffset;
   MDV_master_header_t   blankMasterHdr;

   size_t                nfields, nlevels, nchunks;
   int                   i;
   Grid                 *grid;
   MdvField             *field;
   MdvChunk             *chunk;

   vector< MdvField* >::iterator  f;
   vector< MdvVlevel >::iterator v;
   vector< MdvChunk* >::iterator  c;

   if( outputFile == NULL )
      return( MDV_FAILURE );

   //
   // Some initialization
   //
   nfields = fields.size();
   nlevels = vlevels.size();
   nchunks = chunks.size();

   //
   // Degenerate case
   //
   if ( nfields == 0 ) {
      return( MDV_SUCCESS );
   }

   //
   // Inconsistency checks -- this would be an internal error!
   //
   if ( nfields != masterHdr.numFields()  ||  
        nchunks != masterHdr.numChunks()  ||
      ( nlevels != 1  &&  nlevels != masterHdr.numLevels() )) {
      return( MDV_FAILURE );
   }

   //
   // Master header offsets and timing
   //
   masterHdr.calcOffsets();

   //
   // Write out a blank master header, in case of failure
   //
   if ( fseek( outputFile, 0, SEEK_SET ) != 0 ) {
      fclose( outputFile );
      return( MDV_FAILURE );
   }
   memset( &blankMasterHdr, 0, sizeof(MDV_master_header_t) );
   if ( MDV_write_master_header( outputFile, &blankMasterHdr ) 
                                 != MDV_SUCCESS ) {
      fclose( outputFile );
      return( MDV_FAILURE );
   }

   //
   // Write out the vlevel headers
   //
   if ( nlevels > 0 ) {
      if ( nlevels == 1 ) {
         //
         // Use the same vlevel header for each field
         //
         v = (vlevels.begin());
         for( i=0; (size_t)i < nfields; i++ ) {
            if ( MDV_write_vlevel_header( outputFile, &(v->getInfo()),
                                          &(masterHdr.getInfo()), i ) 
                                          != MDV_SUCCESS ) {
                  fclose( outputFile );
                  return( MDV_FAILURE );
            }
         }
      }
      else {
         //
         // Use a different vlevel header for each field
         //
         for( i=0, v=vlevels.begin(); v != vlevels.end(); i++, v++ ) {
            if ( MDV_write_vlevel_header( outputFile, &(v->getInfo()),
                                          &(masterHdr.getInfo()), i ) 
                                          != MDV_SUCCESS ) {
                  fclose( outputFile );
                  return( MDV_FAILURE );
            }
         }
      }
      nextOffset = sizeof(MDV_master_header_t) +
                   (nfields * sizeof(MDV_field_header_t)) +
                   (nfields * sizeof(MDV_vlevel_header_t)) +
                   (nchunks * sizeof(MDV_chunk_header_t)) +
                   sizeof(si32);
   }
   else {
      nextOffset = sizeof(MDV_master_header_t) +
                   (nfields * sizeof(MDV_field_header_t)) +
                   (nchunks * sizeof(MDV_chunk_header_t)) +
                   sizeof(si32);
   }

   //
   // Process each field, calculating the data offsets as we go,
   // since the output offsets will be different than the offsets in
   // memory if the output encoding format is different.
   //
   for( i=0, f=fields.begin(); f != fields.end(); i++, f++ ) {
      field = *f;

      //
      // Make sure the field data geometry is updated
      //
      grid = field->getGrid();
      if ( !grid  ||  !grid->isGeometryKnown() ) {
         return( MDV_FAILURE );
      }
      checkGeometry( *grid );
      field->updateGeometry();

      //
      // Calculate the field offset and write out the field
      //
      field->setOffset( nextOffset );

      outputVolSize = MDV_write_field( outputFile, &(field->getInfo()),
				       field->getUnscaledData(), i,
				       nextOffset, encodeType );

      if ( outputVolSize < 0 ) {
         fclose( outputFile );
         return( MDV_FAILURE );
      }
      nextOffset += outputVolSize + (2 * sizeof(si32));
   }

   //
   // Write the chunks
   //
   for( i=0, c=chunks.begin(); c != chunks.end(); i++, c++ ) {
      chunk = *c;
      chunk->setOffset( (size_t)nextOffset );
      if ( MDV_write_chunk( outputFile, &(chunk->getInfo()), chunk->getData(),
                            &(masterHdr.getInfo()), i, 
                            nextOffset, FALSE ) != MDV_SUCCESS) {
         fclose( outputFile );
         return( MDV_FAILURE );
       }
      nextOffset += chunk->getInfo().size + (2 * sizeof(si32));
   }

   //
   // Write the master header and clear out its timestamps
   //
   if ( MDV_write_master_header( outputFile, 
                                 &(masterHdr.getInfo()) ) != MDV_SUCCESS ) {
      fclose( outputFile );
      return( MDV_FAILURE );
   }
   masterHdr.clearTime();
 
   //
   // Close the MDV file.
   //
   fclose( outputFile );

   return( MDV_SUCCESS );
}

FILE*
MdvFile::getOutputFile( time_t when )
{
   date_time_t *timeStamp = udate_time( when );

   //
   // Set the output path to: topDir/YYYYMMDD/HHMMSS
   //
   outputPath.clear();
   outputPath.setDirectory( topDir, 
                            timeStamp->year, timeStamp->month, timeStamp->day );
   outputPath.setFile( timeStamp->hour, timeStamp->min, timeStamp->sec, "mdv" );

   //
   // Create the directory, if it doesn't already exist
   //
   if ( outputPath.makeDir() != 0 ) {
      outputPath.clear();
      return( NULL );
   }

   //
   // Open a temporary file in the output directory
   //
   ta_tmp_path_from_final( (char *)outputPath.getPath().c_str(), tmpOutputPath,
                           MAX_PATH_LEN, "TMP_MDV" );
   return( fopen( tmpOutputPath, "w" ) );
}

void MdvFile::setMasterHeader(const MDV_master_header_t & header)
{
   // Set the master header's info.
   masterHdr.setInfo(header);

   // Reset the critical stuff on the header.
   masterHdr.clearGeometry();
   masterHdr.setNumFields( fields.size() );
   masterHdr.setNumLevels( vlevels.size() );
   masterHdr.setNumChunks( chunks.size() );

   // Reset the max geometry.
   bool first = true;
   bool differ = false;
   size_t nx = 0, ny = 0, nz = 0, dim = 0;
   size_t nxMax = 0, nyMax = 0, nzMax = 0, dimMax = 0;
   vector< MdvField* >::iterator f;
   for( f = fields.begin(); f != fields.end(); f++ ) {
      MdvField * field = *f;
      Grid * grid = field->getGrid();
      if (grid == NULL) {
         continue;
      }
     
      // After the first field, check for differences between fields.
      if (first) {
         first = false;
      }
      else if (nx != nxMax || ny != nyMax || nz != nzMax || dim != dimMax) {
        differ = true;
      }

      nx  = grid->getNx();
      ny  = grid->getNy();
      nz  = grid->getNz();
      dim = grid->getDimension();
      
      if ( nx > nxMax ) nxMax = nx;
      if ( ny > nyMax ) nyMax = ny;
      if ( nz > nzMax ) nzMax = nz;
      if ( dim > dimMax ) dimMax = dim;
   }
   masterHdr.setGeometry( nxMax, nyMax, nzMax, dimMax, differ );
}

////////////////////////////////////////////////////////////////////////////////
// MDV DATA INPUT
////////////////////////////////////////////////////////////////////////////////

int MdvFile::read(const string & dataFile,
                  int fieldIndex,
                  MDV_master_header_t & replyMasterHeader,
                  string & errString)
{
   if (_isVerbose) {
      cerr << "MdvFile::read(...): Performing MdvFile::read(...)." << endl;
   }

   MdvRead mdvReader;
   int status = mdvReader.openFile(dataFile);
   if (status < 0) {
      errString  = "MdvFile::read(...): Could not open file: ";
      errString += dataFile;
      if (_isDebug) {
         cerr << errString << endl;
      }
      return -1;
   }

   // Open was successful. Capture the master header.
   status = mdvReader.readMasterHeader();
   if (status < 0) {
      errString  = "MdvFile::read(...): Could not read master header on file: ";
      errString += dataFile;
      if (_isDebug) {
         cerr << errString << endl;
      }
      return -1;
   }

   replyMasterHeader = mdvReader.getMasterHeader();

   // Now put the data in the MdvFields.
   MdvField * currField = fields[fieldIndex];;
   
   // Verify that the field exists.
   if (currField == NULL) {
      errString  = "MdvFile::read(...): Attempted to read a NULL field.";
      if (_isDebug) {
         cerr << errString << endl;
      }
      return -1;
   }

   string statusString;
   status = _readField(mdvReader, *currField, statusString);
   if (status < 0) {
      char buf[10];
      sprintf(buf, "%d:\n", fieldIndex);
      errString  = "Could not read requested field: ";
      errString += buf;
      errString += statusString;
      if (_isDebug) {
         cerr << errString << endl;
      }
      return -1;
   }

   // Read vlevel, if any are present in disk file.
   if (masterHdr.getInfo().vlevel_included) {
      int srcNum = currField->getSourceFieldNum();
      const MDV_vlevel_header_t & vlevel
                          = mdvReader.getField(srcNum).getVlevelHeader();

      setVlevel(fieldIndex, vlevel);
   }

   return 0;
}

// Reads from a single mdv file on disk, to fill all the fields
//   currently in the MdvFile. Resets the master header info to
//   match read data.
// 
// Adds vlevels to the MdvFile if they are present in the disk file.
// 
// Todo: Change prototype to include flag "bool getChunks" which 
//       specifies whether to read all chunks out of disk file.
// 
int MdvFile::read(const string & dataFile, string & errString)
{
   if (_isVerbose) {
      cerr << "MdvFile::read(...): Performing MdvFile::read(...)." << endl;
   }

   MdvRead mdvReader;
   int status = mdvReader.openFile(dataFile);
   if (status < 0) {
      errString  = "MdvFile::read(...): Could not open file: ";
      errString += dataFile;
      if (_isDebug) {
         cerr << errString << endl;
      }
      return -1;
   }

   // Open was successful. Capture the master header.
   status = mdvReader.readMasterHeader();
   if (status < 0) {
      errString  = "MdvFile::read(...): Could not read master header on file: ";
      errString += dataFile;
      if (_isDebug) {
         cerr << errString << endl;
      }
      return -1;
   }

   // This call does an edited "set",
   //   and loses info such as presence of vlevels.
   // 
   setMasterHeader(mdvReader.getMasterHeader());

   // Now put the data in the MdvFields.
   int index;
   vector< MdvField * >::iterator f;
   for( f = fields.begin(), index = 0; f != fields.end(); f++, index++ ) {
      MdvField *currField = *f;

      // Verify that the field exists.
      if (currField == NULL) {
         errString  = "MdvFile contains a NULL field.";
         if (_isDebug) {
            cerr << errString << endl;
         }

         return -1;
      }

      // Read the data into the field.
      string statusString;
      status = _readField(mdvReader, *currField, statusString);
      if (status < 0) {
         char buf[10];
         sprintf(buf, "%d:\n", currField->getSourceFieldNum());
         errString  = "Could not read requested field: ";
         errString += buf;
         errString += ", ";
         errString += currField->getName();
         errString += ": ";
         errString += statusString;
         if (_isDebug) {
            cerr << errString << endl;
         }

         return -1;
      }

      // Read vlevel, if any are present in disk file.
      //   Use the original file's header, since it shows whether
      //   vlevels are in the file. This object's master header
      //   will say there are no vlevels if this is the first.
      // 
      if (mdvReader.getMasterHeader().vlevel_included) {
         int srcNum = currField->getSourceFieldNum();
         const MDV_vlevel_header_t & vlevel
                             = mdvReader.getField(srcNum).getVlevelHeader();

         setVlevel(index, vlevel);
      }
   }

   if (_isVerbose) {
      cerr << "MdvFile::read(...): Done reading fields." << endl;
   }

   return 0;
}

// Reads all fields from a single mdv file on disk, creating a new
//   MdvField for each. Uses the passed-in grid for geometry.
//   Resets the master header info on the file.
// 
// NOTE:
//   Allocates a Grid for each field found in the file.
//      These need to be deleted by the caller!
// 
int MdvFile::readAllFields(const string & dataFile,
                           const MdvField & protoField, string & errString)
{

   // Todo: Change to pointer to MdvField so as to allow for null field.
   //       Then construct a default grid if the field is null.

   // Extract the proto grid from the proto field.
   const Grid * protoGrid = protoField.getGrid();

   if (_isVerbose) {
      cerr << "MdvFile::readAllFields(...): Performing MDV_read_all." << endl;
   }


   MdvRead mdvReader;
   int status = mdvReader.openFile(dataFile);
   if (status < 0) {
      errString  = "MdvFile::readAllFields(...): Could not open file: ";
      errString += dataFile;
      if (_isDebug) {
         cerr << errString << endl;
      }
      return -1;
   }

   // Open was successful. Capture the master header.
   status = mdvReader.readMasterHeader();
   if (status < 0) {
      errString  = "MdvFile::readAllFields(...): Could not read master header on file: ";
      errString += dataFile;
      if (_isDebug) {
         cerr << errString << endl;
      }
      return -1;
   }

   // Read was successful. Capture the master header.
   MDV_master_header_t mhdr = mdvReader.getMasterHeader();

   setMasterHeader(mhdr);

   // Now put the data in the MdvFields.
   for (int i = 0; i < mhdr.n_fields; i++) {

      // Create a new grid of the proper type.
      Grid::DataType protoGridType = protoGrid->getDataType();
      Grid * currGrid      = NULL;
      MdvField * currField = NULL;
      switch (protoGridType) {
      case Grid::CHAR_GRID:
	currGrid =
	  new TypeGrid<ui08>(*((const TypeGrid<ui08> *) protoGrid) );
	break;
	
      case Grid::SHORT_GRID:
	currGrid =
	  new TypeGrid<ui16>(*((const TypeGrid<ui16> *) protoGrid) );
	break;
	
      case Grid::FLOAT_GRID:
	currGrid =
	  new TypeGrid<fl32>(*((const TypeGrid<fl32> *) protoGrid) );
	break;
	
      default:
	errString  = "MdvFile::readAllFields(...): proto grid type ";
	errString += "not supported.";
	if (_isDebug) {
	  cerr << errString << endl;
	}
	return -1;
	break;
      }
      currField = addField("", *currGrid);
      currField->setSourceFieldNum(i);
      
      if (_isVerbose) {
         cerr << "Reading field: " << currField->getSourceFieldNum() << endl;
      }

      // Read the data into the field.
      string statusString;
      status = _readField(mdvReader, *currField, statusString);
      if (status < 0) {
         char buf[10];
         sprintf(buf, "%d:\n", currField->getSourceFieldNum());
         errString  = "Could not read requested field: ";
         errString += buf;
         errString += statusString;
         if (_isDebug) {
            cerr << errString << endl;
         }

         return -1;
      }
   }

   // Add vlevels, if they are in the disk file.
   //   Use the original file's header, since it shows whether
   //   vlevels are in the file. This object's master header
   //   will say there are no vlevels if this is the first.
   // 
   if (mhdr.vlevel_included) {
      int index;
      vector< MdvField* >::iterator f;
      for( f = fields.begin(), index = 0; f != fields.end(); f++, index++ ) {
         MdvField * currField = *f;
   
         // Read vlevel, if any are present in disk file.
         int srcNum = currField->getSourceFieldNum();
         const MDV_vlevel_header_t & vlevel
                             = mdvReader.getField(srcNum).getVlevelHeader();

         setVlevel(index, vlevel);
      }
   }

   if (_isVerbose) {
      cerr << "MdvFile::readAllFields(...): Done." << endl;
   }

   return 0;
}

int MdvFile::_readField(MdvRead & mdvReader,
                        MdvField & field,
                        string & errString)
{

   if (_isVerbose) {
      cerr << "    MdvFile::_readField(): Reading a field" << endl;
   }

   Grid * outGrid = NULL;
   outGrid = field.getGrid();
   if (outGrid == NULL) {
      errString  = "Field has a NULL grid. Can't process.";
      if (_isDebug) {
         cerr << errString << endl;
      }

      return -1;
   }

   // Do the appropriate type of read -- field num or name.
   if (field.getSourceFieldNum() < 0) {
      if (strlen(field.getName()) <= 0) {
         errString  = "Field has no name or number. Can't process.";
         if (_isDebug) {
            cerr << errString << endl;
         }

         return -1;
      }

      int status = mdvReader.loadFieldNames();
      if (status < 0) {
         errString  = "Could not load field names from MdvRead.";
         if (_isDebug) {
            cerr << errString << endl;
         }

         return -1;
      }

      int dataFieldNum = mdvReader.getFieldNum(field.getName());
      if (dataFieldNum < 0) {
         errString  = "Field name \"";
         errString += field.getName();
         errString  = "\" not present in file.";
         if (_isDebug) {
            cerr << errString << endl;
         }

         return -1;
      }

      field.setSourceFieldNum(dataFieldNum);
   }

   // Set up the outGrid encode type, based on the request grid's type.

   int outGridEncodeType = -1;
   // int outGridEncodeBytes = -1;
   Grid::DataType outGridType = outGrid->getDataType();
   switch (outGridType) {
     case Grid::CHAR_GRID:
       outGridEncodeType = MDV_INT8;
       // outGridEncodeBytes = 1;
       break;
     case Grid::SHORT_GRID:
       outGridEncodeType = MDV_INT16;
       // outGridEncodeBytes = 2;
       break;
     case Grid::FLOAT_GRID:
       outGridEncodeType = MDV_FLOAT32;
       // outGridEncodeBytes = 4;
       break;
     default:
       errString  = "MdvFile Cannot Read Into A Grid Of The Supplied Type.";
       if (_isDebug) {
          cerr << errString << endl;
       }
       return -1;
       break;
   }

   // Read the volume
   // 
   int fieldNum = field.getSourceFieldNum();
   MDV_field_header_t outFieldHdr;
   int status = mdvReader.readVol(fieldNum, outGridEncodeType, &outFieldHdr);
   if (status < 0) {
      errString  = "Could not read field.";
      if (_isDebug) {
         cerr << errString << endl;
      }
      return -1;
   }

   // This is guaranteed to be a good ref if readVol(...) succeeded.
   MdvReadField & dataField = mdvReader.getField(fieldNum);
   
   if (_isVerbose) {
      cerr << "    MdvFile::_readField(): Getting field header ..." << endl;
   }

   // Set the field header appropriately.

   field.setInfo(outFieldHdr);

   if (_isVerbose) {
      cerr << "    Instantiating a grid object for the field data..." << endl;
   }

   // Instantiate a Grid object and put the field data in it.

   GridGeom geom(outFieldHdr.nx, outFieldHdr.ny, outFieldHdr.nz,
                 outFieldHdr.grid_dx, outFieldHdr.grid_dy, outFieldHdr.grid_dz,
                 outFieldHdr.grid_minx, outFieldHdr.grid_miny, outFieldHdr.grid_minz,
                 (double)outFieldHdr.proj_origin_lat, (double)outFieldHdr.proj_origin_lon,
                 Projection::lookupProjId(outFieldHdr.proj_type),
                 (double) outFieldHdr.proj_rotation);

   Grid * dataGrid = NULL;
   void * buffer = dataField.getVol1D();
   switch (outGridEncodeType) {
   case MDV_INT8:
     {
       TypeGrid<ui08> * tGrid =
	 new TypeGrid<ui08> ( Grid::CHAR_GRID, geom,
			      (ui08) 0,
			      (ui08) outFieldHdr.bad_data_value,
			      (ui08) outFieldHdr.missing_data_value);
       dataGrid = tGrid;
       
       status = tGrid->setFromTArray((ui08 *) buffer, geom,
				     (ui08) outFieldHdr.bad_data_value,
				     (ui08) outFieldHdr.missing_data_value);
       break;
     }
   case MDV_INT16:
     {
       TypeGrid<ui16> * tGrid =
	 new TypeGrid<ui16> ( Grid::CHAR_GRID, geom,
			      (ui16) 0,
			      (ui16) outFieldHdr.bad_data_value,
			      (ui16) outFieldHdr.missing_data_value);
       dataGrid = tGrid;
       
       status = tGrid->setFromTArray((ui16 *) buffer, geom,
				     (ui16) outFieldHdr.bad_data_value,
				     (ui16) outFieldHdr.missing_data_value);
       break;
     }
   case MDV_FLOAT32:
     {
       TypeGrid <fl32> * tGrid =
	 new TypeGrid <fl32> ( Grid::FLOAT_GRID, geom,
			       (float) 0,
			       (float) outFieldHdr.bad_data_value,
			       (float) outFieldHdr.missing_data_value);
       dataGrid = tGrid;
       status = tGrid->setFromTArray( (float *) buffer, geom,
				      (float) outFieldHdr.bad_data_value,
				      (float) outFieldHdr.missing_data_value);
       break;
     }
   default:
     status = -1;
     break;
   }

   if (status < 0) {
      errString  = "Could not resample file data into the temporary data grid ";
      errString += "in MdvFile::_readField().";
      if (_isDebug) {
         cerr << errString << endl;
      }

      delete dataGrid;
      return -1;
   }
    
   // 
   // Set up the request Grid with geometry from:
   //         o Bounding box (On Field -- combine with xy geom in dataGrid)
   //         o Plane Num    (On Field -- turn into minz on outGrid)
   //         o Plane Height (In Grid  -- single nz. Handled by using suggest)
   //         o isSearchZ    (On Field -- use to pick closest Z)
   // 

   // Convert bounding box to grid coordinates.
   GridGeom dataGeomCopy = dataGrid->getGeometry();
   if (field.isBoundingBoxRequest()) {

      int nx, ny;
      float minX, minY;
      double dataMinX, dataMinY, dataDx, dataDy;
      double boxMinX, boxMinY, boxMaxX, boxMaxY;
      double boxMinLat, boxMinLon, boxMaxLat, boxMaxLon;
      int minXOffset, minYOffset, maxXOffset, maxYOffset;

      field.getBoundingBox(boxMinLat, boxMinLon, boxMaxLat, boxMaxLon);
      int status = dataGeomCopy.latlon2xy(boxMinLat, boxMinLon, &boxMinX, &boxMinY);
      if (status < 0) {
         errString = "Could not determine minX and minY for bounding box.";
         if (_isDebug) {
            cerr << errString << endl;
         }

         delete dataGrid;
         return -1;
      }
      status = dataGeomCopy.latlon2xy(boxMaxLat, boxMaxLon, &boxMaxX, &boxMaxY);
      if (status < 0) {
         errString = "Could not determine maxX and maxY for bounding box.";
         if (_isDebug) {
            cerr << errString << endl;
         }

         delete dataGrid;
         return -1;
      }

      if (_isVerbose) {
         cerr << "    Box stuff: " << boxMinX << " " << boxMinY << " "
              << boxMaxX << " " << boxMaxY << endl;
      }

      dataMinX = (double) dataGeomCopy.getMinx();
      dataMinY = (double) dataGeomCopy.getMiny();
      dataDx   = (double) dataGeomCopy.getDx();
      dataDy   = (double) dataGeomCopy.getDy();

      if (_isVerbose) {
         cerr << "    Data stuff: " << dataMinX << " " << dataMinY << " "
              << dataDx << " " << dataDy << endl;
      }

      if (dataDx == 0.0 || dataDy == 0.0) {
         errString = "Data grid has zero dx or dy. Can't use bounding box.";
         if (_isDebug) {
            cerr << errString << endl;
         }

         delete dataGrid;
         return -1;
      }

      minXOffset = (int) floor( (boxMinX - dataMinX) / dataDx );
      minYOffset = (int) floor( (boxMinY - dataMinY) / dataDy );
      maxXOffset = (int) ceil( (boxMaxX - dataMinX) / dataDx );
      maxYOffset = (int) ceil( (boxMaxY - dataMinY) / dataDy );

      if (_isVerbose) {
         cerr << "    Offset stuff: " << minXOffset << " " << minYOffset
              << " " << maxXOffset << " " << maxYOffset << endl;
      }

      if (minXOffset < 0) minXOffset = 0;
      if (minYOffset < 0) minYOffset = 0;
      if (maxXOffset < minXOffset) maxXOffset = minXOffset;
      if (maxYOffset < minYOffset) maxYOffset = minYOffset;

      nx = maxXOffset - minXOffset;
      ny = maxYOffset - minYOffset;
      minX = (float) (dataMinX + ( ((double) nx) * dataDx ));
      minY = (float) (dataMinY + ( ((double) ny) * dataDy ));

      // Set the xy geometry on the outGrid.
      GridGeom outGeomCopy = outGrid->getGeometry();
      outGeomCopy.set(nx, ny, (float) dataDx, (float) dataDy, minX, minY);
      outGrid->setGeometry(outGeomCopy);
   }

   // Request for a specific plane index.
   if (field.getPlaneNum() != GridGeom::UNKNOWN_SIZE) {
      int planeNum = field.getPlaneNum();
      float dataMinz = dataGeomCopy.getMinz();
      float dataDz = dataGeomCopy.getDz();
      float minz = dataMinz + ( ((float) planeNum) * dataDz );
      
      // Set the z geometry on the outGrid.
      GridGeom outGeomCopy = outGrid->getGeometry();
      outGeomCopy.set(1, dataDz, minz);
      outGrid->setGeometry(outGeomCopy);
   }

   // Check if have to recalc Z geometry to search for a closest level.
   if (field.isSearchZRequest()) {

      // Note: Search z is only valid if the client has specified *one* level.

      if (outGrid->getNz() != 1) {
         errString = "Not looking for exactly one z level. Can't use search Z.";
         if (_isDebug) {
            cerr << errString << endl;
         }

         delete dataGrid;
         return -1;
      }

      float dataDz = dataGeomCopy.getDz();
      if (dataDz == 0.0) {
         errString = "Data grid has zero dz. Can't use search Z.";
         if (_isDebug) {
            cerr << errString << endl;
         }

         delete dataGrid;
         return -1;
      }

      float minz = outGrid->getMinz();
      int index = (int) ( (minz - dataGeomCopy.getMinz()) / dataDz );

      if (index < 0) {
         minz = dataGeomCopy.getMinz();
      }
      else if (index >= (int) dataGeomCopy.getNz()) {
         minz = dataGeomCopy.getMinz()
              + (dataGeomCopy.getDz() * ((float) (dataGeomCopy.getNz() - 1)));
      }
      else {
         // Calculate the correct minz closest to the requested level.
         minz = ((float) index) * dataDz + dataGeomCopy.getMinz();
      }
      
      // Set the z geometry on the outGrid.
      GridGeom outGeomCopy = outGrid->getGeometry();
      outGeomCopy.set(1, dataDz, minz);
      outGrid->setGeometry(outGeomCopy);
   }

   outGrid->suggestGeometry(*dataGrid);
   outGrid->suggestValueDefs(*dataGrid);

   if (_isVerbose) {
      cerr << "    Transferring the data from the data grid "
           << "into the request grid..."
           << endl;
   }

   // resample the data grid

   status = outGrid->resampleData(*dataGrid);

   if (status < 0) {
     errString = "Could not resample from data grid to output grid.";
     if (_isDebug) {
       cerr << errString << endl;
     }
     
     delete dataGrid;
     return -1;
   }
   
   // Include this code if no data is an error.
   // 
   // NOTE:  Now that the outGrid is a base class Grid*
   //        this code cannot be included because getData()
   //        is defined on the subclasses.  A different method
   //        for checking for null data would be needed.  (t.b. 6/15/99)
   //
   // // No data is an error, b/c the client asked for something not there..
   // // 
   // if (outGrid->getData() == NULL) {
   //     char buf[24];
   //     sprintf(buf, "%d", field.getSourceFieldNum());
   //     errString  = "No data found after resampling into request grid in field: ";
   //     errString += buf;
   //     errString += ".";
   //     if (_isDebug) {
   //        cerr << errString << endl;
   //     }
   //     return -1;
   // }

   if (_isVerbose) {
      cerr << "   MdvFile::_readField(): Done reading field. Success." << endl;
   }

   delete dataGrid;
   return 0;
}
