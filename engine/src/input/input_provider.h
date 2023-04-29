#pragma once

#include <singleton.h>
#include <common.h>

#include <input/input_context.h>

namespace Sunset
{
	class InputProvider : public Singleton<InputProvider>
	{
		friend class Singleton;

		public:
			static class InputContext* default_context();

		public:
			void initialize();
			void update(class Window* window);

			void push_context(class InputContext* context);
			class InputContext* pop_context();

			bool get_state(const InputKey& key);
			bool get_state(const char* name);

			bool get_action(const InputKey& key);
			bool get_action(const char* name);

			float get_range(const InputRange& range);
			float get_range(const char* name);

		private:
			InputProvider() = default;

		private:
			bool b_initialized{ false };
			class InputProcessor* processor{ nullptr };
			std::vector<class InputContext*> contexts;
			std::vector<InputState> current_dirty_states;
	};
}
