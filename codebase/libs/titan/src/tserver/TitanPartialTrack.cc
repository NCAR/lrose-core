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
////////////////////////////////////////////////////////////////
// TitanPartialTrack.cc
//
// Partial track object for data from TitanServer
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2001
//
////////////////////////////////////////////////////////////////
//  Identifies the partial track requested.
// 
//  A complex track represents a storm which may or may not exhibit
//  mergers or splits during its lifetime. It is made up of one or
//  more simple tracks. A simple track represents a part of the
//  complex track for which no mergers or splits occurred. A merger
//  is represented by two or more simple tracks ending and one simple
//  track starting at the following scan time. The connections between
//  the simple tracks are recorded by noting the parents and children
//  of each simple track. Similarly a split is represented by one simple
//  track ending and two or more simple tracks starting at the next
//  scan time.
// 
//  In this module we represent the complex track as a tree of vertices
//  and edges, in which each simple track is a vertex, and the edges are the
//  connections between the simple tracks, represented by the list of
//  parents and children of each simple track.
//  
//  We only consider simple tracks which start on or before
//  the verify scan number and end after or on the generate
//  scan number. This breaks the tree into a forest of sub-trees,
//  and deletes some of the edges in the complex tree.
// 
//  The first major step is to determine the connectivity of the forest,
//  thereby identifying the sub-trees.
//
////////////////////////////////////////////////////////////////////
 

#include <titan/TitanPartialTrack.hh>
using namespace std;


////////////////////////////////////////////////////////////
// Constructor

TitanPartialTrack::TitanPartialTrack(bool debug /* = false*/) :
  _debug(debug)

{

  BD_TREE_init_handle(&_tree);
  _sparams = NULL;
  _n_sparams_alloc = 0;

}

////////////////////////////////////////////////////////////
// destructor

TitanPartialTrack::~TitanPartialTrack()

{
  clear();
  BD_TREE_free_handle(&_tree);
}

////////////////////////////////////////////////////////////
// clear the object

void TitanPartialTrack::clear()

{

  if (_sparams != NULL) {
    ufree(_sparams);
    _sparams = NULL;
    _n_sparams_alloc = 0;
  }
  
}

////////////////////////////////////////////////////////////
// identify a partial track
//
// Returns 0 on success, -1 on failure
  
int TitanPartialTrack::identify(time_t partial_time,
				int past_period,
				int future_period,
				int target_complex_num,
				int target_simple_num,
				const TitanServer &tserver)

{

  _past_period = past_period;
  _future_period = future_period;
  _start_time = partial_time - past_period;
  _end_time = partial_time + future_period;
  _complex_num = target_complex_num;
  _simple_num = target_simple_num;

  // loop through the complex tracks, looking for the selected one

  if (_debug) {
    fprintf(stderr, "partial complex_track: %d\n", target_complex_num);
    fprintf(stderr, "partial simple_track: %d\n", target_simple_num);
    fprintf(stderr, "partial start time: %s\n",
	    utimstr(_start_time));
    fprintf(stderr, "partial end time: %s\n",
	    utimstr(_end_time));
  }

  bool found = false;
  int icomplex = 0;

  for (size_t ii = 0; ii < tserver.complex_tracks().size(); ii++) {
    
    const TitanComplexTrack *ctrack = tserver.complex_tracks()[ii];
    const complex_track_params_t &ct_params = ctrack->complex_params();
    if (ct_params.complex_track_num == target_complex_num) {
      icomplex = ii;
      found = true;
      break;
    }

  }

  // check that the track was found
  
  if (!found) {
    if (_debug) {
      cerr << "WARNING - TitanPartialTrack::identify" << endl;
      cerr << "  Cannot find complex_track_num: "
	   << target_complex_num << endl;
    }
    return -1;
  }
  
  // allocate index array for vertices (one per simple track)
  
  const TitanComplexTrack *ctrack = tserver.complex_tracks()[icomplex];
  const complex_track_params_t &ct_params = ctrack->complex_params();
  int n_simple_tracks = ct_params.n_simple_tracks;

  // alloc for simple tracks array
      
  _allocSparams(n_simple_tracks);
  BD_TREE_alloc_vertices(&_tree, n_simple_tracks,
			 MAX_PARENTS + MAX_CHILDREN);

  // simple tracks in this complex track
  
  BD_TREE_start(&_tree);
  simple_track_params_t *sparams = _sparams;
  bd_tree_vertex_t *vertex = _tree.vertices;
  
  for (int isimple = 0; isimple < n_simple_tracks; isimple++, sparams++) {
	
    const TitanSimpleTrack *strack = ctrack->simple_tracks()[isimple];
    const simple_track_params_t &st_params = strack->simple_params();

    int simple_num = st_params.simple_track_num;

    // copy simple params into array

    *sparams = st_params;
    
    // Get the relevant set of simple tracks -
    // we only consider simple tracks which overlap with the
    // partial track period
	
    if (st_params.start_time <= _end_time &&
	st_params.end_time >= _start_time) {
      
      BD_TREE_add_vertex(&_tree, simple_num, sparams);
      
      // load up vertex with adjacency information
      
      if (sparams->start_time >= _start_time) {
	for (int i = 0; i < sparams->nparents; i++) {
	  BD_TREE_add_adjacent(&_tree, sparams->parent[i]);
	}
      }
      if (sparams->end_time <= _end_time) {
	for (int i = 0; i < sparams->nchildren; i++) {
	  BD_TREE_add_adjacent(&_tree, sparams->child[i]);
	}
      }
      
    } // if (st_params.start_time <= _end_time 
    
  } // isimple

  if (_debug) {
    
    for (int i = 0; i < _tree.n_vertices; i++) {
      
      simple_track_params_t *sp =
	(simple_track_params_t *) _tree.vertices[i].user_data;
      
      fprintf(stderr,"        Simple track %ld, %s - %s\n",
	      (long) sp->simple_track_num,
	      utimstr(sp->start_time),
	      utimstr(sp->end_time));
      
      fprintf(stderr, "              Adjacent array: ");
      for (int j = 0; j < _tree.vertices[i].nadjacent; j++) {
	fprintf(stderr, "%d", _tree.vertices[i].adjacent[j]);
	if (j < _tree.vertices[i].nadjacent - 1) {
	  fprintf(stderr, ", ");
	}
      } // j
      fprintf(stderr, "\n");
    
    } // i 

  } // if (_debug) 
  
  // tag the members of the set to indicate which are
  // members of the same sub-tree
  
  int n_sub_trees = BD_TREE_tag_sub_trees(&_tree);
  
  if (n_sub_trees < 0) {
    cerr << "WARNING - TitanPartialTrack::identify" << endl;
    cerr << "  Cannot tag sub trees" << endl;
    return -1;
  }

  // find the tag for the chosen track

  _tag = -1;
  vertex = _tree.vertices;
  for (int k = 0; k < _tree.n_vertices; k++, vertex++) {
    simple_track_params_t *sp = (simple_track_params_t *) vertex->user_data;
    if (sp->simple_track_num == target_simple_num &&
	sp->complex_track_num == target_complex_num) {
      _tag = vertex->tag;
      break;
    }
  }
  
  if (_debug) {
    fprintf(stderr, "n_sub_trees: %d\n", n_sub_trees);
    fprintf(stderr, "Chosen_tag: %d\n", _tag);
    vertex = _tree.vertices;
    for (int k = 0; k < _tree.n_vertices; k++, vertex++) {
      if (vertex->tag == _tag) {
	fprintf(stderr, "----> ");
      } else {
	fprintf(stderr, "      ");
      }
      fprintf(stderr, "Vertex %d, tag %d, ", k, vertex->tag);
      simple_track_params_t *sp = (simple_track_params_t *) vertex->user_data;
      fprintf(stderr, "simple num %d, ", (int) sp->simple_track_num);
      fprintf(stderr, "complex num %d, ", (int) sp->complex_track_num);
      for (int i = 0; i < vertex->nadjacent; i++) {
	fprintf(stderr, "%d", vertex->adjacent[i]);
	if (i < vertex->nadjacent - 1) {
	  fprintf(stderr, ", ");
	}
      } // i 
      fprintf(stderr, "\n");
    } // k 
  }
  
  if (_tag < 0) {
    return -1;
  }
  
  return 0;

}

///////////////////////////////////////
// Is this entry in the partial track?
//
// Returns true or false

bool TitanPartialTrack::entryIncluded(const track_file_entry_t &entry) const

{

  int simple_track_num = entry.simple_track_num;
  int complex_track_num = entry.complex_track_num;
  
  if (entry.time < _start_time || entry.time > _end_time) {
    return false;
  }

  bd_tree_vertex_t *vertex = _tree.vertices;
  for (int k = 0; k < _tree.n_vertices; k++, vertex++) {
    if (vertex != NULL && vertex->tag == _tag) {
      simple_track_params_t *sparams =
	(simple_track_params_t *) vertex->user_data;
      if (sparams->simple_track_num == simple_track_num &&
	  sparams->complex_track_num == complex_track_num)
	return true;
    }
  } // k

  return false;

}

/////////////////////////////
// alloc simple params array

void TitanPartialTrack::_allocSparams(int n_simple_tracks)

{
  
  int n_extra_needed = n_simple_tracks - _n_sparams_alloc;
  
  if (n_extra_needed > 0) {
    
    if (_sparams == NULL) {
      
      _sparams = (simple_track_params_t *) umalloc
	(n_simple_tracks * sizeof(simple_track_params_t));

    } else {
      
      _sparams = (simple_track_params_t *) urealloc
	(_sparams,
	 n_simple_tracks * sizeof(simple_track_params_t));
      
    }
    
    _n_sparams_alloc = n_simple_tracks;

  }

}

