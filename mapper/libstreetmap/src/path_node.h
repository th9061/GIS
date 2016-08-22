/* 
 * File:   path_node.h
 * Author: gaoshua1
 *
 * Created on March 9, 2015, 6:33 PM
 */

#ifndef PATH_NODE_H
#define	PATH_NODE_H

#include "intersection.h"

class path_node 
{
private:
    //intersection* node;
    
    unsigned id;
    unsigned fromSegment;
    double priority;
    
public:
    path_node ();
    path_node (unsigned _id);
    //path_node (intersection* _node);
    //path_node (intersection* _node, double _priority);
    // customized copy constructor and destructor are not needed since the pointer is not supposed to dynamically allocate any new space
    
    // modifiers
    //void setNode (intersection* _id);
    void setID (unsigned _id);
    void setPriority (double priority);
    void setFromSegment (unsigned _fromSegment);
    
    // accessors
    //intersection* getNode ();
    unsigned getID ();
    double getPriority ();
    unsigned getFromSegment ();
    
    // comparisons
    bool operator< (path_node right) const;
    bool operator> (path_node right) const;
    bool operator== (path_node right) const;
    bool operator<= (path_node right) const;
    bool operator>= (path_node right) const;
    bool operator!= (path_node right) const;
    
};

#endif	/* PATH_NODE_H */

