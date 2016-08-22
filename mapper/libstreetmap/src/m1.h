#pragma once //protects against multiple inclusions of this header file

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include "StreetsDatabaseAPI.h"
#include "m2.h"
#include "intersection.h"
#include "layer.h"
#include <ctype.h>

//use these defines whenever you need earth radius 
//or conversion from degrees to radians
#define EARTH_RADIUS_IN_METERS 6372797.560856
#define DEG_TO_RAD 0.017453292519943295769236907684886

#define PI 3.14159265
#define POIARRAYMAX 50

using namespace std;

//structure for map bounds data
struct mapbounds_info {
    double maxlat;
    double maxlon;
    double minlat;
    double minlon;
    double mapheight;
    double mapwidth;
};

//structure to contain relevant data associated to a particular street
struct street_info {
    unsigned id;
    unsigned tier;
    std::vector<unsigned> street_segments;
    std::vector<unsigned> street_intersections;
    //std::string type;
    
  
};

//structure to contain relevant data associated to a particular intersection
struct intersection_info {
    unsigned id;
};

//structure for poi data
struct poi_info {
    unsigned id;
    LatLon position;
};

struct search_info{
    unsigned id;
    unsigned type;
    string name;
};

extern mapbounds_info map_bounds;
extern std::unordered_map<std::string, intersection> intersection_data;
extern std::unordered_map<std::string, street_info> street_data;
extern std::unordered_map<std::string, std::vector<poi_info>> poi_data;
extern vector <string> list_street_type;
extern std::unordered_map<char, vector<search_info>> search_engine;

extern layer layerSys;



//function to load bin or osm map
bool load_map(std::string map_name);

//close the loaded map
void close_map();

//function to return intersection id for an intersection name
unsigned find_intersection_id_from_name(std::string intersection_name);

//function to return street id for an intersection name
unsigned find_street_id_from_name(std::string street_name);

//function to return the street segments for a given intersection 
std::vector<unsigned> find_intersection_street_segments(std::string intersection_name);
std::vector<unsigned> find_intersection_street_segments(unsigned intersection_id);

//function to return street names at an intersection
std::vector<std::string> find_intersection_street_names(std::string intersection_name);
std::vector<std::string> find_intersection_street_names(unsigned intersection_id);

//returns vector<unsigned>  of street segments connect to intersection, without duplicates
//vector of street segments is gotten from find_intersection_street_segments and parses through the duplicates
std::vector<unsigned> find_intersection_street_ids(unsigned intersection_id);

//can you get from intersection1 to intersection2 using a single street segment (hint: check for 1-way streets too)
bool are_directly_connected(std::string intersection_name1, std::string intersection_name2);

//find all intersections connected by one street segment from given intersection (hint: check for 1-way streets too)
std::vector<unsigned> find_adjacent_intersections(std::string intersection_name);
std::vector<unsigned> find_adjacent_intersections(unsigned intersection_id);

//for a given street, return all the street segments
std::vector<unsigned> find_street_street_segments(std::string street_name);

//for a given street, find all the intersections
std::vector<unsigned> find_all_street_intersections(std::string street_name);

//find distance between two coordinates
double find_distance_between_two_points(LatLon point1, LatLon point2);

//find the length of a given street segments
double find_street_segment_length(unsigned street_segment_id);

//find the length of a whole street
double find_street_length(std::string street_name);

//find the travel time to drive a street segment (time(minutes) = distance(Km)/speed_limit(Km/h)*60)
double find_segment_travel_time(unsigned street_segment_id);

//find the nearest point of interest (by name) to a given position
std::string find_closest_point_of_interest(LatLon my_position);

string define_street_type(string);

void define_street_tier();

std::unordered_map<unsigned, unsigned> find_adjcent_intersections_by_segments (unsigned intersection_id);



