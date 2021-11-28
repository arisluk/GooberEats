#include "ExpandableHashMap.h"
#include <iostream> // needed for any I/O
#include <fstream>  // needed in addition to <iostream> for file I/O
#include <sstream>  // needed in addition to <iostream> for string stream I/O

#include "provided.h"
#include <string>
#include <vector>
#include <functional>
using namespace std;

unsigned int hasher(const GeoCoord& g)
{
    return std::hash<string>()(g.latitudeText + g.longitudeText);
}

//unsigned int hasher(const string& g)
//{
//    return std::hash<string>()(g);
//}

class StreetMapImpl
{
public:
    StreetMapImpl();
    ~StreetMapImpl();
    bool load(string mapFile);
    bool getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const;
    
private:
    // Data Members
    ExpandableHashMap<GeoCoord, vector<StreetSegment>> m_coordToSegs;
    // Member functions
};

StreetMapImpl::StreetMapImpl()
{
}

StreetMapImpl::~StreetMapImpl()
{
}

bool StreetMapImpl::load(string mapFile)
{
    // Open map file
    ifstream inf(mapFile);
    
    // If cannot open file
    if (!inf)
    {
        cerr << "Cannot open map file!" << endl;
        return false;
    }
    
    string line;
    while (getline(inf, line)) // Loop for every street
    {
        string streetName = line; // Save first line (street name)
        int segments; // Set up int to record number of segments
        if (!(inf >> segments)) // Save number of segments in var
        {
            cerr << "Expected segment number but found " << line << endl;
            return false; // If fails return false
        }
        inf.ignore(10000, '\n'); // Skip rest of line
        for (int i = 0; i < segments; i++) // Loop through all segments
        {
            if (!(getline(inf, line))) // Get next coord line
            {
                cerr << "Couldn't get coord line!" << endl;
                return false; // If fails return false
            }
            
            istringstream iss(line); // Set up iss for coord line
            string startLat, startLong, endLat, endLong; // Set vars for storing coords
            if (!(iss >> startLat >> startLong >> endLat >> endLong)) // Save all coords in vars
            {
                cerr << "Expected coord line but found " << line << endl;
                return false; // If fails return false
            }
            
            // Create start/end coords
            GeoCoord startCoord = GeoCoord(startLat, startLong);
            GeoCoord endCoord = GeoCoord(endLat, endLong);
            
            // Insert first pair (start coord to segment)
            StreetSegment streetSeg = StreetSegment(startCoord, endCoord, streetName);
            if (m_coordToSegs.find(startCoord) != nullptr) // If coord already exists in map, just add on to vector
                m_coordToSegs.find(startCoord)->push_back(streetSeg);
            else // If coord not already in map associate new coord
            {
                vector<StreetSegment> v = vector<StreetSegment>();
                v.push_back(streetSeg);
                m_coordToSegs.associate(startCoord, v);
            }
            
            // Insert second pair (end coord to reversed segment)
            StreetSegment reverseStreetSeg = StreetSegment(endCoord, startCoord, streetName);
            if (m_coordToSegs.find(endCoord) != nullptr) // If coord already exists in map, just add on to vector
                m_coordToSegs.find(endCoord)->push_back(reverseStreetSeg);
            else // If coord not already in map associate new coord
            {
                vector<StreetSegment> v = vector<StreetSegment>();
                v.push_back(reverseStreetSeg);
                m_coordToSegs.associate(endCoord, v);
            }
        }
    }
    
    return true; // Return true after everything is loaded
}

bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
    const vector<StreetSegment>* assocSegs = m_coordToSegs.find(gc); // Attempt to find associated segs to gc
    if (assocSegs == nullptr) // If we couldn't find, the key gc, return false
        return false;
    else // Otherwise copy over segments
        segs = *assocSegs;
    return true;  // Return true after successful get
}

//******************** StreetMap functions ************************************

// These functions simply delegate to StreetMapImpl's functions.
// You probably don't want to change any of this code.

StreetMap::StreetMap()
{
    m_impl = new StreetMapImpl;
}

StreetMap::~StreetMap()
{
    delete m_impl;
}

bool StreetMap::load(string mapFile)
{
    return m_impl->load(mapFile);
}

bool StreetMap::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
   return m_impl->getSegmentsThatStartWith(gc, segs);
}
