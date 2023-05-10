/*
 * SharedPointerPool.hh
 *
 * Adapted from goto40's implementation in https://stackoverflow.com/questions/31090302
 *
 * This implementation keeps nodesInUse_ as a map<T*, unique_ptr<T>>, matching
 * a raw "T*" to the associated unique_ptr<T>. Since std::map is a sorted
 * container, finding nodes in use by their associated raw T* is O(log(n))
 * problem rather than O(n), so free_() moves items from nodesInUse_ to
 * freeNodes_ *MUCH* faster.
 *
 *  Created on: Apr 25, 2023
 *      Author: Chris Burghart <burghart@ucar.edu>
 */

#ifndef SHAREDPOINTERPOOL_HH_
#define SHAREDPOINTERPOOL_HH_

#include <algorithm>
#include <functional>
#include <atomic>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>

/// Class which provides shared pointers to nodes from a pool of
/// type T items.
///
/// Nodes which are allocated from the pool are automatically returned to the
/// pool when their reference counts go to zero.
///
/// @param T the type of node for which shared pointers will be kept
/// @param grow_on_demand if true, a new node will be added to the pool
///        automatically if no free nodes are available when alloc() is called
template<class T, bool grow_on_demand=true>
class SharedPointerPool
{
public:
    /// @brief constructor
    /// @param n the number of objects initially allocated in the pool
    /// @param factoryFunction std::function used to create new T* nodes for
    /// the pool (default is just "new T()").
    SharedPointerPool(size_t n = 0, std::function<T*()> factoryFunction = []() { return new T(); }) :
        nodeCount_(n),
        mutex_(),
        freeNodes_(),
        nodesInUse_(),
        factoryFunction_(factoryFunction)
    {
        // Start the pool with the requested number of nodes
        for (size_t i = 0; i < nodeCount_; i++)
        {
            // Create a unique_ptr to a  newly created node and push it onto
            // freeNodes_
            freeNodes_.push_back(std::unique_ptr<T>(factoryFunction_()));
        }
    }

    /// @brief destructor
    virtual ~SharedPointerPool() = default;

    /// @brief Return a shared_ptr<T> for a node from the pool.
    ///
    /// If no nodes are currently available, a new node is created and added to
    /// the pool.
    std::shared_ptr<T> alloc()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (freeNodes_.empty())
        {
            if (grow_on_demand)
            {
                freeNodes_.push_back(std::unique_ptr<T>(factoryFunction_()));
                nodeCount_++;
            }
            else
            {
                throw std::bad_alloc();
            }
        }

        // Get a reference to the last unique_ptr<T> in freeNodes_. We'll shift
        // its ownership to nodesInUse_ below using std::move.
        std::unique_ptr<T>& uPtr(freeNodes_.back());

        // Get the raw T* pointer from the unique_ptr
        T* rawPtr = uPtr.get();

        // Create a shared_ptr<T> from the raw pointer. A lambda using our
        // _free() method is passed in as the new shared_ptr's destructor; it
        // will move the associated unique_ptr back to freeNodes_ as soon as the
        // shared_ptr's reference count goes to zero.
        std::shared_ptr<T> sptr(rawPtr, [=](T* ptr){ this->free_(ptr); });

        // Now shift the unique_ptr<T> ownership from freeNodes_ to nodesInUse_,
        // and remove the associated node from freeNodes_.
        nodesInUse_[rawPtr] = std::move(uPtr);
        freeNodes_.pop_back();

        return sptr;
    }

    /// @brief Return the count of all nodes in the pool (used and free)
    inline size_t getCount() { return nodeCount_; }

    /// @brief Return the number of free nodes in the pool
    size_t getFreeCount()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        return freeNodes_.size();
    }

    /// @brief Return the number of pool nodes currently in use
    size_t getInUseCount()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        return nodesInUse_.size();
    }

    /// @brief Set the total pool size to a given node count
    /// @param size the desired new total pool size
    /// @throw std::runtime error if the new size is smaller than the number
    /// of nodes currently in use
    void resize(size_t size)
    {
        std::unique_lock<std::mutex> lock(mutex_);

        // We can't resize smaller than the count of nodes currently in use
        if (size < nodesInUse_.size()) {
            std::ostringstream os;
            os << "Cannot resize SharedPointerPool to " << size
               << " nodes: " << nodesInUse_.size() << " nodes are in use!";
            throw std::runtime_error(os.str());
        }

        int delta = size - getCount();
        if (delta == 0)
        {
            // no change
        }
        else if (delta < 0)
        {
            // Permanently delete nodes from freeNodes()
            for (int i = delta; i; i++)
            {
                freeNodes_.pop_back();
                nodeCount_--;
            }
        }
        else
        {
            // Add new free nodes
            for (int i = 0; i < delta; i++)
            {
                freeNodes_.push_back(std::unique_ptr<T>(factoryFunction_()));
                nodeCount_++;
            }
        }
    }
private:

    /// @brief Move the unique_ptr associated with a given raw pointer from the
    // "in use" list to the "free" list
    /// @param objPtr raw pointer to the node to be returned to the free list
    void free_(T* objPtr)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        /// Find the unique_ptr in nodesInUse_ and move it to freeNodes_
        auto it = nodesInUse_.find(objPtr);
        if (it != nodesInUse_.end())
        {
            freeNodes_.push_back(std::move(it->second));    // it->second is the unique_ptr<T>
            nodesInUse_.erase(it);
        }
        else
        {
            std::ostringstream os;
            os << "SharedPointerPool::_free(): failed to free unknown node 0x"
               << std::hex << objPtr;
            throw std::runtime_error(os.str());
        }
    }

    /// @brief Total count of nodes in the pool, both free and in-use
    std::atomic<size_t> nodeCount_;
    std::mutex mutex_;

    /// @brief Set of unique_ptr-s to all of the free nodes
    std::vector<std::unique_ptr<T>> freeNodes_;

    /// @brief Container mapping raw node pointers to unique_ptr-s of the
    /// nodes currently in use
    std::map<T*, std::unique_ptr<T>> nodesInUse_;

    /// @brief Factory function taking no arguments which will return a pointer
    /// to a new T node
    ///
    /// This function will be called to create new nodes for the pool as needed.
    std::function<T*()> factoryFunction_;
};

#endif /* SHAREDPOINTERPOOL_HH_ */
