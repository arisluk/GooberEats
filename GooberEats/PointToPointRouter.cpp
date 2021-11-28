#include "provided.h"
#include <list>

#include <set>
#include <map>
#include <cmath> // For distance calculations
#include <float.h> // For DBL_MAX

#include <iostream>

using namespace std;

class PointToPointRouterImpl
{
public:
    PointToPointRouterImpl(const StreetMap* sm);
    ~PointToPointRouterImpl();
    DeliveryResult generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const;
    
private:
    // Private structs
    struct DoubleMaxVal // Default starting g/f values for A Star (double max value)
    {
        DoubleMaxVal() {score = DBL_MAX;}
        double score;
    };
    struct CoordSegPair // To store both previous coord and segment in traceback map
    {
        CoordSegPair()
        {
            coord = GeoCoord();
            seg = StreetSegment();
        }
        GeoCoord coord;
        StreetSegment seg;
    };
    // Data members
    const StreetMap* m_streetMap;
    // Private Member Functions
    double distance(const GeoCoord& start, const GeoCoord& end) const; // Returns euclidean distance between two coords
};

PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm)
{
    m_streetMap = sm;
}

PointToPointRouterImpl::~PointToPointRouterImpl()
{
}

DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    // TEST FOR BAD COORDS
    vector<StreetSegment> temp; // Create temp vector to test if start/end is valid coords using getSegmentsThatStartWith
    // Try start/end coords
    if (!m_streetMap->getSegmentsThatStartWith(start, temp) || !m_streetMap->getSegmentsThatStartWith(end, temp))
        return BAD_COORD; // Return if bad coord
    
    // A STAR ROUTING
    // Set up structures for A Star
    set<GeoCoord> openSet; // Set of nodes we are going to explore
    map<GeoCoord, CoordSegPair> cameFrom; // Map GeoCoord to CoordSegPair as we traverse a path so we can trace back
    map<GeoCoord, DoubleMaxVal> gScore; // Map GeoCoord to g score (a double that defaults to infinity/max val)
    map<GeoCoord, DoubleMaxVal> fScore; // Map GeoCoord to f score (a double that defaults to infinity/max val)
    
    openSet.insert(start); // Start node exploration at start coord
    gScore[start].score = 0; // Set start node g score to 0 because the distance from start to start is 0
    fScore[start].score = distance(start, end);
    
    while (!(openSet.empty())) // Loop while there are more nodes to explore
    {
        // Get node with lowest f score in openSet
        auto currentPtr = openSet.begin();
        GeoCoord current = *(openSet.begin());
        for (auto it = openSet.begin(); it != openSet.end(); it++) // Loop through all coords in set
        {
            if (fScore[*it].score < fScore[current].score) // Check if f score of coord in iterator is < than in current
            {
                current = *it; // Set current to coord w/ smaller f score
                currentPtr = it; // Set currentPtr to point to coord w/ smaller f score
            }
        }
        
        // Check if we found end
        if (current == end)
        {
            // Reconstruct full path
            list<StreetSegment> tempRoute; // Construct temp route list to store current route
            while (cameFrom.find(current) != cameFrom.end()) // Loop while current coord is still in trackback path
            {
                tempRoute.push_front(cameFrom[current].seg); // Push current seg to front of tempRoute
                totalDistanceTravelled += distanceEarthMiles(cameFrom[current].coord, current); // Add distance to count
                current = cameFrom[current].coord; // Go back one coord on path
            }
            route.splice(route.end(), tempRoute); // Append tempRoute to the end of the passed route var
            return DELIVERY_SUCCESS; // Return if we get to the end
        }
        
        openSet.erase(currentPtr); // Erase current coord from set
        // Get segments connected to current coord
        vector<StreetSegment> neighborSegs;
        m_streetMap->getSegmentsThatStartWith(current, neighborSegs);
        
        for (size_t i = 0; i < neighborSegs.size(); i++) // Loop through every connected seg
        {
            GeoCoord neighborCoord = neighborSegs[i].end;
            double tempGScore = gScore[current].score + distance(current, neighborCoord); // Calculate gScore of neighbor through current coord
            if (tempGScore < gScore[neighborCoord].score) // Check if tempGScore is better than currently stored g score (better path)
            {
                cameFrom[neighborCoord].coord = current; // Record coord in path so far
                cameFrom[neighborCoord].seg = neighborSegs[i]; // Record segment in path so far
                gScore[neighborCoord].score = tempGScore; // Update gScore
                fScore[neighborCoord].score = gScore[neighborCoord].score + distance(neighborCoord, end); // Update fScore
                if (openSet.find(neighborCoord) == openSet.end()) // If openSet doesn't have the neighbor coord
                    openSet.insert(neighborCoord); // Add neighbor coord to openSet
            }
        }
    }
    
    return NO_ROUTE;  // Return if no route found
}

double PointToPointRouterImpl::distance(const GeoCoord& start, const GeoCoord& end) const
{
    // Uses pythagorean theorem
    return abs(sqrt(pow((end.latitude - start.latitude), 2.0) +  pow((end.longitude - start.longitude), 2.0)));
}

//******************** PointToPointRouter functions ***************************

// These functions simply delegate to PointToPointRouterImpl's functions.
// You probably don't want to change any of this code.

PointToPointRouter::PointToPointRouter(const StreetMap* sm)
{
    m_impl = new PointToPointRouterImpl(sm);
}

PointToPointRouter::~PointToPointRouter()
{
    delete m_impl;
}

DeliveryResult PointToPointRouter::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    return m_impl->generatePointToPointRoute(start, end, route, totalDistanceTravelled);
}
