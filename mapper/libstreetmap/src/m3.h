#pragma once
#include <vector>
#include <string>
#include "graphics.h"
#include "m1.h"
#include "m2.h"
#include "intersection.h"
#include "path_node.h"
#include "assert.h"
#include <iostream>
#include <unistd.h>

#define MAX_SPEED_FOR_HEURISTIC 1430   // m/min
//#define MAX_SPEED_FOR_HEURISTIC 15000

//#define DEBUG_PRINT 1 
//#define DEBUG_GRAPHIC 1
//#define TESTER 1


extern vector<unsigned> path_data;
extern vector<unsigned> path_intersection;
extern vector<string> directions;

// Returns a path (route) between the start intersection and the end 
// intersection, if one exists. If no path exists, this routine returns 
// an empty (size == 0) vector. If more than one path exists, the path 
// with the shortest travel time is returned. The path is returned as a vector 
// of street segment ids; traversing these street segments, in the given order,
// would take one from the start to the end intersection.
std::vector<unsigned> find_path_between_intersections(unsigned 
                   intersect_id_start, unsigned intersect_id_end);

bool a_star_search_path (unsigned 
                   intersect_id_start, unsigned intersect_id_end);


// Returns the time required to travel along the path specified. The path
// is passed in as a vector of street segment ids, and this function can 
// assume the vector either forms a legal path or has size == 0.
// The travel time is the sum of the length/speed-limit of each street 
// segment, plus 15 seconds per turn implied by the path. A turn occurs
// when two consecutive street segments have different street names.
double compute_path_travel_time(const std::vector<unsigned>& path);


// Returns the shortest travel time path (vector of street segments) from 
// the start intersection to a point of interest with the specified name.
// If no such path exists, returns an empty (size == 0) vector.
std::vector<unsigned> find_path_to_point_of_interest (unsigned 
                   intersect_id_start, std::string point_of_interest_name);


std::vector<unsigned> trace_path (unsigned intersect_id_end);

double get_priority_variable (unsigned intersection_id, unsigned destination, double cost,
        double speed_for_heuristic);

unsigned find_street_segment_between (unsigned intersection1, unsigned intersection2);


unsigned find_street_segment_street_id (unsigned segment_id);

// draw the path in orange on the screen
void draw_path (vector<unsigned> path, double scale);

void draw_mini_path(vector<unsigned> path, double scale);


void testPath (void (*drawscreen_ptr) (void));
void inputPathFunction (unsigned startingID, unsigned endingID, void (*drawscreen_ptr) (void));

// return the id of the closest intersection to the given location
unsigned find_closest_intersection (LatLon location);

unsigned find_the_fastest_segment_between(unsigned intersection_from, unsigned intersection_to);

// search paths to the closest among all input of end intersections, return the destination in the path that found
unsigned Dijkstra_search_path (unsigned 
                   intersect_id_start, vector<unsigned> intersect_id_end);





bool path_is_legal(unsigned start_intersection, unsigned end_intersection, const vector<unsigned>& path);



vector<string> give_user_direction(vector<unsigned> path, 
                                    vector<unsigned> path_intersection);











