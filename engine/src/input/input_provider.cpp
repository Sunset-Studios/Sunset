#include <input/input_provider.h>
#include <input/input_processor.h>

namespace Sunset
{
	InputContext* InputProvider::default_context()
	{
		static InputContext context;
		if (context.num_states() == 0)
		{
			std::vector<InputState> input_states;
			for (int16_t i = 0; i < static_cast<int16_t>(InputKey::NumKeys); ++i)
			{
				input_states.push_back({ "", static_cast<InputKey>(i), InputRange::NumRanges, InputType::State });
				input_states.push_back({ "", static_cast<InputKey>(i), InputRange::NumRanges, InputType::Action });
			}
			for (int16_t i = 0; i < static_cast<int16_t>(InputRange::NumRanges); ++i)
			{
				input_states.push_back({ "", InputKey::NumKeys, static_cast<InputRange>(i), InputType::Range });
			}

			context.set_states(input_states);
		}
		return &context;
	}

	void InputProvider::initialize()
	{
		if (!b_initialized)
		{
			b_initialized = true;
			processor = InputProcessorFactory::create();
		}
	}

	void InputProvider::update(class Window* window)
	{
		ZoneScopedN("InputProvider::update");
		if (!contexts.empty())
		{
			InputContext* context = contexts.back();
			processor->update(context, window);

			current_dirty_states.clear();
			current_dirty_states.reserve(context->input_states.size());
			context->visit_dirty_states([this](const InputState& state)
			{
				current_dirty_states.push_back(state);
			});
		}
	}

	void InputProvider::push_context(InputContext* context)
	{
		contexts.push_back(context);
	}

	InputContext* InputProvider::pop_context()
	{
		if (!contexts.empty())
		{
			InputContext* context = contexts.back();
			contexts.pop_back();
			return context;
		}
		return nullptr;
	}

	bool InputProvider::get_state(const InputKey& key)
	{
		return std::find_if(current_dirty_states.cbegin(), current_dirty_states.cend(), [key](const InputState& state)
			{
				return state.raw_input == key && state.input_type == InputType::State;
			}) != current_dirty_states.cend();
	}

	bool InputProvider::get_state(const char* name)
	{
		return std::find_if(current_dirty_states.cbegin(), current_dirty_states.cend(), [name](const InputState& state)
			{
				return state.mapped_name == name && state.input_type == InputType::State;
			}) != current_dirty_states.cend();
	}

	bool InputProvider::get_action(const InputKey& key)
	{
		return std::find_if(current_dirty_states.cbegin(), current_dirty_states.cend(), [key](const InputState& state)
			{
				return state.raw_input == key && state.input_type == InputType::Action;
			}) != current_dirty_states.cend();
	}

	bool InputProvider::get_action(const char* name)
	{
		return std::find_if(current_dirty_states.cbegin(), current_dirty_states.cend(), [name](const InputState& state)
			{
				return state.mapped_name == name && state.input_type == InputType::Action;
			}) != current_dirty_states.cend();
	}

	float InputProvider::get_range(const InputRange& range)
	{
		auto it = std::find_if(current_dirty_states.cbegin(), current_dirty_states.cend(), [range](const InputState& state)
			{
				return state.raw_range == range && state.input_type == InputType::Range;
			});
		return (it != current_dirty_states.cend()) ? (*it).range_value : 0.0f;
	}

	float InputProvider::get_range(const char* name)
	{
		auto it = std::find_if(current_dirty_states.cbegin(), current_dirty_states.cend(), [name](const InputState& state)
			{
				return state.mapped_name == name && state.input_type == InputType::Range;
			});
		return (it != current_dirty_states.cend()) ? (*it).range_value : 0.0f;
	}
}
