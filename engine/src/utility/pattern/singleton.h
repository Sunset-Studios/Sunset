#pragma once

namespace Sunset
{
	template<class T>
	class Singleton
	{
		public:
			static T* get()
			{
				static T t;
				t.initialize();
				return &t;
			}

		protected:
			Singleton() = default;
	};
}