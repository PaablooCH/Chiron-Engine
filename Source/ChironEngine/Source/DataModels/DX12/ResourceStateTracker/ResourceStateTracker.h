#pragma once

class CommandList;
class Resource;

class ResourceStateTracker
{
public:
    ResourceStateTracker();
    ~ResourceStateTracker();

    /**
     * Push a transition resource barrier to the resource state tracker.
     *
     * @param resource The resource to transition.
     * @param stateAfter The state to transition the resource to.
     * @param subResource The subresource to transition. By default, this is D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES
     * which indicates that all subresources should be transitioned to the same state.
     */
    void TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES stateAfter,
        UINT subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

    /**
     * Push a UAV resource barrier for the given resource.
     *
     * @param resource The resource to add a UAV barrier for. Can be NULL which
     * indicates that any UAV access could require the barrier.
     */
    void UAVBarrier(ID3D12Resource* resource = nullptr);

    /**
    * Push an aliasing barrier for the given resource.
    *
    * @param beforeResource The resource currently occupying the space in the heap.
    * @param afterResource The resource that will be occupying the space in the heap.
    *
    * Either the beforeResource or the afterResource parameters can be NULL which
    * indicates that any placed or reserved resource could cause aliasing.
    */
    void AliasingBarrier(ID3D12Resource* resourceBefore = nullptr, ID3D12Resource* resourceAfter = nullptr);

    /**
     * Flush any pending resource barriers to the command list.
     *
     * @return The number of resource barriers that were flushed to the command list.
     */
    uint32_t FlushPendingResourceBarriers(const CommandList* commandList);

    // Flush any (non-pending) resource barriers that have been pushed to the resource state tracker.
    void FlushResourceBarriers(CommandList* commandList);

    // Commit final resource states to the global resource state map. This must be called when the command list is closed.
    void CommitFinalResourceStates();

    // Reset state tracking. This must be done when the command list is reset.
    void Reset();

    // The global state must be locked before flushing pending resource barriers and committing the final resource state
    // to the global resource state. This ensures consistency of the global resource state between command list executions.
    static void Lock();

    // Unlocks the global resource state after the final states have been committed to the global resource state array.
    static void Unlock();

    // Add a resource with a given state to the global resource state array (map). This should be done when the resource
    // is created for the first time.
    static void AddGlobalResourceState(ID3D12Resource* resource, D3D12_RESOURCE_STATES state);

    // Remove a resource from the global resource state array (map). This should only be done when the resource is
    // destroyed.
    static void RemoveGlobalResourceState(ID3D12Resource* resource);

private:

    // Push a resource barrier to the resource state tracker.
    void ResourceBarrier(const D3D12_RESOURCE_BARRIER& barrier);

    inline static bool ExistsResource(ID3D12Resource* resource);

private:

    // Pending resource transitions are committed before a command list is executed on the command queue. This guarantees
    // that resources will be in the expected state at the beginning of a command list.
    std::vector<D3D12_RESOURCE_BARRIER> _pendingResourceBarriers;

    // Resource barriers that need to be committed to the command list.
    std::vector<D3D12_RESOURCE_BARRIER> _resourceBarriers;

    // Tracks the state of a particular resource and all of its subresources.
    struct ResourceState
    {
        explicit ResourceState(D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON)
            : State(state)
        {
        }

        void SetSubresourceState(UINT subresource, D3D12_RESOURCE_STATES state)
        {
            if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
            {
                State = state;
                SubresourceState.clear();
            }
            else
            {
                SubresourceState[subresource] = state;
            }
        }

        D3D12_RESOURCE_STATES GetSubresourceState(UINT subresource) const
        {
            D3D12_RESOURCE_STATES state = State;
            const auto iter = SubresourceState.find(subresource);
            if (iter != SubresourceState.end())
            {
                state = iter->second;
            }
            return state;
        }

        D3D12_RESOURCE_STATES State;
        std::map<UINT, D3D12_RESOURCE_STATES> SubresourceState;
    };

    // The final (last known state) of the resources within a command list.
    // The final resource state is committed to the global resource state when the
    // command list is closed but before it is executed on the command queue.
    std::unordered_map<ID3D12Resource*, ResourceState> _finalResourceState;

    // The global resource state array (map) stores the state of a resource
    // between command list execution.
    static std::unordered_map<ID3D12Resource*, ResourceState> _globalResourceState;

    // The mutex protects shared access to the GlobalResourceState map.
    static std::mutex _globalMutex;
    static bool _isLocked;
};

inline bool ResourceStateTracker::ExistsResource(ID3D12Resource* resource)
{
    return _globalResourceState.contains(resource);
}