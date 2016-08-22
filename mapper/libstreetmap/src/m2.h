#pragma once
#include <string>

#include <cfloat> // for FLT_MAX
#include <chrono>
#include <thread>
#include <cstdlib>
#include <math.h>
#include <vector>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <cstring>
#include <time.h>
#include <iomanip>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>


#include "layer.h"
#include "userInput.h"
#include "graphics.h"
#include "m1.h"

// #define MAP_ORIGIN_X 0
// #define MAP_ORIGIN_Y 0

#define LAT_TO_COOR_RATIO   100000
#define LON_TO_COOR_RATIO   100000

//to scale text
//use (res.top_height/BASE_RES)
#define BASE_RES    1000.0
//area of toroto map
#define TORONTO_A_STANDARD  (43999.*79000)

#define STREET_COLOR WHITE

using namespace std;

struct track_curr_view {
    t_bound_box curr_bounds;
    double h_ratio;
    double w_ratio;
};

struct _legend {
    int font_size = 12;
    int num_div = 10;
    int scroll_count = 0;
    bool more = false;
    float div_offset;
    float sdiv_offset;
    t_point up_center;
    t_point down_center;
    t_bound_box up_bounds;
    t_bound_box down_bounds;
    t_bound_box scroll_bounds;
    t_bound_box bar_bounds;
    bool up_backdrop = false;
    bool down_backdrop = false;
    vector <string> active_layers;
    vector <t_color> layer_colors;
};

struct dir_data {
    int type;
    string street;
    string direction;
    double distance;
    vector<unsigned> path_seg;
};


struct _directions {
    int nesw_font_size = 13.5;
    int main_font_size = 12;
    int default_font_size = 11;
    int minor_font_size = 10;
    int num_div = 10;
    int scroll_count = 0;
    float header_offset;
    float sdiv_offset;
    float div_offset;
    bool more = false;
    bool spec = false;
    bool hide = false;
    t_bound_box header;
    t_point up_center;
    t_point down_center;
    t_bound_box up_bounds;
    t_bound_box down_bounds;
    t_bound_box scroll_bounds;
    t_bound_box bar_bounds;
    t_bound_box spec_bounds;
    bool up_backdrop = false;
    bool down_backdrop = false;
    vector <dir_data> dir;
    vector <vector<string>> dir_parsed;
    vector <unsigned> index;
};

struct main{
    //vector <t_bound_box> button_bounds;
    vector <string> caption{"About"};
    
    //ABOUT BUTTON BOUNDS
    t_bound_box about_bBounds;
    t_bound_box about_exit;
    t_bound_box about_box;
    
    //COURIER BUTTON BOUNDS
    t_bound_box cour_bBounds;
    t_bound_box cour_box;
    t_bound_box cour_header;
    t_bound_box cour_next;
    t_bound_box cour_done;
    
    //SINGLE PATH BUTTON BOUNDS
    t_bound_box path_bBounds;
    t_bound_box path_box;
    t_bound_box path_header;
    t_bound_box path_next;
    t_bound_box path_done;
               
    //ABOUT VARIABLES
    t_bound_box c_circ;
    t_bound_box r_circ;
    t_bound_box s_circ;
    bool c_about_msg = false;
    bool r_about_msg = false;
    bool s_about_msg = false;
    
    //COURIER VARIABLES
    //cour state
    //0 = choose depots
    //1 = choose intr
    //2 = calculating
    //3 = choose path
    int cour_state = 0;
    bool cour_onNext = false;
    bool cour_onDone = false;
    vector<unsigned> cour_depots;
    vector<unsigned> cour_intrs;
    vector<unsigned> cour_path;
    vector<unsigned> cour_path_intrs;
    int cour_first = -1;
    int cour_last = -1;
    
    //SINGLE PATH VARIABLES
    //path sate
    //0 = choose waypoints
    //1 = path done
    int path_state = 0;
    bool path_onNext = false;
    bool path_onDone = false;
    vector<unsigned> path_waypoints;
    vector<unsigned> path_path;
    vector<unsigned> path_path_intrs;
    
    //button numbers
    //-1 = none
    // 0 = about
    // 1 = courier
    // 2 = single path
    
    //which button the mouse is currently hovering over
    int button_spec = -1;
    
    //which button is activated
    int active_button = -1;
    
    bool main_active = false;
    
};

extern bool path_set;
extern bool legend_state;
extern bool prev_state;
extern t_point mouse_location_current;
extern _legend legend;
extern _directions dir;
extern t_bound_box layermenu_bounds;
extern t_bound_box dir_bounds;
extern t_bound_box minimap_bounds;
extern int posi;
extern main menu;


// boudary of Latitude and Longitude
// -79.79 , 43.48 , -79.00001 , 43.91999

const double default_line_width = 10;      // relative to the scale of the map which is 78999 x 43999
const double default_high_way_width = 0.3;
const double default_street_width = 0.1;      // in pixels
const double default_one_way = 0.05;         // half of the both way streets
const double default_font_size = 9; // 4;    
// default size of word on the map
const double default_water_way_width = 0.5;
const double pin_size = 2000.0 * LAT_TO_COOR_RATIO /100000;
const double arrow_size = 4.0 * LAT_TO_COOR_RATIO /100000;

// color code
const t_color coastline_color (42,129,54);
const t_color default_land(130,168,128);
const t_color natural_land_color(171,210,168);
const t_color land_color (190,190,190);
// const t_color land_color (240,234,224);
const t_color street_color (255,255,255);
const t_color highway_color (220,170,0);
const t_color highway_color_2 (230,190,0);
const t_color blvd_color (245, 190, 0);

const t_color lake_color (130,180,255);
const t_color natural_water_color (50, 210, 120);
const t_color natural_sand_color (255,255,120);
const t_color natural_wood_color (50, 210, 120);
const t_color natural_tree_row_color (180,255,180);
const t_color natural_grassland_color (180,255,180);
const t_color natural_beach_color (215,215,100);
const t_color waterway_river_color (190,230,255);
const t_color waterway_stream_color (190,210,255);
// const t_color waterway_stream_color (255,0,0);

const t_color waterway_boatyard_color (190,230,255);


const t_color pin_color (255,0,0);
const t_color search_result_color (0,0,255);

const t_color start_point_color (0,255,0);
const t_color end_point_color (255,255,0);

const t_color path_color (0,0,255);
const t_color unselected_colour(255,255,255);
const t_color selected_colour (255,255,0);
const double path_width = 10;

extern t_bound_box minimap_bounds;
extern t_bound_box textbox_bounds;
extern track_curr_view track_spec;
extern t_bound_box initial_coords;


//coordinates for user click POI
extern std::vector<t_point> pinned_points;
extern std::vector<unsigned> path_data;


// Callbacks for event-driven window handling.
void drawscreen (void);
void act_on_enable_input(void (*drawscreen_ptr) (void));
void act_on_search_func (void (*drawscreen_ptr) (void));
void act_on_clear_func (void (*drawscreen_ptr) (void));
void act_on_zoomfit_func (void (*drawscreen_ptr) (void));
void act_on_streetview_func(void (*drawscreen_ptr) (void));
void act_on_route_planning_func (void (*drawscreen_ptr) (void));
void act_on_button_press (float x, float y, t_event_buttonPressed event);
void act_on_mouse_move (float x, float y);
void act_on_key_press (char c);

void delay (long milliseconds);

// const t_bound_box initial_coords = t_bound_box(-MAP_WIDTH/2,-MAP_HEIGHT/2,MAP_WIDTH/2,MAP_HEIGHT/2); 
// const t_bound_box initial_coords = t_bound_box(MAP_ORIGIN_X,MAP_ORIGIN_Y,MAP_ORIGIN_X+MAP_WIDTH,MAP_ORIGIN_Y+MAP_HEIGHT);
/*const t_bound_box initial_coords = t_bound_box(LON_LEFT*LON_TO_COOR_RATIO, LAT_BOTTOM*LAT_TO_COOR_RATIO,
            LON_RIGHT*LON_TO_COOR_RATIO, LAT_TOP*LAT_TO_COOR_RATIO); */
// const t_bound_box initial_coords = t_bound_box(-2000, 4010, -1950, 4025);

// Draws the map whose at map_path; this should be a .bin file.
void draw_map(std::string map_path);



// Underlying functions

// calculate where a set of latitude and longtitude correspond to on the map
t_point get_coordinate_from_LatLon (LatLon geo_location);

// convert t_point coordinate back to LatLon
LatLon get_LatLon_from_coordinate (t_point );

// connect entire street
void draw_street (std::string stree_name);

// draw natural feature by name
// void draw_natural_feature (std::string natural_feature_name);


// helper functions
// find what is the map boundry in term of latitude and longitude
void find_bound ();
// find how much the map is zoomed in or out
double get_scale ();

double get_relative_zoom();

// find out the direction based on two input points in lat and lon
// return angle in degree
double calculate_line_direction (LatLon point1, LatLon point2);
void draw_city_name(double scale);
// this function draws all natural feature on the map
void draw_all_natural_features (double scale);

// this function draws all water category in the natural feature
void draw_all_water ();
// this function draws all natural category in the natural feature
void draw_all_natural ();
// this function draws all waterway category in the natural feature
void draw_all_waterways (double scale);
// this function draws all land category in the natural feature
void draw_all_lands (double scale);

// this function draws all the streets
void draw_all_streets (double scale);
void draw_streets (double ratio, double scale, t_color input_color);

// this function draws all the street names
void draw_street_names(double scale);
// this function draws all the pinned points
void draw_all_pinend_points (double scale);

// prepare the locations for drawscreen to draw
void save_search_results (std::string search_input);
// display search result
void display_search_results (double scale);

// draw arrow to indicate direction of one way streets
void draw_arrow (t_point direction_vector, t_point center);

//Minimap functions

//convert LatLon to coordinates on minimap
t_point get_mini_coordinate_from_LatLon (LatLon geo_location);

void draw_minimap(double scale);

//draws viewable world boundaries on minimap
void draw_trackmap(t_bound_box visible_world);

//when user clicks on minimap, map changes to that location based on zoom
void set_minimap_spec();

//draw features on minimap, similar to actual map features
void draw_minimap_features(double scale);
void draw_all_miniwater ();
void draw_all_mininatural ();
void draw_all_minilands ();
void draw_all_minwaterways ();

//draw searches on minimap
void draw_minimap_search(double scale);



t_point find_street_segment_center (int segID);

double get_street_name_length (unsigned long streetID, double font_size);


void draw_user_input(double scale, std::string text);

void draw_suggested_poi(double scale, std::string text);

void draw_suggested_intersections(double scale, std::string text);

void draw_text(std::string text);

void draw_suggested_intersection_texts(std::string text);

void draw_suggested_poi_texts(std::string text);

std::vector<std::string> parse_input(std::string text);

void draw_layermenu(double scale);
void draw_legend_spec(double scale);
void draw_legend_sbar();

const t_color layerland_color(130,168,128);
const t_color header_color(53,161,255);
const t_color scrollbar_color(157,191,250);

extern void layermenu_handler (void (*drawscreen_ptr) (void));
void naturalmenu_handler (void (*drawscreen_ptr) (void));
void waterwaymenu_handler (void (*drawscreen_ptr) (void));
void backmenu_handler (void (*drawscreen_ptr) (void));
void legendmenu_handler(void (*drawscreen_ptr) (void));

void layermenu_create();
void naturalmenu_create();
void waterwaymenu_create();
void back_create();
void legendmenu_create();

void layermenu_destroy();
void naturalmenu_destroy();
void waterwaymenu_destroy();
void legendmenu_destroy();

void reset_layer_state();

void set_active_layers(string);
void remove_active_layers(string);
void set_layer_colors(t_color);
void remove_layer_colors(t_color);

void legend_click_handler();

void nother_set (void (*drawscreen_ptr) (void));
void grassland_set (void (*drawscreen_ptr) (void));
void treerow_set (void (*drawscreen_ptr) (void));
void sand_set (void (*drawscreen_ptr) (void));
void heath_set (void (*drawscreen_ptr) (void));
void scrub_set (void (*drawscreen_ptr) (void));
void wetland_set (void (*drawscreen_ptr) (void));
void peak_set (void (*drawscreen_ptr) (void));
void cliff_set (void (*drawscreen_ptr) (void));
void tree_set (void (*drawscreen_ptr) (void));
void land_set (void (*drawscreen_ptr) (void));
void beach_set (void (*drawscreen_ptr) (void));
void wood_set (void (*drawscreen_ptr) (void));
void water_set (void (*drawscreen_ptr) (void));

void ditch_set (void (*drawscreen_ptr) (void));
void boatyard_set (void (*drawscreen_ptr) (void));
void rapids_set (void (*drawscreen_ptr) (void));
void breakwater_set (void (*drawscreen_ptr) (void));
void dam_set (void (*drawscreen_ptr) (void));
void riverbank_set (void (*drawscreen_ptr) (void));
void drain_set (void (*drawscreen_ptr) (void));
void stream_set (void (*drawscreen_ptr) (void));
void river_set (void (*drawscreen_ptr) (void));

void layer_river();
void layer_stream();
void layer_drain();
void layer_riverbank();
void layer_dam();
void layer_breakwater();
void layer_rapids();
void layer_boatyard();
void layer_ditch();

void layer_water();
void layer_wood();
void layer_beach();
void layer_land();
void layer_tree();
void layer_cliff();
void layer_peak();
void layer_wetland();
void layer_scrub();
void layer_heath();
void layer_sand();
void layer_treerow();
void layer_grassland();
void layer_nother();

std::string ConvertToCaps(std::string input);
std::string switchWords(std::string input);

const t_color river_color(1,143,195);
const t_color stream_color(110,190,219);
const t_color drain_color(33,98,182);
const t_color riverbank_color(79,148,165);
const t_color dam_color(108,96,74);
const t_color breakwater_color(255,255,255);
const t_color rapids_color(95,86,228);
const t_color boatyard_color(0,0,0);
const t_color ditch_color(230,32,32);

const t_color water_color(58,198,193);
const t_color wood_color(87,196,126);
const t_color beach_color(172,176,109);
const t_color land2_color(111,111,111);
const t_color tree_color(0,92,15);
const t_color cliff_color(222,222,222);
const t_color peak_color(0,92,15);
const t_color wetland_color(55,55,55);
const t_color scrub_color(234,123,45);
const t_color heath_color(123,45,67);
const t_color sand_color(255,0,0);
const t_color treerow_color(0,0,255);
const t_color grassland_color(0,255,0);
const t_color nother_color(32,179,110);

void draw_directions_load();
void draw_directions();
void draw_directions_helper(float, t_bound_box);
void draw_directions_arrow(t_bound_box, string);
void draw_directions_sbar();
void draw_directions_collapsed(float, t_bound_box);
void direction_click_handler();
void draw_directions_exspec_parse(int);
void draw_dir_exspec(int, vector<string>);
void set_default_layers();

void draw_main_menu();
void main_click_handler();
void draw_about_box();

const t_color depot_color(255,0,127);
const t_color intr_color(153,51,255);

void draw_cour_box();
void draw_positions(vector<unsigned>, t_color);
void draw_mini_courpos(double, t_color, vector<unsigned>);
void deter_cour_intrs();

void draw_path_box();
void deter_path_intrs();

void reset_states();
