#include "provided.h"
#include <vector>
#include <list>
#include <string>
using namespace std;

class DeliveryPlannerImpl
{
public:
    DeliveryPlannerImpl(const StreetMap* sm);
    ~DeliveryPlannerImpl();
    DeliveryResult generateDeliveryPlan(
        const GeoCoord& depot,
        const vector<DeliveryRequest>& deliveries,
        vector<DeliveryCommand>& commands,
        double& totalDistanceTravelled) const;
    
private:
    // Data Members
    const StreetMap* m_streetMap; // Pointer to StreetMap
    // Member functions
    string getDirection(const StreetSegment& s) const;
};

DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm)
{
    m_streetMap = sm;
}

DeliveryPlannerImpl::~DeliveryPlannerImpl()
{
}

DeliveryResult DeliveryPlannerImpl::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    // OPTIMIZE DELIVERIES
    
    DeliveryOptimizer dOptimizer(m_streetMap); // Construct delivery optimizer
    double oldCrowsDist, newCrowsDist; // Setup vars to optimize delivery
    vector<DeliveryRequest> optimizedDeliveries = deliveries; // Create new vector to store optimized deliveries
    dOptimizer.optimizeDeliveryOrder(depot, optimizedDeliveries, oldCrowsDist, newCrowsDist); // Optimize delivery
    if (newCrowsDist > oldCrowsDist) // If optimization makes it slower
        optimizedDeliveries = deliveries; // Reset optimized to orginal order
    
    // GENERATE POINT TO POINT ROUTE
    
    PointToPointRouter p2pRouter(m_streetMap); // Construct PointToPointRouter
    list<StreetSegment> route; // Construct route list to store route
    double totalDistTravelled; // Construct var to store total distance
    
    DeliveryResult dr = p2pRouter.generatePointToPointRoute(depot, optimizedDeliveries[0].location, route, totalDistTravelled); // Attempt to generage route from depot to first delivery
    if (dr != DELIVERY_SUCCESS) // Return error if not success
        return dr;
    
    size_t i = 1; // Set up traversal variable to traverse deliveries
    for (i = 1; i < optimizedDeliveries.size(); i++) // For each delivery location (starting from second location)
    {
        // Attempt to generate point to point route from each delivery to next (up to last delivery)
        dr = p2pRouter.generatePointToPointRoute(optimizedDeliveries[i-1].location, optimizedDeliveries[i].location, route, totalDistTravelled);
        if (dr != DELIVERY_SUCCESS) // Return error if not success
            return dr;
    }
    dr = p2pRouter.generatePointToPointRoute(optimizedDeliveries[optimizedDeliveries.size()-1].location, depot, route, totalDistTravelled); // Attempt to generate route from last delivery location back to depot
    if (dr != DELIVERY_SUCCESS) // Return error if not success
        return dr;
    
    totalDistanceTravelled = totalDistTravelled; // Update total distance
    
    // PROCESS ROUTE INTO DELIVERY COMMANDS
    
    auto routeIt = route.begin(); // Set up route iterator
    auto routePeeker = routeIt; // Set up iterator to peek at next segment
    routePeeker++; // Advance iterator to the segment after current
    
    double currentStreetDist = 0; // Set up tracker for distance in current path
    int deliveryIndex = 0; // Set up counter for which delivery we are on currently (start at first element)
    StreetSegment streetStart = *(route.begin()); // Set up var to track the first segment in current proceed path
    bool justDelivered = false; // Set up var to track whether we just delivered
    
    // First check all the beginning deliveries that are at the starting depot
    while (depot == optimizedDeliveries[deliveryIndex].location && deliveryIndex < optimizedDeliveries.size())
    {
        // Create deliver command and push back into commands vector
        DeliveryCommand deliver;
        deliver.initAsDeliverCommand(optimizedDeliveries[deliveryIndex].item);
        commands.push_back(deliver);
        deliveryIndex++; // Increment deliveries made
    }
    
    if (deliveryIndex == optimizedDeliveries.size()) // If thats all deliveries
        return DELIVERY_SUCCESS;
    
    // Otherwise, if there are more deliveries, loop
    while (routePeeker != route.end()) // Loop through whole route
    {
        // Check if delivery is to be made at curent location, and do all of them (if > 1)
        while (routePeeker->start == optimizedDeliveries[deliveryIndex].location && deliveryIndex < optimizedDeliveries.size())
        {
            // Conclude previous proceed command (if there is one)
            if (currentStreetDist != 0) // If there is a current route
            {
                currentStreetDist += distanceEarthMiles(routeIt->start, routeIt->end); // Add to current path distance
                // Generate previous proceed command
                DeliveryCommand proceed;
                proceed.initAsProceedCommand(getDirection(streetStart), routeIt->name, currentStreetDist); // init as proceed cmd using getDirection for direction
                commands.push_back(proceed); // Add proceed to commands
                streetStart = *routePeeker; // Update first street segment
            }
            // Create deliver command and push back into commands vector
            DeliveryCommand deliver;
            deliver.initAsDeliverCommand(optimizedDeliveries[deliveryIndex].item);
            commands.push_back(deliver);
            // Reset curr path distance and increment deliveryIndex
            currentStreetDist = 0;
            deliveryIndex++;
            justDelivered = true; // Update justDelivered
        }
        if (justDelivered) // If we just finished delivering, increment iterators and continue
        {
            routeIt++;
            routePeeker++;
            justDelivered = false; // Update justDelivered
            continue;
        }
        
        // If we don't deliver this turn, we continue path
        if (routeIt->name == routePeeker->name) // If we continue to be on the same street
            currentStreetDist += distanceEarthMiles(routeIt->start, routeIt->end); // Add to current path distance
        else // If we go on a new street
        {
            // Conclude previous proceed command
            currentStreetDist += distanceEarthMiles(routeIt->start, routeIt->end); // Add to current path distance
            // Generate previous proceed command
            DeliveryCommand proceed;
            proceed.initAsProceedCommand(getDirection(streetStart), routeIt->name, currentStreetDist); // init as proceed cmd using getDirection for direction
            commands.push_back(proceed); // Add proceed to commands
            // Reset counter and update starting street segment
            currentStreetDist = 0;
            streetStart = *routePeeker;
            
            // Test for turn
            double angleBtwn = angleBetween2Lines(*routeIt, *routePeeker); // Get angle
            if (angleBtwn < 1.0 || angleBtwn > 359.0) // No turn
            {}
            else if (angleBtwn >= 1.0 && angleBtwn < 180.0) // Left turn
            {
                DeliveryCommand turn;
                turn.initAsTurnCommand("left", routePeeker->name);
                commands.push_back(turn);
            }
            else if (angleBtwn >= 180.0 && angleBtwn <= 359.0) // Right turn
            {
                DeliveryCommand turn;
                turn.initAsTurnCommand("right", routePeeker->name);
                commands.push_back(turn);
            }
        }
        routeIt++;
        routePeeker++;
    }
    
    // Conclude last proceed command
    currentStreetDist += distanceEarthMiles(routeIt->start, routeIt->end); // Add to current path distance
    // Generate previous proceed command
    DeliveryCommand proceed;
    proceed.initAsProceedCommand(getDirection(*routeIt), routeIt->name, currentStreetDist); // init as proceed cmd using getDirection for direction
    commands.push_back(proceed); // Add proceed to commands
    
    // Repeatedly check deliveries for last point until the end
    while (routeIt->end == optimizedDeliveries[deliveryIndex].location && deliveryIndex < optimizedDeliveries.size())
    {
        // Create deliver command and push back into commands vector
        DeliveryCommand deliver;
        deliver.initAsDeliverCommand(optimizedDeliveries[deliveryIndex].item);
        commands.push_back(deliver);
        deliveryIndex++;
    }
    
    return DELIVERY_SUCCESS; // Return success
}

string DeliveryPlannerImpl::getDirection(const StreetSegment& s) const
{
    double angle = angleOfLine(s); // Get angle of street segment
    
    if (angle < 0.0 || angle >= 360.0) // Error Check
        return "ERROR - BAD ANGLE";
    
    // Determine which direction angle is in
    if (angle < 22.5)
        return "east";
    else if (angle < 67.5)
        return "northeast";
    else if (angle < 112.5)
        return "north";
    else if (angle < 157.5)
        return "northwest";
    else if (angle < 202.5)
        return "west";
    else if (angle < 247.5)
        return "southwest";
    else if (angle < 292.5)
        return "south";
    else if (angle < 337.5)
        return "southeast";
    else if (angle < 360.0)
        return "east";
    else
        return "ERROR - BAD ANGLE";
}

//******************** DeliveryPlanner functions ******************************

// These functions simply delegate to DeliveryPlannerImpl's functions.
// You probably don't want to change any of this code.

DeliveryPlanner::DeliveryPlanner(const StreetMap* sm)
{
    m_impl = new DeliveryPlannerImpl(sm);
}

DeliveryPlanner::~DeliveryPlanner()
{
    delete m_impl;
}

DeliveryResult DeliveryPlanner::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    return m_impl->generateDeliveryPlan(depot, deliveries, commands, totalDistanceTravelled);
}
