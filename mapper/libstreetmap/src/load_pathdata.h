/* 
 * File:   load_pathdata.h
 * Author: enriqu10
 *
 * Created on March 28, 2015, 12:13 PM
 */

#ifndef LOAD_PATHDATA_H
#define	LOAD_PATHDATA_H
#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <unordered_map>
#include "m3.h"

using namespace std;

struct to
{
    unsigned id;
    double time;
    vector<unsigned> path;
    
    /*bool operator<(const to &other) const
    {
        return this->time < other.time;
    }*/
};

bool compare_time(const to& a, const to& b);

class load_pathdata 
{
public:
    unordered_map<unsigned, vector<to>> intersection_to_other;
    unordered_map<unsigned, vector<to>> intersection_to_depots;
    unordered_map<unsigned, vector<to>> depot_to_other;
    vector<unsigned> path_by_ID;
    load_pathdata();
    load_pathdata(const load_pathdata& orig);
    virtual ~load_pathdata();

private:

};

#endif	/* LOAD_PATHDATA_H */

