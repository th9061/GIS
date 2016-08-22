#include <vector>
#include "intersection.h"


using namespace std;


intersection::intersection ()
{
    id = -1;
    previous = NO_PREVIOUS;
    // priority = 0;
}

/*intersection::intersection (unsigned _id)
{
    id = _id;
}*/

intersection::intersection (unsigned _id, unsigned _previous)
{
    id = _id;
    previous = _previous;
    // priority = 0;
}

void intersection::setID (unsigned _id)
{
    id = _id;
}

void intersection::setPrevious (unsigned _previous)
{
    previous = _previous;
}

/* // moved to path_node class
void intersection::setPriority (double _priority)
{
    priority = _priority;
}
 */

/*
void intersection::setAdjacentIntersections (vector<unsigned> _adjacent_intersections)
{
    adjacent_intersections = _adjacent_intersections;
}*/



unsigned intersection::getID ()
{
    return id;
}

unsigned intersection::getPrevious ()
{
    return previous;
}

/*std::vector<unsigned> intersection::getAdjacentIntersections ()
{
    return adjacent_intersections;
}*/
