#pragma once

#include <minimal.h>

namespace Sunset
{
	template<typename T, uint32_t Size = 0>
	class FreeListArray
	{
	public:
		explicit FreeListArray(uint32_t size = Size)
			: array_size(size)
		{
			assert(size > 0 && "Cannot make a zero sized free list");
			elements.resize(size);
			used_elements.reserve(size);
			reset();
		}

		~FreeListArray() = default;

		T& operator[](uint32_t index)
		{
			return elements[index];
		}

		size_t size() const
		{
			return array_size;
		}

		T* data()
		{
			return &(*elements.begin());
		}

		bool has_free() const
		{
			return !free_indices.empty();
		}

		int32_t index_of(T* element) const
		{
			auto it = used_elements.find(element);
			return it != used_elements.end() ? (*it).second : -1;
		}

		T* get_new()
		{
			assert(!free_indices.empty() && "Trying to fetch an element from a free list with no more available elements!");
			const uint32_t index = free_indices.back();
			free_indices.pop_back();
			T* element = &elements[index];
			used_elements.insert({ element, index });
			return element;
		}

		void free(T* element)
		{
			if (auto it = used_elements.find(element); it != used_elements.end())
			{
				free_indices.push_back((*it).second);
				used_elements.erase(element);
			}
		}

		void free(uint32_t index)
		{
			assert(index < array_size && "Must free from this freelist with a valid index!");
			free_indices.push_back(index);
			used_elements.erase(&elements[index]);
		}

		void reset()
		{
			free_indices.clear();
			used_elements.clear();
			for (int32_t i = array_size - 1; i >= 0; --i)
			{
				free_indices.push_back(i);
			}
		}

	private:
		size_t array_size;
		std::vector<T> elements;
		std::vector<uint32_t> free_indices;
		phmap::flat_hash_map<T*, uint32_t> used_elements;
	};
}