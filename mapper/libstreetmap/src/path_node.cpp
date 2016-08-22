#include "path_node.h"


path_node::path_node ()
{
    
}

path_node::path_node (unsigned _id)
{
    id = _id;
    
}

/*
path_node::path_node (intersection* _node)
{
    // this is for referencing existing data only, so this is OK as
    // long as not using this pointer in deletion or dynamic allocation
    node = _node;
}

path_node::path_node (intersection* _node, double _priority)
{
    // this is for referencing existing data only, so this is OK as
    // long as not using this pointer in deletion or dynamic allocation
    node = _node;
    priority = _priority;
}


// destructor is not needed since the pointer is not supposed to dynamically allocate any new space

// modifiers
void path_node::setNode (intersection* _node)
{
    node = _node;
}
*/

void path_node::setID (unsigned _id)
{
    id = _id;
}

void path_node::setPriority (double _priority)
{
    priority = _priority;
}

void path_node::setFromSegment (unsigned _fromSegment)
{
    fromSegment = _fromSegment;
}

/*
// accessors
intersection* path_node::getNode ()
{
    return node;
}
*/

unsigned path_node::getID ()
{
    return id;
}


double path_node::getPriority ()
{
    return priority;
}

unsigned path_node::getFromSegment ()
{
    return fromSegment;
}



// comparisons
bool path_node::operator< (path_node right) const
{
    if (priority > right.priority)
    {
        return true;
    }
    return false;
}

bool path_node::operator> (path_node right) const
{
    if (priority < right.priority)
    {
        return true;
    }
    return false;
}

bool path_node::operator== (path_node right) const
{
    if (priority == right.priority)
    {
        return true;
    }
    return false;
}

bool path_node::operator<= (path_node right) const
{
    if (priority >= right.priority)
    {
        return true;
    }
    return false;
}

bool path_node::operator>= (path_node right) const
{
    if (priority <= right.priority)
    {
        return true;
    }
    return false;
}

bool path_node::operator!= (path_node right) const
{
    if (priority != right.priority)
    {
        return true;
    }
    return false;
}
