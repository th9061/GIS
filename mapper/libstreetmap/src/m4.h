#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include "graphics.h"
#include "m1.h"
#include "m2.h"
#include "m3.h"
#include "intersection.h"
#include "path_node.h"
#include "load_pathdata.h"
#include "assert.h"
#include <iostream>
#include <time.h>

using namespace std;

extern load_pathdata bin;

// This routine takes in a vector of N intersections that must be 
// traversed in a path, and another vector of M intersections that 
// are legal start and end points for the path. The first vector 
// gives the intersection ids of a set of locations at which 
// deliveries must be made. The second vector gives the intersection
// ids of courier company depots containing trucks; you start at any
// one of these depots and end at any one of the depots.
// You can assume that N is always at least 1, and M is always at 
// least one (i.e. both input vectors are non-empty).  You can also
// assume that no intersection is repeated in either list and that
// no intersection is both a delivery location and a depot (i.e. no
// intersection will appear in both vectors).
// This routine returns a vector of street segment ids that form a 
// path, where the first street segment id is connected to a depot
// intersection, and the last street segment id also connects to a
// depot intersection.  If no path connecting all the delivery locations
// and a start and end depot exists, this routine should return an 
// empty (size == 0) vector.

vector<unsigned> traveling_salesman(vector<unsigned> intersections_to_traverse, vector<unsigned> depot_locations);

// void set_travel_map (const vector<unsigned>& intersections_to_traverse, const vector<unsigned>& depot_locations);

void load_distance_data(vector<unsigned>& intersections, vector<unsigned>& depot);
pair<vector<to>, vector<to> > dijkstra_get_intersection_connections (unsigned intersection_id, 
        vector<unsigned>& intersections, vector<unsigned>& depots);
vector<to> dijkstra_get_depot_connections (unsigned depot_id, vector<unsigned>& intersections);

/*
pair<vector<to>, vector<to> > a_star_get_intersection_connections (unsigned intersection_id, 
        vector<unsigned>& intersections, vector<unsigned>& depots);
vector<to> a_star_get_depot_connections (unsigned depot_id, vector<unsigned>& intersections);
*/

// following is different approaches

// this function just choose path according to the order in the intersection list, and always go back to the first depot
vector<unsigned> insane_traveling_salesman (vector<unsigned>& intersections_to_traverse, vector<unsigned>& depot_locations);

vector<unsigned> greedy_closest_first (const vector<unsigned>& intersections_to_traverse, 
                                                const vector<unsigned>& depot_locations);

vector<unsigned> greedy_closest_first_s (const vector<unsigned>& intersections_to_traverse, 
                                                const vector<unsigned>& depot_locations);



void testTravelingSalesman (void (*drawscreen_ptr) (void));



//  the function from m3 is modified to fit m4 
vector<unsigned> trace_path_m4 (unsigned intersect_id_end, unordered_map<unsigned, path_node> & closed);

//clears data structure that holds the path to allow for new path
void clear_bin();








