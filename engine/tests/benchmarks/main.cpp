#include <benchmark/benchmark.h>
#include <array>
#include <memory/allocators/pool_allocator.h>
#include <graphics/resource/mesh.h>

constexpr size_t allocation_size = 1024;

std::array<Sunset::Vertex*, allocation_size> allocated;
Sunset::StaticPoolAllocator<Sunset::Vertex, allocation_size> allocator;

static void BM_DefaultHeapAllocations(benchmark::State& state)
{
	for (auto _ : state)
	{
		for (int i = 0; i < allocation_size; ++i)
		{
			allocated[i] = new Sunset::Vertex;
		}
		for (int i = 0; i < allocation_size; ++i)
		{
			delete allocated[i];
		}
	}
}
BENCHMARK(BM_DefaultHeapAllocations);

static void BM_PoolAllocatorAllocations(benchmark::State& state)
{
	for (auto _ : state)
	{
		for (int i = 0; i < allocation_size; ++i)
		{
			allocated[i] = allocator.allocate();
		}
		for (int i = 0; i < allocation_size; ++i)
		{
			allocator.deallocate(allocated[i]);
		}
	}
}
BENCHMARK(BM_PoolAllocatorAllocations);

BENCHMARK_MAIN();
