/* 
 * File:   layer.cpp
 * Author: enriqu10
 * 
 * Created on March 9, 2015, 2:39 AM
 */

#include "layer.h"


unordered_map<std::string, layer_state> natural_layer;
unordered_map<std::string, layer_state> water_layer;
unordered_map<std::string, layer_state> waterway_layer;
unordered_map<std::string, layer_state> land_layer;


//layer_state waterways_other;

layer::layer() {
    type = {"natural", "water", "waterway", "land"};
    layermenu_active = false;
    layermenu_state = "menu";
    scroll_count = 0;
    load_data();
}

layer::layer(const layer& orig) {
}

layer::~layer() {
}

void layer::sortByType() {
    int num_features = getFeatureCount();
    
    for(int i = 0; i < num_features; ++i)
    {
        unsigned j = 0;
        string currType = type[j];
        string subType = getFeatureAttribute(i, currType);
        bool done = false;
        
        if(subType == "" && currType == "natural")
        {
            if(getFeatureAttribute(i, "land") == "" &&
                getFeatureAttribute(i,"waterway") == "" &&
                    getFeatureAttribute(i, "water") ==  "")
                subType = "n_other";
                //cout << subType << endl;
            
        }

        //determine valid return for feature id
        while(subType == "\0" && !done)
        {            
            j++;
            if(j > type.size() - 1) done = true;
            if(!done)
            {
                currType = type[j];
                subType = getFeatureAttribute(i, currType);
            }          
        }
        
        if(subType != "\0")
        {//figure out what type the attribute is
            if(currType == "natural")
            {
                //if(subType == "") cout << "hi" << endl;
                if(natural_layer.count(subType) == 0)
                {
                    layer_state temp;
                    temp.active = false;
                    temp.id.push_back(i);
                    natural_layer.insert(std::make_pair(subType, temp));
                    //push new subType string into list
                    natural_type_list.push_back(subType);
                }
                else
                {
                    natural_layer[subType].id.push_back(i);
                }
            }
            else if(currType == "water")
            {
            }
            else if(currType == "waterway")
            {
                if(subType == "weir" ||
                        subType == "wier" ||
                        subType == "yes") ;
                //make new pair if new subType and push id_list
                //else just push id into list
                else if(waterway_layer.count(subType) == 0)
                {
                    layer_state temp;
                    temp.active = false;
                    temp.id.push_back(i);
                    waterway_layer.insert(std::make_pair(subType, temp));
                    //push new subType string into list
                    waterway_type_list.push_back(subType);
                }
                else
                {
                    waterway_layer[subType].id.push_back(i);
                    
                }
            }
            else if(currType == "land")
            {
            }
        }
                    
    }
    
   // natural_other.active = false;
   // natural_type_list.push_back("n_other");
    //natural_layer.insert(std::make_pair("n_other", natural_other));

#ifdef PRINT
    cout << "there are " << natural_type_list.size() << " natural" << endl;
    cout << "but really there are " << natural_layer.size() << " natural" << endl;
    for(int i  = 0; i < natural_type_list.size(); i ++)
        cout << "for key: " << natural_type_list[i] << " there are " << 
                natural_layer[natural_type_list[i]].id.size() << " things to draw" << endl ;   
    
    cout << "there are " << waterway_type_list.size() << " natural" << endl;
    cout << "but really there are " << waterway_layer.size() << " natural" << endl;
    for(int i  = 0; i < waterway_type_list.size(); i ++)
        cout << "for key: " << waterway_type_list[i] << " there are " << 
                waterway_layer[waterway_type_list[i]].id.size() << " things to draw" << endl;
#endif
    //cout << num_features << endl;
}

void layer::load_data() {
    //type.push_back("n_other");
    //layer_state natural_other;
    //natural_other.active = false;
    //natural_other.id.push_back(0);
    //natural_layer.insert(std::make_pair(type[type.size() - 1], natural_other));
    sortByType();
   
}

void layer::draw_legend() {
    
}

vector<string> layer::getList(string type_)
{
    if(type_ == "natural")
        return natural_type_list;
    else if(type_ == "water")
        return water_type_list;
    else if(type_ == "waterway")
        return waterway_type_list;
    else if(type_ == "land")
        return land_type_list;
    else{
        vector<string> error;
        return error;
    }
}

bool layer::getLayermenuActive()
{
    return layermenu_active;
}

vector<string> layer::getTypeList(){
    return type;
}

string layer::getLayermenuState(){
    return layermenu_state;
}

void layer::setLayermenuActive(bool mode){
    layermenu_active = mode;
}

void layer::setLayermenuState(string state){
    layermenu_state = state;
}

void layer::setLegendBounds(float _top, float _bottom, float _left, float _right){
    bounds.top = _top;
    bounds.bottom = _bottom;
    bounds.left = _left;
    bounds.right = _right;
}

void layer::setActiveLayer(string layer){
    active_layers.push_back(layer);
}

void layer::removeActiveLayer(string layer){
    
    int it = 0;
    
    for(unsigned i = 0; i < active_layers.size(); ++i)
    {
        if(active_layers[i] == layer)
        {
            it = i;
            break;
        }
    }
    
    active_layers.erase(active_layers.begin() + it);
}