#include "deliver_point.h"
#include <vector>
#include <unordered_map>

#include "path.h"
using namespace std;

DeliverPoint::DeliverPoint (unsigned _intersection_id)
{
    intersection_id = _intersection_id;
}


DeliverPoint::DeliverPoint (unsigned _intersection_id, unordered_map<unsigned, path> _connectivity)
{
    intersection_id = _intersection_id;
    connectivity = _connectivity;
}








