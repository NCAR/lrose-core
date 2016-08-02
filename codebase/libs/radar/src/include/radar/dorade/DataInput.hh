//
// Declarations of the various datainput classes.
//
# include <radar/dorade/dd_sweepfiles.hh>

//
// DataInput itself is virtual, and meant to be subclassed with something
// specific.
//

class DataInput
{
protected:
	dd_mapper *Mapper;	// Mapper used to represent a beam
	
public:
	DataInput ()
	{
		Mapper = new dd_mapper;
	}
	virtual ~DataInput ()
	{
		delete Mapper;
	}
	virtual dd_mapper *get_beam () = 0;	// Error states?
	virtual int start_input () = 0;
	virtual void stop_input () = 0;
};



//
// Now we get to subclasses which actually do something.  This one takes
// data from a sweep file.
//

class DFileInput : public DataInput
{
private:
	dd_sweepfile_access *SFile;
	int Zorch;
	int FirstBeam;
	
public:
	int OK;
	DFileInput (const char *file, int zorch = 0);
	DFileInput (int zorch = 0);
	~DFileInput ();
	int NewFile (const char *file);
	dd_mapper *get_beam ();
//
// Start and stop currently null for sweepfile input -- it should ought
// to be quick so we don't do it async.
//
	int start_input () {return 0; };
	void stop_input () {};
};
