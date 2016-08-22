#pragma once
#include <queue>
#include <bits/stl_queue.h>
#include "path_node.h"
#include "m1.h"
#include "m2.h"
#include "m3.h"

using namespace std;

class path
{
private:
    
public:
    unsigned start;
    unsigned end;
    
    double cost;
    vector<unsigned> path_segments;
    // vector<unsigned> path_intersection;
    
    // unordered_map<unsigned, unsigned> closed;
    // priority_queue<path_node> open;
    
    
    
    // functions
    path(unsigned _start, unsigned _end);
};

vector<unsigned> a_star_get_path (unsigned intersect_id_start, unsigned intersect_id_end);
    
//vector<unsigned> dijkstra_get_connections (unsigned intersect_id_start, vector<unsigned> intersect_id_end);



























