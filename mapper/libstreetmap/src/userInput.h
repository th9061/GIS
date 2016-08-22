/* 
 * File:   UserInput.h
 * Author: leetaihy
 *
 * Created on March 27, 2015, 9:35 PM
 */

#pragma once //protects against multiple inclusions of this header file

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include "StreetsDatabaseAPI.h"
//#include "m2.h"
#include "m1.h"
#include "intersection.h"

struct inter_information{
    std::string inter_name;
    int id;
};

class userInput{
private:
    std::string users_input;
    bool enabled;
    bool first_search_inter;
    bool first_search_poi;
    std::vector<std::vector<inter_information>> intersections;
    std::vector<std::vector<inter_information>> poi;
    
    std::vector<inter_information> matching_intersections;
    std::vector<inter_information> matching_poi;

public:
    userInput();
    userInput(std::string users);
    ~userInput();
    
    void setEnabled(bool input);
    bool getEnable();
    
    void set_users_input(std::string input);
    std::string get_users_input();
    
    void organize_intersections();
    std::vector<std::vector<inter_information>> get_intersections();
    
    void organize_poi();
    std::vector<std::vector<inter_information>> get_poi();
    
    string ConvertToCaps(std::string input);
    string switchWords(std::string input);
    char getfirstcharacter(std::string input);
    
    void find_match_intersections(std::string input);
    void find_match_poi(std::string input);
    
    void sort_matching_intersections(std::string input);
    void sort_matching_poi(std::string input);
    
    std::string get_first_intersection();
    std::string get_first_poi();
    
    unsigned get_first_intersectionID();
    unsigned get_first_poiID();
    
    void set_search_first(bool input);
    
    void print(std::string output);
};





