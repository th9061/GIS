/* 
 * File:   load_pathdata.cpp
 * Author: enriqu10
 * 
 * Created on March 28, 2015, 12:13 PM
 */

#include "load_pathdata.h"

load_pathdata::load_pathdata() {
}

load_pathdata::load_pathdata(const load_pathdata& orig) {
}

load_pathdata::~load_pathdata() {
}

bool compare_time(const to& a, const to& b)
{
    return a.time < b.time;
}


