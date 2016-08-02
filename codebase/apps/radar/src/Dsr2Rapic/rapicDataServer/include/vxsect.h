#ifndef	__VXSECT_H
#define __VXSECT_H

/* 
   vxsect.h
    
   Vertical cross-section product class    
*/

#include "rdr.h"
#include "rdrutils.h"
#include "rdrscan.h"
#include <vector>
using namespace std;

#define MAXELEVCOUNT 30

enum VXSECTMODE {VXS_RHI, VXS_VXSECT, VXS_DUALSECT};

struct RTH {
    float	r, th;
    };

/* 
    vxsect node, 1 per az, with distance from ref pnt 1 (d), 
    gnd rng (rg) for ht calc and prev ht and prev val for mesh use
    
    IMPORTANT NOTE:
    IN ORDER TO CONVERT STANDARD MATHS ANGLES WHERE 0DEGREES LIES
    ON THE X-AXIS AND ANGLES ARE +VE COUNTERCLOCKWISE TO
    0DEGREES AZ LIES ALONG THE NORTH AXIS AND ANGLES ARE +VE 
    CLOCKWISE,  SIMPLY USE KMN=X AND KME=Y
    IE TRANSPOSE NORMAL X/Y AXIS. THIS REFLECTS ABOUT THE X=Y AXIS
    WHICH BOTH REVERSES ANGLE DIRECTION AND ROTATES 0DEGREES TO THE
    NORTH AXIS. SIMPLE
    
*/    
    

struct vxsect_data
{
  float d,        // distance from West-most point (draw as x)
    h;            // height above earth            (draw as y)
  uchar idx;      // index val
};
    

struct vxSectNodeData {
  rdr_angle az;   // az of this node
  float d,  // this node distance from western most node in km
    rg,	    // this node tangent rng from radar origin in km
    ht,	    // this node ht from tangent plane in km
    kmn,    // this node radar rel. kmn
    kme;    // this node radar rel. kmn/kme
  int       datapos;   // position of this data in vxSectTilt data vector
  bool      valid;
};

struct vxSectNode
{
  vxSectNodeData node1, centrenode, node2;  
  // node1 is entry extreme of radl, node2 is exit extreme and centre is centre
  int            rngbins;  // number of rng bins btwn node1, node2
  bool           valid;
  vxSectNode();
  ~vxSectNode() {};
};



struct vxsectnode {
  rdr_angle az;   // az of this node
  bool valid;
  float d,	    // this node distance from western most node in km
    rg,	    // this node tangent rng from radar origin in km
    kmn,   // this node radar rel. kmn
    kme, // this node radar rel. kmn/kme
    ht,	    // height of this node (km)
    //	    val,    //	value of this node
    //	    prevval,// previous value for this node, for mesh rendering
    prevht; // previous height for this node (km), for mesh rendering
  char    data, prevdata;
  
  void reset() { prevht = ht = 0; prevdata = data = 0; };
};

#define MAXNODES 182

struct vxsect_set {
  void nodes_reqd(int nodescount);
  int azres;
  int nodecount;
  vector<vxsectnode> entry_nodes;  // add an extra "entry node" to use as final exit node
  vector<vxsectnode> centre_nodes;
  //  vxsectnode nodes[MAXNODES];
  //  char data[MAXNODES];
};

class vxSectTilt
{
 public:
  bool valid;
  //  uchar *data;
  vector<uchar> data;
  int   data_size;
  int   buff_pos;
  int   valid_nodes;
  rdr_angle el;
  float sin_el,cos_el;
  rdr_scan *tilt_scan;
  vector<vxSectNode> nodes;
  vxSectTilt(int nodecount = 182, int buffsize = 512);
  ~vxSectTilt();
  void init(int nodecount = 182, int buffsize = 512);
  void setNodeCount(int nodecount);
  void setBuffSize(int buffsize);
  void clear();
  void close();
  uchar getData(int pos);
  // sub node data allows multiple rng values btwn az1 and az2 in a node to 
  // be used
  float d_az, d_d, d_rg, d_ht, d_kmn, d_kme;
  int d_datapos;
  int  subNodeSteps;
  vxSectNode *thissubnode;
  int resetSubNodeData(vxSectNode *node);  // set up subnode step deltas, return steps
  int resetSubNodeData(int nodenum);  // set up subnode step deltas, return steps
  bool getSubNodeData(vxSectNode *node, int step, vxSectNodeData *subnodedata);
  bool getSubNodeData(int step, vxSectNodeData *subnodedata);
  bool getSubNodeDataFlat(vxSectNode *node, int step, vxSectNodeData *subnodedata);
};

class VertXSect {
    bool	    ConstKmN1, nodes_swapped;	/* if true, SPECIAL CASE, line parallel with east axis */
    bool            posSet;                     // true until pos has been set
    float	    m1, c1;			/* variables for kmn=m1*kme+c1 xsect line */
public:
    float	    orgn1, orge1, orgn2, orge2;	/* raw origin points, kmn etc may be swapped */
    int		    selnode;			/* selected node for move */
    float	    LineLen;			/* length of chosen section (km) */
    float	    kme1, kmn1, kme2, kmn2; 
    float	    az1, r1, az2, r2;		/* to allow wrap around allow az2 > 359 */
    /* az1 will always be the westmost az  */
    bool	    ThroughOrigin;
    bool	    NewSection;
    VertXSect(int tiltcount = 20);
    ~VertXSect();
    void NewKmNE(float kmn, float kme,
		 bool forceCentreMove = false);	/* move closest point to new x/y */
    void New2KmNE(float kmn1, float kme1, 
	float kmn2, float kme2);		/* Set both coordinates */
    void Get2KmNE(float &kmn1, float &kme1, 
	float &kmn2, float &kme2);		/* Set both coordinates */

    void resizeTilts(int newsize);
    //    void closeTilts();
    rdr_scan *thisVolScan; // currently loaded vol scan
    e_data_type data_type;
    void  calcNodeData(rdr_angle az, vxSectNodeData *nodedata, 
		       float SinEl, float CosEl);
    void  calcNode(s_radl *radl, vxSectTilt *tilt, int pos, bool ccw);
    void  calcMissingNode(vxSectTilt *tilt, int pos);
    void  calcNewTilt(rdr_scan *newscan, int tiltnum);		/* */
    void  clearTilts();
    void  loadNewVolScan(rdr_scan *volscan = NULL, e_data_type datatype = e_refl);
    int   tiltCount;
    vector<vxSectTilt> tilts;  // tilts 

};
 
#endif	/* __VXSECT_H */
