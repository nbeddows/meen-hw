/*
Copyright (c) 2021-2024 Nicolas Beddows <nicolas.beddows@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef MEEN_HW_MH_RESOURCEPOOL_H
#define MEEN_HW_MH_RESOURCEPOOL_H

#include <memory>
#include <mutex>
#include <list>

namespace meen_hw
{
	/** A basic resource pool.

		Allocates a list of resources of a specified size.
        Returns the resource to the pool once the resource
        has been destructed.
	*/
    template<class T>
	class MH_ResourcePool final
	{
    private:
        /** Resource pool mutex
        
            A resource can be returned to the resource pool from any thread. This is the mutex
            used for mutual exclusion between that thread and the thread that this resource pool
            uses for resource access.
        
            @remark     marked as mutable so GetResource can remain const
        */
        mutable std::shared_ptr<std::mutex> resourceMutex_;

        /** Resource pool
        
            A pool of resources.

            @remark     A resource is automatically returned to the resource pool when it is destructed.

            @remark     Marked as mutable so GetResource can remain const.
        */
        mutable std::shared_ptr<std::list<std::unique_ptr<T>>> resourcePool_;

        /** Custom resource deleter
        
            A deleter that is attached to each resource that allows it to be returned to the resource pool
            once it has been destructed.
        */
        class ResourceDeleter
        {
        private:
            /** resourcePool_
            
                A weak pointer to MH_ResourcePool::resourcePool that can be used to check
                if the resource pool is still alive. When it is alive the destructed frame will
                be returned to it, otherwise it will be deleted.

                @see    MH_ResourcePool::resourcePool_
            */
            std::weak_ptr<std::list<std::unique_ptr<T>>> resourcePool_;
            
            /** rsourceMutex_
            
                A weak pointer to  MH_ResourcePool::resourceMutex that can be used
                to check if the resource mutex is still alive. When it is alive it will be
                used to ensure mutual exculsion between the thread that this deleter was
                invoked from and the thread on which this resource pool is running,
                otherwise a warning message will be logged.

                @see    MH_ResourcePool::resourceMutex_
            */
            std::weak_ptr<std::mutex> resourceMutex_;

        public:
            /** Default constructor
            
                An empty deleter.
            */
            ResourceDeleter() = default;

            /** Initialisation contstructor

                A deleter with the specified resource pool and resource mutex.

                @param      resourcePool       The resource pool that desructed resources will be returned to.
                @param      resourceMutex      The resource pool mutex that will be used for mutual exclusion.
            */
            ResourceDeleter(const std::shared_ptr<std::list<std::unique_ptr<T>>>& resourcePool, const std::shared_ptr<std::mutex>& resourceMutex)
                : resourcePool_(resourcePool)
                , resourceMutex_{resourceMutex}
            {

            }

            /** Custom deleter
            
                A deleter used to recycle resources.
            */
            void operator()(T* resource)
            {
                if(auto resourcePool = resourcePool_.lock())
                {
                    auto resourceMutex = resourceMutex_.lock();
                    
                    if(resourceMutex)
                    {
                        std::lock_guard<std::mutex> lg(*resourceMutex);
                        resourcePool->emplace_back(std::unique_ptr<T>{resource});
                    }
                    else
                    {
                        printf("Failed to acquire frame mutex\n");
                        resourcePool->emplace_back(std::unique_ptr<T>{resource});
                    }
                }
                else
                {
                    printf("Failed to get the frame pool shared_ptr, deleting the video frame to prevent a memory leak\n");
                    std::default_delete<T>{}(resource);
                }
            }
        };

    public:
        /** Custom resource unique_ptr with custom deleter attached
        
            A using directive for convenience.

            @remark     When this resource is destructed it will be automatically returned to the resource pool.
        */
        using ResourcePtr = std::unique_ptr<T, MH_ResourcePool::ResourceDeleter>;

        /** Constructor

            A very basic resource pool.

            @param      resourcePoolSize       The number of resources to allocate.

            @remark     default resource pool size is 1.

            @see resourcePool_
        */
        explicit MH_ResourcePool(int resourcePoolSize = 1)
        {
            for (int i = 0; i < resourcePoolSize; i++)
            {
                resourcePool_->emplace_back(std::make_unique<T>());
            }
        }

        /** Destructor

            Free the resource pool
        */
        ~MH_ResourcePool() = default;

        /** Get a resource from the resource pool

            @return         A valid resource or empty if a resource is unavailable.
        */
        ResourcePtr GetResource() const
        {
            std::unique_ptr<T> resource;

            // This method is called from ServiceInterrupts so we don't 
            // want to block waiting for this mutex as we could stall the cpu,
            // if we don't get it, this resource will be dropped (host is too
            // slow, the machine clock resolution is too high or the function
            // call spuriously failed).
            if (resourceMutex_->try_lock() == true)
            {
                if (resourcePool_->empty() == false)
                {
                    resource = std::move(resourcePool_->back());
                    resourcePool_->pop_back();
                }

                resourceMutex_->unlock();
            }

            return ResourcePtr{resource.release(), ResourceDeleter{resourcePool_, resourceMutex_}};
        }
    };
} // namespace meen_hw

#endif // MEEN_HW_MH_RESOURCEPOOL_H