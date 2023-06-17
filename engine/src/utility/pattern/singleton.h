#pragma once

#include <utility>

namespace Sunset
{
	template<class T>
	class Singleton
	{
		public:
			template<typename ...Args>
			static T* get(Args&&... args)
			{
				static T t(std::forward<Args>(args)...);
				t.initialize();
				return &t;
			}

		protected:
			Singleton() = default;
	};
}