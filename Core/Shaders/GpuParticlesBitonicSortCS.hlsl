struct ParticleSortKey
{
    float Key;
    uint Index;
    uint Alive;
    uint Padding;
};

cbuffer ParticleSortConstants : register(b0)
{
    uint SortLevel;
    uint CompareDistance;
    uint ElementCount;
    uint Padding;
};

RWStructuredBuffer<ParticleSortKey> SortKeys : register(u0);

[numthreads(256, 1, 1)]
void CSMain(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    const uint leftIndex = dispatchThreadId.x;
    if (leftIndex >= ElementCount)
    {
        return;
    }

    const uint rightIndex = leftIndex ^ CompareDistance;
    if (rightIndex <= leftIndex || rightIndex >= ElementCount)
    {
        return;
    }

    const bool ascending = (leftIndex & SortLevel) == 0u;
    const ParticleSortKey left = SortKeys[leftIndex];
    const ParticleSortKey right = SortKeys[rightIndex];
    const bool shouldSwap = ascending ? (left.Key > right.Key) : (left.Key < right.Key);

    if (shouldSwap)
    {
        SortKeys[leftIndex] = right;
        SortKeys[rightIndex] = left;
    }
}
