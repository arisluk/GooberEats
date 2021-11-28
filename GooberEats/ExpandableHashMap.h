// ExpandableHashMap.h

// Skeleton for the ExpandableHashMap class template.  You must implement the first six
// member functions.

#include <list>

template<typename KeyType, typename ValueType>
class ExpandableHashMap
{
public:
	ExpandableHashMap(double maximumLoadFactor = 0.5);
	~ExpandableHashMap();
	void reset();
	int size() const;
	void associate(const KeyType& key, const ValueType& value);

	  // for a map that can't be modified, return a pointer to const ValueType
	const ValueType* find(const KeyType& key) const;

	  // for a modifiable map, return a pointer to modifiable ValueType
	ValueType* find(const KeyType& key)
	{
		return const_cast<ValueType*>(const_cast<const ExpandableHashMap*>(this)->find(key));
	}

	  // C++11 syntax for preventing copying and assignment
	ExpandableHashMap(const ExpandableHashMap&) = delete;
	ExpandableHashMap& operator=(const ExpandableHashMap&) = delete;

private:
    // Node struct for hashmap
    struct Node
    {
        Node(KeyType k, ValueType v)
        {
            m_key = k;
            m_value = v;
        }
        KeyType m_key;
        ValueType m_value;
    };
    // Data members
    double m_maxLoadFactor; // Max load
    int m_size; // Current size counter
    unsigned int m_numBuckets; // Current number of buckets counter
    std::list<Node>* m_map; // Array of linked lists of nodes
    // Private member functions
    unsigned int getBucketNum(const KeyType& key) const; // Hashes a key and returns a corresponding bucket to hashed value
    void expandMap(unsigned int size); // Rehashes all items in map and inserts it into new map structure with size as inputted
};

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::ExpandableHashMap(double maximumLoadFactor)
{
    // Set up maxLoadFactor and other default data members
    m_maxLoadFactor = maximumLoadFactor;
    m_size = 0;
    m_numBuckets = 8;
    m_map = new std::list<Node>[8];
}

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::~ExpandableHashMap()
{
    delete[] m_map; // Deletes dynamically allocated array of lists
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::reset()
{
    delete[] m_map; // Deletes current dynamically allocated array of lists
    m_map =  new std::list<Node>[8]; // Dynamically allocated new clean map of default bucket num
    m_size = 0; // Reset size
    m_numBuckets = 8; // Reset bucket num
}

template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType, ValueType>::size() const
{
    return m_size;
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::associate(const KeyType& key, const ValueType& value)
{
    ValueType* foundValue = find(key); // Search for key in map
    if (foundValue != nullptr) // If key is found
    {
        *foundValue = value; // Set new value and return
        return;
    }
    
    if ((m_size+1.0)/m_numBuckets > m_maxLoadFactor) // Otherwise check if load is > maxLoad
        expandMap(m_numBuckets*2); // Expand the map
    
    unsigned int bucket = getBucketNum(key); // Get bucket for new key
    m_map[bucket].push_back(Node(key, value)); // Push key value pair into bucket
    m_size++; // Increase size counter
}

template<typename KeyType, typename ValueType>
const ValueType* ExpandableHashMap<KeyType, ValueType>::find(const KeyType& key) const
{
    unsigned int bucket = getBucketNum(key); // Find associated bucket for key
    for (auto it = m_map[bucket].begin(); it != m_map[bucket].end(); it++) // Loop through linked list in bucket
    {
        if (it->m_key == key) // If key found return pointer to value
            return &(it->m_value);
    }
    return nullptr; // return nullptr if not found
}

// Private member function implementations

template<typename KeyType, typename ValueType>
unsigned int ExpandableHashMap<KeyType, ValueType>::getBucketNum(const KeyType& key) const
{
    unsigned int hasher(const KeyType& k); // Prototype hasher
    unsigned int hash = hasher(key); // Hash key
    unsigned int bucket = hash % m_numBuckets; // Modulus hash by number of buckets
    
    return bucket; // Return bucket number
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::expandMap(unsigned int size)
{
    // Record old num of buckets and set data member to new size
    unsigned int oldNumBuckets = m_numBuckets;
    m_numBuckets = size;
    
    std::list<Node>* newMap = new std::list<Node>[size]; // Create new array of linked lists of new size
    
    for (int i = 0; i < oldNumBuckets; i++) // Loop through all buckets in old array
    {
        auto it = m_map[i].begin(); // Make new iterator to point to first list element
        while(it != m_map[i].end()) // Loop through all nodes in list for each bucket
        {
            unsigned int newBucket = getBucketNum(it->m_key); // Get new bucket number
            newMap[newBucket].splice(newMap[newBucket].end(), m_map[i], it); // Insert current item into new bucket in new map
            it = m_map[i].begin(); // Reset iterator to start of map because splice removes item from original list
        }
    }
    
    delete[] m_map; // Delete old map
    
    m_map = newMap; // Assign new map to Data Member
}
