#ifndef INTERSECTION_H
#define	INTERSECTION_H


#include <limits.h>
#include <vector>

// meaning is the starting point, usually used for the start intersection of a path
#define NO_PREVIOUS UINT_MAX

// data type for path planning
class intersection
{
private:
    unsigned id;
    unsigned previous;
    // std::vector<unsigned> adjacent_intersections;
    
public:
    intersection ();
    // intersection (unsigned _id);
    intersection (unsigned _id, unsigned _previous);
    
    // modifiers
    void setID (unsigned _id);
    void setPrevious (unsigned _edge);
    // void setPriority (double priority);  // moved into path_node class
    // void setAdjacentIntersections (std::vector<unsigned> _adjacent_intersections);
    
    // accessors
    unsigned getID ();
    unsigned getPrevious ();
    // double getPriority ();  // moved into path_node class
    // std::vector<unsigned> getAdjacentIntersections ();
 
};


#endif	/* PATH_INTERSECTION_H */

