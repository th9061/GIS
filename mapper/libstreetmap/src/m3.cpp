
#include <string>
#include <queue>
#include <bits/stl_queue.h>
#include <stack>
#include <limits.h>
#include <bits/hashtable.h>
#include <unordered_map>
#include <vector>

#include "m4.h"
#include "m3.h"
#include "m2.h"
#include "m1.h"
#include "StreetsDatabaseAPI.h"

using namespace std;

//unordered_map<string, intersection> intersection_data;


vector<unsigned> path_data;
vector<unsigned> path_intersection;
vector<string> directions;

//unordered_map<unsigned, int> open_map;
unordered_map<unsigned, unsigned> closed;
unordered_map<unsigned, double> closed_cost;
unordered_map<unsigned, double> open_cost;


// Returns a path (route) between the start intersection and the end 
// intersection, if one exists. If no path exists, this routine returns 
// an empty (size == 0) vector. If more than one path exists, the path 
// with the shortest travel time is returned. The path is returned as a vector 
// of street segment ids; traversing these street segments, in the given order,
// would take one from the start to the end intersection.
vector<unsigned> find_path_between_intersections(unsigned 
                   intersect_id_start, unsigned intersect_id_end)
{
#ifdef DEBUG_PRINT
    cout << "start a star" << endl;
#endif

    bool path_found = a_star_search_path(intersect_id_start, intersect_id_end);
    
    vector<unsigned> path;
    
    if (path_found == false)
    {
#ifdef DEBUG_PRINT
        cout << "path not found" << endl;
#endif
        return path;
    }
    else
    {
#ifdef DEBUG_PRINT
       cout << "back tracing path" << endl;
#endif
        
        path = trace_path (intersect_id_end);
#ifdef DEBUG_PRINT
        cout << "finished back tracing path" << endl;
#endif
        return path;
    }
}


// Returns the time required to travel along the path specified. The path
// is passed in as a vector of street segment ids, and this function can 
// assume the vector either forms a legal path or has size == 0.
// The travel time is the sum of the length/speed-limit of each street 
// segment, plus 15 seconds per turn implied by the path. A turn occurs
// when two consecutive street segments have different street names.
double compute_path_travel_time(const vector<unsigned>& path)
{
    double time = 0;
    int num_of_segments = path.size ();
    
    // if the path is too short to be a real path, return 0s
    if (path.size() == 0)
    {
        return 0;
    }
    
    if (path.size() == 1)
    {
        time = find_segment_travel_time (path[0]);
        return time;
    }
    
    for (int i=0; i<num_of_segments-1; i++)
    {
        // get travel time on segment
        time += find_segment_travel_time (path[i]);
        
        // check turn
        unsigned street_id = getStreetSegmentStreetID (path[i]);
        unsigned street_id_next = getStreetSegmentStreetID (path[i+1]);
        
        if (street_id != street_id_next)
        {
            time += 0.25;     // 15 seconds are a quarter of a minute
        }
    }
    
    time += find_segment_travel_time (path[num_of_segments-1]);
    
    return time;
}


// Returns the shortest travel time path (vector of street segments) from 
// the start intersection to a point of interest with the specified name.
// If no such path exists, returns an empty (size == 0) vector.
vector<unsigned> find_path_to_point_of_interest (unsigned 
                   intersect_id_start, string point_of_interest_name)
{
    vector<unsigned> path;
    vector<poi_info> poi_list = poi_data[point_of_interest_name];
    vector<unsigned> poi_intersect_list;
    
    
    int size = poi_list.size();
    poi_intersect_list.resize (size);
    
    // find closest poi
    LatLon closest_poi_location;
    LatLon second_closest;
    double shortest_distance = numeric_limits<double>::max();
    double second_shortest = numeric_limits<double>::max();

    for (int i=0; i<size; i++)
    {
        LatLon poi_position = poi_list[i].position;
        double current_distance = find_distance_between_two_points (getIntersectionPosition(intersect_id_start), poi_position);
        if (current_distance < shortest_distance)
        {
            second_shortest = shortest_distance;
            shortest_distance = current_distance;
            second_closest = closest_poi_location;
            closest_poi_location = poi_position;
        }
    }
    double ratio = (second_shortest - shortest_distance) / shortest_distance;
    
    if (size == 0)
    {
        cout << "poi not found" << endl;
        return path;
    }
    else if (size == 1)
    {
        LatLon poi_location = poi_list[0].position;
        unsigned intersect_id_end = find_closest_intersection (poi_location);
        path = find_path_between_intersections (intersect_id_start, intersect_id_end);
    }
    else if (ratio > 0.38)
    {
        unsigned intersect_id_end = find_closest_intersection (closest_poi_location);
        path = find_path_between_intersections (intersect_id_start, intersect_id_end);
    }
    else
    {
        for (int i=0; i<size; i++)
        {
            LatLon poi_location = poi_list[i].position;
            poi_intersect_list[i] = find_closest_intersection (poi_location);
        }

        unsigned intersect_end = Dijkstra_search_path (intersect_id_start, poi_intersect_list);
        path = trace_path (intersect_end);
    }
    
    return path;
}


bool a_star_search_path (unsigned 
                   intersect_id_start, unsigned intersect_id_end)
{    
    path_node start_intersection (intersect_id_start);
    start_intersection.setFromSegment (NO_PREVIOUS);
    start_intersection.setPriority (numeric_limits<double>::max());
    
        
#ifdef DEBUG_PRINT
    cout << "In a_star_search_path: start intersection: " << start_intersection.getNode().getID() 
            << ", previous: " <<  start_intersection.getNode().getPrevious() << endl;
#endif
    
    priority_queue<path_node> waveFront;
    waveFront.push(start_intersection);
    
    
    closed.clear();
    // closed.insert(make_pair(intersect_id_start, NO_PREVIOUS));
    
    closed_cost.clear();
    // closed_cost.insert(make_pair(intersect_id_start, 0));

    open_cost.clear();
    open_cost.insert(make_pair(intersect_id_start, 0));
    
    
#ifdef DEBUG_PRINT
    cout << "In a_star_search_path: entering the while loop" << endl;
#endif
    
    
    // intersection current_intersection = start_intersection.getNode();
    
    while (waveFront.size() != 0)
    {        
        path_node current_node = waveFront.top();
        waveFront.pop();
        unsigned current_id = current_node.getID();
        
        
#ifdef DEBUG_GRAPHIC
        t_point point_new = get_coordinate_from_LatLon (getIntersectionPosition(current_id));
        // graphic animation of path finding start here
        setcolor (255,0,0);
        fillarc (point_new.x, point_new.y, 80, 0, 360);
        flushinput ();
        usleep (1000);
#endif
        
        closed.insert (make_pair(current_id, current_node.getFromSegment()));
        // closed.emplace (current_id, current_intersection.getPrevious ());
        
        
#ifdef DEBUG_PRINT
        cout << "current_id: " << current_id << "Previous: " << current_intersection.getPrevious () << endl;
#endif
        
        if (current_id == intersect_id_end)
        {
#ifdef DEBUG_PRINT
            cout << "is the end" << current_id << endl;
            cout << "total travel time: " << open_cost[current_id] << endl;
#endif
            
            return true;
        }
        
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
            
            
#ifdef DEBUG_PRINT
            cout << "Next_id: " << next_id << "; ";
#endif
            
            unordered_map<unsigned, unsigned>::iterator search_closed = closed.find(next_id);
            unordered_map<unsigned, double>::iterator search_open = open_cost.find (next_id);
            bool not_in_closed = {search_closed == closed.end()};
            bool not_in_open = {search_open == open_cost.end()};
            
            
            double previous_cost = closed_cost[current_id];
            double turn_cost = 0;
            double add_cost = 0;

            if (current_id != intersect_id_start)
            {
                unsigned segmentId1 = closed[current_id];
                unsigned segmentId2 = segment;

                unsigned streetId1 = getStreetSegmentStreetID (segmentId1);
                unsigned streetId2 = getStreetSegmentStreetID (segmentId2);

                if (streetId1 != streetId2)
                {
                    turn_cost = 0.25; // in minute
                }

#ifdef DEBUG_PRINT
                cout << "In cost calculation: " << add_cost << " " << endl;
#endif
            }
            add_cost = turn_cost + segment_travel_time;

            double cost = add_cost + previous_cost;
         
            
            if (not_in_closed)
            {
                if (not_in_open)
                {
                    path_node next_node;
                    next_node.setID (next_id);
                    
                    double new_priority = get_priority_variable(next_id, intersect_id_end, cost, MAX_SPEED_FOR_HEURISTIC);
                        
                    next_node.setPriority(new_priority);
                    
                    next_node.setFromSegment (segment);
                    waveFront.push(next_node);
                    open_cost.insert (make_pair (next_id, cost));
#ifdef DEBUG_PRINT
                    cout << "Next_previous: " << next_node.getNode().getPrevious() << " - " << endl;
#endif
                }
                else
                {                   
                    double previous_g_score = open_cost[next_id];
                    
                    if (cost < previous_g_score)
                    {                       
                        path_node next_node;
                        next_node.setID (next_id);
                        
                        double new_priority = get_priority_variable(next_id, intersect_id_end, cost, MAX_SPEED_FOR_HEURISTIC);
                        
                        next_node.setPriority(new_priority);
                        
                        next_node.setFromSegment (segment);
                        waveFront.push(next_node);
                        open_cost[next_id] = cost;
                    }
                }
            }
        }
        
       
#ifdef DEBUG_PRINT
        cout << "In a_start_search_path: planning" << current_intersection.getID () << endl;
#endif
    }    
    
    return false;
}


// setting priority of exploring when search path
double get_priority_variable (unsigned intersection_id, unsigned destination, double cost, double speed_for_heuristic)
{
    // LatLon intersection_current =  getIntersectionPosition (current_intersection);
    LatLon intersection_position = getIntersectionPosition (intersection_id);
    LatLon intersection_destination = getIntersectionPosition (destination);
    // double current_distance = find_distance_between_two_points (intersection_current, intersection_destination);  
    double next_distance = find_distance_between_two_points (intersection_position, intersection_destination);
    
    double priority_var = cost + next_distance/speed_for_heuristic;
    
    return priority_var;
}


vector<unsigned> trace_path (unsigned intersect_id_end)
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
    stack<unsigned> intersection_reversed;    // path's intersection in reverses order
    intersection_reversed.push (intersect_id_end);
    
    unsigned current_segment = closed[intersect_id_end];
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
        current_segment = closed[current_intersect];
        
        intersection_reversed.push (current_intersect);
        
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
    path_intersection.resize(num_of_segments + 1);
    for (int i=0; i<num_of_segments; i++)
    {
        // put segment into path segments' id container
        path[i] = path_reversed.top();
        path_reversed.pop();
        
        // put segment into path segments' id container
        path_intersection[i] = intersection_reversed.top();
        intersection_reversed.pop();
    }
    
    path_intersection[num_of_segments] = intersection_reversed.top();
    intersection_reversed.pop();
    
#ifdef DEBUG_PRINT
        cout << "finish backtrack path" << endl;
#endif
    
    path_data.clear();
    path_data = path;
    
    return path;
}

unsigned get_intersection_from (unsigned intersection_id, unsigned street_segment_id)
{
    StreetSegmentEnds ends = getStreetSegmentEnds (street_segment_id);
    
    unsigned intersection_from, intersection_to;
    intersection_from = ends.from;
    intersection_to = ends.to;
        
    if (intersection_from == intersection_id)
    {
        return intersection_to;
    }
    else
    {
        return intersection_from;
    }
}

// return the segment id that connect two intersection
unsigned find_street_segment_between (unsigned intersection1, unsigned intersection2)
{
    unsigned segmentID = 0;  // if something wired happened, could be caused by this line
    int number_of_street_segments = getIntersectionStreetSegmentCount (intersection1);
    for (int i=0; i<number_of_street_segments; i++)
    {
        segmentID = getIntersectionStreetSegment (intersection1, i);
        StreetSegmentEnds ends = getStreetSegmentEnds (segmentID);
        
        if (ends.from == intersection2 || ends.to == intersection2)
        {
            break;
        }
    }
    
    return segmentID;
}


// return the street name of which the segment belongs to
unsigned find_street_segment_street_id (unsigned segment_id)
{
    StreetSegmentEnds ends = getStreetSegmentEnds (segment_id);
    vector<unsigned> intersection_1_street_ids = find_intersection_street_ids (ends.from);
    vector<unsigned> intersection_2_street_ids = find_intersection_street_ids (ends.to);
    
    
    int m = intersection_1_street_ids.size();
    int n = intersection_2_street_ids.size();
    for (int i=0; i<m; i++)
    {
        for (int j=0; j<n; j++)
        {
            // if found a common street name, then return that name
            if (intersection_1_street_ids[i] == intersection_2_street_ids[i])
            {
                return intersection_1_street_ids[i];
            }
        }
    }
    // not found, return maximum unsigned int
    return UINT_MAX;
}

// draw the output path on screen in orange
void draw_path (vector<unsigned> path, double scale)
{  
    setcolor(path_color);
    if(getZoomLevel() >= 0 && getZoomLevel() <= 2)
        setlinewidth(2);
    else if(getZoomLevel() > 2 && getZoomLevel() <= 5)
        setlinewidth(3);
    else if(getZoomLevel() <= 7 && getZoomLevel() > 5)
        setlinewidth(5);
    else if(getZoomLevel() < 9 && getZoomLevel() > 7)
        setlinewidth(10);
    else if(getZoomLevel() >= 9)
        setlinewidth(10);
    //setlinewidth(path_width/4/sqrt(sqrt(scale*scale*scale)));
    // setlinewidth(default_one_way/scale);
    
    int size = path.size();
    if (size > 1)
    {
        for (int i=0; i<size; i++)
        {
            StreetSegmentEnds ends = getStreetSegmentEnds (path[i]);
            t_point from  = get_coordinate_from_LatLon (getIntersectionPosition(ends.from));
            t_point to  = get_coordinate_from_LatLon (getIntersectionPosition(ends.to));
            
#ifdef DEBUG_GRAPHIC 
            if (i == size - 1 || i == size - 2)
            {
                t_point point_new = from;
                // graphic animation of path finding start here
                setcolor (255,0,0);
                fillarc (point_new.x, point_new.y, 20, 0, 360);
                flushinput ();
            }
#endif
            
            
            int num_of_curve_points = getStreetSegmentCurvePointCount (path[i]);
            if (num_of_curve_points != 0)
            {
                t_point point1 = get_coordinate_from_LatLon (getStreetSegmentCurvePoint (path[i], 0));
                t_point point2;
                
                drawline (from, point1);
                for (int j=0; j<num_of_curve_points-1; j++)
                {
                    point1 = get_coordinate_from_LatLon (getStreetSegmentCurvePoint (path[i], j));
                    point2 = get_coordinate_from_LatLon (getStreetSegmentCurvePoint (path[i], j+1));
                    drawline (point1, point2);
                }
                point2 = get_coordinate_from_LatLon (getStreetSegmentCurvePoint (path[i], num_of_curve_points-1));
                
                drawline (point2, to);
            }
            else
            {              
                drawline (from, to);
            }
            
        }
    }
    
    
#ifdef DEBUG_GRAPHIC
        
        t_point point_new = get_coordinate_from_LatLon (getIntersectionPosition(63371));
        // graphic animation of path finding start here
        setcolor (255,0,0);
        fillarc (point_new.x, point_new.y, 60, 0, 360);
        flushinput ();
        
        point_new = get_coordinate_from_LatLon (getIntersectionPosition(115705));
        // graphic animation of path finding start here
        setcolor (255,0,0);
        fillarc (point_new.x, point_new.y, 60, 0, 360);
        flushinput ();
        
#endif
    
#ifdef DEBUG_PRINT
    cout << "finish draw path" << endl;
#endif
}

void draw_mini_path (vector<unsigned> path, double scale)
{  
    setcolor(path_color);
    setlinewidth(2);
    /*if(getZoomLevel() >= 0 && getZoomLevel() <= 2)
        setlinewidth(2);
    else if(getZoomLevel() > 2 && getZoomLevel() <= 5)
        setlinewidth(3);
    else if(getZoomLevel() <= 7 && getZoomLevel() > 5)
        setlinewidth(5);
    else if(getZoomLevel() < 9 && getZoomLevel() > 7)
        setlinewidth(10);
    else if(getZoomLevel() >= 9)
        setlinewidth(10);*/
    //setlinewidth(path_width/4/sqrt(sqrt(scale*scale*scale)));
    // setlinewidth(default_one_way/scale);
    
    int size = path.size();
    if (size > 1)
    {
        for (int i=0; i<size; i++)
        {
            StreetSegmentEnds ends = getStreetSegmentEnds (path[i]);
            t_point from  = get_mini_coordinate_from_LatLon (getIntersectionPosition(ends.from));
            t_point to  = get_mini_coordinate_from_LatLon (getIntersectionPosition(ends.to));   
            
            int num_of_curve_points = getStreetSegmentCurvePointCount (path[i]);
            if (num_of_curve_points != 0)
            {
                t_point point1 = get_mini_coordinate_from_LatLon (getStreetSegmentCurvePoint (path[i], 0));
                t_point point2;
                
                drawline (from, point1);
                for (int j=0; j<num_of_curve_points-1; j++)
                {
                    point1 = get_mini_coordinate_from_LatLon (getStreetSegmentCurvePoint (path[i], j));
                    point2 = get_mini_coordinate_from_LatLon (getStreetSegmentCurvePoint (path[i], j+1));
                    drawline (point1, point2);
                }
                point2 = get_mini_coordinate_from_LatLon (getStreetSegmentCurvePoint (path[i], num_of_curve_points-1));
                
                drawline (point2, to);
            }
            else
            {              
                drawline (from, to);
            }
            
        }
    }
    
}


// tester function for path finding
void testPath (void (*drawscreen_ptr) (void)) 
{
    
    //find_path_between_intersections (start, end);
    
#ifdef DEBUG_PRINT
    cout << "start to find path" << endl;
#endif    
    
#ifdef TESTER
    
    double optimal_cost;
    double actual_cost;
    
    
    path = find_path_between_intersections(160763, 31504);
    
    path = find_path_between_intersections(145596, 165268);
    optimal_cost = 27.76128430654581;
    actual_cost = compute_path_travel_time(path);
    cout << "optimal: " << optimal_cost << " actual: " << actual_cost <<  endl;
    

    path = find_path_between_intersections(149393, 108453);
    optimal_cost = 36.89741347311812;
    actual_cost = compute_path_travel_time(path);
    cout << "optimal: " << optimal_cost << " actual: " << actual_cost <<  endl;
    
    path = find_path_between_intersections(659, 68866);
    //optimal_cost = 37.41047463988919;
    actual_cost = compute_path_travel_time(path);
    cout << "optimal: " << optimal_cost << " actual: " << actual_cost <<  endl;
    
    
    //path = find_path_to_point_of_interest(659, "Copernicus Lodge");
   /* optimal_cost = 42.28794306766002;*/
    //actual_cost = compute_path_travel_time(path);
    //cout << "optimal: " << optimal_cost << " actual: " << actual_cost <<  endl;
    
    
#endif
   
    
    int num_of_pins = pinned_points.size();
    if (num_of_pins < 2)
    {
        cout << "too few points for path planning" << endl;
        return;
    }
    
    LatLon start_LatLon = get_LatLon_from_coordinate (pinned_points[num_of_pins-2]);
    LatLon end_LatLon = get_LatLon_from_coordinate (pinned_points[num_of_pins-1]);
    
    // get the last two points as input for path
    unsigned start = find_closest_intersection (start_LatLon);
    unsigned end = find_closest_intersection (end_LatLon);
    
    vector<unsigned> path = find_path_between_intersections (start, end);
    
    if(path_set)
    {
        if(path_data == path);
        return;
    }
    
    

    
#ifdef DEBUG_PRINT
    cout << "finish finding path" << endl;
#endif
    
    path_set = true;
    if(legend_state)
    {
        legendmenu_destroy();
        legend_state = false;
        legendmenu_create();
        
    }
    directions = give_user_direction(path_data, path_intersection);
    draw_all_pinend_points(get_scale());
    draw_directions_load();
    draw_directions();    
    drawscreen();
}

void inputPathFunction (unsigned startingID, unsigned endingID, void (*drawscreen_ptr) (void))
{
    
    

    //find_path_between_intersections (start, end);
    
#ifdef DEBUG_PRINT
    cout << "start to find path" << endl;
#endif    
    
#ifdef TESTER
    
    double optimal_cost;
    double actual_cost;
    
    
    path = find_path_between_intersections(160763, 31504);
    
    path = find_path_between_intersections(145596, 165268);
    optimal_cost = 27.76128430654581;
    actual_cost = compute_path_travel_time(path);
    cout << "optimal: " << optimal_cost << " actual: " << actual_cost <<  endl;
    

    path = find_path_between_intersections(149393, 108453);
    optimal_cost = 36.89741347311812;
    actual_cost = compute_path_travel_time(path);
    cout << "optimal: " << optimal_cost << " actual: " << actual_cost <<  endl;
    
    path = find_path_between_intersections(659, 68866);
    //optimal_cost = 37.41047463988919;
    actual_cost = compute_path_travel_time(path);
    cout << "optimal: " << optimal_cost << " actual: " << actual_cost <<  endl;
    
    
    //path = find_path_to_point_of_interest(659, "Copernicus Lodge");
   /* optimal_cost = 42.28794306766002;*/
    //actual_cost = compute_path_travel_time(path);
    //cout << "optimal: " << optimal_cost << " actual: " << actual_cost <<  endl;
    
    
#endif

    /*for(int i=0; i<start.size();i++)
    {
        if(i==0)
        {
            continue;
        }else if(start[i-1]==' '&&start[i]>=65&&start[i]<=90)
        {
            continue;
        }
        if(start[i]!='\0' && start[i]>=65 && start[i]<=90 && start[i]!='&')
        {
            start[i] += 32;
        } else
        {
            continue;
        }
    }
    start.erase(start.end()-1);
    cout <<"starting point: "<< start <<endl; 
    
    for(int j=0; j<end.size();j++)
    {
        if(j==0)
        {
            continue;
        }else if(end[j-1]==' '&&end[j]>=65&&end[j]<=90)
        {
            continue;
        }
        if(end[j]!='\0' && end[j]>=65 && end[j]<=90 && end[j]!='&')
        {
            end[j] += 32;
        } else
        {
            continue;
        }
    }
    end.erase(end.end()-1);
    cout <<"ending point: " <<end <<endl;
    
    unsigned start_id=find_intersection_id_from_name(start);
    unsigned end_id=find_intersection_id_from_name(end);*/
    
    //cout << "starting at " << startingID << " ending at " << endingID <<endl;
    /*int num_of_pins = pinned_points.size();
    if (num_of_pins < 2)
    {
        cout << "too few points for path planning" << endl;
        return;
    }*/
    
    
    
    /*LatLon start_LatLon = getIntersectionPosition(start);//get_LatLon_from_coordinate (pinned_points[num_of_pins-2]);
    LatLon end_LatLon = getIntersectionPosition(end);// get_LatLon_from_coordinate (pinned_points[num_of_pins-1]);
    
    // get the last two points as input for path
    unsigned start2 = find_closest_intersection (start_LatLon);
    unsigned end2 = find_closest_intersection (end_LatLon);*/
    
    vector<unsigned> path = find_path_between_intersections (startingID, endingID);

    
    //drawscreen_ptr ();
    if(path_set)
    {
        if(path_data == path);
        return;
    }
       

  
    
    path_set = true;
    if(legend_state)
    {
        legendmenu_destroy();
        legend_state = false;
        legendmenu_create();
        
    }
    directions = give_user_direction(path_data, path_intersection);
    draw_all_pinend_points(get_scale());
    draw_directions_load();
    draw_directions();  
    drawscreen();
    
    //give_user_direction(path_data, path_intersection);
}

unsigned find_closest_intersection (LatLon location)
{
    unsigned intersection_id = numeric_limits<unsigned>::max();
    double min_distance = numeric_limits<double>::max();
    double distance = 0;
    
    unsigned num_of_intersections = getNumberOfIntersections();
    for (unsigned i=0; i<num_of_intersections; i++)
    {
        LatLon intersection_position = getIntersectionPosition (i);
        distance = find_distance_between_two_points(location, intersection_position);
        
        if (distance < min_distance)
        {
            min_distance = distance;
            intersection_id = i;
        }
    }
    return intersection_id;
}



unsigned find_the_fastest_segment_between(unsigned intersection_from, unsigned intersection_to)
{
    int num_of_segments = getIntersectionStreetSegmentCount (intersection_from);
    double best_time = numeric_limits<double>::max();
    double current_time = numeric_limits<double>::max();
    double best_segment = numeric_limits<unsigned>::max();
    
    for (int i=0; i<num_of_segments; i++)
    {
        unsigned segmentID = getIntersectionStreetSegment (intersection_from, i);
        StreetSegmentEnds ends = getStreetSegmentEnds (segmentID);
        // if the segment is not one way street
        if (!getStreetSegmentOneWay(segmentID))
        {
            if (ends.from == intersection_to || ends.to == intersection_to)
            {
                current_time = find_segment_travel_time(segmentID);
                if (current_time < best_time)
                {
                    best_time = current_time;
                    best_segment = segmentID;
                }
            }
        }
        // is one way street
        else
        {
            if (ends.to == intersection_to)
            {
                current_time = find_segment_travel_time(segmentID);
                if (current_time < best_time)
                {
                    best_time = current_time;
                    best_segment = segmentID;
                }
            }
        }
    }
    
    return best_segment;
}

// search paths to the closest among all input of end intersections, return the destination in the path that found
// return intersect_id_start if no path is found
unsigned Dijkstra_search_path (unsigned 
                   intersect_id_start, vector<unsigned> intersect_id_end)
{    
    int num_of_end = intersect_id_end.size();
    
    path_node start_intersection (intersect_id_start);// (intersect_id_start, NO_EDGE);
    
    start_intersection.setFromSegment(NO_PREVIOUS);
    start_intersection.setPriority (numeric_limits<double>::max());
    
        
#ifdef DEBUG_PRINT
    cout << "In a_star_search_path: start intersection: " << start_intersection.getNode().getID() 
            << ", previous: " <<  start_intersection.getNode().getPrevious() << endl;
#endif
    
    priority_queue<path_node> waveFront;
    waveFront.push(start_intersection);
    
    
    closed.clear();
    // closed.insert(make_pair(intersect_id_start, NO_PREVIOUS));
    
    closed_cost.clear();
    // closed_cost.insert(make_pair(intersect_id_start, 0));

    open_cost.clear();
    open_cost.insert(make_pair(intersect_id_start, 0));
    
    
#ifdef DEBUG_PRINT
    cout << "In a_star_search_path: entering the while loop" << endl;
#endif
    
    
    // intersection current_intersection = start_intersection.getNode();
    
    while (waveFront.size() != 0)
    {        
        path_node current_node = waveFront.top();
        waveFront.pop();
        unsigned current_id = current_node.getID();
        
        
#ifdef DEBUG_GRAPHIC
        t_point point_new = get_coordinate_from_LatLon (getIntersectionPosition(current_id));
        // graphic animation of path finding start here
        setcolor (255,0,0);
        fillarc (point_new.x, point_new.y, 80, 0, 360);
        flushinput ();
        usleep (1000);
#endif
        
        closed.insert (make_pair(current_id, current_node.getFromSegment ()));
        // closed.emplace (current_id, current_intersection.getPrevious ());
        
        
#ifdef DEBUG_PRINT
        cout << "current_id: " << current_id << "Previous: " << current_intersection.getPrevious () << endl;
#endif
        for (int i=0; i<num_of_end; i++)
        {
            if (current_id == intersect_id_end[i])
            {            
                return current_id;
            }
        }
        
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
            
            
#ifdef DEBUG_PRINT
            cout << "Next_id: " << next_id << "; ";
#endif
            
            unordered_map<unsigned, unsigned>::iterator search_closed = closed.find(next_id);
            unordered_map<unsigned, double>::iterator search_open = open_cost.find (next_id);
            bool not_in_closed = {search_closed == closed.end()};
            bool not_in_open = {search_open == open_cost.end()};
            
            
            double previous_cost = closed_cost[current_id];
            double turn_cost = 0;
            double add_cost = 0;

            if (current_id != intersect_id_start)
            {
                unsigned segmentId1 = closed[current_id];
                unsigned segmentId2 = segment;

                unsigned streetId1 = getStreetSegmentStreetID (segmentId1);
                unsigned streetId2 = getStreetSegmentStreetID (segmentId2);

                if (streetId1 != streetId2)
                {
                    turn_cost = 0.25; // in minute
                }

#ifdef DEBUG_PRINT
                cout << "In cost calculation: " << add_cost << " " << endl;
#endif
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
#ifdef DEBUG_PRINT
                    cout << "Next_previous: " << next_node.getNode().getPrevious() << " - " << endl;
#endif
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
                        // open_cost.insert (make_pair (next_id, cost));
                        open_cost[next_id] = cost;
                    }
                }
            }
        }
        
       
#ifdef DEBUG_PRINT
        cout << "In a_start_search_path: planning" << current_intersection.getID () << endl;
#endif
    }    
    
    return intersect_id_start;
}














































// tester function for debug use only
bool path_is_legal(unsigned start_intersection, unsigned end_intersection, const vector<unsigned>& path) 
{
    //'start_intersection' is the intersection id of the starting intersection
    //'end_intersection' is the intersection id of the ending intersection
    //'path' is a vector street segment id's

    if (path.size() < 1) {
        cout << "path is too short" << endl;
        return false; //If it is a valid path it must contain at-least one segment

    } else {
        //General case
        //To verify the path by walking along each segment checking:
        //  * That we are going a legal direction (e.g. not against one-ways)
        //  * That each segment is connected to the previous intersection
        //  * That the final segment is connected to the end_intersection
        //We start our intersection at the start_intersection
        assert(path.size() >= 1);

        unsigned curr_intersection = start_intersection;
        for (int i = 0; i < (int) path.size(); i++) {
            StreetSegmentEnds seg_endpoints = getStreetSegmentEnds(path[i]);


            //Are we moving forward or back along the segment?
            bool seg_traverse_forward;
            if (seg_endpoints.to == curr_intersection) {
                seg_traverse_forward = false;

            } else if (seg_endpoints.from == curr_intersection) {
                seg_traverse_forward = true;

            } else {
                cout << "This segment isn't connected to the current intersection" << endl;
                return false;
            }

            //Are we going the wrong way along the segment?
            if (!seg_traverse_forward && getStreetSegmentOneWay(path[i])) {
                cout << "wrong direction on one way street" << endl;
                return false;
            }

            //Advance to the next intersection
            curr_intersection = (seg_traverse_forward) ? seg_endpoints.to : seg_endpoints.from;

        }

        //We should be at the end intersection
        if (curr_intersection != end_intersection) {
            
            cout << "not connected to destination" << endl;
            
            
            t_point dest = get_coordinate_from_LatLon (getIntersectionPosition(end_intersection));
            // t_point stt = get_coordinate_from_LatLon (getIntersectionPosition(start_intersection));
            pinned_points.push_back(dest);
            // pinned_points.push_back(stt);
            
            return false;
        }

    }

    //Everything validated
    return true;
}

std::vector<std::string> give_user_direction(std::vector<unsigned> path, 
                                        std::vector<unsigned> path_intersection)
{
    std::vector<std::string> navigate;
    navigate.push_back("straight");
    for(unsigned i=0;i<(path.size());i++)
    {
        if((path.size()-1)!=i)
        {
            unsigned streetsegment1= getStreetSegmentStreetID(path[i]);
            unsigned streetsegment2= getStreetSegmentStreetID(path[i+1]);
            if(streetsegment1==streetsegment2)
            {
                navigate.push_back("straight");
            } else
            {
                unsigned intersectionOrigin = path_intersection.at(i);
                unsigned intersection1=path_intersection.at(i+1);
                unsigned intersection2=path_intersection.at(i+2);
                
                LatLon pointOrigin, point1, point2;
                
                pointOrigin = getIntersectionPosition(intersectionOrigin);
                point1 = getIntersectionPosition(intersection1);
                point2 = getIntersectionPosition(intersection2);
                
                t_point origin, coordinate1, coordinate2;
                
                origin = get_coordinate_from_LatLon(pointOrigin);
                coordinate1 = get_coordinate_from_LatLon(point1);
                coordinate2 = get_coordinate_from_LatLon(point2);
                
                double vector1x = coordinate1.x-origin.x;
                double vector1y = coordinate1.y-origin.y;
                double vector2x = coordinate2.x-coordinate1.x;
                double vector2y = coordinate2.y-coordinate1.y;
                double result = vector1x*vector2y - vector2x*vector1y;
                
                if(result>0)
                {
                    navigate.push_back("left");
                } else if(result<0)
                {
                    navigate.push_back("right");
                } else
                {
                    navigate.push_back("straight");
                }
                
            }
        }
              
    }
    navigate.push_back("straight");
    /*for(unsigned i=0;i<navigate.size();i++)
    {
        cout << navigate[i] <<endl;
    }*/
    return navigate;
}

