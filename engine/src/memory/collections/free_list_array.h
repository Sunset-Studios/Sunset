#pragma once

#include <minimal.h>

namespace Sunset
{
	template<typename T>
	class FreeListArray
	{
	public:
		explicit FreeListArray(uint32_t size)
			: array_size(size)
		{
			elements.resize(size);
			for (uint32_t i = size - 1; i >= 0; --i)
			{
				free_indices.push_back(i);
			}
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

	private:
		size_t array_size;
		std::vector<T> elements;
		std::vector<uint32_t> free_indices;
		std::unordered_map<T*, uint32_t> used_elements;
	};
}