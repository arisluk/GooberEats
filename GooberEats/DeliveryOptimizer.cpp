#include "provided.h"
#include <vector>
using namespace std;

class DeliveryOptimizerImpl
{
public:
    DeliveryOptimizerImpl(const StreetMap* sm);
    ~DeliveryOptimizerImpl();
    void optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const;
    
private:
    // Data members
    const StreetMap* m_streetMap; // Pointer to a StreetMap
};

DeliveryOptimizerImpl::DeliveryOptimizerImpl(const StreetMap* sm)
{
    m_streetMap = sm;
}

DeliveryOptimizerImpl::~DeliveryOptimizerImpl()
{
}

void DeliveryOptimizerImpl::optimizeDeliveryOrder(
    const GeoCoord& depot,
    vector<DeliveryRequest>& deliveries,
    double& oldCrowDistance,
    double& newCrowDistance) const
{
    oldCrowDistance = 0; // Reset oldCrowDistance
    oldCrowDistance += distanceEarthMiles(depot, deliveries[0].location); // Add distance from depot to first location to oldCrowDistance
    for (size_t i = 1; i < deliveries.size(); i++) // Loop through deliveries starting from second element to end
    {
        oldCrowDistance += distanceEarthMiles(deliveries[i].location, deliveries[i-1].location); // Add distance from previous to current delivery to oldCrowDistance
    }
    
    // Optimize deliveries by finding shortest crows distance path
    vector<DeliveryRequest> optimizedDeliveries; // Create vector to store optimized deliveries
    GeoCoord currentCoord = depot; // Store current last coord in path
    while (deliveries.size() > 0) // Loop while there are still more points
    {
        // SEARCH deliveries FOR NEXT CLOSEST DELIVERY
        auto it = deliveries.begin(); // Setup iterator to loop through deliveries
        auto closest = it; // Store iterator to closest delivery (start with first element)
        while (it != deliveries.end()) // Loop through all deliveries
        {
            if (distanceEarthKM(currentCoord, it->location) < distanceEarthKM(currentCoord, closest->location))
                closest = it; // Replace closest if closer one is found
            it++;
        }
        
        optimizedDeliveries.push_back(*closest); // Push next closest delivery into optimized vector
        deliveries.erase(closest); // Remove optimized DeliveryRequest from deliveries
    }
    // Replace reference deliveries vector with optimmized one
    deliveries = optimizedDeliveries;
    
    newCrowDistance = 0; // Reset newCrowDistance
    newCrowDistance += distanceEarthMiles(depot, deliveries[0].location); // Add distance from depot to first location to oldCrowDistance
    for (size_t i = 1; i < deliveries.size(); i++) // Loop through deliveries starting from second element to end
    {
        newCrowDistance += distanceEarthMiles(deliveries[i].location, deliveries[i-1].location); // Add distance from previous to current delivery to oldCrowDistance
    }
}

//******************** DeliveryOptimizer functions ****************************

// These functions simply delegate to DeliveryOptimizerImpl's functions.
// You probably don't want to change any of this code.

DeliveryOptimizer::DeliveryOptimizer(const StreetMap* sm)
{
    m_impl = new DeliveryOptimizerImpl(sm);
}

DeliveryOptimizer::~DeliveryOptimizer()
{
    delete m_impl;
}

void DeliveryOptimizer::optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const
{
    return m_impl->optimizeDeliveryOrder(depot, deliveries, oldCrowDistance, newCrowDistance);
}
