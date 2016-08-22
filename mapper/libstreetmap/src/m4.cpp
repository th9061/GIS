#include <vector>
#include <iostream>
#include <unordered_map>
#include <queue>

#include <string>
#include <bits/stl_queue.h>
#include <stack>
#include <limits.h>
#include <bits/hashtable.h>

#include "m3.h"
#include "m4.h"
#include "deliver_point.h"

#include "path.h"

using namespace std;

// unordered_map<unsigned, DeliverPoint> travel_map;

load_pathdata bin;
clock_t startTime;


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

vector<unsigned> traveling_salesman(vector<unsigned> intersections_to_traverse, vector<unsigned> depot_locations)
{
    clear_bin();
    startTime = clock();
    // travel_map.clear ();
    int num_of_intersections = intersections_to_traverse.size();
    int num_of_depots = depot_locations.size();
    
    load_distance_data (intersections_to_traverse, depot_locations);
    cout << ((float)(clock()- startTime))/CLOCKS_PER_SEC << endl;
    
    //sort by time, doesnt seem to make a difference
    /*for(unsigned i = 0; i < intersections_to_traverse.size(); ++i)
    {
        sort(bin.intersection_to_other[i].begin(),bin.intersection_to_other[i].end(), compare_time);
        sort(bin.intersection_to_depots[i].begin(),bin.intersection_to_depots[i].end(), compare_time);
        
    }
    for(unsigned i = 0; i < depot_locations.size(); ++i)
    {
        sort(bin.depot_to_other[i].begin(),bin.depot_to_other[i].end(), compare_time);
    }
    cout << ((float)(clock()- startTime))/CLOCKS_PER_SEC << endl;*/
    
    vector<unsigned> path;
    
    // greedy algorithm
    path = greedy_closest_first (intersections_to_traverse, depot_locations);
    path_data = path;
    return path;
}


// define cost and 
/*void set_travel_map (const vector<unsigned>& intersections_to_traverse, const vector<unsigned>& depot_locations)
{
    int num_of_intersections = intersections_to_traverse.size();
    int num_of_depots = depot_locations.size();
    
    for (int i=0; i<num_of_intersections; i++)
    {
        
        
    }
    
    for (int i=0; i<num_of_depots; i++)
    {
        
        
    }
}*/


//to use loaded data:
//call bin.intersection_to_other[intersection_from_id] to return a vector of "to"
//structs with paths/id/travel time to other intersections
//this vector<to> is sorted by closer destinations at lower indices, hence index 0
//will contain the closets intersection
//similarly, call bin.depot_to_other[depot_from_id] to paths to other intersections
void load_distance_data(vector<unsigned>& intersections, vector<unsigned>& depots)
{
    //traverse through given intersection and calculate paths to other intersections
    for(unsigned i  = 0; i < intersections.size(); ++i)
    {
        pair<vector<to>, vector<to> > connectivity = dijkstra_get_intersection_connections (intersections[i], intersections, depots);
        // pair<vector<to>, vector<to> > connectivity = a_star_get_intersection_connections (intersections[i], intersections, depots);
        
        bin.intersection_to_depots.insert(make_pair(intersections[i], connectivity.second));
        bin.intersection_to_other.insert(make_pair(intersections[i], connectivity.first));
    }
    
    //traverse through given depots and calculate path to intersections
    for(unsigned i  = 0; i < depots.size(); ++i)
    {
        vector<to> connectivity = dijkstra_get_depot_connections (depots[i], intersections);
        // vector<to> connectivity = a_star_get_depot_connections (depots[i], intersections);
        
        bin.depot_to_other.insert(make_pair(depots[i], connectivity));    
    }
}


pair<vector<to>, vector<to> > dijkstra_get_intersection_connections (unsigned intersection_id, vector<unsigned>& intersections, vector<unsigned>& depots)
{
    pair<vector<to>, vector<to> > pair_connectivity;
    vector<to> connectivity_depot;
    vector<to> connectivity_inter;
    
    // data structure for storing path finding information
        // unordered_map<unsigned, unsigned> closed;           // intersection id -> previous segment
    // intersection id -> cost for reaching this intersection; for intersections that are already considered
        // unordered_map<unsigned, double> closed_cost;        
    // intersection id -> cost for reaching this intersection; for intersections that is being explored
    unordered_map<unsigned, double> open_cost;    
    unordered_map<unsigned, path_node> closed;
    
    int num_of_intersects = intersections.size();
    int num_of_depots = depots.size();
    
    int num_of_intersects_left = num_of_intersects;
    int num_of_depots_left = num_of_depots;
    
    // set up the start intersection
    path_node start_intersection (intersection_id);
    start_intersection.setFromSegment (NO_PREVIOUS);
    start_intersection.setPriority (0);
    
    
    priority_queue<path_node> waveFront;
    waveFront.push(start_intersection);

    open_cost.insert(make_pair(intersection_id, 0));
    
    
    while (waveFront.size() != 0 && (num_of_intersects_left >=0 || num_of_depots_left >= 0))
    {
        path_node current_node = waveFront.top();
        waveFront.pop();
        unsigned current_id = current_node.getID();
        
        if (current_node.getPriority() != open_cost[current_id])
        {
            // not the best node and segment combination to explore
            continue;
        }

        closed.insert (make_pair(current_id, current_node));
        open_cost.erase (current_id);
        
        for (int i=0; i<num_of_intersects; i++)
        {
            if (current_id == intersections[i])
            {
                to connection_to;
                connection_to.id = current_id;
                connection_to.path = trace_path_m4 (current_id, closed);
                connection_to.time = compute_path_travel_time (connection_to.path);
                connectivity_inter.push_back (connection_to);
                num_of_intersects_left --;
                break;
            }
        }
        for (int i=0; i<num_of_depots; i++)
        {
            if (current_id == depots[i])
            {
                to connection_to;
                connection_to.id = current_id;
                connection_to.path = trace_path_m4 (current_id, closed);
                connection_to.time = compute_path_travel_time (connection_to.path);
                connectivity_depot.push_back (connection_to);
                num_of_depots_left --;
                break;
            }
        }
        
              
        int num_of_segments = getIntersectionStreetSegmentCount (current_id);     
        for (int i=0; i<num_of_segments; i++)
        {
            unsigned segment = getIntersectionStreetSegment (current_id, i);
            double segment_travel_time = find_segment_travel_time (segment);
            StreetSegmentEnds ends = getStreetSegmentEnds (segment);
            unsigned next_id;
            if (ends.from == current_id)
            {
                next_id = ends.to;
            }
            else
            {
                if (!getStreetSegmentOneWay (segment))
                {
                    next_id = ends.from;
                }
                else
                {
                    continue;
                }
            }

            unordered_map<unsigned, path_node>::iterator search_closed = closed.find(next_id);
            unordered_map<unsigned, double>::iterator search_open = open_cost.find (next_id);
            bool not_in_closed = {search_closed == closed.end()};
            bool not_in_open = {search_open == open_cost.end()};
            
            
            double previous_cost = current_node.getPriority();
            double turn_cost = 0;
            double add_cost = 0;

            if (current_id != intersection_id)
            {
                unsigned streetId1 = getStreetSegmentStreetID (current_node.getFromSegment());
                unsigned streetId2 = getStreetSegmentStreetID (segment);

                if (streetId1 != streetId2)
                {
                    turn_cost = 0.25; // in minute
                }
            }
            add_cost = turn_cost + segment_travel_time;

            double cost = add_cost + previous_cost;
            
            if (not_in_closed)
            {
                if (not_in_open)
                {
                    path_node next_node;
                    next_node.setID (next_id);
                    next_node.setPriority(cost);
                    next_node.setFromSegment (segment);
                    waveFront.push(next_node);
                    open_cost.insert (make_pair (next_id, cost));
                }
                else
                {                   
                    double previous_g_score = open_cost[next_id];
                    
                    // only update the cost and the fromSegmentwhen the cost is now lower
                    if (cost < previous_g_score)
                    {
                        path_node next_node;
                        next_node.setID (next_id);
                        next_node.setPriority(cost);
                        next_node.setFromSegment (segment);
                        waveFront.push(next_node);
                        open_cost[next_id] = cost;
                    }
                }
            }
        }
    }
    
    pair_connectivity = make_pair (connectivity_inter, connectivity_depot);
    return pair_connectivity;
}

// set up the connectivity for intersect_start
vector<to> dijkstra_get_depot_connections (unsigned depot_id, vector<unsigned>& intersections)
{
    vector<to> connectivity;
    
    // data structure for storing path finding information
        // unordered_map<unsigned, unsigned> closed;           // intersection id -> previous segment
    // intersection id -> cost for reaching this intersection; for intersections that are already considered
        // unordered_map<unsigned, double> closed_cost;        
    // intersection id -> cost for reaching this intersection; for intersections that is being explored
    unordered_map<unsigned, double> open_cost;
    unordered_map<unsigned, path_node> closed;
    
    int num_of_intersects = intersections.size();
    
    int num_of_intersects_left = num_of_intersects;
    
    // set up the start intersection
    path_node start_intersection (depot_id);
    start_intersection.setFromSegment (NO_PREVIOUS);
    start_intersection.setPriority (0);
    
    // wavefront container that used to get the next intersection which is in highest priority
    priority_queue<path_node> waveFront;
    waveFront.push(start_intersection);
    
    open_cost.insert(make_pair(depot_id, 0));
    
    while (waveFront.size() != 0 && num_of_intersects_left >=0)
    {
        path_node current_node = waveFront.top();
        waveFront.pop();
        unsigned current_id = current_node.getID();
        
        
        
        if (current_node.getPriority() != open_cost[current_id])
        {
            // not the best node and segment combination to explore
            continue;
        }

        closed.insert (make_pair(current_id, current_node));
        open_cost.erase (current_id);
        
        for (int i=0; i<num_of_intersects; i++)
        {
            if (current_id == intersections[i])
            {
                to connection_to;
                connection_to.id = current_id;
                connection_to.path = trace_path_m4 (current_id, closed);
                connection_to.time = compute_path_travel_time (connection_to.path);
                connectivity.push_back (connection_to);
                num_of_intersects_left --;
                break;
            }
        }
        
        int num_of_segments = getIntersectionStreetSegmentCount (current_id);     
        for (int i=0; i<num_of_segments; i++)
        {
            unsigned segment = getIntersectionStreetSegment (current_id, i);
            double segment_travel_time = find_segment_travel_time (segment);
            StreetSegmentEnds ends = getStreetSegmentEnds (segment);
            unsigned next_id;
            if (ends.from == current_id)
            {
                next_id = ends.to;
            }
            else
            {
                if (!getStreetSegmentOneWay (segment))
                {
                    next_id = ends.from;
                }
                else
                {
                    continue;
                }
            }
            
            unordered_map<unsigned, path_node>::iterator search_closed = closed.find(next_id);
            unordered_map<unsigned, double>::iterator search_open = open_cost.find (next_id);
            bool not_in_closed = {search_closed == closed.end()};
            bool not_in_open = {search_open == open_cost.end()};
            
            
            double previous_cost = current_node.getPriority();
            double turn_cost = 0;
            double add_cost = 0;

            if (current_id != depot_id)
            {
                unsigned streetId1 = getStreetSegmentStreetID (current_node.getFromSegment());
                unsigned streetId2 = getStreetSegmentStreetID (segment);

                if (streetId1 != streetId2)
                {
                    turn_cost = 0.25; // in minute
                }
            }
            add_cost = turn_cost + segment_travel_time;

            double cost = add_cost + previous_cost;
         
            
            if (not_in_closed)
            {
                if (not_in_open)
                {
                    path_node next_node;
                    next_node.setID (next_id);
                    next_node.setPriority(cost);
                    next_node.setFromSegment (segment);
                    waveFront.push(next_node);
                    open_cost.insert (make_pair (next_id, cost));
                }
                else
                {                   
                    double previous_g_score = open_cost[next_id];
                    
                    if (cost < previous_g_score)
                    {
                        path_node next_node;
                        next_node.setID (next_id);
                        next_node.setPriority(cost);
                        next_node.setFromSegment (segment);
                        waveFront.push(next_node);
                        open_cost[next_id] = cost;
                    }
                }
            }
        }
    }
    
    return connectivity;
}


/*
// a-star experiment
pair<vector<to>, vector<to> > a_star_get_intersection_connections (unsigned intersection_id, vector<unsigned>& intersections, vector<unsigned>& depots)
{
    pair<vector<to>, vector<to> > pair_connectivity;
    vector<to> connectivity_depot;
    vector<to> connectivity_inter;
    
    // data structure for storing path finding information
    unordered_map<unsigned, unsigned> closed;           // intersection id -> previous segment
    // intersection id -> cost for reaching this intersection; for intersections that are already considered
    unordered_map<unsigned, double> closed_cost;        
    // intersection id -> cost for reaching this intersection; for intersections that is being explored
    unordered_map<unsigned, double> open_cost;          
    
    int num_of_intersects = intersections.size();
    int num_of_depots = depots.size();
    
    int num_of_intersects_left = num_of_intersects;
    int num_of_depots_left = num_of_depots;
    
    // set up the start intersection
    path_node start_intersection (intersection_id);
    
    start_intersection.setFromSegment(NO_PREVIOUS);
    start_intersection.setPriority (numeric_limits<double>::max());
    
    
    priority_queue<path_node> waveFront;
    waveFront.push(start_intersection);

    open_cost.insert(make_pair(intersection_id, 0));
    
    
    while (waveFront.size() != 0 && (num_of_intersects_left >=0 || num_of_depots_left >= 0))
    {        
        path_node current_node = waveFront.top();
        waveFront.pop();
        unsigned current_id = current_node.getID();

        closed.insert (make_pair(current_id, current_node.getFromSegment ()));
        
        for (int i=0; i<num_of_intersects; i++)
        {
            if (current_id == intersections[i])
            {
                to connection_to;
                connection_to.id = current_id;
                connection_to.path = trace_path_m4 (current_id, closed);
                connection_to.time = compute_path_travel_time (connection_to.path);
                connectivity_inter.push_back (connection_to);
                num_of_intersects_left --;
                break;
            }
        }
        for (int i=0; i<num_of_depots; i++)
        {
            if (current_id == depots[i])
            {
                to connection_to;
                connection_to.id = current_id;
                connection_to.path = trace_path_m4 (current_id, closed);
                connection_to.time = compute_path_travel_time (connection_to.path);
                connectivity_depot.push_back (connection_to);
                num_of_depots_left --;
                break;
            }
        }
        
        closed_cost.insert (make_pair(current_id, open_cost[current_id]));
              
        int num_of_segments = getIntersectionStreetSegmentCount (current_id);     
        for (int i=0; i<num_of_segments; i++)
        {
            unsigned segment = getIntersectionStreetSegment (current_id, i);
            double segment_travel_time = find_segment_travel_time (segment);
            StreetSegmentEnds ends = getStreetSegmentEnds (segment);
            unsigned next_id;
            if (ends.from == current_id)
            {
                next_id = ends.to;
            }
            else
            {
                if (!getStreetSegmentOneWay (segment))
                {
                    next_id = ends.from;
                }
                else
                {
                    continue;
                }
            }

            unordered_map<unsigned, unsigned>::iterator search_closed = closed.find(next_id);
            unordered_map<unsigned, double>::iterator search_open = open_cost.find (next_id);
            bool not_in_closed = {search_closed == closed.end()};
            bool not_in_open = {search_open == open_cost.end()};
            
            
            double previous_cost = closed_cost[current_id];
            double turn_cost = 0;
            double add_cost = 0;

            if (current_id != intersection_id)
            {
                unsigned segmentId1 = closed[current_id];
                unsigned segmentId2 = segment;

                unsigned streetId1 = getStreetSegmentStreetID (segmentId1);
                unsigned streetId2 = getStreetSegmentStreetID (segmentId2);

                if (streetId1 != streetId2)
                {
                    turn_cost = 0.25; // in minute
                }

            }
            add_cost = turn_cost + segment_travel_time;

            double cost = add_cost + previous_cost;
            
            if (not_in_closed)
            {
                if (not_in_open)
                {
                    path_node next_node;
                    next_node.setID (next_id);
                    
                    // next_node.setPriority(cost);
                    // set up the priority in the a* manner
                    double highest_priority = 0.0, priority = 0.0;
                    
                    for (int i=0; i<num_of_intersects; i++)
                    {
                        priority = get_priority_variable(next_id, intersections[i], cost, MAX_SPEED_FOR_HEURISTIC);
                        if (priority > highest_priority)
                        {
                            highest_priority = priority;
                        }
                    }
                    for (int i=0; i<num_of_depots; i++)
                    {
                        priority = get_priority_variable(next_id, depots[i], cost, MAX_SPEED_FOR_HEURISTIC);
                        if (priority > highest_priority)
                        {
                            highest_priority = priority;
                        }
                    }
                    
                    next_node.setPriority(highest_priority);
                    
                    next_node.setFromSegment (segment);
                    waveFront.push(next_node);
                    open_cost.insert (make_pair (next_id, cost));
                }
                else
                {                   
                    double previous_g_score = open_cost[next_id];
                    
                    // only update the cost and the fromSegmentwhen the cost is now lower
                    if (cost < previous_g_score)
                    {
                        path_node next_node;
                        next_node.setID (next_id);
                        // next_node.setPriority(cost);
                        // set up the priority in the a* manner
                        double highest_priority = 0.0, priority = 0.0;

                        for (int i=0; i<num_of_intersects; i++)
                        {
                            priority = get_priority_variable(next_id, intersections[i], cost, MAX_SPEED_FOR_HEURISTIC);
                            if (priority > highest_priority)
                            {
                                highest_priority = priority;
                            }
                        }
                        for (int i=0; i<num_of_depots; i++)
                        {
                            priority = get_priority_variable(next_id, depots[i], cost, MAX_SPEED_FOR_HEURISTIC);
                            if (priority > highest_priority)
                            {
                                highest_priority = priority;
                            }
                        }

                        next_node.setPriority(highest_priority);
                        
                        next_node.setFromSegment (segment);
                        waveFront.push(next_node);
                        open_cost[next_id] = cost;
                    }
                }
            }
        }
    }    
    
    pair_connectivity = make_pair (connectivity_inter, connectivity_depot);
    return pair_connectivity;
}
*/

/*
// set up the connectivity for intersect_start
vector<to> a_star_get_depot_connections (unsigned depot_id, vector<unsigned>& intersections)
{
    vector<to> connectivity;
    
    // data structure for storing path finding information
    unordered_map<unsigned, unsigned> closed;           // intersection id -> previous segment
    // intersection id -> cost for reaching this intersection; for intersections that are already considered
    unordered_map<unsigned, double> closed_cost;        
    // intersection id -> cost for reaching this intersection; for intersections that is being explored
    unordered_map<unsigned, double> open_cost;          
    
    int num_of_intersects = intersections.size();
    
    int num_of_intersects_left = num_of_intersects;
    
    // set up the start intersection
    path_node start_intersection (depot_id);
    start_intersection.setFromSegment(NO_PREVIOUS);
    start_intersection.setPriority (numeric_limits<double>::max());
    
    // wavefront container that used to get the next intersection which is in highest priority
    priority_queue<path_node> waveFront;
    waveFront.push(start_intersection);
    
    open_cost.insert(make_pair(depot_id, 0));
    
    while (waveFront.size() != 0 && num_of_intersects_left >=0)
    {        
        path_node current_node = waveFront.top();
        waveFront.pop();
        unsigned current_id = current_node.getID();
        
        
        closed.insert (make_pair(current_id, current_node.getFromSegment ()));
        
        for (int i=0; i<num_of_intersects; i++)
        {
            if (current_id == intersections[i])
            {
                to connection_to;
                connection_to.id = current_id;
                connection_to.path = trace_path_m4 (current_id, closed);
                connection_to.time = compute_path_travel_time (connection_to.path);
                connectivity.push_back (connection_to);
                num_of_intersects_left --;
                break;
            }
        }
        
        // insert the current node in the closed_cost
        closed_cost.insert (make_pair(current_id, open_cost[current_id]));
        
        open_cost.erase (current_id);
              
        int num_of_segments = getIntersectionStreetSegmentCount (current_id);     
        for (int i=0; i<num_of_segments; i++)
        {
            unsigned segment = getIntersectionStreetSegment (current_id, i);
            double segment_travel_time = find_segment_travel_time (segment);
            StreetSegmentEnds ends = getStreetSegmentEnds (segment);
            unsigned next_id;
            if (ends.from == current_id)
            {
                next_id = ends.to;
            }
            else
            {
                if (!getStreetSegmentOneWay (segment))
                {
                    next_id = ends.from;
                }
                else
                {
                    continue;
                }
            }
            
            unordered_map<unsigned, unsigned>::iterator search_closed = closed.find(next_id);
            unordered_map<unsigned, double>::iterator search_open = open_cost.find (next_id);
            bool not_in_closed = {search_closed == closed.end()};
            bool not_in_open = {search_open == open_cost.end()};
            
            
            double previous_cost = closed_cost[current_id];
            double turn_cost = 0;
            double add_cost = 0;

            if (current_id != depot_id)
            {
                unsigned segmentId1 = closed[current_id];
                unsigned segmentId2 = segment;

                unsigned streetId1 = getStreetSegmentStreetID (segmentId1);
                unsigned streetId2 = getStreetSegmentStreetID (segmentId2);

                if (streetId1 != streetId2)
                {
                    turn_cost = 0.25; // in minute
                }
            }
            add_cost = turn_cost + segment_travel_time;

            double cost = add_cost + previous_cost;
         
            
            if (not_in_closed)
            {
                if (not_in_open)
                {
                    path_node next_node;
                    next_node.setID (next_id);
                    
                    // next_node.setPriority(cost);
                    // set up the priority in the a* manner
                    double highest_priority = 0.0, priority = 0.0;
                    
                    for (int i=0; i<num_of_intersects; i++)
                    {
                        priority = get_priority_variable(next_id, intersections[i], cost, MAX_SPEED_FOR_HEURISTIC);
                        if (priority > highest_priority)
                        {
                            highest_priority = priority;
                        }
                    }
                    
                    next_node.setPriority(highest_priority);
                    
                    next_node.setFromSegment (segment);
                    waveFront.push(next_node);
                    open_cost.insert (make_pair (next_id, cost));
                }
                else
                {                   
                    double previous_g_score = open_cost[next_id];
                    
                    if (cost < previous_g_score)
                    {
                        path_node next_node;
                        next_node.setID (next_id);
                        
                        // next_node.setPriority(cost);
                        // set up the priority in the a* manner
                        double highest_priority = 0.0, priority = 0.0;

                        for (int i=0; i<num_of_intersects; i++)
                        {
                            priority = get_priority_variable(next_id, intersections[i], cost, MAX_SPEED_FOR_HEURISTIC);
                            if (priority > highest_priority)
                            {
                                highest_priority = priority;
                            }
                        }

                        next_node.setPriority(highest_priority);

                        next_node.setFromSegment (segment);
                        waveFront.push(next_node);
                        open_cost[next_id] = cost;
                    }
                }
            }
        }
    }
    
    return connectivity;
}
*/


// try out different algorithm here

vector<unsigned> greedy_closest_first (const vector<unsigned>& intersections_to_traverse, 
                                                const vector<unsigned>& depot_locations)
{
    clock_t currentTime = clock();
    float timeSecs = ((float)(currentTime - startTime))/CLOCKS_PER_SEC;
    vector<unsigned> final_path;
    double final_time = 0;
    bool first = true;

   // cout << "size: " << bin.intersection_to_depots[41999].size() << endl;
    
    //cout << "closests is: " << bin.intersection_to_depots[103969].id << endl;
    
    for(unsigned i = 0; i < depot_locations.size() && timeSecs < 29.5; ++i)
    {
        //used for storing path order by intersection ID
        vector<unsigned> path_by_ID;
        path_by_ID.push_back(depot_locations[i]);
        
        //cout << "checking " << depot_locations[i] << endl;
        
        vector<unsigned> temp_path;
        vector<unsigned> remaining = intersections_to_traverse;
        double temp_time = 0;
        
        to destination = bin.depot_to_other[depot_locations[i]][0];
        unsigned next = destination.id;
        path_by_ID.push_back(next);
        temp_time = temp_time + destination.time;
        temp_path = destination.path;
        
        //cout << "to " << next << endl;
        
        remaining.erase(remove(remaining.begin(), remaining.end(), next), remaining.end());
// cout << "before while" << endl;
        
        bool out = false;
        
        while(remaining.size() > 0 && !out)
        {
                        
            currentTime = clock();
            timeSecs = ((float)(currentTime - startTime))/CLOCKS_PER_SEC;
            if(timeSecs > 0.9*30) return final_path;
            
            vector<to> dest_list = bin.intersection_to_other[next];
            
            bool found = false;
            
            for(unsigned j = 0; j < dest_list.size() && !found; ++j)
            {              
                if(find(remaining.begin(), remaining.end(), dest_list[j].id) != remaining.end())
                {
                    destination = dest_list[j];
                    found = true;                  
                }
                else if(j == dest_list.size() - 1)
                {
                    out = true;
                }
            }
            
            if(out) break;
            
            //destination = bin.intersection_to_other[next][0];
            next = destination.id;
            path_by_ID.push_back(next);
            //cout << "to " << next << endl;
            temp_time = temp_time + destination.time;
            temp_path.insert(temp_path.end(), destination.path.begin(), destination.path.end());
            remaining.erase(remove(remaining.begin(), remaining.end(), next), remaining.end());
            
// cout << "while" << endl;
        }
        
        temp_time = temp_time + destination.time;
        //cout << "to zzzz" << next;
        destination = bin.intersection_to_depots[next][0];
        next = destination.id;
        path_by_ID.push_back(next);
        //cout << "to " << next << endl;
        temp_path.insert(temp_path.end(), destination.path.begin(), destination.path.end());
        
// cout << "before if else" << endl;


        if(first)
        {
            first = false;
            final_time = temp_time;
            final_path = temp_path;
            bin.path_by_ID = path_by_ID;
        }
        else
        {
            if(temp_time < final_time && !out)
            {
                //cout << temp_time << " < " << final_time << endl;
                final_time = temp_time;
                final_path = temp_path;
                bin.path_by_ID = path_by_ID;
            }
        }
        
        currentTime = clock();
        timeSecs = ((float)(currentTime - startTime))/CLOCKS_PER_SEC;
        
    }
    
    return final_path;
}




void testTravelingSalesman (void (*drawscreen_ptr) (void))
{
    vector<unsigned> delivery_ids = {26713, 111288, 159340};
    vector<unsigned> depot_ids = {9085, 122230};

    path_data = traveling_salesman (delivery_ids, depot_ids);
    
    
    drawscreen_ptr ();
    
    directions = give_user_direction(path_data, path_intersection);
    draw_directions();
    
}

// helper functions
vector<unsigned> trace_path_m4 (unsigned intersect_id_end, unordered_map<unsigned, path_node> & closed)
{
#ifdef DEBUG_PRINT
    if (got != closed.end())
    {
        unordered_map<unsigned,unsigned>::const_iterator got = closed.find (0);
        cout << "found" << endl;
    }
#endif
    
    vector<unsigned> path;
    
    stack<unsigned> path_reversed;             // path's segment in reversed order
    
    unsigned current_segment = closed[intersect_id_end].getFromSegment();
    unsigned current_intersect = intersect_id_end;
    
    unsigned counter = 0;
    
    while (current_segment != NO_PREVIOUS)
    {
#ifdef DEBUG_PRINT
        cout << "In trace_path: in while ";
#endif
        path_reversed.push (current_segment);
        
        StreetSegmentEnds ends = getStreetSegmentEnds (current_segment);
        if (ends.to == current_intersect)
        {
            current_intersect = ends.from;
        }
        else
        {
            current_intersect = ends.to;
        }
        current_segment = closed[current_intersect].getFromSegment();
        
        counter ++;
        if (counter >= closed.size())
        {
            cout << "infinite loop" << current_segment << endl;
            break;
        }
    }
    
#ifdef DEBUG_PRINT
    cout << "In trace_path: out of while " << endl;
#endif
    
    // reverse the order or node back
    int num_of_segments = path_reversed.size();
    path.resize(num_of_segments);
    for (int i=0; i<num_of_segments; i++)
    {
        // put segment into path segments' id container
        path[i] = path_reversed.top();
        path_reversed.pop();
    }
    
#ifdef DEBUG_PRINT
        cout << "finish backtrack path" << endl;
#endif
    
    return path;
}

        //cout << "to " << next;

void clear_bin()
{
    bin.depot_to_other.clear();
    bin.intersection_to_depots.clear();
    bin.intersection_to_other.clear();
    bin.path_by_ID.clear();
}