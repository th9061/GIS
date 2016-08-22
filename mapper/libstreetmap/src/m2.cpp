#include "m2.h"
#include "m3.h"
#include "m4.h"
#include "userInput.h"
#include <sstream>
#include <string>
#include <cmath>
#include <math.h>
#include <cfloat> // for FLT_MAX

//stores ctrl+rightclick coordinates for selective zoom features
std::vector<t_point> selectzoom_points;

//street viewing mode flag
bool force_draw_streets;
bool enable_suggested_bar=false;
bool path_enabled=false;
int force_draw_street_counter=0;
int force_draw_input_counter=0;
int arrow_key_pressed=0;
int start_to_destination=0;
int path_activated=0;
unsigned saved_start_id=0;
unsigned saved_end_id=0;

//parsed city name from accessor
std::vector<char> city_name;

//coordinates of minimap
t_bound_box minimap_bounds;

//coordinates of textbox
t_bound_box textbox_bounds;
t_bound_box suggested_intersection;
t_bound_box suggested_poi;
t_color colorSelect;

//coordinates of tracking box in minimap
track_curr_view track_spec;

//coordinates of layermenu
t_bound_box layermenu_bounds;

//coordinates of directions box
t_bound_box dir_bounds;

// variable for mouse behaviors
t_point mouse_location_before;
t_point mouse_location_current;

//coordinates for max map resolution
t_bound_box initial_coords;

//coordinates for searched POI
std::vector<t_point> search_results;
std::vector<t_point> pinned_points;

//true if legend is viewable, else false
bool legend_state = false;

//string for the textbox
std::string textbox_text="";
std::string saved_poi="";
std::string saved_inter="";
std::string start_location="";
std::string end_location="";

//used for commands
std::vector<std::string> command_lines;
bool enable_input = false;

//info for legend
_legend legend;

userInput userinputSys(textbox_text);

//info for direction
bool path_set;
_directions dir;
int posi;

//for when left click is held down
//prev_state is used to confirm that the left mouse button is being held down
//without it the mouse would need to remain in the scrollbar bounds
//for drag scrolling to work, however we want it so that the user just needs
//to click on the scrollbar and from there if the left mouse button
//remains held they can drag it up and down, regardless of mouse location
//prev_state is set back to false once left click is released (handled in graphics.cpp)
bool prev_state = false;
t_point click_local;

//for text scaling
t_report res;

//info for main menu
main menu;

// Draws the map whose at map_path; this should be a .bin file.
void draw_map(std::string map_path)
{
    userinputSys.organize_intersections();
    userinputSys.organize_poi();

    std::cout << "About to start graphics.\n";
    
    //extract map name
    bool done = false;
    for(unsigned i = 0; !done; ++i)
    {
        char temp = map_path[26+i];
        //std::cout << temp;
        if(i == 0)
        {
            city_name.push_back(temp - 32); //capitalize
        }
        else if(temp == '_')
        {
            city_name.push_back(' '); //space
            city_name.push_back(map_path[26+i+1] - 32);
            i++;
        }
        else if(temp == '.') done = true;
        else
            city_name.push_back(temp);
    }
    /**************** initialize display **********************/
    initial_coords = t_bound_box(map_bounds.minlon*LON_TO_COOR_RATIO, map_bounds.minlat*LAT_TO_COOR_RATIO,
            map_bounds.maxlon*LON_TO_COOR_RATIO, map_bounds.maxlat*LAT_TO_COOR_RATIO); 
    
    // Set the name of the window (in UTF-8), with white background.
    init_graphics("SRC Mapper :)", t_color(240,230,210));

    // Set-up coordinates. The coordinates passed in define what coordinate
    // limits you want to use; this coordinate system is mapped to fill 
    // the screen.
    set_visible_world(initial_coords);

    // Set the message at the bottom (also UTF-8)
    update_message("Map Displaying");

    // create the search button
    create_button ("Window", "Search", act_on_search_func); // name is UTF-8
    create_button ("Search", "Clear", act_on_clear_func); // name is UTF-8
    //replace Zoom Fit with custom
    destroy_button("Zoom Fit");
    create_button ("Zoom Out", "Zoom Fit", act_on_zoomfit_func);
    
    //create street view button and initialize street view mode flag
    create_button("Clear", "Street View", act_on_streetview_func);
    create_button ("Street View", "Path", testPath); // name is UTF-8
    create_button("Street View","Input", act_on_enable_input);
    force_draw_streets = false;
    
    //destroy buttons not used
    destroy_button("Zoom In");
    destroy_button("Zoom Out");
    destroy_button("Window");
    destroy_button("Search");
    
    //create Layer Menu button
    create_button("Path", "Layers ON", layermenu_handler);
    
    //t_color header_color(53,161,255);
    legend.active_layers.push_back("Legend");
    legend.layer_colors.push_back(header_color); 
    
    //set default graphic layer states
    set_default_layers();
    
    //initialize for direction drawing
    path_set = false;
    
    //initialize window dimension reference
    get_report_structure(&res);
    
    // Pass control to the window handling routine.  It will watch for user input
    // and redraw the screen / pan / zoom / etc. the graphics in response to user
    // input or windows being moved around the screen.  We have to pass in 
    // at least one callback -- the one to redraw the graphics.
    // Three other callbacks can be provided to handle mouse button presses,
    // mouse movement and keyboard button presses in the graphics area, 
    // respectively. Those 3 callbacks are optional, so we can pass NULL if
    // we don't need to take any action on those events, and we do that
    // below.
    // This function will return if and when
    // the user presses the proceed button.
    set_keypress_input (true);
    set_mouse_move_input (true);
    event_loop(act_on_button_press, act_on_mouse_move, act_on_key_press, drawscreen);   

    
    /******************* animation section *******************************/
    // User might have panned and zoomed, which changes what world 
    // coordinates are visible on the screen. Get back to a known view.
    set_visible_world(initial_coords);
    clearscreen();

    close_graphics ();
    std::cout << "Map Closed.\n";
}

// drawing out the map
void drawscreen (void)
{
    get_report_structure(&res);
    //cout << res.top_width << " by " << res.top_height << endl;
    
    double scale = get_scale ();

    clearscreen ();

    //colouring of the map
    if (scale <= 0.005714285)    // arbitrary number for making the color change smoothly
    {
        setcolor (land_color.red + 50, land_color.green + 50, land_color.blue + 20);
    }
    else if(getZoomLevel() >= 3 || force_draw_streets) //draws the green forests and lakes
    {
        setcolor (land_color.red + 0.3/scale, land_color.red + 0.2/scale, land_color.red + 0.09/scale);
    }
    else if(layerSys.getLayermenuActive())
    {
        setcolor(layerland_color);
        //setcolor(230,230,230);
    }
    else //shows the green lands when the map is zoomed to fit its size
    {
        //setcolor(land_color);
        //setcolor(230,230,230);
        setcolor(130,168,128);
        //setcolor(120,164,116);
    } 

    //draws the rectangle for the map and the size is set accordingly to the 
    //LON_TO_COOR_RATIO
    fillrect (map_bounds.minlon*LON_TO_COOR_RATIO, map_bounds.minlat*LON_TO_COOR_RATIO, 
            map_bounds.maxlon*LON_TO_COOR_RATIO, map_bounds.maxlat*LON_TO_COOR_RATIO);

    
      
    
    //draw layers
    if(true)
    {
        if(natural_layer["n_other"].active)
            layer_nother();       
        if(natural_layer["water"].active)
            layer_water();
        if(natural_layer["wood"].active)
            layer_wood();
        if(natural_layer["beach"].active)
            layer_beach();
        if(natural_layer["land"].active)
            layer_land();
        if(natural_layer["tree"].active)
            layer_tree();
        if(natural_layer["cliff"].active)
            layer_cliff();
        if(natural_layer["peak"].active)
            layer_peak();
        if(natural_layer["wetland"].active)
            layer_wetland();
        if(natural_layer["scrub"].active)
            layer_scrub();
        if(natural_layer["heath"].active)
            layer_heath();
        if(natural_layer["sand"].active)
            layer_sand();
        if(natural_layer["tree_row"].active)
            layer_treerow();
        if(natural_layer["grassland"].active)
            layer_grassland();  
        if(waterway_layer["river"].active)
        layer_river();
        if(waterway_layer["stream"].active)
            layer_stream();
        if(waterway_layer["drain"].active)
            layer_drain();
        if(waterway_layer["riverbank"].active)
            layer_riverbank();
        if(waterway_layer["dam"].active)
            layer_dam();
        if(waterway_layer["breakwater"].active)
            layer_breakwater();
        if(waterway_layer["rapids"].active)
            layer_rapids();
        if(waterway_layer["boatyard"].active)
            layer_boatyard();
        if(waterway_layer["ditch"].active)
            layer_ditch();
        
    }
    
    //draws all the natural features
    draw_all_natural_features (scale);
    
    
    //draws all the streets and street name when zoomed in to certain level
    if(getZoomLevel() >= 0 || force_draw_streets)
    {
        draw_all_streets (scale);
        draw_street_names(scale);
    }  
    
    //draws the input terminal for the user
    draw_path (path_data, scale);
    
    //draws all the arbitrary pinned points and the pinned points of the search results
    draw_all_pinend_points (scale);
    display_search_results (scale);
    
    //enable button
    if(force_draw_input_counter!=0)
    {
        if(enable_input)
        {
        draw_user_input(scale,textbox_text);
            if(enable_suggested_bar==true && !path_enabled)
            {
                draw_suggested_poi(scale, textbox_text);
                draw_suggested_intersections(scale, textbox_text);
            }
            if(userinputSys.getEnable() && !path_enabled && enable_suggested_bar==true)
            {
                draw_suggested_intersection_texts(saved_inter);
                draw_suggested_poi_texts(saved_poi);
            }
        draw_text (textbox_text);
    }

        if(path_enabled)
        {
            if(enable_suggested_bar==true && enable_input)
            {
                draw_user_input(scale, textbox_text);
                draw_suggested_poi(scale, textbox_text);
                draw_suggested_intersections(scale, textbox_text);
                draw_suggested_poi_texts(start_location);
                draw_suggested_intersection_texts(saved_inter);
            }
        }
    }
    //#ifdef DEBUG_GRAPHIC
    
    //draws courier path
    if(menu.cour_path.size() > 0 || menu.active_button == 1)
    {
        if(menu.cour_state == 3)
                draw_path(menu.cour_path, get_scale());
        if(menu.cour_depots.size() > 0)
            draw_positions(menu.cour_depots, depot_color);
        if(menu.cour_intrs.size() > 0)
            draw_positions(menu.cour_intrs, intr_color);
    }
    
    if(menu.path_path.size() > 0 || menu.active_button == 2)
    {
        if(menu.path_state == 1)
            draw_path(path_data, get_scale());
        if(menu.path_waypoints.size() > 0)
            draw_positions(menu.path_waypoints, pin_color);
    } 
    
    
    //draws the minimap
    draw_minimap(scale);
    
    //draw legend
    if(legend_state)
        draw_layermenu(scale);
    
    //draw directions
    if(path_set)
    {
        //draw_directions();
        if(dir.hide) draw_directions_collapsed(dir.div_offset, dir.header);
        else draw_directions();
        
    }
    
    if(menu.main_active)
    {
        draw_main_menu();
        if(menu.active_button == 0) draw_about_box();
        else if(menu.active_button == 1)
        {
            draw_cour_box();
            if(menu.cour_path.size() > 0) return;
            
            if(menu.cour_state == 3)
                draw_path(menu.cour_path, get_scale());
            if(menu.cour_depots.size() > 0)
                draw_positions(menu.cour_depots, depot_color);
            if(menu.cour_intrs.size() > 0)
                draw_positions(menu.cour_intrs, intr_color);           
        }
        else if(menu.active_button == 2)
        {
            draw_path_box();
            if(menu.path_path.size() > 0) return;
            if(menu.path_state == 1)
                draw_path(path_data, get_scale());
            if(menu.path_waypoints.size() > 0)
             draw_positions(menu.path_waypoints, pin_color);
        }
    }
    
    
    
}

void delay (long milliseconds) 
{
    // if you would like to use this function in your
    // own code you will need to #include <chrono> and
    // <thread>
    std::chrono::milliseconds duration (milliseconds);
    std::this_thread::sleep_for (duration);
}

//function that gets called when search button is pressed
//handlers for when side button is pressed

//button function for turning input terminal on and off
void act_on_enable_input(void (*drawscreen_ptr) (void))
{
    if(true/*zoomlvl.level!=0*/)
    {
        if(force_draw_input_counter==0)
        {
            enable_input = true;
            force_draw_input_counter++;
        }else
        {
            textbox_text="";
            enable_input = false;
            force_draw_input_counter=0;
        }
    } 
    get_report_structure(&res);
    drawscreen_ptr ();
}

void act_on_search_func (void (*drawscreen_ptr) (void)) 
{ 
    std::string search_location;
    std::getline (std::cin, search_location);
    std::cout << search_location << std::endl;
    save_search_results (search_location);
    get_report_structure(&res);
    drawscreen_ptr ();
}

//function that gets called when clear button is pressed
void act_on_clear_func (void (*drawscreen_ptr) (void))
{
    force_draw_streets=false;
    force_draw_street_counter=0;
    search_results.clear ();
    pinned_points.clear ();
    selectzoom_points.clear();
    path_set = false;
    dir.spec = false;
    dir.more = false;
    dir.scroll_count = 0;
    dir.dir.clear();
    path_data.clear();
    textbox_text="";
    path_enabled=false;
    enable_suggested_bar=false;
    start_to_destination=0;
    arrow_key_pressed=0;
    saved_inter="";
    saved_poi="";
    get_report_structure(&res);
    drawscreen_ptr ();
    reset_states();
}

//function that gets called when zoom fit button is pressed
void act_on_zoomfit_func(void (*drawscreen_ptr) (void))
{
    set_visible_world(initial_coords);
    zoomlvl.level = 0;
    force_draw_streets = false;
    if(force_draw_street_counter != 0)
        force_draw_street_counter--;
    textbox_text="";
    get_report_structure(&res);
    drawscreen_ptr ();
}

//function that gets called when street view button is pressed
void act_on_streetview_func(void (*drawscreen_ptr) (void))
{
    if(force_draw_street_counter==0)
    {
        force_draw_streets = true;
        force_draw_street_counter++;
    } else
    {
        force_draw_streets = false;
        force_draw_street_counter=0;
    }
    drawscreen_ptr ();
}


//Button1 = leftclick
//Button2 = scrollwheel press
//Button3 = rightclick
//Button4 = scroll forward
//Button5 = scroll backwards
void act_on_button_press (float x, float y, t_event_buttonPressed event) 
{
     get_report_structure(&res);   
    //left click
    if(event.button == 1)
    {
        click_local= t_point(mouse_location_current.x, mouse_location_current.y);
        //mouse click is on minimap
        if(mouse_location_current.x > minimap_bounds.left() && 
            mouse_location_current.x < minimap_bounds.right() &&
            mouse_location_current.y < minimap_bounds.top() && 
            mouse_location_current.y > minimap_bounds.bottom() &&
                getZoomLevel() > 0)
        {
            set_minimap_spec();
            drawscreen();
        } 
        else if(mouse_location_current.x > textbox_bounds.left() && 
            mouse_location_current.x < textbox_bounds.right() &&
            mouse_location_current.y < textbox_bounds.top() && 
            mouse_location_current.y > textbox_bounds.bottom() && 
                force_draw_input_counter > 0)
        {
            enable_input = true;
            return;
        }
        //mouse click on legend if on
        else if(legend_state && mouse_location_current.x > layermenu_bounds.left() && 
            mouse_location_current.x < layermenu_bounds.right() &&
            mouse_location_current.y < layermenu_bounds.top() && 
            mouse_location_current.y > layermenu_bounds.bottom())
        {
            legend_click_handler();
        }
        else if(path_set && mouse_location_current.x > dir_bounds.left() && 
            mouse_location_current.x < dir_bounds.right() &&
            mouse_location_current.y < dir_bounds.top() && 
            mouse_location_current.y > dir_bounds.bottom() && !dir.hide)
        {
            t_bound_box check(dir_bounds.left(), dir_bounds.bottom(),
                    dir_bounds.right() - dir.up_bounds.left(), dir.header.bottom());
            if(mouse_location_current.y > dir_bounds.top() - dir.header_offset)
            {
                if(dir.hide)
                {
                    dir.hide = false;
                }
                else dir.hide = true;
                drawscreen();
                //draw_directions_collapsed(dir.div_offset,dir.header);
            }         
            else if(!dir.scroll_bounds.intersects(mouse_location_current))
            {
                if(dir.up_bounds.intersects(mouse_location_current) ||
                        dir.down_bounds.intersects(mouse_location_current))
                {
                    direction_click_handler();
                }
                else
                {
                    if(dir.spec && dir.spec_bounds.intersects(mouse_location_current))
                    {
                        dir.spec = false;
                        draw_path(path_data, get_scale());
                        if(menu.cour_path.size() > 0)
                        {
                            draw_positions(menu.cour_depots, depot_color);
                            draw_positions(menu.cour_intrs, intr_color);
                        }
                        else if(menu.path_path.size() > 0)
                        {
                            draw_positions(menu.path_waypoints, pin_color);
                        }                    
                        else draw_all_pinend_points (get_scale());
                        //drawscreen();
                        draw_directions();
                        draw_minimap(get_scale());
                        return;
                    }
                    
                    int pos = 0;
                
                    for(int i = 0; i < dir.num_div; ++i)
                    {
                        t_bound_box check1(dir_bounds.left(), dir.header.bottom() - (i+1)*dir.div_offset,
                                dir_bounds.right() - dir.up_bounds.left(), dir.header.bottom() - (i)*dir.div_offset);
                        if(dir.spec)
                        {
                            if(i < posi && check1.intersects(mouse_location_current))
                            {
                                pos = i;
                                posi = i;
                                break;
                            }
                            //else if(i == posi || i+1 == posi);
                            else if(i >= pos + 2 && check1.intersects(mouse_location_current))
                            {
                                pos = i - 1;
                                posi = i - 1;
                            }
                        }
                        else if(check1.intersects(mouse_location_current))
                        {
                            pos = i;
                            posi = i;
                            break;
                        }
                    }
                    
                    if(pos + 1 > (int)dir.dir_parsed.size()) return;
                    
                    if(pos != dir.num_div - 1)
                    {
                        dir.spec_bounds = t_bound_box(dir_bounds.left(), dir.header.bottom() - (pos+2)*dir.div_offset,
                        dir_bounds.right(), dir.header.bottom() - pos*dir.div_offset);
                    }
                    else
                    {
                        dir.spec_bounds = t_bound_box(dir_bounds.left(), dir_bounds.bottom(),
                            dir_bounds.right(), dir_bounds.bottom() + 2*dir.div_offset);
                    }
                    
                    dir.spec = true;

                    draw_directions_exspec_parse(pos);
                    draw_minimap(get_scale());
                }
                // do something
            }
           
        }
        else if(dir.hide && path_set && mouse_location_current.x > dir_bounds.left() && 
            mouse_location_current.x < dir_bounds.right() &&
            mouse_location_current.y < dir_bounds.top() && 
            mouse_location_current.y > dir_bounds.top() - dir.header_offset)
        {
            if(dir.hide)
            {
                dir.hide = false;
            }
            else dir.hide = true;
            
            if(legend_state)
            {
                legendmenu_destroy();
                legend_state = false;
                legendmenu_create();
        
            }
            draw_directions();
        }
        else if(menu.main_active && ((menu.about_bBounds.intersects(mouse_location_current) ||
                menu.cour_bBounds.intersects(mouse_location_current)) ||
                menu.path_bBounds.intersects(mouse_location_current)))
        {                     
            if(menu.about_bBounds.intersects(mouse_location_current))
            {
                if(menu.active_button == 0)
                {
                    if(menu.cour_depots.size() > 0)
                        menu.active_button = 1;
                    else if(menu.path_waypoints.size() > 0 )
                        menu.active_button = 2;
                    else
                    {
                        reset_states();
                    }
                       
                }
                else
                {
                    menu.c_about_msg = false;
                    menu.r_about_msg = false;
                    menu.s_about_msg = false;
                    menu.active_button = 0;
                }
            }
            else if(menu.cour_bBounds.intersects(mouse_location_current))
            {
                if(menu.active_button == 1)
                {
                    menu.active_button = -1;
                    reset_states();
                }
                else
                {
                    reset_states();
                    menu.active_button = 1;
                }
            }
            else if(menu.path_bBounds.intersects(mouse_location_current))
            {
                if(menu.active_button == 2)
                {
                    menu.active_button = -1;
                    reset_states();
                }
                else
                {
                    reset_states();
                    menu.active_button = 2;
                    
                }
            }
           
            draw_main_menu();
            main_click_handler();         
        }
        else if(menu.active_button != -1 && menu.main_active)
        {
            if(menu.active_button == 0)
            {
                if(menu.about_exit.intersects(mouse_location_current))
                {
                    if(menu.cour_depots.size() > 0)
                        menu.active_button = 1;
                    else if(menu.path_waypoints.size() > 0 )
                        menu.active_button = 2;
                    else
                    {
                        reset_states();
                    }
                }
                else if(menu.c_circ.intersects(mouse_location_current))
                {
                    if(menu.c_about_msg) menu.c_about_msg = false;
                    else 
                    {
                        menu.c_about_msg = true;
                        menu.r_about_msg = false;
                        menu.s_about_msg = false;
                    }
                    draw_about_box();
                }
                else if(menu.r_circ.intersects(mouse_location_current))
                {
                    if(menu.r_about_msg) menu.r_about_msg = false;
                    else
                    {
                        menu.r_about_msg = true;
                        menu.c_about_msg = false;
                        menu.s_about_msg = false;
                    }
                    draw_about_box();
                }
                else if(menu.s_circ.intersects(mouse_location_current))
                {
                    if(menu.s_about_msg) menu.s_about_msg = false;
                    else
                    {
                        menu.s_about_msg = true;
                        menu.c_about_msg = false;
                        menu.r_about_msg = false;
                    }
                    draw_about_box();
                }
            }
            else if(menu.active_button == 1 )
            {
                if(menu.cour_state == 0)
                {
                    if(menu.cour_done.intersects(mouse_location_current))
                    {
                        reset_states();
                    }
                    else if(menu.cour_next.intersects(mouse_location_current))
                    {
                        if(menu.cour_depots.size() < 1) return;
                        else
                        {
                            menu.cour_state = 1;
                            draw_cour_box();
                            return;
                        }
                    }
                    else
                    {
                        if(menu.cour_box.intersects(mouse_location_current)) return;
                        LatLon mouse_location_currentl(mouse_location_current.y/LAT_TO_COOR_RATIO, 
                                mouse_location_current.x/LAT_TO_COOR_RATIO);
                        unsigned closest_intr = find_closest_intersection(mouse_location_currentl);
                        menu.cour_depots.push_back(closest_intr);
                        draw_positions(menu.cour_depots, depot_color);
                        return;
                    }
                }
                else if(menu.cour_state == 1)
                {
                     if(menu.cour_done.intersects(mouse_location_current))
                    {
                         reset_states();
                    }
                    else if(menu.cour_next.intersects(mouse_location_current))
                    {
                        if(menu.cour_intrs.size() < 1) return;
                        else
                        {
                            path_data.clear();
                            path_intersection.clear();
                            menu.cour_state = 2;
                            menu.cour_path = traveling_salesman(menu.cour_intrs, menu.cour_depots);                        
                            //draw_path(menu.cour_path, get_scale());
                            menu.cour_first = 1;
                            menu.cour_last = menu.cour_intrs.size() + 2;
                            menu.cour_state = 3;
                            //draw_cour_box();
                            //draw_positions(menu.cour_intrs, intr_color);
                            //draw_positions(menu.cour_depots, depot_color);
                            deter_cour_intrs();                           
                            {
                                path_data = menu.cour_path;
                                path_intersection = menu.cour_path_intrs;
                                path_set = true;
                                if(legend_state)
                                {
                                    legendmenu_destroy();
                                    legend_state = false;
                                    legendmenu_create();

                                }
                                directions = give_user_direction(menu.cour_path, menu.cour_path_intrs);
                                draw_directions_load();                               
                                //draw_directions(); 
                            }
                            drawscreen();
                            //draw_minimap(get_scale());
                            return;
                        }
                    }
                    else
                    {
                        if(menu.cour_box.intersects(mouse_location_current)) return;
                        LatLon mouse_location_currentl(mouse_location_current.y/LAT_TO_COOR_RATIO, 
                                mouse_location_current.x/LAT_TO_COOR_RATIO);
                        unsigned closest_intr = find_closest_intersection(mouse_location_currentl);
                        menu.cour_intrs.push_back(closest_intr);
                        draw_positions(menu.cour_intrs, intr_color);
                        return;
                    }
                }
                else if(menu.cour_state == 2)
                {
                    draw_cour_box();
                }
                else if(menu.cour_state == 3)
                {
                    if(menu.cour_done.intersects(mouse_location_current))
                    {
                        reset_states();
                    }
                    else if(menu.cour_next.intersects(mouse_location_current))
                    {
                        path_data.clear();
                        path_intersection.clear();
                        path_set = false;
                        dir.spec = false;
                        dir.more = false;
                        dir.dir.clear();
                        dir.dir_parsed.clear();
                        dir.index.clear();
                        dir.scroll_count = 0;
                        menu.cour_state = 0;
                        menu.cour_depots.clear();
                        menu.cour_intrs.clear();
                        menu.cour_path.clear();
                        menu.cour_path_intrs.clear();
                    }
                    else return;
                }
            }
            else if(menu.active_button == 2)
            {
                if(menu.path_state == 0)
                {
                    if(menu.path_done.intersects(mouse_location_current))
                    {
                        reset_states();
                    }
                    else if(menu.path_next.intersects(mouse_location_current))
                    {
                        if(menu.path_waypoints.size() < 1) return;
                        else
                        {
                            
                            menu.path_state = 1;
                            menu.path_path = find_path_between_intersections(
                                    menu.path_waypoints[0], menu.path_waypoints[1]);                    
                            //draw_path(menu.path_path, get_scale());
                            menu.path_state = 1;
                            //draw_path_box();
                            //draw_positions(menu.path_waypoints, pin_color);     
                            deter_path_intrs();
                            {
                                path_data = menu.path_path;
                                path_set = true;
                                if(legend_state)
                                {
                                    legendmenu_destroy();
                                    legend_state = false;
                                    legendmenu_create();

                                }
                                directions = give_user_direction(menu.path_path, menu.path_path_intrs);
                                draw_directions_load();                               
                                //draw_directions(); 
                            }
                            drawscreen();
                            //draw_minimap(get_scale());
                            return;
                        }
                    }
                    else
                    {
                        if(menu.path_box.intersects(mouse_location_current)) return;
                        if(menu.path_waypoints.size() + 1 > 2)
                        {
                            menu.path_waypoints.erase(menu.path_waypoints.begin());
                            LatLon mouse_location_currentl(mouse_location_current.y/LAT_TO_COOR_RATIO, 
                                mouse_location_current.x/LAT_TO_COOR_RATIO);
                            unsigned closest_intr = find_closest_intersection(mouse_location_currentl);
                            menu.path_waypoints.push_back(closest_intr);
                            draw_positions(menu.path_waypoints, path_color);
                            drawscreen();
                            return;
                        }
                        
                        LatLon mouse_location_currentl(mouse_location_current.y/LAT_TO_COOR_RATIO, 
                                mouse_location_current.x/LAT_TO_COOR_RATIO);
                        unsigned closest_intr = find_closest_intersection(mouse_location_currentl);
                        menu.path_waypoints.push_back(closest_intr);
                        draw_positions(menu.path_waypoints, path_color);
                        return;
                    }
                }
                else if(menu.path_state == 1)
                {
                    if(menu.path_done.intersects(mouse_location_current))
                    {
                        reset_states();
                    }
                    else if(menu.path_next.intersects(mouse_location_current))
                    {
                        path_data.clear();
                        path_intersection.clear();
                        path_set = false;
                        dir.spec = false;
                        dir.more = false;
                        dir.dir.clear();
                        dir.dir_parsed.clear();
                        dir.scroll_count = 0;
                        dir.index.clear();
                        menu.path_state = 0;
                        menu.path_waypoints.clear();
                        menu.path_path.clear();
                        menu.path_path_intrs.clear();
                        //draw_path_box();
                        //drawscreen();
                    }
                    else return;
                }
            }
            draw_main_menu();
            main_click_handler(); 
            
        }
        //for user input POI
        else 
        {       
            if(enable_input)
            {
                enable_input = false;
            }
            else if(mouse_location_current.x <= map_bounds.maxlon*LAT_TO_COOR_RATIO &&
                    mouse_location_current.x >= map_bounds.minlon*LAT_TO_COOR_RATIO &&
                    mouse_location_current.y <= map_bounds.maxlat*LAT_TO_COOR_RATIO &&
                    mouse_location_current.y >= map_bounds.minlat*LAT_TO_COOR_RATIO)
            {
                LatLon mouse_location_currentl(mouse_location_current.y/LAT_TO_COOR_RATIO, mouse_location_current.x/LAT_TO_COOR_RATIO);
                unsigned closest_intr = find_closest_intersection(mouse_location_currentl);
                LatLon c_intr = getIntersectionPosition(closest_intr);
                t_point c_intrp = get_coordinate_from_LatLon(c_intr);
                
                if(pinned_points.size() < 2)
                {
                    pinned_points.push_back(c_intrp);
                }
                else
                {
                    path_set = false;
                    dir.spec = false;
                    dir.more = false;
                    dir.scroll_count = 0;
                    dir.dir.clear();
                    dir.hide = false;
                    //pinned_points.erase(pinned_points.begin());
                    pinned_points.push_back(c_intrp);
                    path_data.erase(path_data.begin(), path_data.end());
                }
                //pinned_points.push_back (mouse_location_current);
                draw_all_pinend_points(get_scale());
            }
            
        }
    }
    //ctrl+rightclick
    else if(event.button == 3 && event.ctrl_pressed)
    {
        //first instance
        if(selectzoom_points.size() == 0)
        {
            drawscreen();
            selectzoom_points.push_back(mouse_location_current);
        }
        //second instance
        else
        {
            selectzoom_points.push_back(mouse_location_current);
        }
        if(selectzoom_points.size() == 2)
        { 
            //doesn't actually draw the rectangle, may be needed later
            //setcolor(0,0,0);
            //setlinewidth(4);
            
            //coordinates for set_visible_world
            t_point bl; //bottom left
            t_point ur; //top right
            
            //determine bottom left and top right coordinates
            if(selectzoom_points[0]. x < selectzoom_points[1].x)
            {
                bl.x = selectzoom_points[0].x;
                ur.x = selectzoom_points[1].x;
            }
            else
            {
                bl.x = selectzoom_points[1].x;
                ur.x = selectzoom_points[0].x;
            }
            if(selectzoom_points[0].y < selectzoom_points[1].y)
            {
                bl.y = selectzoom_points[0].y;
                ur.y = selectzoom_points[1].y;
            }
            else
            {
                bl.y = selectzoom_points[1].y;
                ur.y = selectzoom_points[0].y;
            }
            //drawrect(bl, ur);
            
            //initialize viewable world
            setcolor(BLACK);
            setlinewidth(4);
            drawrect(bl.x, bl.y,ur.x,ur.y);
            //set_visible_world(bl.x, bl.y, ur.x, ur.y);
            
            //clear zoompoints storage to allow for more instances
            selectzoom_points.clear();
            
            //drawscreen();
        }
            
    }
}

// update the variables for mouse behaviors
// update the current latitude and longitude cursor is pointing at
void act_on_mouse_move (float x, float y) {
    // function to handle mouse move event, the current mouse position in the current world coordinate
    // system (as defined in your call to init_world) is returned
    get_report_structure(&res);
    mouse_location_before = mouse_location_current;
    mouse_location_current = t_point(x, y);
    
    if((int)legend.active_layers.size() > legend.num_div && legend_state
            && layermenu_bounds.intersects(mouse_location_current))
     {
        if(legend.up_bounds.intersects(mouse_location_current))
        {
            legend.up_backdrop = true;
        }
        else if(legend.down_bounds.intersects(mouse_location_current))
        {
            legend.down_backdrop = true;
        }
        else if(legend.bar_bounds.intersects(mouse_location_current))
        {
            legend.up_backdrop = true;
            legend.down_backdrop = true;
        }
        else if(!left_hold)
        {
            legend.up_backdrop = false;
            legend.down_backdrop = false;
        }
        draw_legend_sbar();
    }
    
    if((int)dir.dir.size() - 2 > dir.num_div + 1 && path_set && !dir.hide &&
            dir_bounds.intersects(mouse_location_current))
     {
        if(dir.up_bounds.intersects(mouse_location_current))
        {
            dir.up_backdrop = true;
        }
        else if(dir.down_bounds.intersects(mouse_location_current))
        {
            dir.down_backdrop = true;
        }
        else if(dir.bar_bounds.intersects(mouse_location_current))
        {
            dir.up_backdrop = true;
            dir.down_backdrop = true;
        }
        else if(!left_hold)
        {
            dir.up_backdrop = false;
            dir.down_backdrop = false;
        }
        draw_directions_sbar(); 
    }
    
    if(path_set && left_hold && !dir.hide && (int)dir.dir.size() - 2 > legend.num_div)
    {
        //prev_state is used to confirm that the left mouse button is being held down
        //without it the mouse would need to remain in the scrollbar bounds
        //for drag scrolling to work, however we want it so that the user just needs
        //to click on the scrollbar and from there if the left mouse button
        //remains held they can drag it up and down, regardless of mouse location
        if(dir.scroll_bounds.intersects(mouse_location_current) || prev_state)
        {
            if(!prev_state) prev_state = true;
            direction_click_handler();
        }
    }
    
    if(legend_state && left_hold && (int)legend.active_layers.size() > legend.num_div)
    {
        if(legend.scroll_bounds.intersects(mouse_location_current) || prev_state)
        {
            if(!prev_state) prev_state = true;
            legend_click_handler();
        }
        
    }
    
    if(menu.main_active && menu.about_bBounds.intersects(mouse_location_current))
    {
        menu.button_spec = 0;
        draw_main_menu();
    }
    else if(menu.main_active && menu.cour_bBounds.intersects(mouse_location_current))
    {
        menu.button_spec = 1;
        draw_main_menu();
    }
    else if(menu.main_active && menu.path_bBounds.intersects(mouse_location_current))
    {
        menu.button_spec = 2;
        draw_main_menu();
    }
    else  if(menu.main_active)
    {  
        if(menu.button_spec != -1)
        {
            menu.button_spec = -1;
            draw_main_menu();
        }
    }
    
    if(menu.main_active && menu.active_button == 1 &&
            menu.cour_box.intersects(mouse_location_current))
    {
        if(menu.cour_next.intersects(mouse_location_current))
        {
            menu.cour_onNext = true;
            menu.cour_onDone = false;
        }
        else if(menu.cour_done.intersects(mouse_location_current))
        {
            menu.cour_onDone = true;
            menu.cour_onNext = false;
        }
        else
        {
            menu.cour_onNext = false;
            menu.cour_onDone = false;
        }
        draw_cour_box();
       
    }
    
    if(menu.main_active && menu.active_button == 1 &&
            !menu.cour_box.intersects(mouse_location_current))
    {
        if(menu.cour_onNext || menu.cour_onDone)
        {
            menu.cour_onNext = false;
            menu.cour_onDone = false;
            draw_cour_box();
        }
    }
    
    if(menu.main_active && menu.active_button == 2 &&
            menu.path_box.intersects(mouse_location_current))
    {
        if(menu.path_next.intersects(mouse_location_current))
        {
            menu.path_onNext = true;
            menu.path_onDone = false;    
        }
        else if(menu.path_done.intersects(mouse_location_current))
        {
            menu.path_onDone = true;
            menu.path_onNext = false;
        }
        else
        {
            menu.path_onNext = false;
            menu.path_onDone = false;
        }
        draw_path_box();
        
    }
    
    if(menu.main_active && menu.active_button == 2 &&
            !menu.path_box.intersects(mouse_location_current))
    {
        if(menu.path_onNext || menu.path_onDone)
        {
            menu.path_onNext = false;
            menu.path_onDone = false;
            draw_path_box();
        }
    }
    /*time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y/%m/%d - %X",&tstruct);
    // std::cout << x << ", " << y << std::endl;*/
    
    std::string message = "";
    std::ostringstream strs;
    
    
    if(menu.main_active && menu.about_bBounds.intersects(mouse_location_current))
    {
        strs << "About" << endl;
    }
    else if(menu.main_active && menu.cour_bBounds.intersects(mouse_location_current))
    {
        strs << "Multi Path for Courier Planning" << endl;
    }
    else if(menu.main_active && menu.path_bBounds.intersects(mouse_location_current))
    {
        strs << "Single Path Planning" << endl;
    }
    else
    {
        double lat = y / LAT_TO_COOR_RATIO;
        double lon = x / LON_TO_COOR_RATIO;
    
        //create name string using constructor, uses parsed vector of char city_name
        std::string map_name = std::string(city_name.begin(), city_name.end());
    
        strs << "Map of " << map_name << " - Lat: " << lat << ", " << "Lon: " << lon;
        strs << " Zoom Level: " << getZoomLevel();
    }
    
    
    
    message = strs.str();
    
    update_message(message);
}


void act_on_key_press (char c) 
{
    get_report_structure(&res);
    // function to handle keyboard press event, the ASCII character is returned
    if(c == 27)
    {
        if(menu.main_active)
        {
            //menu.main_active = false;
            //menu.c_about_msg = false;
            //menu.r_about_msg = false;
            //menu.s_about_msg = false;
            menu.active_button = -1;
            menu.main_active = false;
            reset_states();
            //menu.cour_state = 0;
            //menu.cour_path.clear();
            //menu.cour_intrs.clear();
            //menu.cour_depots.clear();
            drawscreen();
            return;
        }
        else
        {
            menu.main_active = true;
            draw_main_menu();
            return;
        }
    }
    //else if(true)
    //{
    //   return;
    //}
    else if(enable_input&&!path_enabled)
    {
        double scale = get_scale ();
        if(c==0)
        {
            return;
        }
        if(c=='\b') //backspace key erases the last character that was typed
        {
            if(textbox_text!="")
        {
                textbox_text.erase(textbox_text.end()-1);
            }
        } 
        else if(c==13) //when enter is pressed different functions will be called
        {
            //the first character of each word is converted into capital if the 
            //words are consisted of all lower cases
            if(textbox_text=="")
            {
                return;
            }
            command_lines = parse_input(textbox_text);
            std::string input_text="";
            for(int i=0;i< (int) command_lines.size();i++)
            {
                if(command_lines[i].at(0)>=97)
                {
                    char temporary = command_lines[i][0];
                    int numofchar=temporary;
                    numofchar -= 32;
                    char changedchar = (char)numofchar;
                    command_lines[i][0] = changedchar;
                }
                input_text += command_lines[i];
                input_text += " ";
            }
            input_text.erase(input_text.end()-1);
            cout << input_text << endl;
            if(input_text=="Path") //path function
            {
                //testPath(drawscreen);
                saved_poi="";
                saved_inter="";
                path_enabled=true;
            } 
            else if(input_text=="Clear")  //clear function
            {
                act_on_clear_func(drawscreen);
            } 
            else if(input_text=="Street View")  //street view function
            {
                act_on_streetview_func(drawscreen);
            } 
            else if(input_text=="Zoom Fit")   //zoom fit function
            {
                act_on_zoomfit_func(drawscreen);
            } 
            else if(arrow_key_pressed!=0)
            {
                if(arrow_key_pressed==1)
                {
                    //path_enabled=true;
                   // cout << "okay" << endl;
                    input_text="";
                    //cout << input_text << endl;
                    //cout << saved_poi << endl;
                    for(int i=0;i< (int) saved_poi.size();i++)
                    {
                        if(i==0||(saved_poi[i-1]==' ' && (saved_poi.at(i)>=65 && saved_poi.at(i)<=90)))
                        {
                            input_text+=saved_poi[i];
                            continue;
                        }else if(saved_poi.at(i)>=65 && saved_poi.at(i)<=90)
                        {
                            char temporary = saved_poi[i];
                            int numofchar=temporary;
                            numofchar += 32;
                            char changedchar = (char)numofchar;
                            saved_poi[i] = changedchar;
                        }
                        input_text += saved_poi[i];
                        //input_text += " ";
                    }
                    //input_text.erase(input_text.end()-1);
                    cout << input_text <<endl;
                    arrow_key_pressed=0;
                    save_search_results(input_text);
                    drawscreen();
                    //start_location=saved_poi;
                    //cout << "start " << start_location << endl;
                    //path_enabled=true;
                } else if(arrow_key_pressed==2)
                {
                    //path_enabled=true;
                    arrow_key_pressed=0;
                    /*if(!path_data.empty())
                    {
                        path_data.clear();
                    }*/
                    if(start_to_destination==1)
                    {
                        end_location=saved_inter;
                        saved_end_id=userinputSys.get_first_intersectionID();
                        LatLon ending;
                        ending = getIntersectionPosition(saved_end_id);
                        t_point ending2 = get_coordinate_from_LatLon(ending);
                        pinned_points.push_back(ending2);
                        if(path_activated==1)
                        {
                            path_data.clear();
                            path_activated=0;
                        }
                        inputPathFunction(saved_start_id, saved_end_id, drawscreen);
                        path_activated++;
                    }
                    if(start_to_destination==0)
                    {
                        start_location=saved_inter;
                        saved_start_id=userinputSys.get_first_intersectionID();
                        LatLon starting;
                        starting = getIntersectionPosition(saved_start_id);
                        t_point starting2 = get_coordinate_from_LatLon(starting);
                        pinned_points.push_back(starting2);
                    }
                    start_to_destination++;
                    //cout << "start " << start_location << endl;
                    //path_enabled=true;
                }
            } 
            else  //search function
            {
                save_search_results(input_text);
                drawscreen();
            }
            textbox_text="";
            draw_user_input(scale,textbox_text);
            draw_text (textbox_text);
        } 
        else if(c==127) //delete key erases the first character
        {
            if(textbox_text!="")
            {
                textbox_text.erase(textbox_text.begin());
            }
        } 
        else if(c==9)
        {
            if(arrow_key_pressed>2)
            {
                arrow_key_pressed=0;
            }
            arrow_key_pressed++;
            ///////////////////////////////////////////////////////////////////    
            if(force_draw_input_counter!=0)
            {
                if(enable_input)
                {
                draw_user_input(scale,textbox_text);
                    if(enable_suggested_bar==true && !path_enabled)
                    {
                        draw_suggested_poi(scale, textbox_text);
                        draw_suggested_intersections(scale, textbox_text);
                    }
                    if(userinputSys.getEnable() && !path_enabled && enable_suggested_bar==true)
                    {
                        draw_suggested_intersection_texts(saved_inter);
                        draw_suggested_poi_texts(saved_poi);
                    }
                draw_text (textbox_text);
            }

                if(path_enabled)
                {
                    if(enable_suggested_bar==true && enable_input)
                    {
                        draw_user_input(scale, textbox_text);
                        draw_suggested_poi(scale, textbox_text);
                        draw_suggested_intersections(scale, textbox_text);
                        draw_suggested_poi_texts(start_location);
                        draw_suggested_intersection_texts(saved_inter);
                    }
                }
            }
            /////////////////////////////////////////////////////////////////////////////
            //drawscreen();
        }
        /*else if(c == -54) //up
        {
            cout << "ascii is: " << (int)c << endl;
        }
        else if(c == -53) //down
        {
            cout << "ascii is: " << (int)c << endl;
        }
        else if(c == -56)//left
        {
            cout << "ascii is: " << (int)c << endl;
        }
        else if(c == -55)//right
        {
            cout << "ascii is: " << (int)c << endl;

        }*/
        else //adds the new character into the existing string
        {
            textbox_text += c;
            userinputSys.set_users_input(textbox_text);
            enable_suggested_bar=userinputSys.getEnable();
            if(enable_suggested_bar==true)
            {
                std::string inter = userinputSys.get_first_intersection();
                std::string poi = userinputSys.get_first_poi();
                if(inter!="" && poi!="")
                {
                    saved_poi = poi;
                    saved_inter = inter;
                    draw_suggested_poi_texts(saved_poi);
                    draw_suggested_intersection_texts(saved_inter);
                }
            ///////////////////////////////////////////////////////////////////    
                if(force_draw_input_counter!=0)
                {
                    if(enable_input)
                    {
                    draw_user_input(scale,textbox_text);
                        if(enable_suggested_bar==true && !path_enabled)
                        {
                            draw_suggested_poi(scale, textbox_text);
                            draw_suggested_intersections(scale, textbox_text);
                        }
                        if(userinputSys.getEnable() && !path_enabled && enable_suggested_bar==true)
                        {
                            draw_suggested_intersection_texts(saved_inter);
                            draw_suggested_poi_texts(saved_poi);
                        }
                    draw_text (textbox_text);
                }

                    if(path_enabled)
                    {
                        if(enable_suggested_bar==true && enable_input)
                        {
                            draw_user_input(scale, textbox_text);
                            draw_suggested_poi(scale, textbox_text);
                            draw_suggested_intersections(scale, textbox_text);
                            draw_suggested_poi_texts(start_location);
                            draw_suggested_intersection_texts(saved_inter);
                        }
                    }
                }
                /////////////////////////////////////////////////////////////////////////////
                //drawscreen();
            }
        }
        //draws the newly inputted string
        draw_user_input(scale,textbox_text);
        draw_text (textbox_text);
    }
    if(path_enabled)
    {
        //draw_suggested_poi(scale, textbox_text);
        //draw_suggested_intersections(scale, textbox_text);
        
        //if(start_to_destination==0)
        //{
        if(c==0)
        {
            return;
        }
        if(c==13)
        {
            saved_poi="";
            saved_inter="";

        }else if(c=='\b') //backspace key erases the last character that was typed
        {
            if(saved_inter!="")
            {
                saved_inter.erase(saved_inter.end()-1);
            } 
        }else if(c==127)
        {
            saved_inter="";
        }else if(c==27)
        {
            end_location=saved_inter;
        }else
        {
            saved_inter+=c;
            //draw_suggested_poi_texts(saved_poi);
        }
        //}else if(start_to_destination==1)
        //{
            /*if(c==0)
            {
                return;
            }
            if(c==13)
            {
                start_to_destination++;
            } else if(c=='\b') //backspace key erases the last character that was typed
            {
                if(saved_poi!="")
                {
                    saved_poi.erase(saved_poi.end()-1);
                } 
            }else
            {
                saved_poi+=c;
                //draw_suggested_intersection_texts(saved_inter);
            }*/
            
        // }else if(start_to_destination==2)
        // {
            
        //    path_enabled=false;
        //}
        drawscreen();
        
    }
    int a=c;
    //std::cout << "number " << a << std::endl;
    //std::cout << "Key press: " << c << std::endl;
}


    

// calculate where a set of latitude and longitude correspond to on the map
// this function consider the relative difference of the top-left corner and the input 
// and located it in the coordinate system
t_point get_coordinate_from_LatLon (LatLon geo_location)
{
    // calculate the coordinate value according to preset scaling factor
    double x_coor = geo_location.lon * LON_TO_COOR_RATIO;
    double y_coor = geo_location.lat * LAT_TO_COOR_RATIO;
    
    t_point point (x_coor, y_coor);
    
    return point;
}

// convert t_point coordinate back to LatLon 
LatLon get_LatLon_from_coordinate (t_point point)
{
    double lon = point.x / LON_TO_COOR_RATIO;
    double lat = point.y / LAT_TO_COOR_RATIO;
    
    LatLon location (lat,lon);
    return location;
}

// draw natural feature by name
void draw_all_natural_features (double scale)
{
    //water,natural, waterway, land
    //std::cout << r_zoom << std::endl;
    
    draw_all_water ();   
    draw_all_lands (scale);
    
    //draws after 1 zoom level
    /*if(getZoomLevel() >= 1 || force_draw_streets)
    {
        draw_all_natural();
        draw_all_waterways (scale);        
    }*/
    
    
}

//function that draws all types of water bodies
void draw_all_water ()
{
    setcolor (lake_color);
    
    int num_of_natural_features = getFeatureCount ();
    // loop through all natural features and draw polygon according to their points
    for (int i=0; i<num_of_natural_features; i++)
    {
        int num_of_points = getFeaturePointCount (i);
        
        t_point * point_ptr = NULL;
        point_ptr = new t_point [num_of_points];
        
        //this for loop stores all the coordinates of each natural feature to the 
        //dynamically created array called point_ptr 
        for (int j=0; j<num_of_points; j++)
        {
            LatLon point = getFeaturePoint (i, j);
            point_ptr[j] = get_coordinate_from_LatLon (point);
        }
        
        if (point_ptr == NULL)
        {
            continue;
        }
        //this checks if the natural feature is a type of water body, 
        //if yes, then draw the water body
        std::string water_type = getFeatureAttribute (i, "water");
        if (water_type == "lake" || water_type == "lagoon" || water_type == "pond")
        {
            fillpoly (point_ptr, num_of_points);
        }
        //delete the dynamically created array for no memory leaks
        delete [] point_ptr;
    }
}

//function that draws all the natural features exlcuding water bodies
void draw_all_natural ()
{
    int num_of_natural_features = getFeatureCount ();
    // loop through all natural features and draw polygon according to their points
    for (int i=0; i<num_of_natural_features; i++)
    {
        int num_of_points = getFeaturePointCount (i);
        
        t_point * point_ptr =  new t_point [num_of_points];
        
        //this for loop stores all the coordinates of each natural feature to the 
        //dynamically created array called point_ptr 
        for (int j=0; j<num_of_points; j++)
        {
            LatLon point = getFeaturePoint (i, j);
            point_ptr[j] = get_coordinate_from_LatLon (point);
        }
        
        if (point_ptr == NULL)
        {
            continue;
        }
        
        //this checks if the natural feature is a type of natural feature 
        //that is not a type of water body, 
        //if yes, then draw the natural feature
        std::string water_type = getFeatureAttribute (i, "natural");
        if (water_type == "sand")
        {
            setcolor (natural_sand_color);
            fillpoly (point_ptr, num_of_points);
        }
        else if (water_type == "water")
        {
            setcolor (natural_water_color);
            fillpoly (point_ptr, num_of_points);
        }
        else if (water_type == "wood")
        {
            setcolor (natural_wood_color);
            fillpoly (point_ptr, num_of_points);
        }
        else if (water_type == "tree_row")
        {
            setcolor (natural_tree_row_color);
            // fillpoly (point_ptr, num_of_points);
            // draw dash line
            setlinestyle (DASHED);
            drawline (point_ptr[0], point_ptr[1]);
        }
        else if (water_type == "grassland")
        {
            setcolor (natural_grassland_color);
            fillpoly (point_ptr, num_of_points);
        }
        else if (water_type == "beach")
        {
            setcolor (natural_beach_color);
            fillpoly (point_ptr, num_of_points);
        }
        else
        {
            continue;
        }
        
        setlinestyle (SOLID);
        //delete the dynamically created array for no memory leaks
        delete [] point_ptr;
    }
}

//function that draws all waterways
void draw_all_waterways (double scale)
{
    setlinestyle (SOLID);
    int num_of_natural_features = getFeatureCount ();
    // loop through all natural features and draw polygon according to their points
    for (int i=0; i<num_of_natural_features; i++)
    {
        int num_of_points = getFeaturePointCount (i);
        //t_point * point_ptr=new t_point[num_of_points];
        //this checks if the natural feature is a type of water way 
        //if yes, then sets the width length and the colour of the water way
        std::string water_type = getFeatureAttribute (i, "waterway");
        double line_width = 0;
        if (water_type == "river")
        {
            setcolor (waterway_river_color);
            line_width =default_water_way_width / scale;
        }
        else if (water_type == "stream")
        {
            setcolor (waterway_stream_color);
            line_width = (default_water_way_width / scale) * 2 /3;
        }
        else if (water_type == "boatyard")
        {
            setcolor (waterway_boatyard_color);
            line_width = (default_water_way_width / scale) / 3;
        }
        else
        {
            continue;
        }
        setlinewidth (line_width);
        
        double point_radius = line_width * scale * 100 / 2;
        t_point point1;
        t_point point2;
        //draws the water way accordingly to the points by drawing lines
        for (int j=0; j<num_of_points-1; j++)
        {
            LatLon way_point_1 = getFeaturePoint (i, j);
            LatLon way_point_2 = getFeaturePoint (i, j+1);
            point1 = get_coordinate_from_LatLon (way_point_1);
            point2 = get_coordinate_from_LatLon (way_point_2);
            
            drawline (point1, point2);
            fillarc (point1.x, point1.y, point_radius, 0, 360);
        }
        fillarc (point2.x, point2.y, point_radius, 0, 360);
    }
}

void draw_all_lands (double scale)
{
    int num_of_natural_features = getFeatureCount ();
    // loop through all natural features and draw polygon according to their points
    for (int i=0; i<num_of_natural_features; i++)
    {
        int num_of_points = getFeaturePointCount (i);
        
        t_point * point_ptr = NULL;
        point_ptr = new t_point [num_of_points];
        
        if (num_of_points == 0)
        {
            continue;
        }
        
        //this for loop stores all the coordinates of each land to the 
        //dynamically created array called point_ptr 
        for (int j=0; j<num_of_points; j++)
        {
            LatLon point = getFeaturePoint (i, j);
            point_ptr[j] = get_coordinate_from_LatLon (point);
        }
        
        //this checks if the natural feature is a type of land
        //if yes, then sets the colour of the land and draws it
        std::string water_type = getFeatureAttribute (i, "land");
        if (water_type == "island")
        {
            if (scale <= 0.005714285)    // arbitrary number for making the color change smoothly
            {
                setcolor (land_color.red + 70, land_color.blue + 60, land_color.blue + 50);
            }
            else if(getZoomLevel() >= 3 || force_draw_streets)
                setcolor (land_color.red + 0.3/scale, land_color.red + 0.2/scale, land_color.red + 0.09/scale);
            else if(layerSys.getLayermenuActive())
            {
                setcolor(layerland_color);
            }
            else
            {
                setcolor(default_land);
        
            }   
            fillpoly (point_ptr, num_of_points);
        }
        //if the land is a coastline, then the coastline was set to a default
        //width of water way and the coastline was drawn accordingly to its
        //coordinates, otherwise the land would have the colour of the land
        if(getFeatureAttribute(i, "natural")==  "coastline")
        {
            setcolor(coastline_color);
            setlinewidth (default_water_way_width / scale);
            for (int k=0; k<num_of_points-1; k++)
            {
                LatLon way_point_1 = getFeaturePoint (i, k);
                LatLon way_point_2 = getFeaturePoint (i, k+1);
                t_point point1 = get_coordinate_from_LatLon (way_point_1);
                t_point point2 = get_coordinate_from_LatLon (way_point_2);

                drawline (point1, point2);
            }
        }
        
        else 
            setcolor(land_color);

        setlinestyle(SOLID);
        //delete the dynamically created array for no memory leaks
        delete [] point_ptr;
    }
}

//function that finds the bounds of the map
void find_bound ()
{
    int num_of_intersections = getNumberOfIntersections ();
    double lat_north, lat_south, lon_east, lon_west;
    LatLon intersection0 = getIntersectionPosition (0);
    
    lat_north = intersection0.lat;
    lat_south = intersection0.lat;
    lon_east = intersection0.lon;
    lon_west = intersection0.lon;
    
    for (int i=1; i<num_of_intersections; i++)
    {
        LatLon intersection_i = getIntersectionPosition (i);
        if (intersection_i.lat > lat_north)
        {
            lat_north = intersection_i.lat;
        }
        
        if (intersection_i.lat < lat_south)
        {
            lat_south = intersection_i.lat;
        }
        
        if (intersection_i.lon > lon_east)
        {
            lon_east = intersection_i.lon;
        }
        
        if (intersection_i.lon < lon_west)
        {
            lon_west = intersection_i.lon;
        }
    }
    std::cout << std::setprecision(15) << lon_west << " , " << lat_south << " , " << lon_east << " , " << lat_north << std::endl;
}

// return the ratio of how much the map has been zoomed in
double get_scale ()
{
    t_bound_box current_world = get_visible_world ();
    double current_width = current_world.get_width ();
    double scale_factor = current_width / map_bounds.mapwidth;
    return scale_factor;
}

//calculate area ratio of current map to toronto map
double get_relative_zoom()
{
    double r_z = (map_bounds.mapheight * map_bounds.mapwidth)/TORONTO_A_STANDARD;

    //inverse if map is bigger than standard
    //if(r_z > 1) r_z = 1/r_z;
    
    //some arbitrary scaling formula
    if(r_z == 1)
    {
        return 1;
    }
    
    //temp fix
    //return r_z = 1;
    //std::cout << r_z << std::endl;
    
    return r_z;
}

//this function calculates the direction of the vector by using the formula:
//tangent theta = opposite/adjacent
double calculate_line_direction (LatLon point1, LatLon point2)
{
    double delta_lat = point1.lat - point2.lat;
    double delta_lon = point1.lon - point2.lon;
    // std::cout << delta_lat << std::endl;
    // std::cout << delta_lon << std::endl;
    
    // angle will range from 270 to 90
    double theta;
    double tan_theta = delta_lat / delta_lon;
    
    theta = atan (tan_theta) * 180 / PI;
    
    
    if (theta < 0)
    {
        theta += 360;
    }
    
    return theta;
}

//function that draws all the streets
void draw_all_streets (double scale)
{
    t_color edge_color (130,130,130);
    // drawing two layer of street to get the boundary plot of streets
    //draw_streets (1.2, scale, edge_color);
    draw_streets (1, scale, street_color);
}

// draw all street with set width and color, help to create outer boundary for every street
void draw_streets (double ratio, double scale, t_color input_color)
{
    // drawing streets
    setcolor (street_color);
    int num_of_street_segments = getNumberOfStreetSegments ();
    
    for (int i=0; i<num_of_street_segments; i++)
    {
        StreetSegmentEnds endsp = getStreetSegmentEnds (i);
        LatLon intersection_startp = getIntersectionPosition (endsp.from);
        LatLon intersection_endp = getIntersectionPosition (endsp.to);
        
        t_point startp = get_coordinate_from_LatLon (intersection_startp);
        t_point endp = get_coordinate_from_LatLon (intersection_endp);
        t_point midp((startp.x + endp.x)/2, (startp.y + endp.y)/2);
        t_point midrp((startp.x + midp.x)/2, (startp.y + midp.y)/2);
        t_point midlp((midp.x + endp.x)/2, (midp.y + endp.y)/2);
        if(!(get_visible_world().intersects(startp) || get_visible_world().intersects(endp)
                || get_visible_world().intersects(midp) || get_visible_world().intersects(midrp)
                || get_visible_world().intersects(midlp)))
            continue;
        
        int num_of_curve_points = getStreetSegmentCurvePointCount (i);
        bool one_way = getStreetSegmentOneWay (i) == true;
        
        //if the street has a speed limit higher than 60km/h then it is 
        //considered as a highway. When the map is being drawn, the color of 
        //the road that is considered as a highway will be drawn with a different
        //color. If the road is not considered as a highway, then it is drawn
        //with the color of the street
        //double speed_limit = getStreetSegmentSpeedLimit (i);
        //double width_inflate = 1;
        if (street_data[getStreetName(getStreetSegmentStreetID(i))].tier == 3
                || (street_data[getStreetName(getStreetSegmentStreetID(i))].tier == 3
                && force_draw_streets)/*speed_limit >= 70*/)
        {
            setcolor (highway_color);
            //setlinewidth(5);
            if(getZoomLevel() >= 0 && getZoomLevel() <= 2)
                setlinewidth(3);
            else if(getZoomLevel() > 2 && getZoomLevel() <= 5)
                setlinewidth(5);
            else if(getZoomLevel() < 9 && getZoomLevel() > 5)
                setlinewidth(10);
            else if(getZoomLevel() < 9 && getZoomLevel() > 7)
                setlinewidth(16);
            else if(getZoomLevel() >= 9)
                setlinewidth(18);
            //width_inflate = 5;
        }
        
        else if (street_data[getStreetName(getStreetSegmentStreetID(i))].tier == 2
                || (street_data[getStreetName(getStreetSegmentStreetID(i))].tier == 2
                && force_draw_streets)/*speed_limit >= 60*/)
        {
            setcolor(WHITE);
            //setcolor (highway_color_2);
            if(getZoomLevel() >= 0 && getZoomLevel() <= 2)
                setlinewidth(1);
            else if(getZoomLevel() > 2 && getZoomLevel() <= 5)
                setlinewidth(2.5);
            else if(getZoomLevel() <= 7 && getZoomLevel() > 5)
                setlinewidth(5);
            else if(getZoomLevel() < 9 && getZoomLevel() > 7)
                setlinewidth(11);
            else if(getZoomLevel() >= 9)
                setlinewidth(13);
            //continue;
            //width_inflate = 3;
        }
        
        else if (street_data[getStreetName(getStreetSegmentStreetID(i))].tier == 1
                || (street_data[getStreetName(getStreetSegmentStreetID(i))].tier == 1
                && force_draw_streets)/*speed_limit >= 45*/)
        {
            if(getZoomLevel() > 2 || force_draw_streets)
            {               
                setcolor (street_color);
                setlinewidth(1);
                if(getZoomLevel() <= 7 && getZoomLevel() > 5)
                    setlinewidth(2.5);
                else if(getZoomLevel() < 9 && getZoomLevel() > 7)
                    setlinewidth(5);
                else if(getZoomLevel() >= 9)
                    setlinewidth(10);
            }
            else continue;
            //break;
            //width_inflate = 2;
        }
        
        else if(street_data[getStreetName(getStreetSegmentStreetID(i))].tier == 0
                || (street_data[getStreetName(getStreetSegmentStreetID(i))].tier == 0
                && force_draw_streets))
        {
            if(getZoomLevel() > 4 || force_draw_streets)
            {
                setcolor (street_color);
                setlinewidth(0);
                if(getZoomLevel() <= 7 && getZoomLevel() > 5)
                    setlinewidth(1);
                else if(getZoomLevel() < 9 && getZoomLevel() > 7)
                    setlinewidth(3);
                else if(getZoomLevel() >= 9)
                    setlinewidth(6);
            }
            else continue;
        }
        /*else
        {
            continue;
            setcolor (street_color);
            setlinewidth(0);
            //break;
            //width_inflate = 1;
        }*/
        
        /*if (ratio != 1)
        {
            width_inflate = 1;
            setcolor (input_color);
        }*/
        
        
        // sets the width of the streets 
        //if the street is one_way then the width of the one_way street will
        //be half the width of two_way streets
        /*double line_width = 0;
        if (one_way)
        {
            line_width = default_one_way * width_inflate * ratio/ scale;
        }
        else
        {
            line_width = default_street_width * width_inflate * ratio/ scale;
        }
        setlinewidth (line_width);*/
        
        //double point_radius = line_width * scale * 90 / 2;
        
        // the roads are drawn by connecting up nearby intersections 
        // the intersection have its specific coordinate associated with them, 
        // which allows drawing of the road to be much easier
        StreetSegmentEnds ends = getStreetSegmentEnds (i);
        LatLon intersection_start = getIntersectionPosition (ends.from);
        LatLon intersection_end = getIntersectionPosition (ends.to);
        
        t_point start = get_coordinate_from_LatLon (intersection_start);
        t_point end = get_coordinate_from_LatLon (intersection_end);
        
        // for streets with no curve points
        if (num_of_curve_points == 0)
        {
            drawline(start,end);
            //fillarc (start.x, start.y, point_radius, 0, 360);
            //fillarc (end.x, end.y, point_radius, 0, 360);
            // the rest of the loop is for curvy segment
            continue;
        }
        
        // the rest is for streets with curve points
        // if the street were to have curve points then the street is drawn by
        // connecting up all the curve points
        LatLon curve_point1 = getStreetSegmentCurvePoint (i, 0);
        LatLon curve_point2 = getStreetSegmentCurvePoint (i, num_of_curve_points-1);
        
        t_point point1 = get_coordinate_from_LatLon (curve_point1);
        t_point point2 = get_coordinate_from_LatLon (curve_point2);
        
        // this connects the starting of the curve point with the starting intersection
        // and connects the end intersection with the last curve point
        
        drawline (start, point1);
        
        drawline (point2, end);
        //fillarc (start.x, start.y, point_radius, 0, 360);
        //fillarc (end.x, end.y, point_radius, 0, 360);
        
        
        // this loop goes through street segment's curve points and draws
        // each segments by connecting up the curve points
        for (int j=0; j<num_of_curve_points-1; j++)
        {
            curve_point1 = getStreetSegmentCurvePoint (i, j);
            curve_point2 = getStreetSegmentCurvePoint (i, j+1);
            
            point1 = get_coordinate_from_LatLon (curve_point1);
            point2 = get_coordinate_from_LatLon (curve_point2);
            
            drawline (point1, point2);
            //fillarc (point1.x, point1.y, point_radius, 0, 360);
        }
        //fillarc (point2.x, point2.y, point_radius, 0, 360);
        
        
        //if the map were to be zoomed in to a certain scale
        if (scale <= 0.0167977)
        {
            // label the direction of one way streets
            if (one_way)
            {
                t_point direction;
                t_point center; 

                if (num_of_curve_points != 0)
                {
                    if (num_of_curve_points == 1)
                    {
                        direction = end - start;
                        center = get_coordinate_from_LatLon (getStreetSegmentCurvePoint (i, 0));
                    }
                    else if (num_of_curve_points == 2)
                    {
                        direction = end - start;
                        t_point curve_point1 = get_coordinate_from_LatLon (getStreetSegmentCurvePoint (i, 0));
                        t_point curve_point2 = get_coordinate_from_LatLon (getStreetSegmentCurvePoint (i, 1));
                        center.x = (curve_point1.x + curve_point2.x) / 2;
                        center.y = (curve_point1.y + curve_point2.y) / 2;
                    }
                    else if ((num_of_curve_points % 2) == 0)
                    {
                        int middle_curvepoint = (num_of_curve_points + 1) / 2;
                        t_point point_pre, point_post;
                        point_pre = get_coordinate_from_LatLon (getStreetSegmentCurvePoint (i, middle_curvepoint - 1));
                        point_post = get_coordinate_from_LatLon (getStreetSegmentCurvePoint (i, middle_curvepoint + 1));
                        direction = point_post - point_pre;
                        center = get_coordinate_from_LatLon (getStreetSegmentCurvePoint (i, middle_curvepoint));
                    }
                    else
                    {
                        int middle_curvepoint = (num_of_curve_points + 1) / 2;
                        t_point point_pre, point_post;
                        point_pre = get_coordinate_from_LatLon (getStreetSegmentCurvePoint (i, middle_curvepoint - 1));
                        point_post = get_coordinate_from_LatLon (getStreetSegmentCurvePoint (i, middle_curvepoint + 1));
                        direction = point_post - point_pre;
                        center = get_coordinate_from_LatLon (getStreetSegmentCurvePoint (i, middle_curvepoint));
                    }
                    draw_arrow (direction, center);
                }
                else
                {
                    direction = end - start;
                    center.x = (start.x + end.x) / 2;
                    draw_arrow (direction, center);
                }
            }
        }
        
        //t_point center = find_street_segment_center (i);
        //setcolor (RED);
        //fillarc (center.x, center.y, point_radius, 0, 360);
        setcolor (WHITE);
    }
}


//function that draws all the pinned points and the location pinned points were 
//chosen by the user through a mouse
void draw_all_pinend_points (double scale)
{
    int num = pinned_points.size();
    
    //setcolor (pin_color);
    
    //goes through all the pinned points' locations and draws them
    for (int i=0; i<num; i++)
    {
        setcolor (pin_color);
        fillarc (pinned_points[i], pin_size * sqrt(sqrt(scale*scale*scale)), 60, 60);
        
        if(num > 1 && path_set)
        {
            if(i == num - 2)
            {
                t_bound_box start(pinned_points[i].x - pin_size * sqrt(sqrt(scale*scale*scale)),
                        pinned_points[i].y + pin_size * sqrt(sqrt(scale*scale*scale))*0.23,
                        pinned_points[i].x + pin_size * sqrt(sqrt(scale*scale*scale)),
                        pinned_points[i].y + pin_size * sqrt(sqrt(scale*scale*scale)));

                setcolor(255,255,255);
                setfontsize((res.top_height/BASE_RES)*15);
                drawtext_in(start, "A");
            }
            else if(i == num - 1)
            {
                t_bound_box end(pinned_points[i].x - pin_size * sqrt(sqrt(scale*scale*scale)),
                        pinned_points[i].y + pin_size * sqrt(sqrt(scale*scale*scale))*0.23,
                        pinned_points[i].x + pin_size * sqrt(sqrt(scale*scale*scale)),
                        pinned_points[i].y + pin_size * sqrt(sqrt(scale*scale*scale)));

                setcolor(255,255,255);
                setfontsize((res.top_height/BASE_RES)*15);
                drawtext_in(end, "B");
            }
        }
    }
}

//function that finds all the locations of the matching input
void save_search_results (std::string search_input)
{
    // empty the list of result
    search_results.clear();
    // check if the key exist
    if (poi_data.find(search_input) == poi_data.end())
    {
        std::cout << "not found!" << std::endl;
        update_message ("not found");
        return;
    }
    
    //if the search input exists, it goes through all the locations of the
    //searching input then stores them into the search_results vector
    int num_of_locations = poi_data[search_input].size();
    std::cout << num_of_locations << std:: endl;
    for (int i=0; i<num_of_locations; i++)
    {
        double x_coor = (poi_data[search_input])[i].position.lon * LON_TO_COOR_RATIO;
        double y_coor = (poi_data[search_input])[i].position.lat * LAT_TO_COOR_RATIO;
        t_point location (x_coor, y_coor);
        search_results.push_back (location);
    }
}

// function that displays search result
void display_search_results (double scale)
{
    setcolor (search_result_color);
    //checks if there was anything searched that matched the input in the 
    //search_result vector
    if (search_results.size() == 0)
    {
        return;
    }
    
    //if there is some searched results, then it draws all the locations of the
    //serached results accordingly to its coordinates
    int num_of_results = search_results.size();
    for (int i=0; i<num_of_results; i++)
    {
        fillarc (search_results[i], pin_size * sqrt(sqrt(scale*scale*scale)), 60, 60);
    }
}

//fucntion that draws all the street names
void draw_street_names (double scale)
{
    
    setcolor(BLACK);
    
    //first checks if the zoom scale goes beyond the arbitrary value and if it
    //did then it does not draw street names
    int text_size = default_font_size;
    // if (scale > 0.0l167977)//arbitrary value for zoom scale
    if (scale > 0.02)
    {
        return;
    }
    
    //draws all the street names by getting the names of the street names
    //then the coordinates for the street names were determined by getting the 
    //middle point of each street segment
    setfontsize (text_size/scale);
    
    t_bound_box visible_world = get_visible_world ();
    
    unsigned long long number_of_streets = getNumberOfStreets();
    for(unsigned i = 0; i < number_of_streets; ++i)
    {
        std::string street_name = getStreetName(i);
        /*if(getZoomLevel() >= 6)
        {
            if(street_data[street_name].tier == 3 || street_data[street_name].tier == 2)
               ;
            else continue;
            //else if(getZoomLevel() >= 8)
            //{
            //    if(street_data[street_name])
            //}
        }*/
        std::vector <unsigned> street_segments = street_data[street_name].street_segments;
        unsigned number_of_segments = street_data[street_name].street_segments.size();
        // unsigned street_length = find_street_length(street_name);
        if(street_name.find("unknown") == std::string::npos)
        {
            for(unsigned j=0; j<number_of_segments; ++j)
            {           
                //if(j%5 != 0) continue;
                
                double name_length = get_street_name_length (j, text_size * 1.1);
                if(find_street_segment_length(street_segments[j]) > name_length)
                {
                    
                    StreetSegmentEnds ends = getStreetSegmentEnds(street_segments[j]);
                    LatLon from, to, center;
                    float angle = 0;
                    t_point center_coor;
                    
                    unsigned num_of_curve_points = getStreetSegmentCurvePointCount (j);
                    if (num_of_curve_points > 0)
                    {
                        //determines the place for drawing name for curvy street segments                    
                        
                        ends = getStreetSegmentEnds(street_segments[j]);
                        from = getIntersectionPosition(ends.from);
                        to = getIntersectionPosition(ends.to);
                        center = LatLon ((from.lat + to.lat)/2, (from.lon + to.lon)/2);
                        angle = calculate_line_direction(from, to);
                        center_coor = get_coordinate_from_LatLon(center);
                    }
                    else
                    {
                        //determines the location of the street names for each
                        //street segments by getting each street segments' centre
                        //point
                        from = getIntersectionPosition(ends.from);
                        to = getIntersectionPosition(ends.to);
                        center = LatLon ((from.lat + to.lat)/2, (from.lon + to.lon)/2);
                        angle = calculate_line_direction(from, to);
                        center_coor = get_coordinate_from_LatLon(center);                    
                    }
                    
                    //sets the size of the text and sets the angle accordingly
                    //to the street segment so that it lines up with each street
                    //segments
                    setfontsize (text_size); // * 0.0167977/scale);
                    settextrotation (angle);
                    // checks if the determined locations are out of bounds
                    if (center_coor.y < visible_world.bottom() ||
                            center_coor.y > visible_world.top () ||
                            center_coor.x < visible_world.left () ||
                            center_coor.x > visible_world.right ())
                    {
                        continue;
                    }
                    
                    drawtext(center_coor, street_name, FLT_MAX, FLT_MAX);
                }
            }
        }
    }
    settextrotation(0);    
}

//function that draws the arrow head of the street directions
void draw_arrow (t_point direction_vector, t_point center)
{
    double magnitude = sqrt (direction_vector.x*direction_vector.x + direction_vector.y*direction_vector.y);
    t_point unit_direction_vector (direction_vector.x / magnitude, direction_vector.y / magnitude);
    
    t_point perpendicular (unit_direction_vector.y, -unit_direction_vector.x);
    
    t_point front (center.x + unit_direction_vector.x * arrow_size, 
            center.y + unit_direction_vector.y * arrow_size);
    t_point side1 (center.x - (unit_direction_vector.x + perpendicular.x) * arrow_size / 2, 
            center.y - (unit_direction_vector.y + perpendicular.y) * arrow_size / 2);
    t_point side2 (center.x - (unit_direction_vector.x - perpendicular.x) * arrow_size / 2,
            center.y - (unit_direction_vector.y - perpendicular.y) * arrow_size / 2);
    
    t_point points_list[4] = {center, side1, front, side2};
    setcolor (BLACK);
    fillpoly (points_list, 4);
}


//converts LatLon to coordinates on minimap
t_point get_mini_coordinate_from_LatLon (LatLon geo_location)
{
    double x_coor = (geo_location.lon * LAT_TO_COOR_RATIO - map_bounds.minlon * LAT_TO_COOR_RATIO)
                    *track_spec.w_ratio + minimap_bounds.left();
    double y_coor = (geo_location.lat * LAT_TO_COOR_RATIO - map_bounds.minlat * LAT_TO_COOR_RATIO)
                    *track_spec.h_ratio + minimap_bounds.bottom();
    
    t_point point (x_coor, y_coor);
    
    return point;
}

//function to draw minimap, scales minimap based on zoom level as well as relative zoom
void draw_minimap(double scale)
{
    //determine visible world
    t_bound_box visible_world = get_visible_world ();
    
    //minimap dimensions
    
    float height = visible_world.get_height()*0.25;
    float width = height;
    float offset = sqrt(pow((double)(visible_world.top() - visible_world.bottom()),(double)2) +
                    pow((double)(visible_world.right() - visible_world.left()),(double)2.0))*0.01;
    //float height = 15000*scale;
    //float width = 15000*scale;
    //float offset = 1000*scale;
    
    //ratio of actual map to minimap, used for calculations
    double h_ratio = height/(map_bounds.mapheight);
    double w_ratio = width/(map_bounds.mapwidth);
    
    track_spec.h_ratio = h_ratio;
    track_spec.w_ratio = w_ratio;
    
    
    //sets minimap drawing coordinates to a static position, scales
    float minimap_bottom = visible_world.bottom() + offset;
    float minimap_top = minimap_bottom + height;
    float minimap_right = visible_world.right() - offset;
    float minimap_left = minimap_right - width;
    
    //initialize minimap bounds
    minimap_bounds = t_bound_box(minimap_left,minimap_bottom,minimap_right,minimap_top);
    
    //alternate way to determine when to show minimap, replace current conditional
    //state (immediately after these comments) if determined more suitable
    /*if(visible_world.bottom() > map_bounds.minlat * LAT_TO_COOR_RATIO ||
            visible_world.top() < map_bounds.maxlat * LAT_TO_COOR_RATIO ||
            visible_world.left() > map_bounds.minlon * LAT_TO_COOR_RATIO ||
            visible_world.right() < map_bounds.maxlon * LAT_TO_COOR_RATIO)*/
    
    //only show minimap if zoomed in
    if(scale < 0.99 || path_set)
    {              
        //set color of land
        setcolor(235,225,220);
        //setcolor(255,255,255);
        
        //background of minimap
        fillrect (minimap_bounds.left(), minimap_bounds.bottom(), 
                minimap_bounds.right(), minimap_bounds.top());
        
        //draw features on minimap
        draw_minimap_features(scale);
        
        //draw searched/user searches
        draw_mini_path(path_data, scale);
        draw_minimap_search(scale);
        
        if(menu.cour_state == 3)
        {
            draw_mini_path(menu.cour_path, scale);
            draw_mini_courpos(get_scale(), depot_color, menu.cour_depots);
            draw_mini_courpos(get_scale(), intr_color, menu.cour_intrs);
        }
        else if(menu.path_state == 1)
        {
            draw_mini_path(menu.path_path, scale);
            draw_mini_courpos(get_scale(), pin_color, menu.path_waypoints);
        }
        
        //draw track box
        if(getZoomLevel() != 0)
            draw_trackmap(visible_world);
        
        //draw border
        setcolor(0,0,0);
        setlinewidth(3);
        drawrect(minimap_bounds);
        
        
        //draw_city_name(scale);
        
    }
    
}

//function to determine current viewable map and highlights on minimap
void draw_trackmap(t_bound_box visible_world)
{
        //determines current viewable map and converts it to minimap coordinates
        float spec_bottom = (visible_world.bottom() - map_bounds.minlat* LAT_TO_COOR_RATIO)
                            *track_spec.h_ratio + minimap_bounds.bottom();
        float spec_top = (visible_world.top() - map_bounds.minlat* LAT_TO_COOR_RATIO)
                            *track_spec.h_ratio + minimap_bounds.bottom();
        float spec_right = (visible_world.right() - map_bounds.minlon* LAT_TO_COOR_RATIO)
                            *track_spec.w_ratio + minimap_bounds.left();
        float spec_left = (visible_world.left() - map_bounds.minlon* LAT_TO_COOR_RATIO)
                            *track_spec.w_ratio + minimap_bounds.left();
        
        //limit tracking box to be within minimap bounds
        if(spec_bottom < minimap_bounds.bottom()) spec_bottom = minimap_bounds.bottom();
        if(spec_top > minimap_bounds.top()) spec_top = minimap_bounds.top();
        if(spec_left < minimap_bounds.left()) spec_left = minimap_bounds.left();
        if(spec_right > minimap_bounds.right()) spec_right = minimap_bounds.right();
    
        //initializ track box 
        track_spec.curr_bounds = t_bound_box(spec_left,spec_bottom,spec_right,spec_top);
        
        setcolor(255,180,0);
        setlinewidth(2);
        
        //if track box gets too small, becomes a circle
        if(get_scale() <= 0.03)
        {
            t_point center = track_spec.curr_bounds.get_center();
            drawarc(center.x, center.y, minimap_bounds.get_height()*0.022, 0, 360);
        }
        else
            drawrect(track_spec.curr_bounds);
}

//function determines the location to be drawn on main map depending on zoom level
//user clicks and actual map should move to that point
void set_minimap_spec()
{
    //determine visible world
    t_bound_box visible_world = get_visible_world ();
    float height = visible_world.get_height();
    float width = visible_world.get_width();
    
    //determine x,y minimap coord
    float spec_x = mouse_location_current.x - minimap_bounds.left();
    float spec_y = mouse_location_current.y - minimap_bounds.bottom();
    
    //convert to x,y map coord
    spec_x = (spec_x/track_spec.w_ratio) + map_bounds.minlon*LAT_TO_COOR_RATIO;
    spec_y = (spec_y/track_spec.h_ratio) + map_bounds.minlat*LAT_TO_COOR_RATIO;
    
    //determine top, bottom, left, right
    float spec_bottom = spec_y - height/2;
    float spec_top = spec_y + height/2;
    float spec_left = spec_x - width/2;
    float spec_right = spec_x + width/2;
    
    //determine if any are out of bounds
    //if any of spec_ are out of bounds, shifts by difference
    if(spec_bottom < map_bounds.minlat*LAT_TO_COOR_RATIO)
    {
        float shift = map_bounds.minlat*LAT_TO_COOR_RATIO - spec_bottom;
        spec_bottom = spec_bottom + shift;
        spec_top = spec_top + shift;
    }   
    if(spec_top > map_bounds.maxlat*LAT_TO_COOR_RATIO)
    {
        float shift = spec_top - map_bounds.maxlat*LAT_TO_COOR_RATIO;
        spec_bottom = spec_bottom - shift;
        spec_top = spec_top - shift;
    }
    if(spec_left < map_bounds.minlon*LAT_TO_COOR_RATIO)
    {
        float shift = map_bounds.minlon*LAT_TO_COOR_RATIO - spec_left;
        spec_left = spec_left + shift;
        spec_right = spec_right + shift;
    }
    if(spec_right > map_bounds.maxlon*LAT_TO_COOR_RATIO)
    {
        float shift = spec_right - map_bounds.maxlon*LAT_TO_COOR_RATIO;
        spec_left = spec_left - shift;
        spec_right = spec_right - shift;
    }
    
    //void set_visible_world(xl, yb, xr, yt)
    set_visible_world(spec_left, spec_bottom, spec_right, spec_top);
}

//can just use default map functions, instead use get_mini_coordinate_from_LatLon
void draw_minimap_features(double scale)
{
    draw_all_miniwater ();
    draw_all_mininatural ();
    draw_all_minwaterways ();
    draw_all_minilands ();
}


void draw_all_miniwater ()
{
    setcolor (lake_color);
    
    int num_of_natural_features = getFeatureCount ();
    // loop through all natural features and draw polygon according to their points
    for (int i=0; i<num_of_natural_features; i++)
    {
        int num_of_points = getFeaturePointCount (i);
        
        t_point * point_ptr = NULL;
        point_ptr = new t_point [num_of_points];
        
        for (int j=0; j<num_of_points; j++)
        {
            LatLon point = getFeaturePoint (i, j);
            point_ptr[j] = get_mini_coordinate_from_LatLon (point);
        }
        
        if (point_ptr == NULL)
        {
            continue;
        }
        
        std::string water_type = getFeatureAttribute (i, "water");
        if (water_type == "lake" || water_type == "lagoon" || water_type == "pond")
        {
            fillpoly (point_ptr, num_of_points);
        }
        delete [] point_ptr;
    }
}

void draw_all_mininatural ()
{
    int num_of_natural_features = getFeatureCount ();
    // loop through all natural features and draw polygon according to their points
    for (int i=0; i<num_of_natural_features; i++)
    {
        int num_of_points = getFeaturePointCount (i);
        
        t_point * point_ptr =  new t_point [num_of_points];
        
        for (int j=0; j<num_of_points; j++)
        {
            LatLon point = getFeaturePoint (i, j);
            point_ptr[j] = get_mini_coordinate_from_LatLon (point);
        }
        
        if (point_ptr == NULL)
        {
            continue;
        }
        
        std::string water_type = getFeatureAttribute (i, "natural");
        if (water_type == "sand")
        {
            setcolor (natural_sand_color);
            fillpoly (point_ptr, num_of_points);
        }
        else if (water_type == "water")
        {
            setcolor (natural_water_color);
            fillpoly (point_ptr, num_of_points);
        }
        else if (water_type == "wood")
        {
            setcolor (natural_wood_color);
            fillpoly (point_ptr, num_of_points);
        }
        else if (water_type == "tree_row")
        {
            setcolor (natural_tree_row_color);
            // fillpoly (point_ptr, num_of_points);
            // draw dash line
            setlinestyle (DASHED);
            drawline (point_ptr[0], point_ptr[1]);
        }
        else if (water_type == "grassland")
        {
            setcolor (natural_grassland_color);
            fillpoly (point_ptr, num_of_points);
        }
        else if (water_type == "beach")
        {
            setcolor (natural_beach_color);
            fillpoly (point_ptr, num_of_points);
        }
        else
        {
            continue;
        }
        
        setlinestyle (SOLID);
        delete [] point_ptr;
    }
}

void draw_all_minilands ()
{
    int num_of_natural_features = getFeatureCount ();
    // loop through all natural features and draw polygon according to their points
    for (int i=0; i<num_of_natural_features; i++)
    {
        int num_of_points = getFeaturePointCount (i);
        
        t_point * point_ptr = NULL;
        point_ptr = new t_point [num_of_points];
        
        
        if (num_of_points == 0)
        {
            continue;
        }
        
        for (int j=0; j<num_of_points; j++)
        {
            LatLon point = getFeaturePoint (i, j);
            point_ptr[j] = get_mini_coordinate_from_LatLon (point);
        }
        
        
        
        
        std::string water_type = getFeatureAttribute (i, "land");
        if (water_type == "island")
        {
            setcolor(255,255,255);
            fillpoly (point_ptr, num_of_points);
        }        
        if(getFeatureAttribute(i, "natural")==  "coastline")
        {
            setcolor(195,195,195);
            setlinewidth (1);//(default_water_way_width/(pow(3./5,getZoomLevel())));
            for (int k=0; k<num_of_points-1; k++)
            {
                LatLon way_point_1 = getFeaturePoint (i, k);
                LatLon way_point_2 = getFeaturePoint (i, k+1);
                t_point point1 = get_mini_coordinate_from_LatLon (way_point_1);
                t_point point2 = get_mini_coordinate_from_LatLon (way_point_2);

                drawline (point1, point2);
            }
        }
        
        setlinestyle (SOLID);
        delete [] point_ptr;
    }
}

void draw_all_minwaterways ()
{
    setlinestyle (SOLID);
   
    int num_of_natural_features = getFeatureCount ();
    // loop through all natural features and draw polygon according to their points
    for (int i=0; i<num_of_natural_features; i++)
    {
        int num_of_points = getFeaturePointCount (i);
        
        std::string water_type = getFeatureAttribute (i, "waterway");
        if (water_type == "river")
        {
            setcolor (waterway_river_color);
            // setcolor (255,0,0);
            setlinewidth (default_water_way_width);
        }
        else if (water_type == "stream")
        {
            setcolor (waterway_stream_color);
            setlinewidth ((default_water_way_width) * 2 / 12);
        }
        else if (water_type == "boatyard")
        {
            setcolor (waterway_boatyard_color);
            setlinewidth ((default_water_way_width) / 12);
        }
        else
        {
            continue;
        }
        
        for (int j=0; j<num_of_points-1; j++)
        {
            LatLon way_point_1 = getFeaturePoint (i, j);
            LatLon way_point_2 = getFeaturePoint (i, j+1);
            t_point point1 = get_mini_coordinate_from_LatLon (way_point_1);
            t_point point2 = get_mini_coordinate_from_LatLon (way_point_2);
            
            drawline (point1, point2);
        }
    }
}

//draws search function results on minimap and user inputted POI
void draw_minimap_search(double scale)
{
    int num_of_results = search_results.size();
    
    if (num_of_results != 0)
    {
        setcolor (0,0,205);

        for (int i=0; i< num_of_results; i++)
        {
            float conv_x = (search_results[i].x - map_bounds.minlon*LAT_TO_COOR_RATIO)
                            *track_spec.w_ratio + minimap_bounds.left();
            float conv_y = (search_results[i].y - map_bounds.minlat*LAT_TO_COOR_RATIO)
                            *track_spec.h_ratio + minimap_bounds.bottom();
            t_point conv(conv_x, conv_y);
            //LatLon conv(search_results[i].x/LAT_TO_COOR_RATIO, search_results[i].y/LAT_TO_COOR_RATIO);
            //t_point conv_search_results = get_mini_coordinate_from_LatLon(conv);
            fillarc (conv, minimap_bounds.get_height()*0.011, 0, 360);
        }
    }
    
    int num_pinned = pinned_points.size();
    
    if(num_pinned != 0)
    {
        setcolor(255, 0, 0);

        for (int i=0; i< num_pinned; i++)
        {
            float conv_x = (pinned_points[i].x - map_bounds.minlon*LAT_TO_COOR_RATIO)
                            *track_spec.w_ratio + minimap_bounds.left();
            float conv_y = (pinned_points[i].y - map_bounds.minlat*LAT_TO_COOR_RATIO)
                            *track_spec.h_ratio + minimap_bounds.bottom();
            t_point conv(conv_x, conv_y);
            //LatLon conv(search_results[i].x/LAT_TO_COOR_RATIO, search_results[i].y/LAT_TO_COOR_RATIO);
            //t_point conv_search_results = get_mini_coordinate_from_LatLon(conv);
            fillarc (conv, minimap_bounds.get_height()*0.045, 60, 60);
        }
    }
}

//draws city name above minimap box, not used
/*void draw_city_name(double scale)
{
    std::string name = std::string(city_name.begin(), city_name.end());
    setcolor(0,0,0);
    setfontsize(13);
    float bottom = minimap_bounds.top();
    float top = minimap_bounds.top() + 3000*scale;
    float right = minimap_bounds.right();
    float left = minimap_bounds.left() - 10000*scale;
    t_bound_box drawin(left, bottom, right, top);
    drawtext_in(drawin, name);
    
}*/

t_point find_street_segment_center (int segID)
{
    int num_of_curve_points = getStreetSegmentCurvePointCount (segID);
    // get mid point of the street
    double total_length = find_street_segment_length(segID);
    double total_length_half = total_length / 2;
    double offset = 0.0;
    
    LatLon center;
    StreetSegmentEnds ends = getStreetSegmentEnds (segID);
    t_point center_coor;
    
    for (int k=0; k<num_of_curve_points; k++)
    {
        LatLon point_current;
        LatLon point_next;
        if (k == 0)
        {
            point_current = getIntersectionPosition (ends.from);
            point_next = getStreetSegmentCurvePoint (segID, k);

        }
        else if (k >= num_of_curve_points-1)
        {
            point_current = getStreetSegmentCurvePoint (segID, num_of_curve_points-1);
            point_next = getIntersectionPosition (ends.to);

        }
        else
        {
            point_current = getStreetSegmentCurvePoint (segID, k);
            point_next = getStreetSegmentCurvePoint (segID, k+1);
        }
           
        // this is calculating distance in lat & lon, not the actual distance
        double step_distance = sqrt ((point_current.lon - point_next.lon)*(point_next.lon - point_next.lon) 
                                    + (point_current.lat - point_next.lat)*(point_next.lat - point_next.lat));

        offset += step_distance;
        if (offset >= total_length_half)
        {
            double difference = offset - total_length_half;

            LatLon displacement_vector (point_current.lon - point_next.lon, point_current.lat - point_next.lat);
            double displacement_magnitude = sqrt (displacement_vector.lon * displacement_vector.lon 
                                                + displacement_vector.lat * displacement_vector.lat);
            center.lon = point_next.lon + displacement_vector.lon * difference / displacement_magnitude;
            center.lat = point_next.lat + displacement_vector.lat * difference / displacement_magnitude;
            center_coor = get_coordinate_from_LatLon (center);

            break;
        }
    }
    
    return center_coor;
}

double get_street_name_length (unsigned long streetID, double font_size)
{
    std::string street_name = getStreetName (streetID);
    double name_length = 0;
    
    int num_of_char = street_name.length ();
    
    name_length = num_of_char * font_size;
    
    return name_length;
}

void draw_layermenu(double scale)
{
    
    //determine visible world
    t_bound_box visible_world = get_visible_world ();
    
    //draw_triangle(visible_world.get_center(),'N', boatyard_color);
    
    float height = visible_world.get_height()*0.5;
    float width = minimap_bounds.get_width();
    float offset =  sqrt(pow((double)(visible_world.top() - visible_world.bottom()),(double)2) +
                    pow((double)(visible_world.right() - visible_world.left()),(double)2.0))*0.01;
    
    
    //sets layer drawing coordinates to a static position, scales
    float layermenu_bottom = minimap_bounds.top() + offset;
    float layermenu_top = layermenu_bottom + height;
    float layermenu_right = minimap_bounds.right();
    float layermenu_left = layermenu_right - width;
    
    //initialize menu bounds
    layermenu_bounds = t_bound_box(layermenu_left,layermenu_bottom,layermenu_right,layermenu_top);
    layerSys.setLegendBounds(layermenu_bounds.top(), layermenu_bounds.bottom(),
            layermenu_bounds.left(), layermenu_bounds.right());
    
    //draw background of legend (white)
    setcolor(255,255,255);
    fillrect(layermenu_bounds);
    
    //calculate how to divide legend bounds
    legend.div_offset = layermenu_bounds.get_height()/legend.num_div;
    
    //initialize scroll bar arrow parameters 
    legend.up_center = t_point(layermenu_bounds.right() - layermenu_bounds.get_width()*0.05,
                    layermenu_bounds.top() - legend.div_offset - layermenu_bounds.get_width()*0.05);
    
    legend.down_center = t_point(layermenu_bounds.right() - layermenu_bounds.get_width()*0.05,
                layermenu_bounds.bottom() + layermenu_bounds.get_width()*0.06);  
    
    legend.up_bounds = t_bound_box(legend.up_center.x - layermenu_bounds.get_width()*0.05,
                            legend.up_center.y - layermenu_bounds.get_width()*0.05,
                            legend.up_center.x + layermenu_bounds.get_width()*0.05,
                            legend.up_center.y + layermenu_bounds.get_width()*0.05);
    
    legend.down_bounds = t_bound_box(legend.down_center.x - layermenu_bounds.get_width()*0.05,
                            legend.down_center.y - layermenu_bounds.get_width()*0.06,
                            legend.down_center.x + layermenu_bounds.get_width()*0.05,
                            legend.down_center.y + layermenu_bounds.get_width()*0.04);
    
    legend.scroll_bounds = t_bound_box(legend.up_bounds.left(), legend.down_bounds.top(),
                             legend.up_bounds.right(), legend.up_bounds.bottom());
    
    //draw first box for header
    setcolor(53,161,255);
    
    t_bound_box header(layermenu_left, layermenu_top - legend.div_offset, 
            layermenu_right, layermenu_top);
    
    fillrect(header);
    
    if((int)legend.active_layers.size() > legend.num_div)
    {
        draw_legend_sbar();
    }
    
    //draw legend border
    setcolor(0,0,0);
    setlinewidth(3);
    drawrect(layermenu_bounds);
    
    //draw legend contents
    draw_legend_spec(scale);
    
}

void draw_legend_sbar()
{
    setcolor(255,255,255);
    t_bound_box total_sbar(legend.up_bounds.left(), legend.down_bounds.bottom(),
            legend.up_bounds.right(), legend.up_bounds.top());
    fillrect(total_sbar);
        
    int scroll_div = legend.active_layers.size() - 1;
    legend.sdiv_offset = legend.scroll_bounds.get_height()/scroll_div;
    
        
    if(legend.up_backdrop)
    {
        setcolor(171,168,168);
        fillrect(legend.up_bounds);
    }
    if(legend.down_backdrop)
    {
        setcolor(171,168,168);
        fillrect(legend.down_bounds);
    }

    //draw scrollbar
    setcolor(0,0,0);

    t_point triangle[3];

    t_point point0(legend.up_center.x, legend.up_center.y + layermenu_bounds.get_width()*0.03);
    triangle[0] = point0;
    t_point point1(legend.up_center.x + layermenu_bounds.get_width()*0.03, legend.up_center.y);
    triangle[1] = point1;
    t_point point2(legend.up_center.x - layermenu_bounds.get_width()*0.03, legend.up_center.y);
    triangle[2] = point2;

    fillpoly(triangle, 3); 

    t_point point3(legend.down_center.x, legend.down_center.y - layermenu_bounds.get_width()*0.03);
    triangle[0] = point3;
    t_point point4(legend.down_center.x + layermenu_bounds.get_width()*0.03, legend.down_center.y);
    triangle[1] = point4;
    t_point point5(legend.down_center.x - layermenu_bounds.get_width()*0.03, legend.down_center.y);
    triangle[2] = point5;

    fillpoly(triangle, 3);

    //setcolor(53,161,255);
    setcolor(scrollbar_color);
    t_bound_box scrollbar_bounds(legend.scroll_bounds.left(), legend.scroll_bounds.bottom() 
            + legend.sdiv_offset*(legend.active_layers.size() - legend.num_div - legend.scroll_count), 
            legend.scroll_bounds.right(), legend.scroll_bounds.top() - legend.sdiv_offset*legend.scroll_count);
    legend.bar_bounds = scrollbar_bounds;
    fillrect(scrollbar_bounds);
    
    //draw legend border
    setcolor(0,0,0);
    setlinewidth(3);
    drawrect(layermenu_bounds);
        
}

void draw_legend_spec(double scale)
{
    float div_offset = layermenu_bounds.get_height()/legend.num_div;
    string text;
    
    if((int)legend.active_layers.size() > legend.num_div)
    {
        legend.more = true;
    }
    else legend.more = false;
 
    
    settextattrs((res.top_height/BASE_RES)*legend.font_size, 0);
    
    for(int h = legend.scroll_count, i = 0; (i < legend.num_div)
            && h < (int)legend.active_layers.size(); ++h, ++i)
    {
        t_bound_box text_box(layermenu_bounds.left(), layermenu_bounds.top() - (i+1)*div_offset,
                                layermenu_bounds.right(), layermenu_bounds.top() - (i)*div_offset);
        
        
        if(i == 0)
        {
            setcolor(255,255,255);
        }
        else
        {
            setcolor(0,0,0);
        }
        
        if(legend.scroll_count != 0)
        {
            if(i == 0)
            {
                text = legend.active_layers[0];
                drawtext_in(text_box, text);
            }
            else
            {
                text = legend.active_layers[h];
                drawtext_in(text_box, text);
               
                t_point d_point = text_box.get_center();
                d_point.x = d_point.x - layermenu_bounds.get_width()*0.35;
                //d_point.
                setcolor(legend.layer_colors[h]);
                fillarc(d_point, layermenu_bounds.get_width()*0.05, 0, 360);
               
            }
        }
        else
        {
            if(i == 0)
            {
                text = legend.active_layers[0];
                drawtext_in(text_box, text);
            }
            else
            {
                text = legend.active_layers[h];
                drawtext_in(text_box, text);
                
                t_point d_point = text_box.get_center();
                d_point.x = d_point.x - layermenu_bounds.get_width()*0.35;
                //d_point.
                setcolor(legend.layer_colors[h]);
                fillarc(d_point, layermenu_bounds.get_width()*0.05, 0, 360);
            }
        }
    }
}

//function to draw user input terminal
void draw_user_input(double scale, std::string text)
{
    //determine visible world
    t_bound_box visible_world = get_visible_world ();
    
    //text box dimensions
    
    float height = visible_world.get_height()*0.05;
    float width = visible_world.get_width()*0.27;
    float offset = sqrt(pow((double)(visible_world.top() - visible_world.bottom()),(double)2) +
                    pow((double)(visible_world.right() - visible_world.left()),(double)2.0))*0.01;
    
    //ratio of actual map to textbox, used for calculations
    //double h_ratio = height/(map_bounds.mapheight);
    //double w_ratio = width/(map_bounds.mapwidth);
    
    
    
    //sets text box drawing coordinates to a static position, scales
    float textbox_top = visible_world.top()- offset;
    float textbox_bottom = textbox_top - height;
    float textbox_left = visible_world.left() + offset;
    float textbox_right = textbox_left + width;
    //initialize text box bounds
    textbox_bounds = t_bound_box(textbox_left,textbox_bottom,textbox_right,textbox_top);
    
    
    //only show text box if zoomed in
    if(true)
    {              
        //set color of text box
        setcolor(255,255,255);
        
        //draws the text box
        fillrect (textbox_bounds.left(), textbox_bounds.bottom(), 
                textbox_bounds.right(), textbox_bounds.top());
        
        //draw border
        setcolor(0,0,0);
        setlinewidth(3);
        drawrect(textbox_bounds);
       
    }
    
}

void draw_suggested_poi(double scale, std::string text)
{
    //determine visible world
    t_bound_box visible_world = get_visible_world ();
    
    //text box dimensions
    
    float height = visible_world.get_height()*0.05;
    float width = visible_world.get_width()*0.27;
    float offset = sqrt(pow((double)(visible_world.top() - visible_world.bottom()),(double)2) +
                    pow((double)(visible_world.right() - visible_world.left()),(double)2.0))*0.01;
    
    //ratio of actual map to textbox, used for calculations
    //double h_ratio = height/(map_bounds.mapheight);
    //double w_ratio = width/(map_bounds.mapwidth);
    
    
    
    //sets text box drawing coordinates to a static position, scales
    float textbox_top = visible_world.top()- offset*4.0;
    float textbox_bottom = textbox_top - height;
    float textbox_left = visible_world.left() + offset;
    float textbox_right = textbox_left + width;
    //initialize text box bounds
    suggested_poi = t_bound_box(textbox_left,textbox_bottom,textbox_right,textbox_top);
    
    
    //only show text box if zoomed in
    if(true)
    {              
        //set color of text box
        if(arrow_key_pressed==1)
        {
            setcolor(scrollbar_color);
        }else {
            setcolor(unselected_colour);
        }
        
        //draws the text box
        fillrect (suggested_poi.left(), suggested_poi.bottom(), 
                suggested_poi.right(), suggested_poi.top());
        
        //draw border
        setcolor(0,0,0);
        setlinewidth(3);
        drawrect(suggested_poi);
       
    }
    
}

void draw_suggested_intersections(double scale, std::string text)
{
    //determine visible world
    t_bound_box visible_world = get_visible_world ();
    
    //text box dimensions
    
    float height = visible_world.get_height()*0.05;
    float width = visible_world.get_width()*0.27;
    float offset = sqrt(pow((double)(visible_world.top() - visible_world.bottom()),(double)2) +
                    pow((double)(visible_world.right() - visible_world.left()),(double)2.0))*0.01;
    
    //ratio of actual map to textbox, used for calculations
    //double h_ratio = height/(map_bounds.mapheight);
    //double w_ratio = width/(map_bounds.mapwidth);
    
    
    
    //sets text box drawing coordinates to a static position, scales
    float textbox_top = visible_world.top()- offset*7.0;
    float textbox_bottom = textbox_top - height;
    float textbox_left = visible_world.left() + offset;
    float textbox_right = textbox_left + width;
    //initialize text box bounds
    suggested_intersection = t_bound_box(textbox_left,textbox_bottom,textbox_right,textbox_top);
    
    
    //only show text box if zoomed in
    if(true)
    {              
        //set color of text box
        if(arrow_key_pressed==2)
        {
            setcolor(scrollbar_color);
        }else {
            setcolor(unselected_colour);
        }
        
        //draws the text box
        fillrect (suggested_intersection.left(), suggested_intersection.bottom(), 
                suggested_intersection.right(), suggested_intersection.top());
        
        //draw border
        setcolor(0,0,0);
        setlinewidth(3);
        drawrect(suggested_intersection);
       
    }
    
}

//function to draw_text
void draw_text(std::string text)
{
    double text_size = 10;
    std::string street_name = text;
    setcolor(0,0,0);
    setfontsize (text_size);
    settextrotation (0);
    //draws inside the text box
    if(textbox_bounds.get_ycenter()>textbox_bounds.bottom() && 
            textbox_bounds.get_ycenter()<textbox_bounds.top() && 
            textbox_bounds.get_xcenter()>textbox_bounds.left() && 
            textbox_bounds.get_xcenter()<textbox_bounds.right())
    {
        drawtext(textbox_bounds.get_center(), text, textbox_bounds, FLT_MAX);
    }
    
    //flushinput();
    //setfontsize (text_size); // * 0.0167977/scale);
    //drawtext(center_coor, street_name, FLT_MAX, FLT_MAX);
}

void draw_suggested_intersection_texts(std::string text)
{
    double text_size = 10;
    std::string street_name = text;
    /*(0.05/(res.top_width/1900.0))*/
    
    //makes sure text shown stays within textbox boundaries
    //50 is arbitrary number determined based on base max of 1920
    if(text.size() > 50*(res.top_width/1920.0))
    {
        text.resize(50*(res.top_width/1920.0));
        text = text + "...";
    }
    setcolor(0,0,0);
    setfontsize (text_size*(res.top_height/BASE_RES));
    settextrotation (0);
    //draws inside the text box
    if(suggested_intersection.get_ycenter()>suggested_intersection.bottom() && 
            suggested_intersection.get_ycenter()<suggested_intersection.top() && 
            suggested_intersection.get_xcenter()>suggested_intersection.left() && 
            suggested_intersection.get_xcenter()<suggested_intersection.right())
    {
        drawtext(suggested_intersection.get_center(), text, 
                                         suggested_intersection, FLT_MAX);
    }
}

void draw_suggested_poi_texts(std::string text)
{
    double text_size = 10;
    std::string street_name = text;
    setcolor(0,0,0);
    setfontsize (text_size*(res.top_height/BASE_RES));
    settextrotation (0);
    //draws inside the text box
    if(suggested_poi.get_ycenter()>suggested_poi.bottom() && 
            suggested_poi.get_ycenter()<suggested_poi.top() && 
            suggested_poi.get_xcenter()>suggested_poi.left() && 
            suggested_poi.get_xcenter()<suggested_poi.right())
    {
        drawtext(suggested_poi.get_center(), text, suggested_poi, FLT_MAX);
    }
}

//stores each word into an index of the command_lines vector
std::vector<std::string> parse_input(std::string text)
{
    //bool arguement = true;
    std::vector<std::string> command_and_argument;
    std::string command, arguement;
    stringstream lineStream (text);
    lineStream >> command;
    if(lineStream.fail()) //if no input then invalid command
    {
        cout << "invalid command" << endl;
        //return NULL;
    } else {
        command_and_argument.push_back(command);
    }
    
    while(!lineStream.eof())//separates each word and puts them 
                            //accordingly into the command_lines vector
    {
        lineStream >> arguement;
        if(lineStream.fail()) //if the function requires argument and no 
                              //arguments are present then print error message
        {
            cout << "invalid argument" << endl;
        } else {
            command_and_argument.push_back(arguement);
        }
    }
    return command_and_argument;
}

void layermenu_handler (void (*drawscreen_ptr) (void))
{
    bool active = layerSys.getLayermenuActive();
    if(!active)
    {
        layerSys.setLayermenuActive(true);
        layerSys.setLayermenuState("menu");
        destroy_button("Layers ON");
        create_button("Path", "Layers OFF", layermenu_handler); 
        layermenu_create();
        create_button("Layers OFF", "Legend ON", legendmenu_handler);
        drawscreen();
        
    }
    else
    {
        string state = layerSys.getLayermenuState();
        layerSys.setLayermenuActive(false);
        destroy_button("Layers OFF");
        create_button("Path", "Layers ON", layermenu_handler);
        if(state == "menu")
        {
            layermenu_destroy();
            legendmenu_destroy();
            reset_layer_state();
            set_default_layers();
            drawscreen();
        }
        else if(state == "natural")
        {
            naturalmenu_destroy();
            destroy_button("BACK");
            legendmenu_destroy();
            reset_layer_state();           
            layerSys.setLayermenuState("menu");
            set_default_layers();
            drawscreen();
        }
        else if(state == "waterway")
        {
            waterwaymenu_destroy();
            destroy_button("BACK");
            legendmenu_destroy();
            reset_layer_state();
            layerSys.setLayermenuState("menu");
            set_default_layers();
            drawscreen();
        }
    }
    
       
}

void naturalmenu_handler (void (*drawscreen_ptr) (void))
{
    layerSys.setLayermenuState("natural");
    layermenu_destroy();
    back_create();
    //create_button("Layers OFF", "BACK", backmenu_handler);
    naturalmenu_create();   
}

void waterwaymenu_handler (void (*drawscreen_ptr) (void))
{
    layerSys.setLayermenuState("waterway");
    layermenu_destroy();
    back_create();
    //create_button("Layers OFF", "BACK", backmenu_handler);
    waterwaymenu_create();
}

void reset_layer_state()
{
    vector <string> sub_list = layerSys.getList("natural");
    
    for(unsigned i = 0; i < sub_list.size(); ++i)
    {
        natural_layer[sub_list[i]].active = false;
    }
    
    vector <string> sub_list2 = layerSys.getList("waterway");
    
    for(unsigned i = 0; i < sub_list2.size(); ++i)
    {
        waterway_layer[sub_list2[i]].active = false;
    }
    
    legend.active_layers.clear();
    legend.layer_colors.clear();
    legend.active_layers.push_back("Legend");
    legend.layer_colors.push_back(header_color);
    legend_state = false;
}


void backmenu_handler (void (*drawscreen_ptr) (void))
{
    string c_state = layerSys.getLayermenuState();
    if(c_state == "natural")
    {
        destroy_button("BACK");
        naturalmenu_destroy();
        layermenu_create();
        legendmenu_destroy();
        legendmenu_create();
        layerSys.setLayermenuState("menu");       
    }
    else if(c_state == "waterway")
    {
        destroy_button("BACK");
        waterwaymenu_destroy();
        layermenu_create();
        legendmenu_destroy();
        legendmenu_create();
        layerSys.setLayermenuState("menu");
    }
    
}

void legendmenu_handler(void (*drawscreen_ptr) (void))
{
    if(!legend_state)
    {
        //enable drawing
        destroy_button("Legend ON");
        create_button("Layers OFF", "Legend OFF", legendmenu_handler);
        legend_state = true;
        if(path_set)
        {
            dir.hide = true;
        }
        drawscreen();
        //draw_layermenu(get_scale());
    }
    else
    {
        destroy_button("Legend OFF");
        create_button("Layers OFF", "Legend ON", legendmenu_handler);
        legend_state = false;
        if(path_set)
        {
            dir.hide = false;
        }
        drawscreen();
    }
}

void layermenu_create()
{
    create_button("Layers OFF", "waterway", waterwaymenu_handler);
    create_button("Layers OFF", "natural", naturalmenu_handler);
}

void naturalmenu_create()
{
    if(!natural_layer["n_other"].active)
        create_button("BACK", "n_other ON", nother_set);
    else
        create_button("BACK", "n_other OFF", nother_set);
    
    if(!natural_layer["grassland"].active)
        create_button("BACK", "grassland ON", grassland_set);
    else
        create_button("BACK", "grassland OFF", grassland_set);
    
    if(!natural_layer["tree_row"].active)
        create_button("BACK", "tree_row ON", treerow_set);
    else
        create_button("BACK", "tree_row OFF", treerow_set);
    
    if(!natural_layer["sand"].active)
        create_button("BACK", "sand ON", sand_set);
    else
        create_button("BACK", "sand OFF", sand_set);
    
    if(!natural_layer["heath"].active)
        create_button("BACK", "heath ON", heath_set);
    else
        create_button("BACK", "heath OFF", heath_set);
    
    if(!natural_layer["scrub"].active)
        create_button("BACK", "scrub ON", scrub_set);
    else
        create_button("BACK", "scrub OFF", scrub_set);
    
    if(!natural_layer["wetland"].active)
        create_button("BACK", "wetland ON", wetland_set);
    else
        create_button("BACK", "wetland OFF", wetland_set);
    
    if(!natural_layer["peak"].active)
        create_button("BACK", "peak ON", peak_set);
    else
        create_button("BACK", "peak OFF", peak_set);
    
    if(!natural_layer["cliff"].active)
        create_button("BACK", "cliff ON", cliff_set);
    else
        create_button("BACK", "cliff OFF", cliff_set);
    
    if(!natural_layer["tree"].active)
        create_button("BACK", "tree ON", tree_set);
    else
        create_button("BACK", "tree OFF", tree_set);
    
    if(!natural_layer["land"].active)
        create_button("BACK", "land ON", land_set);
    else
        create_button("BACK", "land OFF", land_set);
    
    if(!natural_layer["beach"].active)
        create_button("BACK", "beach ON", beach_set);
    else
        create_button("BACK", "beach OFF", beach_set);
    
    if(!natural_layer["wood"].active)
        create_button("BACK", "wood ON", wood_set);
    else
        create_button("BACK", "wood OFF", wood_set);
    
    if(!natural_layer["water"].active)
        create_button("BACK", "water ON", water_set);
    else
        create_button("BACK", "water OFF", water_set);
    
}

void waterwaymenu_create()
{
    if(!waterway_layer["ditch"].active)
        create_button("BACK", "ditch ON", ditch_set);
    else
        create_button("BACK", "ditch OFF", ditch_set);
    
    if(!waterway_layer["boatyard"].active)
        create_button("BACK", "boatyard ON", boatyard_set);
    else
        create_button("BACK", "boatyard OFF", boatyard_set);
    
    if(!waterway_layer["rapids"].active)
        create_button("BACK", "rapids ON", rapids_set);
    else
        create_button("BACK", "rapids OFF", rapids_set);
    
    if(!waterway_layer["breakwater"].active)
        create_button("BACK", "breakwater ON", breakwater_set);
    else
        create_button("BACK", "breakwater OFF", breakwater_set);
    
    if(!waterway_layer["dam"].active)
        create_button("BACK", "dam ON", dam_set);
    else
        create_button("BACK", "dam OFF", dam_set);
    
    if(!waterway_layer["riverbank"].active)
        create_button("BACK", "riverbank ON", riverbank_set);
    else
        create_button("BACK", "riverbank OFF", riverbank_set);
    
    if(!waterway_layer["drain"].active)
        create_button("BACK", "drain ON", drain_set);
    else
        create_button("BACK", "drain OFF", drain_set);  
    
    if(!waterway_layer["stream"].active)
        create_button("BACK", "stream ON", stream_set);
    else
        create_button("BACK", "stream OFF", stream_set);
    
    if(!waterway_layer["river"].active)
        create_button("BACK", "river ON", river_set);
    else
        create_button("BACK", "river OFF", river_set);
    
}

void back_create()
{
    if(!legend_state)
        create_button("Legend ON", "BACK", backmenu_handler);
    else
        create_button("Legend OFF", "BACK", backmenu_handler);
}

void legendmenu_create()
{
    if(!legend_state)
        create_button("Layers OFF", "Legend ON", legendmenu_handler);
    else
        create_button("Layers OFF", "Legend OFF", legendmenu_handler);
}

void layermenu_destroy()
{
    destroy_button("natural");
    destroy_button("waterway");
}

void naturalmenu_destroy()
{
    if(!natural_layer["water"].active)
        destroy_button("water ON");
    else
        destroy_button("water OFF");
   
    if(!natural_layer["wood"].active)
        destroy_button("wood ON");
    else
        destroy_button("wood OFF");
    
    if(!natural_layer["beach"].active)
        destroy_button("beach ON");
    else
        destroy_button("beach OFF");
    
    if(!natural_layer["land"].active)
        destroy_button("land ON");
    else
        destroy_button("land OFF");
   
    if(!natural_layer["tree"].active)
        destroy_button("tree ON");
    else
        destroy_button("tree OFF");
    
    if(!natural_layer["cliff"].active)
        destroy_button("cliff ON");
    else
        destroy_button("cliff OFF");
    
    if(!natural_layer["peak"].active)
        destroy_button("peak ON");
    else
        destroy_button("peak OFF");
    
    if(!natural_layer["wetland"].active)
        destroy_button("wetland ON");
    else
        destroy_button("wetland OFF");
    
    if(!natural_layer["scrub"].active)
        destroy_button("scrub ON");
    else
        destroy_button("scrub OFF");
    
    if(!natural_layer["heath"].active)
        destroy_button("heath ON");
    else
        destroy_button("heath OFF");
    
    if(!natural_layer["sand"].active)
        destroy_button("sand ON");
    else
        destroy_button("sand OFF");
    
    if(!natural_layer["tree_row"].active)
        destroy_button("tree_row ON");
    else
        destroy_button("tree_row OFF");
    
    if(!natural_layer["grassland"].active)
        destroy_button("grassland ON");
    else
        destroy_button("grassland OFF");
    
    if(!natural_layer["n_other"].active)
        destroy_button("n_other ON");
    else
        destroy_button("n_other OFF");
}

void waterwaymenu_destroy()
{
    if(!waterway_layer["river"].active)
        destroy_button("river ON");
    else
        destroy_button("river OFF");
   
    if(!waterway_layer["stream"].active)
        destroy_button("stream ON");
    else
        destroy_button("stream OFF");
    
    if(!waterway_layer["drain"].active)
        destroy_button("drain ON");
    else
        destroy_button("drain OFF");
    
    if(!waterway_layer["riverbank"].active)
        destroy_button("riverbank ON");
    else
        destroy_button("riverbank OFF");
   
    if(!waterway_layer["dam"].active)
        destroy_button("dam ON");
    else
        destroy_button("dam OFF");
    
    if(!waterway_layer["breakwater"].active)
        destroy_button("breakwater ON");
    else
        destroy_button("breakwater OFF");
    
    if(!waterway_layer["rapids"].active)
        destroy_button("rapids ON");
    else
        destroy_button("rapids OFF");
    
    if(!waterway_layer["boatyard"].active)
        destroy_button("boatyard ON");
    else
        destroy_button("boatyard OFF");
    
    if(!waterway_layer["ditch"].active)
        destroy_button("ditch ON");
    else
        destroy_button("ditch OFF");
}

void legendmenu_destroy()
{
    if(!legend_state)
        destroy_button("Legend ON");
    else
        destroy_button("Legend OFF");
}

void nother_set (void (*drawscreen_ptr) (void))
{
    if(!natural_layer["n_other"].active)
    {
        destroy_button("n_other ON");
        if(!natural_layer["grassland"].active)
            create_button("grassland ON", "n_other OFF", nother_set);
        else
            create_button("grassland OFF", "n_other OFF", grassland_set);
        natural_layer["n_other"].active = true;
        set_active_layers("n_other");
        set_layer_colors(nother_color);
        drawscreen();
    }
    else
    {
        destroy_button("n_other OFF");
        if(!natural_layer["grassland"].active)
            create_button("grassland ON", "n_other ON", nother_set);
        else
            create_button("grassland OFF", "n_other ON", nother_set);
        natural_layer["n_other"].active = false;
        remove_active_layers("n_other");
        remove_layer_colors(nother_color);
        drawscreen();
    }
}

void grassland_set (void (*drawscreen_ptr) (void))
{
    if(!natural_layer["grassland"].active)
    {
        destroy_button("grassland ON");
        if(!natural_layer["tree_row"].active)
            create_button("tree_row ON", "grassland OFF", grassland_set);
        else
            create_button("tree_row OFF", "grassland OFF", grassland_set);
        natural_layer["grassland"].active = true;
        set_active_layers("grassland");
        set_layer_colors(grassland_color);
        drawscreen();
    }
    else
    {
        destroy_button("grassland OFF");
        if(!natural_layer["tree_row"].active)
            create_button("tree_row ON", "grassland ON", grassland_set);
        else
            create_button("tree_row OFF", "grassland ON", grassland_set);
        natural_layer["grassland"].active = false;
        remove_active_layers("grassland");
        remove_layer_colors(grassland_color);
        drawscreen();
    }
}

void treerow_set (void (*drawscreen_ptr) (void))
{
    if(!natural_layer["tree_row"].active)
    {
        destroy_button("tree_row ON");
        if(!natural_layer["sand"].active)
            create_button("sand ON", "tree_row OFF", treerow_set);
        else
            create_button("sand OFF", "tree_row OFF", treerow_set);
        natural_layer["tree_row"].active = true;
        set_active_layers("tree_row");
        set_layer_colors(treerow_color);
        drawscreen();
    }
    else
    {
        destroy_button("tree_row OFF");
        if(!natural_layer["sand"].active)
            create_button("sand ON", "tree_row ON", treerow_set);
        else
            create_button("sand OFF", "tree_row ON", treerow_set);
        natural_layer["tree_row"].active = false;
        remove_active_layers("tree_row");
        remove_layer_colors(treerow_color);
        drawscreen();
    }
}

void sand_set (void (*drawscreen_ptr) (void))
{
    if(!natural_layer["sand"].active)
    {
        destroy_button("sand ON");
        if(!natural_layer["heath"].active)
            create_button("heath ON", "sand OFF", sand_set);
        else
            create_button("heath OFF", "sand OFF", sand_set);
        natural_layer["sand"].active = true;
        set_active_layers("sand");
        set_layer_colors(sand_color);
        drawscreen();
    }
    else
    {
        destroy_button("sand OFF");
        if(!natural_layer["heath"].active)
            create_button("heath ON", "sand ON", sand_set);
        else
            create_button("heath OFF", "sand ON", sand_set);
        natural_layer["sand"].active = false;
        remove_active_layers("sand");
        remove_layer_colors(sand_color);
        drawscreen();
    }
}

void heath_set (void (*drawscreen_ptr) (void))
{
    if(!natural_layer["heath"].active)
    {
        destroy_button("heath ON");
        if(!natural_layer["scrub"].active)
            create_button("scrub ON", "heath OFF", heath_set);
        else
            create_button("scrub OFF", "heath OFF", heath_set);
        natural_layer["heath"].active = true;
        set_active_layers("heath");
        set_layer_colors(heath_color);
        drawscreen();
    }
    else
    {
        destroy_button("heath OFF");
        if(!natural_layer["scrub"].active)
            create_button("scrub ON", "heath ON", heath_set);
        else
            create_button("scrub OFF", "heath ON", heath_set);
        natural_layer["heath"].active = false;
        remove_active_layers("heath");
        remove_layer_colors(heath_color);
        drawscreen();
    }
}

void scrub_set (void (*drawscreen_ptr) (void))
{
     if(!natural_layer["scrub"].active)
    {
        destroy_button("scrub ON");
        if(!natural_layer["wetland"].active)
            create_button("wetland ON", "scrub OFF", scrub_set);
        else
            create_button("wetland OFF", "scrub OFF", scrub_set);
        natural_layer["scrub"].active = true;
        set_active_layers("scrub");
        set_layer_colors(scrub_color);
        drawscreen();
    }
    else
    {
        destroy_button("scrub OFF");
        if(!natural_layer["wetland"].active)
            create_button("wetland ON", "scrub ON", scrub_set);
        else
            create_button("wetland OFF", "scrub ON", scrub_set);
        natural_layer["scrub"].active = false;
        remove_active_layers("scrub");
        remove_layer_colors(scrub_color);
        drawscreen();
    }
}

void wetland_set (void (*drawscreen_ptr) (void))
{
    if(!natural_layer["wetland"].active)
    {
        destroy_button("wetland ON");
        if(!natural_layer["peak"].active)
            create_button("peak ON", "wetland OFF", wetland_set);
        else
            create_button("peak OFF", "wetland OFF", wetland_set);
        natural_layer["wetland"].active = true;
        set_active_layers("wetland");
        set_layer_colors(wetland_color);
        drawscreen();
    }
    else
    {
        destroy_button("wetland OFF");
        if(!natural_layer["peak"].active)
            create_button("peak ON", "wetland ON", wetland_set);
        else
            create_button("peak OFF", "wetland ON", wetland_set);
        natural_layer["wetland"].active = false;
        remove_active_layers("wetland");
        remove_layer_colors(wetland_color);
        drawscreen();
    }
}

void peak_set (void (*drawscreen_ptr) (void))
{
    if(!natural_layer["peak"].active)
    {
        destroy_button("peak ON");
        if(!natural_layer["cliff"].active)
            create_button("cliff ON", "peak OFF", peak_set);
        else
            create_button("cliff OFF", "peak OFF", peak_set);
        natural_layer["peak"].active = true;
        set_active_layers("peak");
        set_layer_colors(peak_color);
        drawscreen();
    }
    else
    {
        destroy_button("peak OFF");
        if(!natural_layer["cliff"].active)
            create_button("cliff ON", "peak ON", peak_set);
        else
            create_button("cliff OFF", "peak ON", peak_set);
        natural_layer["peak"].active = false;
        remove_active_layers("peak");
        remove_layer_colors(peak_color);
        drawscreen();
    }
}

void cliff_set (void (*drawscreen_ptr) (void))
{
    if(!natural_layer["cliff"].active)
    {
        destroy_button("cliff ON");
        if(!natural_layer["tree"].active)
            create_button("tree ON", "cliff OFF", cliff_set);
        else
            create_button("tree OFF", "cliff OFF", cliff_set);
        natural_layer["cliff"].active = true;
        set_active_layers("cliff");
        set_layer_colors(cliff_color);
        drawscreen();
    }
    else
    {
        destroy_button("cliff OFF");
        if(!natural_layer["tree"].active)
            create_button("tree ON", "cliff ON", cliff_set);
        else
            create_button("tree OFF", "cliff ON", cliff_set);
        natural_layer["cliff"].active = false;
        remove_active_layers("cliff");
        remove_layer_colors(cliff_color);
        drawscreen();
    }
}

void tree_set (void (*drawscreen_ptr) (void))
{
    if(!natural_layer["tree"].active)
    {
        destroy_button("tree ON");
        if(!natural_layer["land"].active)
            create_button("land ON", "tree OFF", tree_set);
        else
            create_button("land OFF", "tree OFF", tree_set);
        natural_layer["tree"].active = true;
        set_active_layers("tree");
        set_layer_colors(tree_color);
        drawscreen();
    }
    else
    {
        destroy_button("tree OFF");
        if(!natural_layer["land"].active)
            create_button("land ON", "tree ON", tree_set);
        else
            create_button("land OFF", "tree ON", tree_set);
        natural_layer["tree"].active = false;
        remove_active_layers("tree");
        remove_layer_colors(tree_color);
        drawscreen();
    }
}

void land_set (void (*drawscreen_ptr) (void))
{
    if(!natural_layer["land"].active)
    {
        destroy_button("land ON");
        if(!natural_layer["beach"].active)
            create_button("beach ON", "land OFF", land_set);
        else
            create_button("beach OFF", "land OFF", land_set);
        natural_layer["land"].active = true;
        set_active_layers("land");
        set_layer_colors(land_color);
        drawscreen();
    }
    else
    {
        destroy_button("land OFF");
        if(!natural_layer["beach"].active)
            create_button("beach ON", "land ON", land_set);
        else
            create_button("beach OFF", "land ON", land_set);
        natural_layer["land"].active = false;
        remove_active_layers("land");
        remove_layer_colors(land_color);
        drawscreen();
    }
}

void beach_set (void (*drawscreen_ptr) (void))
{
    if(!natural_layer["beach"].active)
    {
        destroy_button("beach ON");
        if(!natural_layer["wood"].active)
            create_button("wood ON", "beach OFF", beach_set);
        else
            create_button("wood OFF", "beach OFF", beach_set);
        natural_layer["beach"].active = true;
        set_active_layers("beach");
        set_layer_colors(beach_color);
        drawscreen();
    }
    else
    {
        destroy_button("beach OFF");
        if(!natural_layer["wood"].active)
            create_button("wood ON", "beach ON", beach_set);
        else
            create_button("wood OFF", "beach ON", beach_set);
        natural_layer["beach"].active = false;
        remove_active_layers("beach");
        remove_layer_colors(beach_color);
        drawscreen();
    }
}

void wood_set (void (*drawscreen_ptr) (void))
{
    if(!natural_layer["wood"].active)
    {
        destroy_button("wood ON");
        if(!natural_layer["water"].active)
            create_button("water ON", "wood OFF", wood_set);
        else
            create_button("water OFF", "wood OFF", wood_set);
        natural_layer["wood"].active = true;
        set_active_layers("wood");
        set_layer_colors(wood_color);
        drawscreen();
    }
    else
    {
        destroy_button("wood OFF");
        if(!natural_layer["water"].active)
            create_button("water ON", "wood ON", wood_set);
        else
            create_button("water OFF", "wood ON", wood_set);
        natural_layer["wood"].active = false;
        remove_active_layers("wood");
        remove_layer_colors(wood_color);
        drawscreen();
    }
}

void water_set (void (*drawscreen_ptr) (void))
{
    if(!natural_layer["water"].active)
    {
        destroy_button("water ON");
        create_button("BACK", "water OFF", water_set);
        natural_layer["water"].active = true;
        set_active_layers("water");
        set_layer_colors(water_color);
        drawscreen();
    }
    else
    {
        destroy_button("water OFF");
        create_button("BACK", "water ON", water_set);
        natural_layer["water"].active = false;
        remove_active_layers("water");
        remove_layer_colors(water_color);
        drawscreen();
    }
}

void ditch_set (void (*drawscreen_ptr) (void))
{
    if(!waterway_layer["ditch"].active)
    {
        destroy_button("ditch ON");
        if(!waterway_layer["boatyard"].active)
            create_button("boatyard ON", "ditch OFF", ditch_set);
        else
            create_button("boatyard OFF", "ditch OFF", ditch_set);
        waterway_layer["ditch"].active = true;
        set_active_layers("ditch");
        set_layer_colors(ditch_color);
        drawscreen();
    }
    else
    {
        destroy_button("ditch OFF");
        if(!waterway_layer["boatyard"].active)
            create_button("boatyard ON", "ditch ON", ditch_set);
        else
            create_button("boatyard OFF", "ditch ON", ditch_set);
        waterway_layer["ditch"].active = false;
        remove_active_layers("ditch");
        remove_layer_colors(ditch_color);
        drawscreen();
    }

}

void boatyard_set (void (*drawscreen_ptr) (void))
{
    if(!waterway_layer["boatyard"].active)
    {
        destroy_button("boatyard ON");
        if(!waterway_layer["rapids"].active)
            create_button("rapids ON", "boatyard OFF", boatyard_set);
        else
            create_button("rapids OFF", "boatyard OFF", boatyard_set);
        waterway_layer["boatyard"].active = true;
        set_active_layers("boatyard");
        set_layer_colors(boatyard_color);
        drawscreen();
    }
    else
    {
        destroy_button("boatyard OFF");
        if(!waterway_layer["rapids"].active)
            create_button("rapids ON", "boatyard ON", boatyard_set);
        else
            create_button("rapids OFF", "boatyard ON", boatyard_set);
        waterway_layer["boatyard"].active = false;
        remove_active_layers("boatyard");
        remove_layer_colors(boatyard_color);
        drawscreen();
    }
}

void rapids_set (void (*drawscreen_ptr) (void))
{
    if(!waterway_layer["rapids"].active)
    {
        destroy_button("rapids ON");
        if(!waterway_layer["breakwater"].active)
            create_button("breakwater ON", "rapids OFF", rapids_set);
        else
            create_button("breakwater OFF", "rapids OFF", rapids_set);
        waterway_layer["rapids"].active = true;
        set_active_layers("rapids");
        set_layer_colors(rapids_color);
        drawscreen();
    }
    else
    {
        destroy_button("rapids OFF");
        if(!waterway_layer["breakwater"].active)
            create_button("breakwater ON", "rapids ON", rapids_set);
        else
            create_button("breakwater OFF", "rapids ON", rapids_set);
        waterway_layer["rapids"].active = false;
        remove_active_layers("rapids");
        remove_layer_colors(rapids_color);
        drawscreen();
    }
}

void breakwater_set (void (*drawscreen_ptr) (void))
{
    if(!waterway_layer["breakwater"].active)
    {
        destroy_button("breakwater ON");
        if(!waterway_layer["dam"].active)
            create_button("dam ON", "breakwater OFF", breakwater_set);
        else
            create_button("dam OFF", "breakwater OFF", breakwater_set);
        waterway_layer["breakwater"].active = true;
        set_active_layers("breakwater");
        set_layer_colors(breakwater_color);
        drawscreen();
    }
    else
    {
        destroy_button("breakwater OFF");
        if(!waterway_layer["dam"].active)
            create_button("dam ON", "breakwater ON", breakwater_set);
        else
            create_button("dam OFF", "breakwater ON", breakwater_set);
        waterway_layer["breakwater"].active = false;
        remove_active_layers("breakwater");
        remove_layer_colors(breakwater_color);
        drawscreen();
    }
}

void dam_set (void (*drawscreen_ptr) (void))
{
    if(!waterway_layer["dam"].active)
    {
        destroy_button("dam ON");
        if(!waterway_layer["riverbank"].active)
            create_button("riverbank ON", "dam OFF", dam_set);
        else
            create_button("riverbank OFF", "dam OFF", dam_set);
        waterway_layer["dam"].active = true;
        set_active_layers("dam");
        set_layer_colors(dam_color);
        drawscreen();
    }
    else
    {
        destroy_button("dam OFF");
        if(!waterway_layer["riverbank"].active)
            create_button("riverbank ON", "dam ON", dam_set);
        else
            create_button("riverbank OFF", "dam ON", dam_set);
        waterway_layer["dam"].active = false;
        remove_active_layers("dam");
        remove_layer_colors(dam_color);
        drawscreen();
    }
}

void riverbank_set (void (*drawscreen_ptr) (void))
{
    if(!waterway_layer["riverbank"].active)
    {
        destroy_button("riverbank ON");
        if(!waterway_layer["drain"].active)
            create_button("drain ON", "riverbank OFF", riverbank_set);
        else
            create_button("drain OFF", "riverbank OFF", riverbank_set);
        waterway_layer["riverbank"].active = true;
        set_active_layers("riverbank");
        set_layer_colors(riverbank_color);
        drawscreen();
    }
    else
    {
        destroy_button("riverbank OFF");
        if(!waterway_layer["drain"].active)
            create_button("drain ON", "riverbank ON", riverbank_set);
        else
            create_button("drain OFF", "riverbank ON", riverbank_set);
        waterway_layer["riverbank"].active = false;
        remove_active_layers("riverbank");
        remove_layer_colors(riverbank_color);
        drawscreen();
    }
}

void drain_set (void (*drawscreen_ptr) (void))
{
    if(!waterway_layer["drain"].active)
    {
        destroy_button("drain ON");
        if(!waterway_layer["stream"].active)
            create_button("stream ON", "drain OFF", drain_set);
        else
            create_button("stream OFF", "drain OFF", drain_set);
        waterway_layer["drain"].active = true;
        set_active_layers("drain");
        set_layer_colors(drain_color);
        drawscreen();
    }
    else
    {
        destroy_button("drain OFF");
        if(!waterway_layer["stream"].active)
            create_button("stream ON", "drain ON", drain_set);
        else
            create_button("stream OFF", "drain ON", drain_set);
        waterway_layer["drain"].active = false;
        remove_active_layers("drain");
        remove_layer_colors(drain_color);
        drawscreen();
    }
}

void stream_set (void (*drawscreen_ptr) (void))
{
    if(!waterway_layer["stream"].active)
    {
        destroy_button("stream ON");
        if(!waterway_layer["river"].active)
            create_button("river ON", "stream OFF", stream_set);
        else
            create_button("river OFF", "stream OFF", stream_set);
        waterway_layer["stream"].active = true;
        set_active_layers("stream");
        set_layer_colors(stream_color);
        drawscreen();
        //ayer_stream();
        //draw_minimap(get_scale());
    }
    else
    {
        destroy_button("stream OFF");
        if(!waterway_layer["river"].active)
            create_button("river ON", "stream ON", stream_set);
        else
            create_button("river OFF", "stream ON", stream_set);
        waterway_layer["stream"].active = false;
        remove_active_layers("stream");
        remove_layer_colors(stream_color);
        drawscreen();
    }
}

void river_set (void (*drawscreen_ptr) (void))
{
    if(!waterway_layer["river"].active)
    {
        destroy_button("river ON");
        create_button("BACK", "river OFF", river_set);
        waterway_layer["river"].active = true;
        drawscreen();
        set_active_layers("river");
        set_layer_colors(river_color);
    }
    else
    {
        destroy_button("river OFF");
        create_button("BACK", "river ON", river_set);
        waterway_layer["river"].active = false; 
        remove_active_layers("river");
        remove_layer_colors(river_color);
        drawscreen();
    }
}

void layer_river()
{
    unsigned num = waterway_layer["river"].id.size();
    
    setcolor(river_color);
    
    int zlvl = getZoomLevel();
    if(zlvl < 3)
        setlinewidth(1.5);
    else if(zlvl < 5)
        setlinewidth(2.5);
    else setlinewidth(3.5);
    
    for(unsigned i = 0; i < num ; ++i)
    {
        
        unsigned id = waterway_layer["river"].id[i];
        unsigned num_point = getFeaturePointCount(id) - 1;
        
        for(unsigned j = 0; j < num_point; ++j)
        {
            LatLon point_one = getFeaturePoint(id, j);
            LatLon point_two = getFeaturePoint(id, j+1);    
            
            t_point p_one = get_coordinate_from_LatLon (point_one);
            t_point p_two = get_coordinate_from_LatLon (point_two);
            
            drawline(p_one, p_two);
        }
    }
}

void layer_stream()
{
    unsigned num = waterway_layer["stream"].id.size();
    
    setcolor(stream_color);
    
    int zlvl = getZoomLevel();
    if(zlvl < 3)
        setlinewidth(1.5);
    else if(zlvl < 5)
        setlinewidth(2.5);
    else setlinewidth(3.5);
    
    for(unsigned i = 0; i < num ; ++i)
    {
        
        unsigned id = waterway_layer["stream"].id[i];
        unsigned num_point = getFeaturePointCount(id) - 1;
        
        for(unsigned j = 0; j < num_point; ++j)
        {
            LatLon point_one = getFeaturePoint(id, j);
            LatLon point_two = getFeaturePoint(id, j+1);    
            
            t_point p_one = get_coordinate_from_LatLon (point_one);
            t_point p_two = get_coordinate_from_LatLon (point_two);
            
            drawline(p_one, p_two);
        }
    }
}

void layer_drain()
{
    unsigned num = waterway_layer["drain"].id.size();
    
    setcolor(drain_color);
    
    int zlvl = getZoomLevel();
    if(zlvl < 3)
        setlinewidth(1.5);
    else if(zlvl < 5)
        setlinewidth(2.5);
    else setlinewidth(3.5);
    
    for(unsigned i = 0; i < num ; ++i)
    {
        
        unsigned id = waterway_layer["drain"].id[i];
        unsigned num_point = getFeaturePointCount(id) - 1;
        
        for(unsigned j = 0; j < num_point; ++j)
        {
            LatLon point_one = getFeaturePoint(id, j);
            LatLon point_two = getFeaturePoint(id, j+1);    
            
            t_point p_one = get_coordinate_from_LatLon (point_one);
            t_point p_two = get_coordinate_from_LatLon (point_two);
            
            drawline(p_one, p_two);
        }
    }
}

void layer_riverbank()
{
    unsigned num = waterway_layer["riverbank"].id.size();
    
    setcolor(riverbank_color);
    
    for(unsigned i = 0; i < num; ++i)
    {
        unsigned id = waterway_layer["riverbank"].id[i];
        unsigned num_point = getFeaturePointCount(id);
        t_point points[num_point];
        
        for(unsigned j = 0; j < num_point; ++j)
        {
            LatLon point = getFeaturePoint(id, j);
            points[j] = get_coordinate_from_LatLon(point);                     
        }
        
        fillpoly(points, num_point);
    }
}

void layer_dam()
{
    unsigned num = waterway_layer["dam"].id.size();
    
    setcolor(dam_color);
    
    int zlvl = getZoomLevel();
    if(zlvl < 3)
        setlinewidth(2);
    else if(zlvl < 5)
        setlinewidth(4);
    else setlinewidth(5);
    
    for(unsigned i = 0; i < num ; ++i)
    {
        
        unsigned id = waterway_layer["dam"].id[i];
        unsigned num_point = getFeaturePointCount(id) - 1;
        
        for(unsigned j = 0; j < num_point; ++j)
        {
            LatLon point_one = getFeaturePoint(id, j);
            LatLon point_two = getFeaturePoint(id, j+1);    
            
            t_point p_one = get_coordinate_from_LatLon (point_one);
            t_point p_two = get_coordinate_from_LatLon (point_two);
            
            drawline(p_one, p_two);
        }
    }
}

void layer_breakwater()
{   
    unsigned num = waterway_layer["breakwater"].id.size();
    
    setcolor(breakwater_color);
    
    int zlvl = getZoomLevel();
    if(zlvl < 3)
        setlinewidth(2);
    else if(zlvl < 5)
        setlinewidth(4);
    else setlinewidth(5);
    
    for(unsigned i = 0; i < num ; ++i)
    {
        
        unsigned id = waterway_layer["breakwater"].id[i];
        unsigned num_point = getFeaturePointCount(id) - 1;
        
        for(unsigned j = 0; j < num_point; ++j)
        {
            LatLon point_one = getFeaturePoint(id, j);
            LatLon point_two = getFeaturePoint(id, j+1);    
            
            t_point p_one = get_coordinate_from_LatLon (point_one);
            t_point p_two = get_coordinate_from_LatLon (point_two);
            
            drawline(p_one, p_two);
        }
    }
}

void layer_rapids()
{   
    unsigned num = waterway_layer["rapids"].id.size();
    
    setcolor(rapids_color);
    
    int zlvl = getZoomLevel();
    if(zlvl < 3)
        setlinewidth(1.5);
    else if(zlvl < 5)
        setlinewidth(2.5);
    else setlinewidth(3.5);
    
    for(unsigned i = 0; i < num ; ++i)
    {
        
        unsigned id = waterway_layer["rapids"].id[i];
        unsigned num_point = getFeaturePointCount(id) - 1;
        
        for(unsigned j = 0; j < num_point; ++j)
        {
            LatLon point_one = getFeaturePoint(id, j);
            LatLon point_two = getFeaturePoint(id, j+1);    
            
            t_point p_one = get_coordinate_from_LatLon (point_one);
            t_point p_two = get_coordinate_from_LatLon (point_two);
            
            drawline(p_one, p_two);
        }
    }
}

void layer_ditch()
{
    unsigned num = waterway_layer["ditch"].id.size();
    
    setcolor(ditch_color);
    
    int zlvl = getZoomLevel();
    if(zlvl < 3)
        setlinewidth(1.5);
    else if(zlvl < 5)
        setlinewidth(2.5);
    else setlinewidth(3.5);
    
    for(unsigned i = 0; i < num ; ++i)
    {
        
        unsigned id = waterway_layer["ditch"].id[i];
        unsigned num_point = getFeaturePointCount(id) - 1;
        
        for(unsigned j = 0; j < num_point; ++j)
        {
            LatLon point_one = getFeaturePoint(id, j);
            LatLon point_two = getFeaturePoint(id, j+1);    
            
            t_point p_one = get_coordinate_from_LatLon (point_one);
            t_point p_two = get_coordinate_from_LatLon (point_two);
            
            drawline(p_one, p_two);
        }
    }
}

void layer_boatyard()
{
    unsigned num = waterway_layer["boatyard"].id.size();
    
    setcolor(boatyard_color);
    
    for(unsigned i = 0; i < num; ++i)
    {
        unsigned id = waterway_layer["boatyard"].id[i];
        unsigned num_point = getFeaturePointCount(id);
        t_point points[num_point];
        
        for(unsigned j = 0; j < num_point; ++j)
        {
            LatLon point = getFeaturePoint(id, j);
            points[j] = get_coordinate_from_LatLon(point);                     
        }
        
        fillpoly(points, num_point);
    }
}

void layer_water()
{
    unsigned num = natural_layer["water"].id.size();
    
    setcolor(water_color);
    
    for(unsigned i = 0; i < num; ++i)
    {
        unsigned id = natural_layer["water"].id[i];
        unsigned num_point = getFeaturePointCount(id);
        t_point points[num_point];
        
        for(unsigned j = 0; j < num_point; ++j)
        {
            LatLon point = getFeaturePoint(id, j);
            points[j] = get_coordinate_from_LatLon(point);                     
        }
        
        fillpoly(points, num_point);
    }
}

void layer_wood()
{
    unsigned num = natural_layer["wood"].id.size();
    
    setcolor(wood_color);
    
    for(unsigned i = 0; i < num; ++i)
    {
        unsigned id = natural_layer["wood"].id[i];
        unsigned num_point = getFeaturePointCount(id);
        t_point points[num_point];
        
        for(unsigned j = 0; j < num_point; ++j)
        {
            LatLon point = getFeaturePoint(id, j);
            points[j] = get_coordinate_from_LatLon(point);                     
        }
        
        fillpoly(points, num_point);
    }
}

void layer_beach()
{
    unsigned num = natural_layer["beach"].id.size();
    
    setcolor(beach_color);
    
    for(unsigned i = 0; i < num; ++i)
    {
        unsigned id = natural_layer["beach"].id[i];
        unsigned num_point = getFeaturePointCount(id);
        t_point points[num_point];
        
        for(unsigned j = 0; j < num_point; ++j)
        {
            LatLon point = getFeaturePoint(id, j);
            points[j] = get_coordinate_from_LatLon(point);                     
        }
        
        fillpoly(points, num_point);
    }
}

void layer_land()
{
    unsigned num = natural_layer["land"].id.size();
    
    setcolor(land2_color);
    
    for(unsigned i = 0; i < num; ++i)
    {
        unsigned id = natural_layer["land"].id[i];
        unsigned num_point = getFeaturePointCount(id);
        t_point points[num_point];
        
        for(unsigned j = 0; j < num_point; ++j)
        {
            LatLon point = getFeaturePoint(id, j);
            points[j] = get_coordinate_from_LatLon(point);                     
        }
        
        fillpoly(points, num_point);
    }
}

void layer_tree()
{
    unsigned num = natural_layer["tree"].id.size();
    setcolor(tree_color);
    
    for(unsigned i = 0; i < num; ++ i)
    {
        unsigned id = natural_layer["tree"].id[i];
        //unsigned num_point = getFeaturePointCount(id);
        
        LatLon point = getFeaturePoint(id, 0);
        
        t_point centre = get_coordinate_from_LatLon(point);
        //void fillarc(float xcen, float ycen, float rad, float startang,
        //float angextent);
        
        
        fillarc(centre,150*get_scale()*(getZoomLevel()+1), 0, 360);
    }
}

void layer_cliff()
{
    unsigned num = natural_layer["cliff"].id.size();
    setcolor(cliff_color);
    
    for(unsigned i = 0; i < num; ++ i)
    {
        unsigned id = natural_layer["cliff"].id[i];
       //unsigned num_point = getFeaturePointCount(id);
        
        LatLon point = getFeaturePoint(id, 0);
        
        t_point centre = get_coordinate_from_LatLon(point);
        //void fillarc(float xcen, float ycen, float rad, float startang,
        //float angextent);
        
        
        fillarc(centre,150*get_scale()*(getZoomLevel()+1), 0, 360);
    }
}

void layer_peak()
{
    unsigned num = natural_layer["peak"].id.size();
    setcolor(peak_color);
    
    for(unsigned i = 0; i < num; ++ i)
    {
        unsigned id = natural_layer["peak"].id[i];
        //unsigned num_point = getFeaturePointCount(id);
        
        LatLon point = getFeaturePoint(id, 0);
        
        t_point centre = get_coordinate_from_LatLon(point);
        //void fillarc(float xcen, float ycen, float rad, float startang,
        //float angextent);
        
        
        fillarc(centre,150*get_scale()*(getZoomLevel()+1), 0, 360);
    }
}

void layer_wetland()
{
    unsigned num = natural_layer["wetland"].id.size();
    
    setcolor(wetland_color);
    
    for(unsigned i = 0; i < num; ++i)
    {
        unsigned id = natural_layer["wetland"].id[i];
        unsigned num_point = getFeaturePointCount(id);
        t_point points[num_point];
        
        for(unsigned j = 0; j < num_point; ++j)
        {
            LatLon point = getFeaturePoint(id, j);
            points[j] = get_coordinate_from_LatLon(point);                     
        }
        
        fillpoly(points, num_point);
    }
}

void layer_scrub()
{
    unsigned num = natural_layer["scrub"].id.size();
    
    setcolor(scrub_color);
    
    for(unsigned i = 0; i < num; ++i)
    {
        unsigned id = natural_layer["scrub"].id[i];
        unsigned num_point = getFeaturePointCount(id);
        t_point points[num_point];
        
        for(unsigned j = 0; j < num_point; ++j)
        {
            LatLon point = getFeaturePoint(id, j);
            points[j] = get_coordinate_from_LatLon(point);                     
        }
        
        fillpoly(points, num_point);
    }
}

void layer_heath()
{
    unsigned num = natural_layer["heath"].id.size();
    
    setcolor(heath_color);
    
    for(unsigned i = 0; i < num; ++i)
    {
        unsigned id = natural_layer["heath"].id[i];
        unsigned num_point = getFeaturePointCount(id);
        t_point points[num_point];
        
        for(unsigned j = 0; j < num_point; ++j)
        {
            LatLon point = getFeaturePoint(id, j);
            points[j] = get_coordinate_from_LatLon(point);                     
        }
        
        fillpoly(points, num_point);
    }
}

void layer_sand()
{
    unsigned num = natural_layer["sand"].id.size();
    
    setcolor(sand_color);
    
    for(unsigned i = 0; i < num; ++i)
    {
        unsigned id = natural_layer["sand"].id[i];
        unsigned num_point = getFeaturePointCount(id);
        t_point points[num_point];
        
        for(unsigned j = 0; j < num_point; ++j)
        {
            LatLon point = getFeaturePoint(id, j);
            points[j] = get_coordinate_from_LatLon(point);                     
        }
        
        fillpoly(points, num_point);
    }
}

void layer_treerow()
{
    unsigned num = natural_layer["tree_row"].id.size();
    setcolor(treerow_color);
    
    for(unsigned i = 0; i < num; ++ i)
    {
        unsigned id = natural_layer["tree_row"].id[i];
        //unsigned num_point = getFeaturePointCount(id);
        
        LatLon point = getFeaturePoint(id, 0);
        
        t_point centre = get_coordinate_from_LatLon(point);
        //void fillarc(float xcen, float ycen, float rad, float startang,
        //float angextent);
        
        
        fillarc(centre,150*get_scale()*(getZoomLevel()+1), 0, 360);
    }
}

void layer_grassland()
{
    unsigned num = natural_layer["grassland"].id.size();
    
    setcolor(grassland_color);
    
    for(unsigned i = 0; i < num; ++i)
    {
        unsigned id = natural_layer["grassland"].id[i];
        unsigned num_point = getFeaturePointCount(id);
        t_point points[num_point];
        
        for(unsigned j = 0; j < num_point; ++j)
        {
            LatLon point = getFeaturePoint(id, j);
            points[j] = get_coordinate_from_LatLon(point);                     
        }
        
        fillpoly(points, num_point);
    }
}

void layer_nother()
{
    unsigned num = natural_layer["n_other"].id.size();
    
    setcolor(nother_color);
    
    for(unsigned i = 0; i < num; ++i)
    {
        unsigned id = natural_layer["n_other"].id[i];
        unsigned num_point = getFeaturePointCount(id);
        t_point points[num_point];
        
        for(unsigned j = 0; j < num_point; ++j)
        {
            LatLon point = getFeaturePoint(id, j);
            points[j] = get_coordinate_from_LatLon(point);                     
        }
        
        fillpoly(points, num_point);
    }
}

void set_active_layers(string _layer){
    legend.active_layers.push_back(_layer);
}

void remove_active_layers(string _layer){
    int it = 0;
    
    for(unsigned i = 0; i < legend.active_layers.size(); ++i)
    {
        if(legend.active_layers[i] == _layer)
        {
            it = i;
            break;
        }
    }
    
    legend.active_layers.erase(legend.active_layers.begin() + it);
}

void set_layer_colors(t_color color){
    legend.layer_colors.push_back(color);
}

void remove_layer_colors(t_color color){
    int it = 0;
    
    for(unsigned i = 0; i < legend.layer_colors.size(); ++i)
    {
        if(legend.layer_colors[i] == color)
        {
            it = i;
            break;
        }
    }
    
    legend.layer_colors.erase(legend.layer_colors.begin() + it);
}


void legend_click_handler(){
    
    if(legend.more && legend.down_bounds.intersects(mouse_location_current))
    {
        if(legend.scroll_count + legend.num_div < (int)legend.active_layers.size()
                && legend_state)
        {
            legend.scroll_count = legend.scroll_count + 1;
            draw_layermenu(get_scale());
        }
    }
    else if(legend.scroll_count > 0 && legend.up_bounds.intersects(mouse_location_current)
            && legend_state)
    {
        legend.scroll_count = legend.scroll_count - 1;
        draw_layermenu(get_scale());
    }
    else if(legend.more /*&& dir.scroll_bounds.intersects(click_local)*/ && left_hold && legend_state)
    {
        float diff = click_local.y - mouse_location_current.y;
        if(diff > 0 && diff > legend.sdiv_offset)
        {
            if(legend.scroll_count + legend.num_div >= (int)legend.active_layers.size()) return;
            else
            {
                legend.scroll_count = legend.scroll_count + 1;
                click_local = mouse_location_current;
                legend.up_backdrop = true;
                legend.down_backdrop = true;
                draw_layermenu(get_scale());
            }
        }
        else if(diff < 0 && diff*-1 > legend.sdiv_offset)
        {
            if(legend.scroll_count == 0) return;
            else
            {
                legend.scroll_count = legend.scroll_count - 1;
                click_local = mouse_location_current;
                legend.up_backdrop = true;
                legend.down_backdrop = true;
                draw_layermenu(get_scale());
            }
        }
    }
    
}

std::string ConvertToCaps(std::string input)
{
    string returningString="";
   // cout << input << endl;
    for(int i=0;i<(int)input.size();i++)
    {
        if(input[i]>=97 && input[i]<=122 && input[i]!='&')
        {
            char lowercase= input[i]-32;
            returningString+=lowercase;
        }else {
            returningString+=input[i];
        }
    }
    return returningString;
}

std::string switchWords(std::string input)
{
    string returningString="";
    string firstword="", secondword=input;
    cout << input << endl;
   // size_t pos;
    bool checkAmpersand = false;
    for(size_t i=0;i<input.size();i++)
    {
        /*if(input[input.size()-i-1]!='&')
        {
            secondword+=input[input.size()-i-1];
        }*/
        //cout << input[i] << endl;
        //cout << checkAmpersand << endl;
        if(input[i]=='&')
        {
            checkAmpersand =true;
            break;
        } else{
            //secondword = input;
          //  pos = i;
            firstword+=input[i];
            secondword.erase(secondword.begin());
        }
    }
    if(checkAmpersand == false)
    {
        return returningString;
    }
    //cout << "what is wrong " << secondword <<endl;
    secondword.erase(secondword.begin()); //gets rid of &
    secondword.erase(secondword.begin()); //gets rid of the space after &
    returningString = secondword + " & " + firstword;
    return returningString;
}

void draw_directions_load()
{
    for(unsigned i = 0; i < dir.dir_parsed.size(); ++i)
        dir.dir_parsed[i].clear();
    dir.dir_parsed.clear();
    
    dir.index.clear();
    
    dir_data init;
    init.type = 0;
    init.street = "Directions";
    init.direction = "\0";
    init.distance = -1;
    
    //cout << path_data.size() << endl;
    
    for(unsigned i = 0; i < path_data.size(); ++i)
    {
        //cout << path_data[i] << endl;
        init.distance = init.distance + find_street_segment_length(path_data[i]);
    }
    
    dir.dir.push_back(init);
    
    dir_data temp_dir;
    
    vector<unsigned> path_segment;
    
    
    double distance = 0;
    
    for(unsigned i = 0; i < path_data.size(); ++i)
    {
        unsigned street_id = getStreetSegmentStreetID(path_data[i]);
        string street = getStreetName(street_id);
        
        //straight initially
        if(i == 0)
        {   
            string start_name = getIntersectionName(path_intersection[0]);
            
            dir_data temp_dir_start{
                1, start_name, "\0", -1
            };
            dir.dir.push_back(temp_dir_start);
            
            temp_dir.type = 2;
            //cout << street << endl;
            temp_dir.street = street;
            
            LatLon x = getIntersectionPosition(path_intersection[0]);
            LatLon y = getIntersectionPosition(path_intersection[1]);
            
            double lat_diff = (y.lat - x.lat);
            double lon_diff = (y.lon - x.lon);
            
            //going north/south
            if(fabs(lat_diff) > fabs(lon_diff))
            {
                if(lat_diff < 0) temp_dir.direction = "S";
                else temp_dir.direction = "N";
            }
            else
            {
                if(lon_diff < 0) temp_dir.direction = "W";
                else temp_dir.direction = "E";
            }
            
//            cout << "begin at " <<  start_name << endl;
            //cout << "head ";
            //cout << directions[i] << " on ";
            //cout << street << endl;
            
            stringstream parse(street);
            vector<string> parse_out{istream_iterator<string>{parse}, istream_iterator<string>{}};
            dir.dir_parsed.push_back(parse_out);
            dir.index.push_back(i);
            
            path_segment.push_back(path_data[i]);
            distance = distance + find_street_segment_length(path_data[i]);
        }
        else
        {
            
            unsigned street_idp = getStreetSegmentStreetID(path_data[i-1]);
            //cout << getStreetName(street_id) << " " << getStreetName(street_idp) << endl;
            //same direction and street being travelled
            if(/*getStreetName(street_id) == getStreetName(street_idp) ||*/ street_id == street_idp)//directions[i] == directions[i-1])
            {
                    distance = distance + find_street_segment_length(path_data[i]);
                    path_segment.push_back(path_data[i]);
            }
            //new direction
            else
            {
                //not same street 
                if(street_id != street_idp /*&& getStreetName(street_id) != getStreetName(street_idp)*/)
                {
                    temp_dir.type = 3;
                    temp_dir.street = street;
                    temp_dir.direction = directions[i];
                    distance = distance + find_street_segment_length(path_data[i]);
                    //cout << "turn "<<directions[i] << " onto ";
                    //cout << street << endl;
                    stringstream parse(street);
                    vector<string> parse_out{istream_iterator<string>{parse}, istream_iterator<string>{}};
                    dir.dir_parsed.push_back(parse_out);
                    dir.index.push_back(i);
                    path_segment.push_back(path_data[i]);
                    //cout << parse_out[0] << endl;
                    //cout << path_segment.size() << endl;
                    //temp_dir.path_seg = path_segment;
                }
                //same street
                else
                {
                    distance = distance + find_street_segment_length(path_data[i]);
                    path_segment.push_back(path_data[i]);
//                    cout << "proceed on " << street << endl;
                }
            }
        }
        
        if(i+1 < path_data.size() )
        {
            //path_segment.push_back(path_data[i]);
            if(getStreetSegmentStreetID(path_data[i+1]) != street_id)
            {
                //cout << temp_dir.street << endl;
                //cout << path_segment.size() << endl;
                temp_dir.path_seg = path_segment;
                temp_dir.distance = distance;
                dir.dir.push_back(temp_dir);
//                cout << "for " << (int)distance << endl;
                distance = 0;
                path_segment.clear();
            }
        }
        else if(i+1 == path_data.size())
        {
            //path_segment.push_back(path_data[i]);
            //cout << temp_dir.street << endl;
            temp_dir.path_seg = path_segment;
            temp_dir.distance = distance;
            dir.dir.push_back(temp_dir);
//            cout << "for " << (int)distance << endl;
            distance = 0;
            path_segment.clear();
        }
        
        if(i == path_data.size() - 1)
        {   
            string dest_name = getIntersectionName(path_intersection[path_intersection.size() -1]);
            dir_data temp_dir_end{
                4, dest_name, "\0", -1
            };
            dir.dir.push_back(temp_dir_end);
//            cout << "arrive at destination " << dest_name;
//            cout << endl;
        }
    }
    
    /*cout << dir.index.size() << endl;
    for(unsigned i = 0; i < dir.index.size(); ++i)
    {
        cout << dir.index[i] << " for " << i << endl;
    }*/
    
    //cout << dir.dir.size() - 2 << endl;
}

void draw_directions()
{   
    //determine visible world
    t_bound_box visible_world = get_visible_world ();
    
   //float height = visible_world.get_height()*0.5;
    float width = minimap_bounds.get_width();
    float offset =  sqrt(pow((double)(visible_world.top() - visible_world.bottom()),(double)2) +
                    pow((double)(visible_world.right() - visible_world.left()),(double)2.0))*0.01;
    
    
    //sets direction drawing coordinates to a static position, scales
    float directions_bottom = minimap_bounds.top() + offset;
    float directions_top = visible_world.top() - offset;
    float directions_right = minimap_bounds.right();
    float directions_left = directions_right - width;
    
    //initialize directions bounds
    dir_bounds = t_bound_box(directions_left, directions_bottom,directions_right,directions_top);
    
    dir.header_offset = dir_bounds.get_height()*0.1;
    
    t_bound_box header(directions_left, directions_top - dir.header_offset, 
           directions_right, directions_top);
    
    dir.header = header;
          
    dir.div_offset = (header.bottom() - dir_bounds.bottom())/dir.num_div;
    
    if(dir.hide) return;
    if(dir.spec)
    {
        draw_directions_exspec_parse(posi);
        return;
    }
    
    
    //draw background of legend (white)
    setcolor(255,255,255);
    fillrect(dir_bounds);
    

    //draw first box for header
    setcolor(53,161,255);

    fillrect(header);
    
    setcolor(219,249,255);
    setlinewidth(1.5);
    for(int i = 0; i < (int)dir.dir_parsed.size() + 1 && i < (int)dir.num_div; ++i)
    {
        if(i == 0) continue;
        drawline(directions_left, header.bottom() - dir.div_offset*i, directions_right,
                header.bottom() - dir.div_offset*i);
    }
    
    //initialize scroll bar arrow parameters 
    dir.up_center = t_point(dir_bounds.right() - dir_bounds.get_width()*0.05,
                    dir_bounds.top() - dir.header_offset - dir_bounds.get_width()*0.05);
    
    dir.down_center = t_point(dir_bounds.right() - dir_bounds.get_width()*0.05,
                dir_bounds.bottom() + dir_bounds.get_width()*0.06);  
    
    dir.up_bounds = t_bound_box(dir.up_center.x - dir_bounds.get_width()*0.05,
                            dir.up_center.y - dir_bounds.get_width()*0.05,
                            dir.up_center.x + dir_bounds.get_width()*0.05,
                            dir.up_center.y + dir_bounds.get_width()*0.05);
    
    dir.down_bounds = t_bound_box(dir.down_center.x - dir_bounds.get_width()*0.05,
                            dir.down_center.y - dir_bounds.get_width()*0.06,
                            dir.down_center.x + dir_bounds.get_width()*0.05,
                            dir.down_center.y + dir_bounds.get_width()*0.04);
    
    dir.scroll_bounds = t_bound_box(dir.up_bounds.left(), dir.down_bounds.top(),
                             dir.up_bounds.right(), dir.up_bounds.bottom());
    
    
    if((int)dir.dir.size() - 2 > dir.num_div + 1)
    {
        draw_directions_sbar();
    }
    
    draw_directions_helper(dir.div_offset, header);
    
    //draw legend border
    setcolor(0,0,0);
    setlinewidth(3);
    drawrect(dir_bounds);
}

void draw_directions_sbar()
{
    setcolor(255,255,255);
    t_bound_box total_sbar(dir.up_bounds.left(), dir.down_bounds.bottom(),
            dir.up_bounds.right(), dir.up_bounds.top());
    fillrect(total_sbar);
        
    int scroll_div = dir.dir.size() - 2;
    dir.sdiv_offset = dir.scroll_bounds.get_height()/scroll_div;
    
        
    if(dir.up_backdrop)
    {
        setcolor(171,168,168);
        fillrect(dir.up_bounds);
    }
    if(dir.down_backdrop)
    {
        setcolor(171,168,168);
        fillrect(dir.down_bounds);
    }

    //draw scrollbar
    setcolor(0,0,0);

    t_point triangle[3];

    t_point point0(dir.up_center.x, dir.up_center.y + dir_bounds.get_width()*0.03);
    triangle[0] = point0;
    t_point point1(dir.up_center.x + dir_bounds.get_width()*0.03, dir.up_center.y);
    triangle[1] = point1;
    t_point point2(dir.up_center.x - dir_bounds.get_width()*0.03, dir.up_center.y);
    triangle[2] = point2;

    fillpoly(triangle, 3); 

    t_point point3(dir.down_center.x, dir.down_center.y - dir_bounds.get_width()*0.03);
    triangle[0] = point3;
    t_point point4(dir.down_center.x + dir_bounds.get_width()*0.03, dir.down_center.y);
    triangle[1] = point4;
    t_point point5(dir.down_center.x - dir_bounds.get_width()*0.03, dir.down_center.y);
    triangle[2] = point5;

    fillpoly(triangle, 3);

    //setcolor(53,161,255);
    setcolor(scrollbar_color);
    t_bound_box scrollbar_bounds(dir.scroll_bounds.left(), dir.scroll_bounds.bottom() 
            + dir.sdiv_offset*(dir.dir.size() - 3 - dir.num_div - dir.scroll_count), 
            dir.scroll_bounds.right(), dir.scroll_bounds.top() - dir.sdiv_offset*dir.scroll_count);
    dir.bar_bounds = scrollbar_bounds;
    fillrect(scrollbar_bounds);
    
    //draw legend border
    setcolor(0,0,0);
    setlinewidth(3);
    drawrect(dir_bounds);
        
}

void draw_directions_helper(float div_offset, t_bound_box header)
{
    unsigned num_dir = dir.dir.size();
    
    if((int)num_dir > dir.num_div +2)
    {
        dir.more = true;
    }
    else dir.more = false;
 
    for(int i = 0, h = dir.scroll_count +1; (int)i < dir.num_div +1 && h < (int)dir.dir.size(); ++i, ++h)
    {
        dir_data curr;
        if(i == 0)
        {
            curr = dir.dir[0];
        }
        else curr = dir.dir[h];
        
        //if(curr.type == 1) curr = dir.dir[h+1];
        
        if(curr.type == 0)
        {
            setcolor(255,255,255);          
            
            //draw directions header text
            t_bound_box temp(header.left() - header.get_width()*0.23, header.bottom(),
                    header.right() - header.get_width()*0.23, header.top());
            setfontsize((res.top_height/BASE_RES)*dir.main_font_size);
            drawtext_in(temp, curr.street);
            
            //draw total time for path
            t_bound_box time_info(header.left() + header.get_width()*0.23, header.bottom() +
                    header.get_height()*0.33, header.right() + header.get_width()*0.23, header.top());
            
            string time = to_string((int)compute_path_travel_time(path_data)) + " min";
            setfontsize((res.top_height/BASE_RES)*dir.minor_font_size);
            drawtext_in(time_info, time);
            
            //draw total distance 
            t_bound_box dist_info(header.left() + header.get_width()*0.23, header.bottom(),
                    header.right() + header.get_width()*0.23, header.top()- header.get_height()*0.33);
            
            string dist;
            
            if(curr.distance < 1000)
            {
                dist = to_string((int)curr.distance) + " m";
            }
            else
            {
                stringstream _dist;
                _dist << fixed << setprecision(2) << curr.distance/1000;
                dist = _dist.str();
                dist = dist + " km";
            }
            setfontsize((res.top_height/BASE_RES)*dir.minor_font_size);
            drawtext_in(dist_info, dist);
            
            if(dir.hide) return;
        }
        else
        {
            t_bound_box draw_box(header.left() + header.get_width()*0.08, header.bottom() - (i)*div_offset ,
                    header.right()+ header.get_width()*0.08, header.bottom() - (i-1)*div_offset);
            t_bound_box street_box(draw_box.left(), draw_box.bottom(),
                    draw_box.right(), draw_box.top() + draw_box.get_height()*0.3);
            t_bound_box dist_box(draw_box.left(), draw_box.bottom(),
                    draw_box.right(), draw_box.top() - draw_box.get_height()*0.35);
            t_bound_box dir_box(header.left(), header.bottom() - (i)*div_offset,
                        header.left() + header.get_width()*0.3, header.bottom() - (i-1)*div_offset);
            
            setfontsize((res.top_height/BASE_RES)*dir.default_font_size);
            setcolor(0,0,0);
            
            string dist;
            
            if(curr.distance < 1000)
            {
                dist = to_string((int)curr.distance) + " m";
            }
            else
            {
                stringstream _dist;
                _dist << fixed << setprecision(2) << curr.distance/1000;
                dist = _dist.str();
                dist = dist + " km";
            }
            
            if(curr.type != 1 && curr.type !=4)
            {
                string street = curr.street;
                string new_street = street;
                if(street.size() >= 12)
                {
                    new_street.resize(12);
                    street = new_street  + "...";
                }
                if(curr.type == 2)
                {
                                       
                    //cout << curr.street << endl;
                    
                    drawtext_in(street_box, street);
                                                          
                    setfontsize((res.top_height/BASE_RES)*dir.nesw_font_size);
                    drawtext_in(dir_box, curr.direction);
                    
                    setfontsize((res.top_height/BASE_RES)*dir.minor_font_size);
                    drawtext_in(dist_box, dist);
                    
                    setlinewidth(2);
                    drawarc(dir_box.get_xcenter(), dir_box.get_ycenter(), 
                            header.get_width()*0.07, 0, 360);
                }
                else
                {
                    drawtext_in(street_box, street);
                    
                    string a;
                    
                    if(curr.direction == "left")  a = "L";
                    else if(curr.direction == "right") a = "R";
                    else if(curr.direction == "straight") a = "St";
                    
                    setfontsize((res.top_height/BASE_RES)*dir.nesw_font_size);
                    drawtext_in(dir_box, a);
                    
                    setfontsize((res.top_height/BASE_RES)*dir.minor_font_size);
                    drawtext_in(dist_box, dist);
                    
                    setlinewidth(2);
                    drawarc(dir_box.get_xcenter(), dir_box.get_ycenter(), 
                            header.get_width()*0.07, 0, 360);
                    
                    //draw_direction_arrow(dir_box, curr.direction);
                }
                
            }
        }
            
    }  
    
}

void draw_directions_arrow(t_bound_box dir_box, string dir)
{
    if(dir == "left")
    {
        
    }
    else if(dir == "right")
    {
        
    }
}

void direction_click_handler()
{       
    if(dir.more && dir.down_bounds.intersects(mouse_location_current))
    {
        if(dir.scroll_count + dir.num_div +3 < (int)dir.dir.size()
                && path_set)
        {
            dir.scroll_count = dir.scroll_count + 1;
            if(dir.spec)
            {
                if(posi == 0)
                {
                    draw_path(path_data, get_scale());
                    draw_all_pinend_points (get_scale());
                    dir.spec = false;
                }
                posi = posi - 1; 
            }
            draw_directions();
        }
    }
    else if(dir.scroll_count > 0 && dir.up_bounds.intersects(mouse_location_current)
            && path_set)
    {
        dir.scroll_count = dir.scroll_count - 1;
        if(dir.spec)
        {
            if(posi == dir.num_div - 2 || posi == dir.num_div - 1)
            {
                draw_path(path_data, get_scale());
                draw_all_pinend_points (get_scale());
                dir.spec = false;
            }
            posi = posi + 1; 
        }
        draw_directions();
    }
    else if(dir.more && left_hold && path_set)
    {
        float diff = click_local.y - mouse_location_current.y;

        if(diff > 0 && (diff > dir.sdiv_offset || diff > dir.scroll_bounds.get_height()/2))
        {
            if(dir.scroll_count + dir.num_div + 3 >= (int)dir.dir.size()) return;
            else
            {
                dir.scroll_count = dir.scroll_count + 1;
                click_local = mouse_location_current;
                if(dir.spec)
                {
                    if(posi == 0)
                    {
                        draw_path(path_data, get_scale());
                        draw_all_pinend_points (get_scale());
                        dir.spec = false;
                    }
                    posi = posi - 1; 
                }
                dir.up_backdrop = true;
                dir.down_backdrop = true;
                draw_directions();
            }
        }
        else if(diff < 0 && (diff*-1 > dir.sdiv_offset || diff*-1 > dir.scroll_bounds.get_height()/2))
        {
            if(dir.scroll_count == 0) return;
            else
            {
                dir.scroll_count = dir.scroll_count - 1;
                click_local = mouse_location_current;
                if(dir.spec)
                {
                    if(posi == dir.num_div - 1)
                    {
                        draw_path(path_data, get_scale());
                        draw_all_pinend_points (get_scale());
                        dir.spec = false;
                    }
                    posi = posi + 1; 
                }
                dir.up_backdrop = true;
                dir.down_backdrop = true;
                draw_directions();
            }
        }
    }   
}

void draw_directions_collapsed(float div_offset, t_bound_box header2)
{
    t_bound_box visible_world = get_visible_world ();
    
   //float height = visible_world.get_height()*0.5;
    float width = minimap_bounds.get_width();
    float offset =  sqrt(pow((double)(visible_world.top() - visible_world.bottom()),(double)2) +
                    pow((double)(visible_world.right() - visible_world.left()),(double)2.0))*0.01;
    
    
    //sets direction drawing coordinates to a static position, scales
    float directions_bottom = minimap_bounds.top() + offset;
    float directions_top = visible_world.top() - offset;
    float directions_right = minimap_bounds.right();
    float directions_left = directions_right - width;
    
    //initialize directions bounds
    dir_bounds = t_bound_box(directions_left, directions_bottom,directions_right,directions_top);
    
    dir.header_offset = dir_bounds.get_height()*0.1;
    
    t_bound_box header(directions_left, directions_top - dir.header_offset, 
           directions_right, directions_top);
    
    dir.header = header;
    
    //drawscreen();
    setcolor(header_color);
    fillrect(dir.header);
    
    draw_directions_helper(div_offset, dir.header);
    
    setcolor(0,0,0);
    setlinewidth(3);
    drawrect(dir.header);
    
}

void draw_directions_exspec_parse(int pos)
{
    vector<string> sep = dir.dir_parsed[pos+dir.scroll_count];
    vector<string> out;
    string temp_out = sep[0];
    
    //only one string to parse
    if(sep.size() == 1)
    {
        //breaks up string to parts
        if(temp_out.size() > 13)
        {
            string a = temp_out;
            a.resize(12);
            a = a + "-";

            string b = temp_out;
            b.erase(0, 11);

            string c = "-" + b;

            out.push_back(a);
            out.push_back(c);
        }
        else out.push_back(temp_out);
    }
    else // more than one string to parse
    {
        temp_out.clear();
        
        for(unsigned i = 0; i < sep.size(); ++i)
        {                    
            if(temp_out.size() == 0)
            {
                temp_out = sep[i];
                
                if(temp_out.size() > 13)
                {
                    string a = temp_out;
                    a.resize(12);
                    a = a + "-";

                    string b = temp_out;
                    b.erase(0, 11);

                    string c = "-" + b;

                    out.push_back(a);
                    
                    temp_out.clear();
                    
                    temp_out = c;
                }
            }
            else temp_out = temp_out + " " + sep[i];
            
            //first string
            if(i == 0)
            {
                
                
                {
                    if(temp_out.size() + sep[i+1].size() + 1 > 13 )
                    {
                        out.push_back(temp_out);
                        temp_out.clear();
                    }
                }       
            }
            else if(i+1 < sep.size())
            {
                if(temp_out.size() + sep[i+1].size() > 13)
                {
                    out.push_back(temp_out);
                    temp_out.clear();
                }
            }
            else
            {
                if(temp_out.size() + sep[sep.size() - 1].size() > 13)
                {
                    out.push_back(temp_out);
                    temp_out.clear();
                }
                else if(i + 1 == sep.size()) out.push_back(temp_out);
            }                                  
        }
    }
    
    draw_dir_exspec(pos, out);
    
    /*for(unsigned i = 0; i < out.size(); ++i)
    {
        cout << out[i] << endl;
    }*/
    
}

void draw_dir_exspec(int pos, vector<string> output)
{
    if(pos != dir.num_div - 1)
    {
        dir.spec_bounds = t_bound_box(dir_bounds.left(), dir.header.bottom() - (pos+2)*dir.div_offset,
        dir_bounds.right(), dir.header.bottom() - pos*dir.div_offset);
    }
    else
    {
        dir.spec_bounds = t_bound_box(dir_bounds.left(), dir_bounds.bottom(),
            dir_bounds.right(), dir_bounds.bottom() + 2*dir.div_offset);
    }
    
    draw_path(path_data, get_scale());
    
    setcolor(YELLOW);
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
    
    /*cout << dir.index[pos + dir.scroll_count + 1] - dir.index[pos + dir.scroll_count] << endl;
    vector<unsigned> path_spec(dir.index[pos + dir.scroll_count + 1] - dir.index[pos + dir.scroll_count]) ;//= dir.dir[2 + pos + dir.scroll_count].path_seg;
    copy(path_data.begin() + dir.index[pos + dir.scroll_count], 
            path_data.begin() + dir.index[pos + dir.scroll_count + 1], path_spec.begin());*/
    
    //int size = path_spec.size();
    //cout << size << endl;
    //if (size > 0)
    //{
         
   
    //cout << path_data.size() << endl;
    unsigned max = dir.index[posi + dir.scroll_count + 1];
    if(posi + dir.scroll_count == (int)dir.index.size() - 1) max = path_data.size();
    //cout << dir.index[posi + dir.scroll_count] << " to " << max << endl;
    //cout << path_data[dir.index[posi + dir.scroll_count]] << " to " << path_data[max] << endl;
    
    for (int i=dir.index[posi + dir.scroll_count]; i< (int)max; i++)
        {
            StreetSegmentEnds ends = getStreetSegmentEnds (path_data[i]);
            t_point from  = get_coordinate_from_LatLon (getIntersectionPosition(ends.from));
            t_point to  = get_coordinate_from_LatLon (getIntersectionPosition(ends.to));   
            
            int num_of_curve_points = getStreetSegmentCurvePointCount (path_data[i]);
            if (num_of_curve_points != 0)
            {
                t_point point1 = get_coordinate_from_LatLon (getStreetSegmentCurvePoint (path_data[i], 0));
                t_point point2;
                
                drawline (from, point1);
                for (int j=0; j<num_of_curve_points-1; j++)
                {
                    point1 = get_coordinate_from_LatLon (getStreetSegmentCurvePoint (path_data[i], j));
                    point2 = get_coordinate_from_LatLon (getStreetSegmentCurvePoint (path_data[i], j+1));
                    drawline (point1, point2);
                }
                point2 = get_coordinate_from_LatLon (getStreetSegmentCurvePoint (path_data[i], num_of_curve_points-1));
                
                drawline (point2, to);
            }
            else
            {              
                drawline (from, to);
            }
            
    }
    //}
    
    if(menu.cour_path.size() > 0)
    {
        draw_positions(menu.cour_depots, depot_color);
        draw_positions(menu.cour_intrs, intr_color);
    }
    else if(menu.path_path.size() > 0)
    {
        draw_positions(menu.path_waypoints, pin_color);
    }
    else draw_all_pinend_points (get_scale());
    
    
    //determine visible world
    t_bound_box visible_world = get_visible_world ();
    
   //float height = visible_world.get_height()*0.5;
    float width = minimap_bounds.get_width();
    float offset =  sqrt(pow((double)(visible_world.top() - visible_world.bottom()),(double)2) +
                    pow((double)(visible_world.right() - visible_world.left()),(double)2.0))*0.01;
    
    
    //sets direction drawing coordinates to a static position, scales
    float directions_bottom = minimap_bounds.top() + offset;
    float directions_top = visible_world.top() - offset;
    float directions_right = minimap_bounds.right();
    float directions_left = directions_right - width;
    
    //initialize directions bounds
    dir_bounds = t_bound_box(directions_left, directions_bottom,directions_right,directions_top);
    
    dir.header_offset = dir_bounds.get_height()*0.1;
    
    t_bound_box header(directions_left, directions_top - dir.header_offset, 
           directions_right, directions_top);
    
    dir.header = header;
    
    dir.div_offset = (header.bottom() - dir_bounds.bottom())/dir.num_div;
    
    //draw background of legend (white)
    setcolor(255,255,255);
    fillrect(dir_bounds);

    //draw first box for header
    setcolor(53,161,255);

    fillrect(header);
    
    //initialize scroll bar arrow parameters 
    dir.up_center = t_point(dir_bounds.right() - dir_bounds.get_width()*0.05,
                    dir_bounds.top() - dir.header_offset - dir_bounds.get_width()*0.05);
    
    dir.down_center = t_point(dir_bounds.right() - dir_bounds.get_width()*0.05,
                dir_bounds.bottom() + dir_bounds.get_width()*0.06);  
    
    dir.up_bounds = t_bound_box(dir.up_center.x - dir_bounds.get_width()*0.05,
                            dir.up_center.y - dir_bounds.get_width()*0.05,
                            dir.up_center.x + dir_bounds.get_width()*0.05,
                            dir.up_center.y + dir_bounds.get_width()*0.05);
    
    dir.down_bounds = t_bound_box(dir.down_center.x - dir_bounds.get_width()*0.05,
                            dir.down_center.y - dir_bounds.get_width()*0.06,
                            dir.down_center.x + dir_bounds.get_width()*0.05,
                            dir.down_center.y + dir_bounds.get_width()*0.04);
    
    dir.scroll_bounds = t_bound_box(dir.up_bounds.left(), dir.down_bounds.top(),
                             dir.up_bounds.right(), dir.up_bounds.bottom());
    
    
    //t_bound_box clear(dir_bounds.left(), dir_bounds.bottom(), dir_bounds.right(),
    //        dir.header.bottom());
    
    //setcolor(255,255,255);
    //fillrect(clear);
    
    setcolor(0,0,0);
    setlinewidth(2);
    
    setcolor(255,255,255);          
            
    //draw directions header text
    
    dir_data fir = dir.dir[0];
    
    t_bound_box temp(dir.header.left() - dir.header.get_width()*0.23, dir.header.bottom(),
            dir.header.right() - dir.header.get_width()*0.23, dir.header.top());
    setfontsize((res.top_height/BASE_RES)*dir.main_font_size);
    drawtext_in(temp, fir.street);
    //cout << fir.street << endl;

    //draw total time for path
    t_bound_box time_info(dir.header.left() + dir.header.get_width()*0.23, dir.header.bottom() +
            dir.header.get_height()*0.33, dir.header.right() + dir.header.get_width()*0.23, dir.header.top());

    string time = to_string((int)compute_path_travel_time(path_data)) + " min";
    setfontsize((res.top_height/BASE_RES)*dir.minor_font_size);
    drawtext_in(time_info, time);

    //draw total distance 
    t_bound_box dist_info(dir.header.left() + dir.header.get_width()*0.23, dir.header.bottom(),
            dir.header.right() + dir.header.get_width()*0.23, dir.header.top()- dir.header.get_height()*0.33);

    string distance;
    
    if(fir.distance < 1000)
    {
        distance = to_string((int)fir.distance) + " m";
    }
    else
    {
        stringstream _dist;
        _dist << fixed << setprecision(2) << fir.distance/1000;
        distance = _dist.str();
        distance = distance + " km";
    }
    setfontsize((res.top_height/BASE_RES)*dir.minor_font_size);
    drawtext_in(dist_info, distance);
    
 
    for(unsigned i = 0; (int)i < dir.num_div && i < dir.dir_parsed.size(); ++i)
    {

        setfontsize((res.top_height/BASE_RES)*dir.default_font_size);
        setcolor(0,0,0);
        
        if(2 + pos + dir.scroll_count > (int)dir.dir.size()) continue;
        
        dir_data curr = dir.dir[2 + pos + dir.scroll_count];
        
        string dist;

        if(curr.distance < 1000)
        {
            dist = to_string((int)curr.distance) + " m";
        }
        else
        {
            stringstream _dist;
            _dist << fixed << setprecision(2) << curr.distance/1000;
            dist = _dist.str();
            dist = dist + " km";
        }
        
        //direction to expand is not last in list
        if(pos != dir.num_div - 1) 
        {
            //directions above specified
            if((int)i < pos)
            {  
                
                if(2 + pos + dir.scroll_count > (int)dir.dir.size()) continue;
                curr = dir.dir[2 + i + dir.scroll_count];
                
                string street = curr.street;
                string new_street = street;
                if(street.size() >= 12)
                {
                    new_street.resize(12);
                    street = new_street  + "...";
                }
        
                string dist_;

                if(curr.distance < 1000)
                {
                    dist_ = to_string((int)curr.distance) + " m";
                }
                else
                {
                    stringstream _dist_;
                    _dist_ << fixed << setprecision(2) << curr.distance/1000;
                    dist_ = _dist_.str();
                    dist_ = dist_ + " km";
                }
                
                t_bound_box pre(dir_bounds.left(), dir.header.bottom() - (i+1)*dir.div_offset,
                       dir_bounds.right(), dir.header.bottom() - i*dir.div_offset);
               
                setcolor(255,255,255);
                fillrect(pre);              
               
                t_bound_box street_box(pre.left() + dir.header.get_width()*0.08, 
                        pre.bottom(), pre.right() + dir.header.get_width()*0.08,
                        pre.top() + pre.get_height()*0.3);
               
                t_bound_box dist_box(pre.left() + dir.header.get_width()*0.08,
                        pre.bottom(), pre.right() + dir.header.get_width()*0.08,
                        pre.top() - pre.get_height()*0.35);
                
                
                t_bound_box dir_box(dir.header.left(), pre.bottom(),
                    dir.header.left() + dir.header.get_width()*0.3, 
                    pre.top());
               
                setcolor(0,0,0);
                
                setlinewidth(dir.default_font_size);
                drawtext_in(street_box, street);
                    
                string a;

                if(curr.type == 2) a = curr.direction;
                else if(curr.direction == "left")  a = "L";
                else if(curr.direction == "right") a = "R"; 
                else if(curr.direction == "straight") a = "St";
                
                setfontsize((res.top_height/BASE_RES)*dir.nesw_font_size);
                drawtext_in(dir_box, a);

                setfontsize((res.top_height/BASE_RES)*dir.minor_font_size);
                drawtext_in(dist_box, dist_);

                setlinewidth(2);
                drawarc(dir_box.get_xcenter(), dir_box.get_ycenter(), 
                        dir.header.get_width()*0.07, 0, 360);

               
            }
            //specified direction
            else if((int)i == pos)
            {
                
                setcolor(255,255,255);
                //fillrect(dir.spec_bounds);
                if(output.size() == 1)
                {
                    setcolor(0,0,0);
                    
                    
                    t_bound_box one(dir.header.left() + dir.header.get_width()*0.08, 
                            dir.spec_bounds.bottom() + dir.spec_bounds.get_height()*0.1,
                            dir.header.right() + dir.header.get_width()*0.08,
                            dir.spec_bounds.top());
                    
                    t_bound_box dir_spec(dir.header.left(), dir.spec_bounds.bottom(),
                            dir.header.left() + dir.header.get_width()*0.3, 
                            dir.spec_bounds.top());
                    
                    t_bound_box dist_spec(one.left(), one.bottom(),
                            one.right(), one.top() - one.get_height()*0.35);
                    
                    string a;
                    
                    if(curr.type == 2) a= curr.direction;
                    else if(curr.direction == "left")  a = "L";
                    else if(curr.direction == "right") a = "R";
                    else if(curr.direction == "straight") a = "St";
                    
                    setfontsize((res.top_height/BASE_RES)*dir.default_font_size);
                    drawtext_in(one, output[0]);
                    
                    setfontsize((res.top_height/BASE_RES)*dir.nesw_font_size);
                    drawtext_in(dir_spec, a);
                    
                    setfontsize((res.top_height/BASE_RES)*dir.minor_font_size);
                    drawtext_in(dist_spec, dist);
                    
                    //setlinewidth(2);
                    drawarc(dir_spec.get_xcenter(), dir_spec.get_ycenter(), 
                            dir.header.get_width()*0.07, 0, 360);
                }
                else if(output.size() == 2)
                {
                    setcolor(0,0,0);
                    
                    
                    t_bound_box one(dir.header.left() + dir.header.get_width()*0.08, 
                            dir.spec_bounds.bottom() + dir.spec_bounds.get_height()*0.36,
                            dir.header.right() + dir.header.get_width()*0.08,
                            dir.spec_bounds.top());
                    
                    t_bound_box two(dir.header.left() + dir.header.get_width()*0.08, 
                            dir.spec_bounds.bottom() + dir.spec_bounds.get_height()*0.09,
                            dir.header.right() + dir.header.get_width()*0.08,
                            dir.spec_bounds.top());
                    
                    t_bound_box dir_spec(dir.header.left(), dir.spec_bounds.bottom(),
                            dir.header.left() + dir.header.get_width()*0.3, 
                            dir.spec_bounds.top());
                    
                    t_bound_box dist_spec(two.left(), two.bottom(),
                            two.right(), two.top() - two.get_height()*0.35);
                    
                    string a;
                    
                    if(curr.type == 2) a= curr.direction;
                    else if(curr.direction == "left")  a = "L";
                    else if(curr.direction == "right") a = "R";
                    else if(curr.direction == "straight") a = "St";
                    
                    setfontsize((res.top_height/BASE_RES)*dir.default_font_size);
                    drawtext_in(one, output[0]);
                    drawtext_in(two, output[1]);
                    
                    setfontsize((res.top_height/BASE_RES)*dir.nesw_font_size);
                    drawtext_in(dir_spec, a);
                    
                    setfontsize((res.top_height/BASE_RES)*dir.minor_font_size);
                    drawtext_in(dist_spec, dist);
                    
                    //setlinewidth(2);
                    drawarc(dir_spec.get_xcenter(), dir_spec.get_ycenter(), 
                            dir.header.get_width()*0.07, 0, 360);
                }
                else if(output.size() == 3)
                {
                    setcolor(0,0,0);
                    
                    
                    t_bound_box one(dir.header.left() + dir.header.get_width()*0.08, 
                            dir.spec_bounds.bottom() + dir.spec_bounds.get_height()*0.36,
                            dir.header.right() + dir.header.get_width()*0.08,
                            dir.spec_bounds.top());
                    
                    t_bound_box two(dir.header.left() + dir.header.get_width()*0.08, 
                            dir.spec_bounds.bottom() + dir.spec_bounds.get_height()*0.1,
                            dir.header.right() + dir.header.get_width()*0.08,
                            dir.spec_bounds.top());
                    
                    t_bound_box three(dir.header.left() + dir.header.get_width()*0.08, 
                            dir.spec_bounds.bottom() - dir.spec_bounds.get_height()*0.17,
                            dir.header.right() + dir.header.get_width()*0.08,
                            dir.spec_bounds.top());
                    
                    t_bound_box dir_spec(dir.header.left(), dir.spec_bounds.bottom(),
                            dir.header.left() + dir.header.get_width()*0.3, 
                            dir.spec_bounds.top());
                    
                    t_bound_box dist_spec(three.left(), three.bottom(),
                            three.right(), three.top() - three.get_height()*0.22);
                    
                    string a;
                    
                    if(curr.type == 2) a= curr.direction;
                    else if(curr.direction == "left")  a = "L";
                    else if(curr.direction == "right") a = "R";
                    else if(curr.direction == "straight") a = "St";
                    
                    setfontsize((res.top_height/BASE_RES)*dir.default_font_size);
                    drawtext_in(one, output[0]);
                    drawtext_in(two, output[1]);
                    drawtext_in(three, output[2]);
                    
                    setfontsize((res.top_height/BASE_RES)*dir.nesw_font_size);
                    drawtext_in(dir_spec, a);
                    
                    setfontsize((res.top_height/BASE_RES)*dir.minor_font_size);
                    drawtext_in(dist_spec, dist);
                    
                    //setlinewidth(2);
                    drawarc(dir_spec.get_xcenter(), dir_spec.get_ycenter(), 
                            dir.header.get_width()*0.07, 0, 360);
                }
                else
                {
                    setcolor(0,0,0);
                    
                    
                    t_bound_box one(dir.header.left() + dir.header.get_width()*0.08, 
                            dir.spec_bounds.bottom() + dir.spec_bounds.get_height()*0.57,
                            dir.header.right() + dir.header.get_width()*0.08,
                            dir.spec_bounds.top());
                    
                    t_bound_box two(dir.header.left() + dir.header.get_width()*0.08, 
                            dir.spec_bounds.bottom() + dir.spec_bounds.get_height()*0.27,
                            dir.header.right() + dir.header.get_width()*0.08,
                            dir.spec_bounds.top());
                    
                    t_bound_box three(dir.header.left() + dir.header.get_width()*0.08, 
                            dir.spec_bounds.bottom() - dir.spec_bounds.get_height()*0.032,
                            dir.header.right() + dir.header.get_width()*0.08,
                            dir.spec_bounds.top());
                    
                    t_bound_box four(dir.header.left() + dir.header.get_width()*0.08, 
                            dir.spec_bounds.bottom() - dir.spec_bounds.get_height()*0.32,
                            dir.header.right() + dir.header.get_width()*0.08,
                            dir.spec_bounds.top());
                    
                    t_bound_box dir_spec(dir.header.left(), dir.spec_bounds.bottom(),
                            dir.header.left() + dir.header.get_width()*0.3, 
                            dir.spec_bounds.top());
                    
                    t_bound_box dist_spec(four.left(), four.bottom(),
                            four.right(), four.top() - four.get_height()*0.2);
                    
                    string a;
                    
                    if(curr.type == 2) a= curr.direction;
                    else if(curr.direction == "left")  a = "L";
                    else if(curr.direction == "right") a = "R";
                    else if(curr.direction == "straight") a = "St";
                    
                    setfontsize((res.top_height/BASE_RES)*dir.default_font_size);
                    drawtext_in(one, output[0]);
                    drawtext_in(two, output[1]);
                    drawtext_in(three, output[2]);
                    drawtext_in(four, output[3] + "..");
                    
                    setfontsize((res.top_height/BASE_RES)*dir.nesw_font_size);
                    drawtext_in(dir_spec, a);
                    
                    setfontsize((res.top_height/BASE_RES)*dir.minor_font_size);
                    drawtext_in(dist_spec, dist);
                    
                    //setlinewidth(2);
                    drawarc(dir_spec.get_xcenter(), dir_spec.get_ycenter(), 
                            dir.header.get_width()*0.07, 0, 360);
                }
                //setcolor(0,0,255);
                //fillrect(dir.spec_bounds);
                //setcolor(0,0,0);
                //setlinewidth(dir.minor_font_size);
                //drawtext_in(dir.spec_bounds, dist);
            }
            //direction below specified
            else
            {
                if((int)(i+2) > dir.num_div) continue;
                setcolor(255,255,255);
                t_bound_box post(dir_bounds.left(), dir.header.bottom() - (i+2)*dir.div_offset,
                       dir_bounds.right(), dir.header.bottom() - (i+1)*dir.div_offset);
                fillrect(post);
                
                if(2 + pos + dir.scroll_count > (int)dir.dir.size()) continue;
                curr = dir.dir[2 + i + dir.scroll_count];
                
                string street = curr.street;
                string new_street = street;
                if(street.size() >= 12)
                {
                    new_street.resize(12);
                    street = new_street  + "...";
                }
        
                string dist_;

                if(curr.distance < 1000)
                {
                    dist_ = to_string((int)curr.distance) + " m";
                }
                else
                {
                    stringstream _dist_;
                    _dist_ << fixed << setprecision(2) << curr.distance/1000;
                    dist_ = _dist_.str();
                    dist_ = dist_ + " km";
                }
                       
               
                t_bound_box street_box(post.left() + dir.header.get_width()*0.08, 
                        post.bottom(), post.right() + dir.header.get_width()*0.08,
                        post.top() + post.get_height()*0.3);
               
                t_bound_box dist_box(post.left() + dir.header.get_width()*0.08,
                        post.bottom(), post.right() + dir.header.get_width()*0.08,
                        post.top() - post.get_height()*0.35);
                
                
                t_bound_box dir_box(dir.header.left(), post.bottom(),
                    dir.header.left() + dir.header.get_width()*0.3, 
                    post.top());
               
                setcolor(0,0,0);
                
                setlinewidth(dir.default_font_size);
                drawtext_in(street_box, street);
                    
                string a;

                if(curr.type == 2) a = curr.direction;
                else if(curr.direction == "left")  a = "L";
                else if(curr.direction == "right") a = "R";    
                else if(curr.direction == "straight") a = "St";
                
                setfontsize((res.top_height/BASE_RES)*dir.nesw_font_size);
                drawtext_in(dir_box, a);

                setfontsize((res.top_height/BASE_RES)*dir.minor_font_size);
                drawtext_in(dist_box, dist_);

                setlinewidth(2);
                drawarc(dir_box.get_xcenter(), dir_box.get_ycenter(), 
                        dir.header.get_width()*0.07, 0, 360);
            }
        }
        //direction to expand is last in list
        else
        {
            if((int)i < dir.num_div - 2)
            {
                setcolor(255,255,255);
                t_bound_box pre(dir_bounds.left(), dir.header.bottom() - (i+1)*dir.div_offset,
                       dir_bounds.right(), dir.header.bottom() - i*dir.div_offset);
               fillrect(pre);
               
               curr = dir.dir[3 + i + dir.scroll_count];
                
                string street = curr.street;
                string new_street = street;
                if(street.size() >= 12)
                {
                    new_street.resize(12);
                    street = new_street  + "...";
                }
        
                string dist_;

                if(curr.distance < 1000)
                {
                    dist_ = to_string((int)curr.distance) + " m";
                }
                else
                {
                    stringstream _dist_;
                    _dist_ << fixed << setprecision(2) << curr.distance/1000;
                    dist_ = _dist_.str();
                    dist_ = dist_ + " km";
                }                
               
                setcolor(255,255,255);
                fillrect(pre);              
               
                t_bound_box street_box(pre.left() + dir.header.get_width()*0.08, 
                        pre.bottom(), pre.right() + dir.header.get_width()*0.08,
                        pre.top() + pre.get_height()*0.3);
               
                t_bound_box dist_box(pre.left() + dir.header.get_width()*0.08,
                        pre.bottom(), pre.right() + dir.header.get_width()*0.08,
                        pre.top() - pre.get_height()*0.35);
                
                
                t_bound_box dir_box(dir.header.left(), pre.bottom(),
                    dir.header.left() + dir.header.get_width()*0.3, 
                    pre.top());
               
                setcolor(0,0,0);
                
                setlinewidth(dir.default_font_size);
                drawtext_in(street_box, street);
                    
                string a;

                if(curr.type == 2) a = curr.direction;
                else if(curr.direction == "left")  a = "L";
                else if(curr.direction == "right") a = "R"; 
                else if(curr.direction == "straight") a = "St";
                
                setfontsize((res.top_height/BASE_RES)*dir.nesw_font_size);
                drawtext_in(dir_box, a);

                setfontsize((res.top_height/BASE_RES)*dir.minor_font_size);
                drawtext_in(dist_box, dist_);

                setlinewidth(2);
                drawarc(dir_box.get_xcenter(), dir_box.get_ycenter(), 
                        dir.header.get_width()*0.07, 0, 360);
            }
            //direction to expand
            else
            {
                
                setcolor(255,255,255);
                fillrect(dir.spec_bounds);
                if(output.size() == 1)
                {
                    setcolor(0,0,0);
                    
                    
                    t_bound_box one(dir.header.left() + dir.header.get_width()*0.08, 
                            dir.spec_bounds.bottom() + dir.spec_bounds.get_height()*0.1,
                            dir.header.right() + dir.header.get_width()*0.08,
                            dir.spec_bounds.top());
                    
                    t_bound_box dir_spec(dir.header.left(), dir.spec_bounds.bottom(),
                            dir.header.left() + dir.header.get_width()*0.3, 
                            dir.spec_bounds.top());
                    
                    t_bound_box dist_spec(one.left(), one.bottom(),
                            one.right(), one.top() - one.get_height()*0.35);
                    
                    string a;
                    
                    if(curr.type == 2) a= curr.direction;
                    else if(curr.direction == "left")  a = "L";
                    else if(curr.direction == "right") a = "R";
                    else if(curr.direction == "straight") a = "St";
                    
                    setfontsize((res.top_height/BASE_RES)*dir.default_font_size);
                    drawtext_in(one, output[0]);
                    
                    setfontsize((res.top_height/BASE_RES)*dir.nesw_font_size);
                    drawtext_in(dir_spec, a);
                    
                    setfontsize((res.top_height/BASE_RES)*dir.minor_font_size);
                    drawtext_in(dist_spec, dist);
                    
                    //setlinewidth(2);
                    drawarc(dir_spec.get_xcenter(), dir_spec.get_ycenter(), 
                            dir.header.get_width()*0.07, 0, 360);
                }
                else if(output.size() == 2)
                {
                    setcolor(0,0,0);
                    
                    
                    t_bound_box one(dir.header.left() + dir.header.get_width()*0.08, 
                            dir.spec_bounds.bottom() + dir.spec_bounds.get_height()*0.36,
                            dir.header.right() + dir.header.get_width()*0.08,
                            dir.spec_bounds.top());
                    
                    t_bound_box two(dir.header.left() + dir.header.get_width()*0.08, 
                            dir.spec_bounds.bottom() + dir.spec_bounds.get_height()*0.09,
                            dir.header.right() + dir.header.get_width()*0.08,
                            dir.spec_bounds.top());
                    
                    t_bound_box dir_spec(dir.header.left(), dir.spec_bounds.bottom(),
                            dir.header.left() + dir.header.get_width()*0.3, 
                            dir.spec_bounds.top());
                    
                    t_bound_box dist_spec(two.left(), two.bottom(),
                            two.right(), two.top() - two.get_height()*0.35);
                    
                    string a;
                    
                    if(curr.type == 2) a= curr.direction;
                    else if(curr.direction == "left")  a = "L";
                    else if(curr.direction == "right") a = "R";
                    else if(curr.direction == "straight") a = "St";
                    
                    setfontsize((res.top_height/BASE_RES)*dir.default_font_size);
                    drawtext_in(one, output[0]);
                    drawtext_in(two, output[1]);
                    
                    setfontsize((res.top_height/BASE_RES)*dir.nesw_font_size);
                    drawtext_in(dir_spec, a);
                    
                    setfontsize((res.top_height/BASE_RES)*dir.minor_font_size);
                    drawtext_in(dist_spec, dist);
                    
                    //setlinewidth(2);
                    drawarc(dir_spec.get_xcenter(), dir_spec.get_ycenter(), 
                            dir.header.get_width()*0.07, 0, 360);
                }
                else if(output.size() == 3)
                {
                    setcolor(0,0,0);
                    
                    
                    t_bound_box one(dir.header.left() + dir.header.get_width()*0.08, 
                            dir.spec_bounds.bottom() + dir.spec_bounds.get_height()*0.36,
                            dir.header.right() + dir.header.get_width()*0.08,
                            dir.spec_bounds.top());
                    
                    t_bound_box two(dir.header.left() + dir.header.get_width()*0.08, 
                            dir.spec_bounds.bottom() + dir.spec_bounds.get_height()*0.1,
                            dir.header.right() + dir.header.get_width()*0.08,
                            dir.spec_bounds.top());
                    
                    t_bound_box three(dir.header.left() + dir.header.get_width()*0.08, 
                            dir.spec_bounds.bottom() - dir.spec_bounds.get_height()*0.17,
                            dir.header.right() + dir.header.get_width()*0.08,
                            dir.spec_bounds.top());
                    
                    t_bound_box dir_spec(dir.header.left(), dir.spec_bounds.bottom(),
                            dir.header.left() + dir.header.get_width()*0.3, 
                            dir.spec_bounds.top());
                    
                    t_bound_box dist_spec(three.left(), three.bottom(),
                            three.right(), three.top() - three.get_height()*0.2);
                    
                    string a;
                    
                    if(curr.type == 2) a= curr.direction;
                    else if(curr.direction == "left")  a = "L";
                    else if(curr.direction == "right") a = "R";
                    else if(curr.direction == "straight") a = "St";
                    
                    setfontsize((res.top_height/BASE_RES)*dir.default_font_size);
                    drawtext_in(one, output[0]);
                    drawtext_in(two, output[1]);
                    drawtext_in(three, output[2]);
                    
                    setfontsize((res.top_height/BASE_RES)*dir.nesw_font_size);
                    drawtext_in(dir_spec, a);
                    
                    setfontsize((res.top_height/BASE_RES)*dir.minor_font_size);
                    drawtext_in(dist_spec, dist);
                    
                    //setlinewidth(2);
                    drawarc(dir_spec.get_xcenter(), dir_spec.get_ycenter(), 
                            dir.header.get_width()*0.07, 0, 360);
                }
                else if(output.size() == 4)
                {
                    setcolor(0,0,0);
                    
                    
                    t_bound_box one(dir.header.left() + dir.header.get_width()*0.08, 
                            dir.spec_bounds.bottom() + dir.spec_bounds.get_height()*0.57,
                            dir.header.right() + dir.header.get_width()*0.08,
                            dir.spec_bounds.top());
                    
                    t_bound_box two(dir.header.left() + dir.header.get_width()*0.08, 
                            dir.spec_bounds.bottom() + dir.spec_bounds.get_height()*0.27,
                            dir.header.right() + dir.header.get_width()*0.08,
                            dir.spec_bounds.top());
                    
                    t_bound_box three(dir.header.left() + dir.header.get_width()*0.08, 
                            dir.spec_bounds.bottom() - dir.spec_bounds.get_height()*0.032,
                            dir.header.right() + dir.header.get_width()*0.08,
                            dir.spec_bounds.top());
                    
                    t_bound_box four(dir.header.left() + dir.header.get_width()*0.08, 
                            dir.spec_bounds.bottom() - dir.spec_bounds.get_height()*0.32,
                            dir.header.right() + dir.header.get_width()*0.08,
                            dir.spec_bounds.top());
                    
                    t_bound_box dir_spec(dir.header.left(), dir.spec_bounds.bottom(),
                            dir.header.left() + dir.header.get_width()*0.3, 
                            dir.spec_bounds.top());
                    
                    t_bound_box dist_spec(four.left(), four.bottom(),
                            four.right(), four.top() - four.get_height()*0.2);
                    
                    string a;
                    
                    if(curr.type == 2) a = curr.direction;
                    else if(curr.direction == "left")  a = "L";
                    else if(curr.direction == "right") a = "R";
                    else if(curr.direction == "straight") a = "St";
                    
                    setfontsize((res.top_height/BASE_RES)*dir.default_font_size);
                    drawtext_in(one, output[0]);
                    drawtext_in(two, output[1]);
                    drawtext_in(three, output[2]);
                    drawtext_in(four, output[3]);
                    
                    setfontsize((res.top_height/BASE_RES)*dir.nesw_font_size);
                    drawtext_in(dir_spec, a);
                    
                    setfontsize((res.top_height/BASE_RES)*dir.minor_font_size);
                    drawtext_in(dist_spec, dist);
                    
                    //setlinewidth(2);
                    drawarc(dir_spec.get_xcenter(), dir_spec.get_ycenter(), 
                            dir.header.get_width()*0.07, 0, 360);
                }
            }
        }
        
    }
    
    setcolor(219,249,255);
    setlinewidth(1.5);
    for(unsigned i = 0, h = 0; i < dir.dir_parsed.size() + 1 && 
            (int)h < dir.num_div + 1; ++i, ++h)
    {
        if(pos + 1 == (int)i || (pos == 9 && i == 9) ) continue;
        drawline(dir_bounds.left(), dir.header.bottom() - dir.div_offset*(i), dir_bounds.right(),
                dir.header.bottom() - dir.div_offset*(i));
    }
    
    if((int)dir.dir.size() - 2 > dir.num_div + 1)
    {
        draw_directions_sbar();
    }
    
    //draw legend border
    setcolor(0,0,0);
    setlinewidth(3);
    drawrect(dir_bounds);
    
}

void draw_main_menu()
{
    t_bound_box visible_world = get_visible_world ();
    
    float height = visible_world.get_height()*0.05;
    float width = height;
    float offset = sqrt(pow((double)(visible_world.top() - visible_world.bottom()),(double)2) +
                    pow((double)(visible_world.right() - visible_world.left()),(double)2.0))*0.01;
    
    //for about button
    float temp_bottom = visible_world.bottom() + offset;
    float temp_top = temp_bottom + height;
    float temp_left = visible_world.left() + offset;
    float temp_right = temp_left + width;
    
    //highlighting button borders on hover
    
    //ABOUT BUTTON
    t_bound_box temp_bounds(temp_left, temp_bottom, temp_right, temp_top);
    
    menu.about_bBounds = temp_bounds;

    setcolor(0,0,0);
    fillarc(temp_bounds.get_center(), temp_bounds.get_height()/2, 0 ,360);
    
    //border
    if(menu.button_spec == 0 || menu.active_button == 0) setcolor(255,255,255);
    else setcolor(195,195,195);
    setlinewidth(4);
    drawarc(temp_bounds.get_xcenter(), temp_bounds.get_ycenter(), temp_bounds.get_height()/2, 0 , 360);
    //text identifier
    if(menu.button_spec == 0 || menu.active_button == 0) setcolor(255,255,255);
    else setcolor(195,195,195);
    setfontsize(22*(res.top_height/BASE_RES));
    drawtext_in(temp_bounds, "?");
    //black border for circle
    setcolor(0,0,0);
    setlinewidth(1);
    drawarc(temp_bounds.get_xcenter(), temp_bounds.get_ycenter(), temp_bounds.get_height()/2*(1+0.1), 0 , 360);
    
    //COURIER BUTTON
    temp_left = temp_right + offset;
    temp_right = temp_left + width;
    
    t_bound_box cour_bounds(temp_left, temp_bottom, temp_right, temp_top);
    
    menu.cour_bBounds = cour_bounds;
    
    setcolor(0,0,0);
    fillarc(cour_bounds.get_center(), cour_bounds.get_height()/2, 0 ,360);
    
    //border
    if(menu.button_spec == 1 || menu.active_button == 1) setcolor(255,255,255);
    else setcolor(195,195,195);
    setlinewidth(4);
    drawarc(cour_bounds.get_xcenter(), cour_bounds.get_ycenter(), cour_bounds.get_height()/2, 0 , 360);
    //text identifier
    if(menu.button_spec == 1 || menu.active_button == 1) setcolor(255,255,255);
    else setcolor(195,195,195);
    setfontsize(18*(res.top_height/BASE_RES));
    drawtext_in(cour_bounds, "MP");
    //black border for circle
    setcolor(0,0,0);
    setlinewidth(1);
    drawarc(cour_bounds.get_xcenter(), cour_bounds.get_ycenter(), cour_bounds.get_height()/2*(1+0.1), 0 , 360);
    
    //SINGLE PATH BUTTON
    temp_left = temp_right + offset;
    temp_right = temp_left + width;
    
    t_bound_box path_bounds(temp_left, temp_bottom, temp_right, temp_top);
    
    menu.path_bBounds = path_bounds;
    
    setcolor(0,0,0);
    fillarc(path_bounds.get_center(), path_bounds.get_height()/2, 0 ,360);
    
    //border
    if(menu.button_spec == 2 || menu.active_button == 2) setcolor(255,255,255);
    else setcolor(195,195,195);
    setlinewidth(4);
    drawarc(path_bounds.get_xcenter(), path_bounds.get_ycenter(), path_bounds.get_height()/2, 0 , 360);
    //text identifier
    if(menu.button_spec == 2 || menu.active_button == 2) setcolor(255,255,255);
    else setcolor(195,195,195);
    setfontsize(18*(res.top_height/BASE_RES));
    drawtext_in(path_bounds, "SP");
    //black border for circle
    setcolor(0,0,0);
    setlinewidth(1);
    drawarc(path_bounds.get_xcenter(), path_bounds.get_ycenter(), path_bounds.get_height()/2*(1+0.1), 0 , 360);
    
    //fillrect(temp_bounds);
}

void main_click_handler()
{
    if(menu.active_button == 0)
    {
        //drawscreen();
        draw_about_box();
    }
    else if(menu.active_button == 1)
    {
        //act_on_clear_func(drawscreen);
        /*search_results.clear ();
        pinned_points.clear ();
        selectzoom_points.clear();
        if(menu.cour_state != 3)
        {
            path_set = false;
            dir.spec = false;
            dir.more = false;
            dir.scroll_count = 0;
            dir.dir.clear();
            path_data.clear();
            path_intersection.clear();
            menu.cour_depots.clear();
            menu.cour_intrs.clear();
        }
        textbox_text="";
        path_enabled=false;
        enable_suggested_bar=false;
        start_to_destination=0;
        arrow_key_pressed=0;
        saved_inter="";
        saved_poi="";
        get_report_structure(&res);
        drawscreen();*/
        //draw_cour_box();
        drawscreen();
        
    }
    else if(menu.active_button == 2)
    {
        drawscreen();
    }
    else
        drawscreen();
}

void draw_about_box()
{
    t_bound_box visible_world = get_visible_world();
    
    t_point curr_center = visible_world.get_center();
    
    float height = visible_world.get_height()*0.2;
    
    t_bound_box about_box(curr_center.x - height*0.8, curr_center.y - height,
            curr_center.x + height*0.8, curr_center.y + height);
    
    menu.about_box = about_box;
    
    setcolor(220,220,220);
    fillrect(about_box);
    
    t_bound_box header(about_box.left(), about_box.top() - about_box.get_height()*0.1,
            about_box.right(), about_box.top());
    
    setcolor(0,0,0);
    setfontsize(13*(res.top_height/BASE_RES));
    drawtext_in(header, "About");
    
    setfontsize(dir.minor_font_size*(res.top_height/BASE_RES));
    
    t_bound_box first(header.left(), header.bottom() - header.get_height()*2.5, header.right(), 
            header.bottom() - header.get_height()*2);
    t_bound_box second(header.left(), first.bottom() - header.get_height()*0.5, header.right(), first.bottom());
    t_bound_box third(header.left(), second.bottom() - header.get_height()*0.5, header.right(), second.bottom());
    t_bound_box fourth(header.left(), third.bottom() - header.get_height()*0.5, header.right(), third.bottom());
    t_bound_box fifth(header.left(), fourth.bottom() - header.get_height()*0.5, header.right(), fourth.bottom());
    t_bound_box sixth(header.left(), fifth.bottom() - header.get_height()*0.5, header.right(), fifth.bottom());
    t_bound_box a(header.left(), sixth.bottom() - header.get_height()*0.5, header.right(), sixth.bottom());
    t_bound_box b(header.left(), a.bottom() - header.get_height()*0.5, header.right(), a.bottom());
    t_bound_box c(header.left(), b.bottom() - header.get_height()*0.5, header.right(), b.bottom());
    t_bound_box d(header.left(), c.bottom() - header.get_height()*0.5, header.right(), c.bottom());
    t_bound_box e(header.left(), d.bottom() - header.get_height()*0.5, header.right(), d.bottom());
    //t_bound_box f(header.left(), e.bottom() - header.get_height()*0.5, header.right(), e.bottom());
    
    drawtext_in(first, "GIS system created for ECE297 at U of T");
    drawtext_in(second, "in 2015 using easyGL.");
    drawtext_in(third, "");
    drawtext_in(fourth, "Developed by:");
    drawtext_in(fifth, "Christopher Enriquez");
    drawtext_in(sixth, "Sampson Gao");
    drawtext_in(a, "Taihyun Lee");
    //drawtext_in(b, "Bg");
    if(menu.c_about_msg)
        drawtext_in(c, "\"I spent way too much time on this.\"");
    else if(menu.s_about_msg)
        drawtext_in(c, "\"I am passionate about algorithm designs.\"");
    else if(menu.r_about_msg)
        drawtext_in(c, "\"hi.\"");
    //drawtext_in(d, "Bg");
    //drawtext_in(e, "Bg");
    //drawtext_in(f, "Bg");
    
    //Ray circle
    t_bound_box mid_circ(header.left(), first.top() + first.get_height()*0.5, header.right(), header.bottom());
    menu.r_circ = t_bound_box(mid_circ.get_center().x - mid_circ.get_height()/2.5,
            mid_circ.get_center().y - mid_circ.get_height()/2.5,
            mid_circ.get_center().x + mid_circ.get_height()/2.5,
            mid_circ.get_center().y + mid_circ.get_height()/2.5);
    
    //color for circle
    setcolor(62,128,73);
    fillarc(mid_circ.get_center(), mid_circ.get_height()/2.5, 0 ,360);
    
    setcolor(255,255,255);
    setlinewidth(4);
    drawarc(mid_circ.get_xcenter(), mid_circ.get_ycenter(), mid_circ.get_height()/2.5, 0 , 360);
    
    setcolor(0,0,0);
    setlinewidth(1);
    drawarc(mid_circ.get_xcenter(), mid_circ.get_ycenter(), mid_circ.get_height()/2.5*(1+0.1), 0 , 360);
    setfontsize(18*(res.top_height/BASE_RES));
    setcolor(255,255,255);
    drawtext_in(mid_circ, "R");
    
    //Chris circle
    t_bound_box r_circ(header.left() + header.get_width()*0.405,
            first.top() + first.get_height()*0.5, header.right(), header.bottom());
    menu.c_circ = t_bound_box(r_circ.get_center().x - r_circ.get_height()/2.5,
            r_circ.get_center().y - r_circ.get_height()/2.5,
            r_circ.get_center().x + r_circ.get_height()/2.5,
            r_circ.get_center().y + r_circ.get_height()/2.5);
    
    //color for circle
    setcolor(182,24,24);
    fillarc(r_circ.get_center(), r_circ.get_height()/2.5, 0 ,360);
    
    setcolor(255,255,255);
    setlinewidth(4);
    drawarc(r_circ.get_xcenter(), r_circ.get_ycenter(), r_circ.get_height()/2.5, 0 , 360);
    
    setcolor(0,0,0);
    setlinewidth(1);
    drawarc(r_circ.get_xcenter(), r_circ.get_ycenter(), r_circ.get_height()/2.5*(1+0.1), 0 , 360);
    setfontsize(18*(res.top_height/BASE_RES));
    setcolor(255,255,255);
    drawtext_in(r_circ, "C");
    
    //Sampson circle
    t_bound_box l_circ(header.left(), first.top() + first.get_height()*0.5,
            header.right() - header.get_width()*0.4, header.bottom());
    menu.s_circ = t_bound_box(l_circ.get_center().x - l_circ.get_height()/2.5,
            l_circ.get_center().y - l_circ.get_height()/2.5,
            l_circ.get_center().x + l_circ.get_height()/2.5,
            l_circ.get_center().y + l_circ.get_height()/2.5);
    
    //color for circle
    setcolor(0,92,172);
    fillarc(l_circ.get_center(), l_circ.get_height()/2.5, 0 ,360);
    
    setcolor(255,255,255);
    setlinewidth(4);
    drawarc(l_circ.get_xcenter(), l_circ.get_ycenter(), l_circ.get_height()/2.5, 0 , 360);
    
    setcolor(0,0,0);
    setlinewidth(1);
    drawarc(l_circ.get_xcenter(), l_circ.get_ycenter(), l_circ.get_height()/2.5*(1+0.1), 0 , 360);
    setfontsize(18*(res.top_height/BASE_RES));
    setcolor(255,255,255);
    drawtext_in(l_circ, "S");
    
    menu.about_exit = t_bound_box(about_box.left() + about_box.get_width()*0.39,
                about_box.bottom()  + about_box.get_height()*0.05,
                about_box.right() - about_box.get_width()*0.39,
                about_box.bottom()  + about_box.get_height()*0.14);
    
    setcolor(230,230,230);
    fillrect(menu.about_exit);
    setcolor(0,0,0);
    drawrect(menu.about_exit);
    
    setcolor(0,0,0);
    setfontsize(dir.main_font_size*(res.top_height/BASE_RES));
    drawtext_in(menu.about_exit, "Close");
    //setcolor(header_color);
    //fillrect(header);
    
    setcolor(0,0,0);
    setlinewidth(3);
    drawrect(about_box);
}

void draw_cour_box()
{
    search_results.clear ();
    pinned_points.clear ();
    selectzoom_points.clear();
    if(menu.cour_state != 3)
    {
        path_set = false;
        dir.spec = false;
        dir.more = false;
        dir.scroll_count = 0;
        dir.dir.clear();
        dir.dir_parsed.clear();
        dir.index.clear();
        path_data.clear();
        path_intersection.clear();
    }
    textbox_text="";
    path_enabled=false;
    enable_suggested_bar=false;
    start_to_destination=0;
    arrow_key_pressed=0;
    saved_inter="";
    saved_poi="";
    get_report_structure(&res);
    
    t_bound_box visible_world = get_visible_world ();
    
    
    float height = minimap_bounds.get_height()*0.25;
    float width = minimap_bounds.get_width()*1.8;
    float offset = sqrt(pow((double)(visible_world.top() - visible_world.bottom()),(double)2) +
                    pow((double)(visible_world.right() - visible_world.left()),(double)2.0))*0.01;
    
    //sets cour box drawing coordinates to a static position, scales
    float cour_bottom = minimap_bounds.bottom();
    float cour_top = cour_bottom + height;
    float cour_right;
    //if there is no minimap, aligns to most right side of map
    if(!path_set && !(getZoomLevel() > 0))
        cour_right = visible_world.right() - offset;
    else
        cour_right = minimap_bounds.left() - offset;
    float cour_left = cour_right - width;
    
    //initialize cour_box bounds
    menu.cour_box = t_bound_box(cour_left,cour_bottom,cour_right,cour_top);
    
    //draw background
    setcolor(255,255,255);
    fillrect(menu.cour_box);
    
    //draw header
    menu.cour_header = t_bound_box(cour_left, cour_bottom, cour_left + menu.cour_box.get_width()*0.3,
            cour_top);
    setcolor(header_color);
    fillrect(menu.cour_header);
    
    //header text
    t_bound_box temp(cour_left, cour_bottom + menu.cour_box.get_height()*0.3,
            menu.cour_header.right(), cour_top);
    
    setcolor(255,255,255);
    setfontsize(dir.main_font_size*(res.top_height/BASE_RES));
    drawtext_in(temp, "Courier");
    
    temp = t_bound_box(cour_left, cour_bottom - menu.cour_box.get_height()*0.30,
            menu.cour_header.right(), cour_top);
    
    drawtext_in(temp, "Planning");
    
    if(menu.cour_state == 0 || menu.cour_state == 1)
    {
        t_bound_box next(cour_right - menu.cour_box.get_width()*0.2,
                cour_bottom + menu.cour_box.get_height()*0.5,
                cour_right - menu.cour_box.get_height()*0.1,
                cour_top - menu.cour_box.get_height()*0.1);
        menu.cour_next = next; 
        setfontsize(dir.default_font_size*(res.top_height/BASE_RES));
        if(menu.cour_onNext)
        {
            setcolor(190,190,190);
            fillrect(next);
        }
        setlinewidth(1);
        setcolor(0,0,0);
        drawrect(next);
        if(menu.cour_state == 0)
            drawtext_in(next, "Next");
        else
            drawtext_in(next, "Search");
        
        t_bound_box done(cour_right - menu.cour_box.get_width()*0.2,
                cour_bottom + menu.cour_box.get_height()*0.1,
                cour_right - menu.cour_box.get_height()*0.1,
                cour_top - menu.cour_box.get_height()*0.5);
        menu.cour_done = done;
        if(menu.cour_onDone)
        {
            setcolor(190,190,190);
            fillrect(done);
        }
        setlinewidth(1);
        setcolor(0,0,0);
        drawrect(done);
        drawtext_in(done, "Done");
        
        t_bound_box instr(menu.cour_header.right(), cour_bottom, next.left(), cour_top);
        if(menu.cour_state == 0)
            drawtext_in(instr, "Select depot locations");
        else
            drawtext_in(instr, "Select delivery locations");
       
    }
    else if(menu.cour_state == 2)
    {
        t_bound_box next(cour_right - menu.cour_box.get_width()*0.2,
                cour_bottom + menu.cour_box.get_height()*0.5,
                cour_right - menu.cour_box.get_height()*0.1,
                cour_top - menu.cour_box.get_height()*0.1);
         t_bound_box instr(menu.cour_header.right(), cour_bottom, next.left(), cour_top);
         setcolor(0,0,0);
         setlinewidth(1);
         drawtext_in(instr, "Calculating...");
    }
    else if(menu.cour_state == 3)
    {
        t_bound_box next(cour_right - menu.cour_box.get_width()*0.2,
                cour_bottom + menu.cour_box.get_height()*0.5,
                cour_right - menu.cour_box.get_height()*0.1,
                cour_top - menu.cour_box.get_height()*0.1);
        menu.cour_next = next; 
        setfontsize(dir.default_font_size*(res.top_height/BASE_RES));
        if(menu.cour_onNext)
        {
            setcolor(190,190,190);
            fillrect(next);
        }
        setlinewidth(1);
        setcolor(0,0,0);
        drawrect(next);
        drawtext_in(next, "New Search");

        
        t_bound_box done(cour_right - menu.cour_box.get_width()*0.2,
                cour_bottom + menu.cour_box.get_height()*0.1,
                cour_right - menu.cour_box.get_height()*0.1,
                cour_top - menu.cour_box.get_height()*0.5);
        menu.cour_done = done;
        if(menu.cour_onDone)
        {
            setcolor(190,190,190);
            fillrect(done);
        }
        setlinewidth(1);
        setcolor(0,0,0);
        drawrect(done);
        drawtext_in(done, "Done");
        
        t_bound_box instr(menu.cour_header.right(), cour_bottom, next.left(), cour_top);
        string in = "Showing directions from " + to_string(menu.cour_first) + " to "
                +  to_string(menu.cour_last);
        drawtext_in(instr, in);
        
    }
    //draw border
    setcolor(0,0,0);
    setlinewidth(3);
    drawrect(menu.cour_box);
        
    
}

void draw_path_box()
{
    t_bound_box visible_world = get_visible_world ();
    
    
    float height = minimap_bounds.get_height()*0.25;
    float width = minimap_bounds.get_width()*1.8;
    float offset = sqrt(pow((double)(visible_world.top() - visible_world.bottom()),(double)2) +
                    pow((double)(visible_world.right() - visible_world.left()),(double)2.0))*0.01;
    
    //sets path box drawing coordinates to a static position, scales
    float path_bottom = minimap_bounds.bottom();
    float path_top = path_bottom + height;
    float path_right;
    //if there is no minimap, aligns to most right side of map
    if(!path_set && !(getZoomLevel() > 0))
        path_right = visible_world.right() - offset;
    else
        path_right = minimap_bounds.left() - offset;
    float path_left = path_right - width;
    
    //initialize cour_box bounds
    menu.path_box = t_bound_box(path_left,path_bottom,path_right,path_top);
    
    //draw background
    setcolor(255,255,255);
    fillrect(menu.path_box);
    
    //draw header
    menu.path_header = t_bound_box(path_left, path_bottom, path_left + menu.path_box.get_width()*0.3,
            path_top);
    setcolor(header_color);
    fillrect(menu.path_header);
    
    //header text
    t_bound_box temp(path_left, path_bottom + menu.path_box.get_height()*0.3,
            menu.path_header.right(), path_top);
    
    setcolor(255,255,255);
    setfontsize(dir.main_font_size*(res.top_height/BASE_RES));
    drawtext_in(temp, "Single Path");
    
    temp = t_bound_box(path_left, path_bottom - menu.path_box.get_height()*0.30,
            menu.path_header.right(), path_top);
    
    drawtext_in(temp, "Planning");
    
    if(menu.path_state == 0)
    {
        t_bound_box next(path_right - menu.path_box.get_width()*0.2,
                path_bottom + menu.path_box.get_height()*0.5,
                path_right - menu.path_box.get_height()*0.1,
                path_top - menu.path_box.get_height()*0.1);
        menu.path_next = next; 
        setfontsize(dir.default_font_size*(res.top_height/BASE_RES));
        if(menu.path_onNext)
        {
            setcolor(190,190,190);
            fillrect(next);
        }
        setlinewidth(1);
        setcolor(0,0,0);
        drawrect(next);
        drawtext_in(next, "Search");
        
        t_bound_box done(path_right - menu.path_box.get_width()*0.2,
                path_bottom + menu.path_box.get_height()*0.1,
                path_right - menu.path_box.get_height()*0.1,
                path_top - menu.path_box.get_height()*0.5);
        menu.path_done = done;
        if(menu.path_onDone)
        {
            setcolor(190,190,190);
            fillrect(done);
        }
        setlinewidth(1);
        setcolor(0,0,0);
        drawrect(done);
        drawtext_in(done, "Done");
        
        t_bound_box instr(menu.path_header.right(), path_bottom, next.left(), path_top);
        drawtext_in(instr, "Select waypoints");

       
    }
    else if(menu.path_state == 1)
    {
        t_bound_box next(path_right - menu.path_box.get_width()*0.2,
                path_bottom + menu.path_box.get_height()*0.5,
                path_right - menu.path_box.get_height()*0.1,
                path_top - menu.path_box.get_height()*0.1);
        menu.path_next = next; 
        setfontsize(dir.default_font_size*(res.top_height/BASE_RES));
        if(menu.path_onNext)
        {
            setcolor(190,190,190);
            fillrect(next);
        }
        setlinewidth(1);
        setcolor(0,0,0);
        drawrect(next);
        drawtext_in(next, "New Path");

        
        t_bound_box done(path_right - menu.path_box.get_width()*0.2,
                path_bottom + menu.path_box.get_height()*0.1,
                path_right - menu.path_box.get_height()*0.1,
                path_top - menu.path_box.get_height()*0.5);
        menu.path_done = done;
        if(menu.path_onDone)
        {
            setcolor(190,190,190);
            fillrect(done);
        }
        setlinewidth(1);
        setcolor(0,0,0);
        drawrect(done);
        drawtext_in(done, "Done");
        
        t_bound_box instr(menu.path_header.right(), path_bottom, next.left(), path_top);
        string in = "Showing directions from A to B";
        drawtext_in(instr, in);
        
    }
    //draw border
    setcolor(0,0,0);
    setlinewidth(3);
    drawrect(menu.path_box);
        
    
}

void draw_positions(vector<unsigned> points, t_color color)
{
    setcolor(color);
    //setcolor(YELLOW);
    if(menu.cour_state == 3)
    {
        for(unsigned i = 0; i < menu.cour_depots.size(); ++i)
        {
            LatLon c_intr = getIntersectionPosition(menu.cour_depots[i]);
            t_point c_intrp = get_coordinate_from_LatLon(c_intr);
            setcolor(depot_color);
            fillarc(c_intrp,pin_size * sqrt(sqrt(get_scale()*get_scale()*get_scale())), 60, 60);
        }
        
        for(unsigned i = 0; i < bin.path_by_ID.size(); ++i)
        {
            LatLon c_intr = getIntersectionPosition(bin.path_by_ID[i]);
            t_point c_intrp = get_coordinate_from_LatLon(c_intr);
            
            if(i == 0 || i == (bin.path_by_ID.size() - 1))
            {
                setcolor(depot_color);
                t_bound_box start(c_intrp.x - pin_size * sqrt(sqrt(get_scale()*get_scale()*get_scale())),
                        c_intrp.y + pin_size * sqrt(sqrt(get_scale()*get_scale()*get_scale()))*0.23,
                        c_intrp.x + pin_size * sqrt(sqrt(get_scale()*get_scale()*get_scale())),
                        c_intrp.y + pin_size * sqrt(sqrt(get_scale()*get_scale()*get_scale())));
                fillarc(c_intrp,pin_size * sqrt(sqrt(get_scale()*get_scale()*get_scale())), 60, 60);

                setcolor(255,255,255);
                setfontsize((res.top_height/BASE_RES)*15);
                drawtext_in(start, to_string(i + 1));
            }
            else
            {
                setcolor(intr_color);
                t_bound_box start(c_intrp.x - pin_size * sqrt(sqrt(get_scale()*get_scale()*get_scale())),
                        c_intrp.y + pin_size * sqrt(sqrt(get_scale()*get_scale()*get_scale()))*0.23,
                        c_intrp.x + pin_size * sqrt(sqrt(get_scale()*get_scale()*get_scale())),
                        c_intrp.y + pin_size * sqrt(sqrt(get_scale()*get_scale()*get_scale())));
                fillarc(c_intrp,pin_size * sqrt(sqrt(get_scale()*get_scale()*get_scale())), 60, 60);

                setcolor(255,255,255);
                setfontsize((res.top_height/BASE_RES)*15);
                drawtext_in(start, to_string(i + 1));
            }
        }
        return;
    }
    if(menu.active_button == 2)
    {
        int num = menu.path_waypoints.size();
        for (int i=0; i<num; i++)
        {
            setcolor (pin_color);
            LatLon c_intr = getIntersectionPosition(menu.path_waypoints[i]);
            t_point c_intrp = get_coordinate_from_LatLon(c_intr);
            fillarc(c_intrp,pin_size * sqrt(sqrt(get_scale()*get_scale()*get_scale())), 60, 60);

           
            {
                if(i == num - 2 || menu.path_waypoints.size() == 1)
                {
                    t_bound_box start(c_intrp.x - pin_size * sqrt(sqrt(get_scale()*get_scale()*get_scale())),
                        c_intrp.y + pin_size * sqrt(sqrt(get_scale()*get_scale()*get_scale()))*0.23,
                        c_intrp.x + pin_size * sqrt(sqrt(get_scale()*get_scale()*get_scale())),
                        c_intrp.y + pin_size * sqrt(sqrt(get_scale()*get_scale()*get_scale())));

                    setcolor(255,255,255);
                    setfontsize((res.top_height/BASE_RES)*15);
                    drawtext_in(start, "A");
                }
                else if(i == num - 1)
                {
                    t_bound_box start(c_intrp.x - pin_size * sqrt(sqrt(get_scale()*get_scale()*get_scale())),
                        c_intrp.y + pin_size * sqrt(sqrt(get_scale()*get_scale()*get_scale()))*0.23,
                        c_intrp.x + pin_size * sqrt(sqrt(get_scale()*get_scale()*get_scale())),
                        c_intrp.y + pin_size * sqrt(sqrt(get_scale()*get_scale()*get_scale())));

                    setcolor(255,255,255);
                    setfontsize((res.top_height/BASE_RES)*15);
                    drawtext_in(start, "B");
                }
            }
        }
        return;
    }
    
    for(unsigned i = 0; i < points.size(); ++i)
    {
        LatLon c_intr = getIntersectionPosition(points[i]);
        t_point c_intrp = get_coordinate_from_LatLon(c_intr);
        fillarc(c_intrp,pin_size * sqrt(sqrt(get_scale()*get_scale()*get_scale())), 60, 60);
    }
}

void draw_mini_courpos(double scale, t_color color, vector<unsigned> points)
{
    setcolor(color);
    
    for (unsigned i=0; i < points.size(); ++i)
    {
        LatLon c_intr = getIntersectionPosition(points[i]);
        t_point c_intrp = get_coordinate_from_LatLon(c_intr);
        float conv_x = (c_intrp.x - map_bounds.minlon*LAT_TO_COOR_RATIO)
                        *track_spec.w_ratio + minimap_bounds.left();
        float conv_y = (c_intrp.y - map_bounds.minlat*LAT_TO_COOR_RATIO)
                        *track_spec.h_ratio + minimap_bounds.bottom();
        t_point conv(conv_x, conv_y);
        fillarc (conv, minimap_bounds.get_height()*0.045, 60, 60);
    }
    
}


void deter_cour_intrs()
{
    menu.cour_path_intrs.clear();
    
    for(unsigned i = 0; i < menu.cour_path.size(); ++i)
    {
        if(i == 0)
        {
            unsigned a = getStreetSegmentEnds(menu.cour_path[i]).from;
            unsigned b = getStreetSegmentEnds(menu.cour_path[i]).to;
            
            //know that this is second intersection
            if(a == getStreetSegmentEnds(menu.cour_path[i+1]).from ||
                    a == getStreetSegmentEnds(menu.cour_path[i+1]).to)
            {
                menu.cour_path_intrs.push_back(b);
                menu.cour_path_intrs.push_back(a);
            }
            else
            {
                menu.cour_path_intrs.push_back(a);
                menu.cour_path_intrs.push_back(b);
            }
        }
        else
        {
            unsigned a = getStreetSegmentEnds(menu.cour_path[i]).from;
            unsigned b = getStreetSegmentEnds(menu.cour_path[i]).to;
            
            if(a == menu.cour_path_intrs[i])
            {
                menu.cour_path_intrs.push_back(b);
            }
            else
            {
                menu.cour_path_intrs.push_back(a);
            }
        }
    }
}

void deter_path_intrs()
{
    menu.path_path_intrs.clear();
    
    for(unsigned i = 0; i < menu.path_path.size(); ++i)
    {
        if(i == 0)
        {
            unsigned a = getStreetSegmentEnds(menu.path_path[i]).from;
            unsigned b = getStreetSegmentEnds(menu.path_path[i]).to;
            
            //know that this is second intersection
            if(a == getStreetSegmentEnds(menu.path_path[i+1]).from ||
                    a == getStreetSegmentEnds(menu.path_path[i+1]).to)
            {
                menu.path_path_intrs.push_back(b);
                menu.path_path_intrs.push_back(a);
            }
            else
            {
                menu.path_path_intrs.push_back(a);
                menu.path_path_intrs.push_back(b);
            }
        }
        else
        {
            unsigned a = getStreetSegmentEnds(menu.path_path[i]).from;
            unsigned b = getStreetSegmentEnds(menu.path_path[i]).to;
            
            if(a == menu.path_path_intrs[i])
            {
                menu.path_path_intrs.push_back(b);
            }
            else
            {
                menu.path_path_intrs.push_back(a);
            }
        }
    }
}



void set_default_layers()
{
    waterway_layer["river"].active = true;
    waterway_layer["stream"].active = true;
    waterway_layer["riverbank"].active = true;
    waterway_layer["rapids"].active = true;
    natural_layer["water"].active = true;
    natural_layer["wood"].active = true;
    natural_layer["beach"].active = true;
    natural_layer["scrub"].active = true;
    natural_layer["n_other"].active = true;
    set_active_layers("river");
    set_active_layers("stream");
    set_active_layers("riverbank");
    set_active_layers("rapids");
    set_active_layers("water");
    set_active_layers("wood");
    set_active_layers("beach");
    set_active_layers("scrub");
    set_active_layers("n_other");
    set_layer_colors(river_color);
    set_layer_colors(stream_color);
    set_layer_colors(riverbank_color);
    set_layer_colors(rapids_color);
    set_layer_colors(water_color);
    set_layer_colors(wood_color);
    set_layer_colors(beach_color);
    set_layer_colors(scrub_color);
    set_layer_colors(nother_color);
}

void reset_states()
{
    menu.c_about_msg = false;
    menu.r_about_msg = false;
    menu.s_about_msg = false;
    menu.cour_depots.clear();
    menu.cour_intrs.clear();
    menu.active_button = -1;
    menu.cour_state = 0;
    menu.cour_path.clear();
    menu.cour_path_intrs.clear();
    path_data.clear();
    path_intersection.clear();
    path_set = false;
    dir.spec = false;
    dir.more = false;
    for(unsigned i = 0; i < dir.dir.size(); ++i)
    {
        dir.dir[i].path_seg.clear();
    }
    dir.dir.clear();
    dir.dir_parsed.clear();
    dir.index.clear();
    dir.scroll_count = 0;
    menu.path_state = 0;
    menu.path_waypoints.clear();
    menu.path_path.clear();
    menu.path_path_intrs.clear();
}