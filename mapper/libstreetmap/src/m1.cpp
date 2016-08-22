#include "m1.h"
#include "IntersectionGraph.h"
#include <unordered_map>
#include "intersection.h"


//structure contains map boundaries
mapbounds_info map_bounds;

//access appropriate structures by using name as key
std::unordered_map<std::string, intersection> intersection_data;
std::unordered_map<std::string, street_info> street_data;
std::unordered_map<std::string, std::vector<poi_info>> poi_data;
vector<string> list_street_type;
std::unordered_map<char, vector<search_info>> search_engine;

layer layerSys;
//used to sort POIs and determine range to search when given a location
//access poi_search to get vector of POI stored within
/*std::vector<unsigned> poi_search[POIARRAYMAX][POIARRAYMAX];
poi_info poi_data;*/



//this function initializes members of intersection_info and places into intersection_data according
//to intersection name

void load_intersection_data() {
    
    intersection data;
    
    unsigned long long number_of_intersections = getNumberOfIntersections();
    
    //loop through intersections and store data into intersection_data
    //may need to revise if more members are added to intersection_info
    for(unsigned i = 0; i < number_of_intersections; ++i)
    {      
        data.setID (i);
        data.setPrevious(NO_PREVIOUS);
        // data.setAdjacentIntersections (find_adjacent_intersections(i));
        
        // vector<unsigned> vectortest(find_adjacent_intersections (i));
        intersection_data.insert (make_pair(getIntersectionName(i), data));
    }
}

//this function initializes members of street_info and places into street_data according to street name
void load_street_data() {
    
    street_info data;
    
    unsigned long long number_of_streets = getNumberOfStreets();
    unsigned long long number_of_street_segments = getNumberOfStreetSegments();
    
    //create vector of vectors to store street segments
    //vector index represents street id, containing vector of streeet segment IDs belonging to street
    vector <vector<unsigned>> street_street_segments_v;
    street_street_segments_v.resize(number_of_streets);
    
    //loop through all street segments IDs and store street segment IDs according to street it belongs
    for(unsigned i = 0; i < number_of_street_segments; ++i)
    {
        street_street_segments_v[getStreetSegmentStreetID(i)].push_back(i);
    }
    
    //create vector of vectors to store street intersections
    //vector index represents street id, containing vector which stores intersections belonging to street
    vector <vector<unsigned>> all_street_intersections_v;
    all_street_intersections_v.resize(number_of_streets);
    
    //loop through all street IDs
    for(unsigned i = 0; i < number_of_streets; ++i)
    {
        //get street segments that belong to current street
        vector<unsigned> street_segments = street_street_segments_v[i];
        vector<unsigned> street_intersections;
        
        //loop through street segments and push intersections
        for(unsigned j = 0; j < street_segments.size(); ++j)
        {
            StreetSegmentEnds ends = getStreetSegmentEnds(street_segments[j]) ;
            street_intersections.push_back(ends.to);
            street_intersections.push_back(ends.from);
        }

        //sort street_intersections to remove duplicates
        sort(street_intersections.begin(), street_intersections.end());
        street_intersections.erase(unique(street_intersections.begin(), street_intersections.end()), street_intersections.end());
         
        //store intersections in appropriate vector (street id it belongs to))
        all_street_intersections_v[i] = street_intersections;
        
    }
    
    //store data in street_data, where key is street name, value is street_info
    for(unsigned i = 0; i < number_of_streets; ++i)
    {
        data.id = i;
        data.street_intersections = all_street_intersections_v[i];
        data.street_segments = street_street_segments_v[i];
        street_data.insert(std::make_pair(getStreetName(i), data));
    }
}

void load_poi_data() {
    
    poi_info data;

    string poi_name;
    unsigned long long number_poi = getNumberOfPointsOfInterest();

    for(unsigned i = 0; i < number_poi; ++i)
    {
	data.id = i;	
	data.position = getPointOfInterestPosition(i);
	poi_name = getPointOfInterestName(i);
	//poi_data[poi_name].push_back(data);

	if(poi_data.count(poi_name) == 0)
	{
		vector<poi_info> poi_list;
		poi_list.push_back(data);	
		poi_data.insert(std::make_pair(poi_name, poi_list));
                //cout << poi_name << endl;
	}
	else
	{
		poi_data[poi_name].push_back(data);
	}	
    }
}

void parse_search_info()
{
    
    
    for(unsigned i = 0; i < getNumberOfIntersections(); ++i)
    {
        string intr_name = getIntersectionName(i);
        
        search_info temp;    
        
        if(getIntersectionName(i).find("(") == std::string::npos && 
                getIntersectionName(i).find("unknown") == std::string::npos)
        {
            temp.id = i;
            temp.type = 0;
            stringstream parse(intr_name);
            vector<string> parse_out{istream_iterator<string>{parse}, istream_iterator<string>{}};
            vector<string> parse_with_type;
            vector<string> parse_no_type;
            string temp_str;
            
            //parse without types
            for(unsigned j = 0; j < parse_out.size(); ++j)
            {
                if(define_street_type(parse_out[j]) == "")
                {
                    
                    if(j == 0)
                    {
                        temp_str = parse_out[j] + " ";
                    }
                    else if(j != parse_out.size() - 1)
                    {
                        temp_str = temp_str + parse_out[j] + " ";
                    }
                    else
                    {
                        temp_str = temp_str + parse_out[j];
                    }

                    parse_no_type.push_back(parse_out[j]);
                    
                }  
            }
            
            temp.name = temp_str;
            //cout << temp_str << endl;
            
            
            /*if(search_engine.count(toupper(temp_str.at(0))) == 0)
            {
                vector<search_info> temp_list;
                temp_list.push_back(temp);	
                search_engine.insert(std::make_pair(temp_str.at(0), temp_list));
            }
            else
            {
                search_engine[temp_str.at(0)].push_back(temp);
            }*/
        }
        
        
    }
}

//determine map boundaries by looping through all features
void load_mapbounds_data(){
    unsigned num_feat = getFeatureCount();
    
    //initialize maximum and minimum latlon values
    map_bounds.maxlat = getFeaturePoint(0,0).lat;
    map_bounds.minlat = getFeaturePoint(0,0).lat;
    map_bounds.maxlon = getFeaturePoint(0,0).lon;
    map_bounds.minlon = getFeaturePoint(0,0).lon;
    
    //determine maximum and minimum latlon of all feature
    for(unsigned i = 0; i < num_feat; ++i)
    {
        unsigned num_point = getFeaturePointCount(i);
        for(unsigned j = 0; j < num_point; ++j)
        {
            LatLon curr = getFeaturePoint(i,j);
            if(curr.lat > map_bounds.maxlat) map_bounds.maxlat = curr.lat;
            else if(curr.lat < map_bounds.minlat) map_bounds.minlat = curr.lat;
            if(curr.lon > map_bounds.maxlon) map_bounds.maxlon = curr.lon;
            else if(curr.lon < map_bounds.minlon) map_bounds.minlon = curr.lon;
        }
    }
    map_bounds.mapwidth = (map_bounds.maxlon - map_bounds.minlon)*LON_TO_COOR_RATIO;
    map_bounds.mapheight = (map_bounds.maxlat - map_bounds.minlat)*LON_TO_COOR_RATIO;
}

//returns a 2 element vector, where the first element is row, second element is col
/*vector<unsigned> load_poi_array_helper(LatLon curr) {
    vector<unsigned> coord;
    //determines if appropriate bin is found
    bool found = false;
        
    //determine row to bin
    //determines if lat is out of bounds (may not be necessary)
    if(curr.lat >= poi_data.maxlat)
        {
            found = true;
            coord.push_back(POIARRAYMAX - 1);
        }
    else if(curr.lat < poi_data.minlat)
    {
        found = true;
        coord.push_back(0);
    }
    //if not out of bounds, determine appropriate row designation
    for(unsigned j = 0; j < POIARRAYMAX && !found ; ++j)
    {
        if((curr.lat >= poi_data.minlat) && (curr.lat < (poi_data.minlat + (poi_data.row_increment) *j)))
        {
            found = true;
            coord.push_back(j);
        }        
    }

    //reset flag, reused below
    found = false;

    //determine col to bin
    //determines if lon is out of bounds (may not be necessary)
    if(curr.lon >= poi_data.maxlon)
    {
        found = true;
        coord.push_back(POIARRAYMAX - 1);
    }
    else if(curr.lon < poi_data.minlon)
    {
        found = true;
        coord.push_back(0);
    }
    //if not out of bounds, determine appropriate col designation
    for(unsigned j = 0; j < POIARRAYMAX && !found ; ++j)
    {
        if((curr.lon >= poi_data.minlon) && (curr.lon < (poi_data.minlon + (poi_data.col_increment) *j)))
        {
            found = true;
            coord.push_back(j);
        }        
    }
    
    return coord;
}

//this functions sorts all POI into an n by n array defined by POIARRAYMAX
//sorts POI according to which coord range they will fall into
void load_poi_array() {
    unsigned num = getNumberOfPointsOfInterest();
    
    //initialize maximum and minimum latlon values
    poi_data.maxlat = getPointOfInterestPosition(0).lat;
    poi_data.minlat = getPointOfInterestPosition(0).lat;
    poi_data.maxlon = getPointOfInterestPosition(0).lon;
    poi_data.minlon = getPointOfInterestPosition(0).lon;
    
    //determine maximum and minimum latlon of all POI
    for(unsigned i = 0; i < num; ++i)
    {
        LatLon curr = getPointOfInterestPosition(i);
        if(curr.lat > poi_data.maxlat) poi_data.maxlat = curr.lat;
        else if(curr.lat < poi_data.minlat) poi_data.minlat = curr.lat;
        if(curr.lon > poi_data.maxlon) poi_data.maxlon = curr.lon;
        else if(curr.lon < poi_data.minlon) poi_data.minlon = curr.lon;
    }
    
    //determines size of bin with respect to POIARRAYMAX
    poi_data.row_increment = (poi_data.maxlat - poi_data.minlat)/POIARRAYMAX;
    poi_data.col_increment = (poi_data.maxlon - poi_data.minlon)/POIARRAYMAX;
    

    //loop through LatLon of POIs and sorts into POIARRAYMAXxPOIARRAYMAX array poi_search
    for(unsigned i = 0; i < num; ++i)
    {
        LatLon curr = getPointOfInterestPosition(i);
        
        //returns a 2 element vector, where the first is row, last is col
        vector<unsigned> coord = load_poi_array_helper(curr);
        
        //place into bin       
        poi_search[coord.front()][coord.back()].push_back(i);
    }
    
}*/

//this functions loads the specified map .bin and creates the data structures
bool load_map(std::string map_name) {
   
    bool load_success = loadStreetDatabaseBIN(map_name);
    
    //load data structures
    load_intersection_data();
    load_street_data();
    load_poi_data();
    load_mapbounds_data();
    define_street_tier();
    layerSys.load_data();
    //parse_search_info();

    //load_poi_array();
    
    return load_success;
}

//close the map
void close_map() {
    closeStreetDatabase();
    
    // destroy any data structures you created in load_map
    // ...
}

//returns associated intersection id from name by accessing intersection_data
unsigned find_intersection_id_from_name(std::string intersection_name)
{
    return intersection_data[intersection_name].getID();   
}

//returns associated street id from name by accessing street_data
unsigned find_street_id_from_name(std::string street_name)
{
    return street_data[street_name].id;
}

//returns vector<unsigned>  of street segments belonging to intersection
//this function calls find_intersection_street_segments(unsigned)
//uses intersection_data to determine id from name
std::vector<unsigned> find_intersection_street_segments(std::string intersection_name)
{   
    return find_intersection_street_segments(intersection_data[intersection_name].getID());
}

//returns vector<unsigned>  of street segments belonging to intersection
//this function loops through an intersection's street segments using API and pushes IDs 
std::vector<unsigned> find_intersection_street_segments(unsigned intersection_id)
{
    unsigned long long number_of_street_segments = getIntersectionStreetSegmentCount(intersection_id);
    vector<unsigned> intersection_street_segments;
 
    //loop through street segments of intersection and pushes IDs into intersection_street_segments
    for(unsigned i = 0; i < number_of_street_segments; ++i)
    {
         unsigned street_segment_id = getIntersectionStreetSegment(intersection_id, i);
         intersection_street_segments.push_back(street_segment_id);
    }
    
    return intersection_street_segments;
}

//returns vector<string>  of street segments connect to intersection, without duplicates
//uses find_intersection_street_segments(unsigned)
//uses intersection_data to determine id from name
std::vector<std::string> find_intersection_street_names(std::string intersection_name)
{
    return find_intersection_street_names(intersection_data[intersection_name].getID());
}

//returns vector<string>  of street segments connect to intersection, without duplicates
//vector of street segments is gotten from find_intersection_street_segments and parses through the duplicates
std::vector<string> find_intersection_street_names(unsigned intersection_id)
{
    vector<unsigned> intersection_street_segments = find_intersection_street_segments(intersection_id);
    vector<string> street_names;
    bool repeat = false;
    string street_name;
    
    //loops through intersections_street_segments
    for(unsigned i = 0; i < intersection_street_segments.size(); ++i)
    {
        unsigned street_id = getStreetSegmentStreetID(intersection_street_segments[i]);
        street_name = getStreetName(street_id);
        //pushes because first element will be unique
        if(i == 0)
        {
            street_names.push_back(street_name);
        }
        else
        {
            
            //loops through street_name to determine if not unique
            for(unsigned j = 0; j < street_names.size(); ++j)
            {
                if(street_names[j] == street_name)
                {
                    //sets flag
                    repeat = true;
                }
            }
            //only pushes if unique
            if(!repeat)
            {
                street_names.push_back(street_name);
                
            }
            //reset flag
            repeat = false;
        }
        repeat = false;
    }
    
    return street_names;
}

//returns vector<unsigned>  of street segments connect to intersection, without duplicates
//vector of street segments is gotten from find_intersection_street_segments and parses through the duplicates
vector<unsigned> find_intersection_street_ids(unsigned intersection_id)
{
    vector<unsigned> intersection_street_segments = find_intersection_street_segments(intersection_id);
    vector<unsigned> street_ids;
    bool repeat = false;
    unsigned street_id = UINT_MAX;
    
    //loops through intersections_street_segments
    for(unsigned i = 0; i < intersection_street_segments.size(); ++i)
    {
        street_id = getStreetSegmentStreetID(intersection_street_segments[i]);
        // pushes because first element will be unique
        if(i == 0)
        {
             street_ids.push_back(street_id);
        }
        else
        {
            
            //loops through street_name to determine if not unique
            for(unsigned j = 0; j < street_ids.size(); ++j)
            {
                if(street_ids[j] == street_id)
                {
                    //sets flag
                    repeat = true;
                }
            }
            //only pushes if unique
            if(!repeat)
            {
                street_ids.push_back(street_id);
                
            }
            //reset flag
            repeat = false;
        }
        repeat = false;
    }
    
    return street_ids;
}

// depends on find_intersection_id_from_name
// This function check if two intersections are connected or not by checking if 
// any segment of intersection1 leads to intersection2
bool are_directly_connected(std::string intersection_name1, std::string intersection_name2)
{
    unsigned intersection_id1 = find_intersection_id_from_name (intersection_name1);
    unsigned intersection_id2 = find_intersection_id_from_name (intersection_name2);
    int num_of_street_segment = getIntersectionStreetSegmentCount (intersection_id1);
    
    // loop through all the street segments
    for (int i=0; i<num_of_street_segment; i++)
    {
        unsigned street_segment_id = getIntersectionStreetSegment (intersection_id1, i);
        StreetSegmentEnds ends = getStreetSegmentEnds (street_segment_id);
        unsigned intersection_from, intersection_to;
        intersection_from = ends.from;
        intersection_to = ends.to;
        // check if this segment is connected to intersection2
        if(intersection_to == intersection_id2)
        {
            return true;
        }
        else if (intersection_from == intersection_id2)
        {   
            // is directly connected unless this street segment is both way
            if (!getStreetSegmentOneWay (street_segment_id))
            {
                return true;
            }
        }
    }
    
    return false;
}

// this function return vector of adjacent intersection of input intersection
// by checking two end intersections of street segments;
// if one of the intersection of the street segment is the given intersection,
// then the other one is adjacent. Also, this function considers the case of one way street
// by making sure that the from intersection is the given intersection if the street segment
// is one way
std::vector<unsigned> find_adjacent_intersections(std::string intersection_name)
{
    unsigned intersection_id = intersection_data[intersection_name].getID();
    int num_of_street_segment = getIntersectionStreetSegmentCount (intersection_id);
    
    vector<unsigned> adjacent_intersections;
    
    // loop through street segments connected to intersection1
    for (int i=0; i<num_of_street_segment; i++)
    {
        unsigned street_segment_id = getIntersectionStreetSegment (intersection_id, i);
        StreetSegmentEnds ends = getStreetSegmentEnds (street_segment_id);
        unsigned intersection_from, intersection_to;
        intersection_from = ends.from;
        intersection_to = ends.to;
        
        // if the ends.to match with the intersection
        if (intersection_from == intersection_id)
        {
            adjacent_intersections.push_back(intersection_to);
        }
        // if the "ends.to" intersection match with the intersection and street segment is both way
        else if(intersection_to == intersection_id)
        {
            // in case of one way street, check if the given intersection is the ends.from
            if (!getStreetSegmentOneWay(street_segment_id))
            {
                adjacent_intersections.push_back(intersection_from);
            }
        }
    }    
    
    return adjacent_intersections;
}

std::vector<unsigned> find_adjacent_intersections(unsigned intersection_id)
{
    int num_of_street_segment = getIntersectionStreetSegmentCount (intersection_id);
    
    vector<unsigned> adjacent_intersections;
    
    // loop through street segments connected to intersection1
    for (int i=0; i<num_of_street_segment; i++)
    {
        unsigned street_segment_id = getIntersectionStreetSegment (intersection_id, i);
        StreetSegmentEnds ends = getStreetSegmentEnds (street_segment_id);
        unsigned intersection_from, intersection_to;
        intersection_from = ends.from;
        intersection_to = ends.to;
        
        // if the ends.to match with the intersection
        if (intersection_from == intersection_id)
        {
            adjacent_intersections.push_back(intersection_to);
        }
        // if the "ends.to" intersection match with the intersection and street segment is both way
        else if(intersection_to == intersection_id)
        {
            // in case of one way street, check if the given intersection is the ends.from
            if (!getStreetSegmentOneWay(street_segment_id))
            {
                adjacent_intersections.push_back(intersection_from);
            }
        }
    }    
    
    return adjacent_intersections;
}


// direct access the element in our data structure
std::vector<unsigned> find_street_street_segments(std::string street_name)
{
    return street_data[street_name].street_segments;
}

// direct access the element in our data structure
std::vector<unsigned> find_all_street_intersections(std::string street_name)
{
    return street_data[street_name].street_intersections;
}

//calculate the distance between two points by using the pythagorean theorem
//respective x and y coordinates were calculated 
//using lat and lon of the corresponding points
double find_distance_between_two_points(LatLon point1, LatLon point2)
{
    double latavg = (point1.lat + point2.lat)/2.0; //calculates the average latitude
    double distance1x = point1.lon*cos(latavg*DEG_TO_RAD)*DEG_TO_RAD; // this is x1 
    double distance1y = point1.lat*DEG_TO_RAD; // this is y1
    double distance2x = point2.lon*cos(latavg*DEG_TO_RAD)*DEG_TO_RAD; // this is x2
    double distance2y = point2.lat*DEG_TO_RAD;  // this is y2
    double Ydistance = distance2y-distance1y; //this is the formula y2-y1
    double Xdistance = distance2x-distance1x; //this is the formula x2-x1
    double distance = EARTH_RADIUS_IN_METERS*sqrt(pow(Ydistance,2.0) + pow(Xdistance,2.0)); 
    //this is the pythagorean theorem*Earth's radius
    return distance;
}

//calculates the distance of a street segment
//first checks to see if there are curve points, if at least curve point exists 
//then it calculates the distances between the endpoints and curve points 
//and returns them at the end 
double find_street_segment_length(unsigned street_segment_id)
{
    double lengthOfSegment=0;
    unsigned numOfCurves = getStreetSegmentCurvePointCount(street_segment_id); 
    //checks the number of curve points
    StreetSegmentEnds startToEnd = getStreetSegmentEnds(street_segment_id); 
    //gets the end points
    if(numOfCurves==0) 
    //if there are no curves just calculate the distance between 2 end points
    {
        LatLon start = getIntersectionPosition(startToEnd.from);
        LatLon ending = getIntersectionPosition(startToEnd.to);
        lengthOfSegment += find_distance_between_two_points(start, ending);
        return lengthOfSegment;
    }
    //else if there is at least one curve point 
    //then the bottom part first calculates the distance between the starting point 
    //and the first curve point
    LatLon startP = getIntersectionPosition(startToEnd.from);
    LatLon curvePoint = getStreetSegmentCurvePoint(street_segment_id,0);
    lengthOfSegment += find_distance_between_two_points(startP, curvePoint);
    for(unsigned i=0; i<numOfCurves; i++) 
    //this calculates the distance between the curve points
    {
        if(i==numOfCurves-1)
        {
            break;
        }
        LatLon currentP = getStreetSegmentCurvePoint(street_segment_id,i);
        LatLon futureP = getStreetSegmentCurvePoint(street_segment_id,i+1);
        lengthOfSegment += find_distance_between_two_points(currentP,futureP);
        curvePoint=futureP;
    }
    LatLon endingP = getIntersectionPosition(startToEnd.to); // this last part calculates the distance between end point and the last curve point
    lengthOfSegment += find_distance_between_two_points(curvePoint,endingP);
    return lengthOfSegment;
}

//first checks to see if the street has multiple segments
//if multiple segments exist, the function calculates each street segments, 
//which later will be added up and returned
double find_street_length(std::string street_name) 
{
    double street_length=0;
    vector<unsigned> streetSegments = find_street_street_segments(street_name); 
    // first gets all the street segments
    unsigned numOfSeg = streetSegments.size(); //gets the number of street segments
    //if there are no street segments which means it's just one long 
    //road then just calculate the distance between the endpoints
    for(unsigned j=0;j<numOfSeg;j++) 
        //else if there is at least one street segment 
        //then calculates the distance using the function find_street_segment_length
    {
        street_length += find_street_segment_length(streetSegments[j]);
    }
    return street_length;
}

//this function first calculates the distance of the street segment
//then divides the the distance by the speed limit of that street segment
double find_segment_travel_time(unsigned street_segment_id)
{
    double streetSegDistance = find_street_segment_length(street_segment_id); 
    //first finds the length of the street segment
    double travel_time = streetSegDistance * 0.06 /getStreetSegmentSpeedLimit(street_segment_id); 
    //then uses the formula time=distance/speed to calculate the time
    return travel_time;
}

//this function first determines whether the given position is within the bounds of poi_search
//if it is not, it does a linear search, else it determines where in poi_search to compare distances
//returns the POI with the shortest distance
std::string find_closest_point_of_interest(LatLon my_position)
{
    unsigned long long num_of_interest = getNumberOfPointsOfInterest (); 
    //gets the number of point of interest
    LatLon current_point_of_interest;
    unsigned closest_point_of_interest=0;
    double shortest_distance = 0;
    double distance = 0;

    current_point_of_interest = getPointOfInterestPosition (0); 
    //gets the lat and lon of the first point of interest
    shortest_distance = find_distance_between_two_points (my_position, current_point_of_interest); 
    //calculates the distance between where the user is and the first point of interest
    for (int i=1; i< (int)num_of_interest; i++) 
    //then calculates the distance between the user's position and 
    //the rest of the points of interest
    //then looks for the shortest distance between the point of interest 
    //and the user's position
    {  
        current_point_of_interest = getPointOfInterestPosition (i);
        distance = find_distance_between_two_points (my_position, current_point_of_interest);
        if (distance < shortest_distance)
        {
            shortest_distance = distance;
            closest_point_of_interest = i;
        }
    }
    return getPointOfInterestName (closest_point_of_interest);
}

/*std::string find_closest_point_of_interest(LatLon my_position)
{
    bool out_of_bounds = false;
    double shortest_distance = find_distance_between_two_points(my_position, getPointOfInterestPosition(0));
    unsigned closest_point_of_interest = 0;
    
    //determines if my_position is out of bounds
    if( (my_position.lat > poi_data.maxlat)||(my_position.lat < poi_data.minlat) ||
            (my_position.lon > poi_data.maxlon)||(my_position.lon < poi_data.minlon))
        out_of_bounds = true;
    
    //if out_of_bounds, only search perimeter coords
    //needs implementation, currently searches every POI
    //could determine which side of the array its on and only search that particular side
    if(out_of_bounds)
    {
        for(unsigned i = 0; i < getNumberOfPointsOfInterest(); ++i)
        {
            double distance = find_distance_between_two_points(my_position, getPointOfInterestPosition(i));
            if(distance < shortest_distance)
            {
                shortest_distance = distance;
                closest_point_of_interest = i;
            }
        }
    }
    
    //check in a 3x3 area around target_coord to ensure accuracy
    else
    {
        //returns 2 element vector of where my_position would be if sorted into poi_search
        //first element is row, second element is col
        vector<unsigned> target_coord = load_poi_array_helper(my_position);
        int target_row = target_coord.front();
        int target_col = target_coord.back();
        
        //loop through from target_row - 1 to target_row + 1
        for(int i = (target_row-1); i <= (target_row+1); ++i)
        {
            //checks if row is in array
            if((i >= 0) && (i < POIARRAYMAX))
            {
                //loop through target_col -1 to target_col + 1
                for(int j = (target_col-1); j <= (target_col+1); ++j)
                {
                    //checks if col is in array
                    if((j >= 0) && (j < POIARRAYMAX))
                    {
                        //checks if bin is empty
                        if(poi_search[i][j].size() != 0)
                        {
                            for(unsigned k = 0; k < poi_search[i][j].size(); ++k)
                            {              
                                unsigned compare = poi_search[i][j][k];
                                double distance = find_distance_between_two_points(my_position, getPointOfInterestPosition(compare));
                                //replace if a new shortest distance is found
                                if(distance < shortest_distance)
                                {
                                    shortest_distance = distance;
                                    closest_point_of_interest = compare;
                                }

                            }
                        }
                    }
                }
            }
        }
    }
    return getPointOfInterestName (closest_point_of_interest);
}*/


string define_street_type(string potential)
{
    std::string street_type = "";
    if(potential == "Avenue")
    {
        street_type = "Avenue";
    }
    else if(potential == "Bridge")
    {
        street_type = "Bridge";
    }
    else if(potential == "Boulevard")
    {
        street_type = "Boulevard";
    }
    else if(potential == "Carseway")
    {
        street_type = "Carseway";
    }
    else if(potential == "Circle")
    {
        street_type = "Circle";
    }
    else if(potential == "Circuit")
    {
        street_type = "Circuit";
    }
    else if(potential == "Crescent")
    {
        street_type = "Crescent";
    }
    else if(potential == "Court")
    {
        street_type = "Court";
    }
    else if(potential == "Close")
    {
        street_type = "Close";
    }
    else if(potential == "Drive")
    {
        street_type = "Drive";
    }
    else if(potential == "Gardens")
    {
        street_type = "Gardens";
    }
    else if(potential == "Green")
    {
        street_type = "Green";
    }
    else if(potential == "Grove")
    {
        street_type = "Grove";
    }
    else if(potential == "Gate")
    {
        street_type = "Gate";
    }
    else if(potential == "Heights")
    {
        street_type = "Heights";
    }
    //else if(potential == "Highway")
    //{
    //    street_type = "Highway";
    //}
    else if(potential == "Hill")
    {
        street_type = "Hill";
    }
    else if(potential == "Lane")
    {
        street_type = "Lane";
    }
    else if(potential == "Line")
    {
        street_type = "Line";
    }
    else if(potential == "Lawn")
    {
        street_type = "Lawn";
    }
    else if(potential == "Mews")
    {
        street_type = "Mews";
    }
    else if(potential == "Paths")
    {
        street_type = "Paths";
    }
    else if(potential == "Park")
    {
        street_type = "Park";
    }
    else if(potential == "Parkway")
    {
        street_type = "Parkway";
    }
    else if(potential == "Place")
    {
        street_type = "Place";
    }
    else if(potential == "Ramp")
    {
        street_type = "Ramp";
    }
    else if(potential == "Roadway")
    {
        street_type = "Roadway";
    }
    else if(potential == "Road")
    {
        street_type == "Road";
    }
    else if(potential == "Square")
    {
        street_type = "Square";
    }
    else if(potential == "Street")
    {
        street_type = "Street";
    }
    else if(potential == "Terrace")
    {
        street_type = "Terrace";
    }
    else if(potential == "Trail")
    {
        street_type = "Trail";
    }
    else if(potential == "View")
    {
        street_type = "View";
    }
    else if(potential == "Walk")
    {
        street_type = "Walk";
    }
    else if(potential == "Way")
    {
        street_type = "Way";
    }
    else if(potential == "Woods")
    {
        street_type = "Woods";
    }
    else
    {
        street_type = "";
    }
    
    return street_type;

}

//determines street tier (1,2,3) by street segments, intersections
//tier 1: minor streets
//tier 2: major streets (i.e yonge)
//tier 3: highways and the like
void define_street_tier() {

    for(unsigned i = 0; i < getNumberOfStreets(); ++i)
    {
        double average_speed = 0;

        vector<unsigned> street_seg = street_data[getStreetName(i)].street_segments;

        for(unsigned j = 0; j < street_seg.size(); ++j)
        {
            average_speed = average_speed + getStreetSegmentSpeedLimit(street_seg[j]);
        }

        average_speed = average_speed/street_seg.size();

        if(average_speed <= 50 )
        {
            if(street_seg.size() < 20)
                street_data[getStreetName(i)].tier = 0;
            else street_data[getStreetName(i)].tier = 1;
        }
        else if(average_speed > 50 && average_speed <= 60)
        {
            street_data[getStreetName(i)].tier = 2;
        }
        else if(average_speed > 60)
        {
            street_data[getStreetName(i)].tier = 3;
        }
        else
        {
            if(street_seg.size() < 20)
                street_data[getStreetName(i)].tier = 0;
            else street_data[getStreetName(i)].tier = 1;
        }
        //if(street_data[getStreetName(i)].tier == 3)
        //cout << getStreetName(i) <<  endl;
    }
}



std::unordered_map<unsigned, unsigned> find_adjcent_intersections_by_segments (unsigned intersection_id)
{
    std::unordered_map<unsigned, unsigned> adjcent_intersections_and_segments;

    vector<unsigned> adj_segments = find_intersection_street_segments(intersection_id);
    StreetSegmentEnds ends;


    int num_of_segments = adj_segments.size();
    for (int i=0; i<num_of_segments; i++)
    {
        ends = getStreetSegmentEnds (adj_segments[i]);
        if (ends.from == intersection_id || !getStreetSegmentOneWay (adj_segments[i]))
        {
            adjcent_intersections_and_segments.insert (make_pair(intersection_id, adj_segments[i]));
        }
    }

    return adjcent_intersections_and_segments;
}

