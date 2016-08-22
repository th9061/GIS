#pragma once
/* 
 * File:   layer.h
 * Author: enriqu10
 *
 * Created on March 9, 2015, 2:39 AM
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include "StreetsDatabaseAPI.h"

using namespace std;

struct layer_state {
    bool active;
    vector <int> id;
};

struct legend_bounds{
    float top;
    float bottom;
    float left;
    float right;           
};

extern unordered_map<std::string, layer_state> natural_layer;
extern unordered_map<std::string, layer_state> water_layer;
extern unordered_map<std::string, layer_state> waterway_layer;
extern unordered_map<std::string, layer_state> land_layer;

class layer {   
    private:
    
    
    vector <string> natural_type_list;
    vector <string> water_type_list;
    vector <string> waterway_type_list;
    vector <string> land_type_list;
    vector <string> type;
    bool layermenu_active;
    string layermenu_state;
    
    legend_bounds bounds;
    
    vector <string> active_layers;
    unsigned scroll_count;
    
    void sortByType();
    void draw_legend();
  
    public:
        layer();
        layer(const layer& orig);
        virtual ~layer();
        void load_data();
        
        void setLayermenuActive(bool mode);
        void setLayermenuState(string state);
        void setLegendBounds(float, float, float, float);
        void setActiveLayer(string layer);
        void removeActiveLayer(string layer);
        
        vector<string> getList(string type_);
        string getLayermenuState();
        bool getLayermenuActive();
        vector<string> getTypeList();
        
        
        
        


};

