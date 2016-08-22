#ifndef DELIEVER_POINT_H
#define	DELIEVER_POINT_H

#include <unordered_map>
#include <vector>
#include "path.h"

using namespace std;

class DeliverPoint
{
public:
    unsigned intersection_id;
    
    unordered_map<unsigned, path> connectivity;
    
    DeliverPoint (unsigned _intersection_id);
    DeliverPoint (unsigned _intersection_id, unordered_map<unsigned, path> _connectivity);
    
    
};



#endif	/* DELIEVER_POINT_H */

