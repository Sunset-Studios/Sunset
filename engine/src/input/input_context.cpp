#include <input/input_context.h>

#include <cassert>

namespace Sunset
{
	InputContext::InputContext(std::initializer_list<InputState> states)
		: input_states(states)
	{
		action_mappings = std::vector<bool>(input_states.size(), false);
		state_mappings = std::vector<bool>(input_states.size(), false);
		range_mappings = std::vector<float>(input_states.size(), 0.0f);

		for (int32_t i = 0; i < input_states.size(); ++i)
		{
			const InputState& input_state = input_states[i];
			raw_to_state_mappings[input_state.raw_input] = i;
			action_to_state_mappings[input_state.mapped_name] = i;
		}
	}

	InputContext::InputContext(const std::vector<InputState>& states)
		: input_states(states)
	{
		action_mappings = std::vector<bool>(input_states.size(), false);
		state_mappings = std::vector<bool>(input_states.size(), false);
		range_mappings = std::vector<float>(input_states.size(), 0.0f);

		for (int32_t i = 0; i < input_states.size(); ++i)
		{
			const InputState& input_state = input_states[i];
			raw_to_state_mappings[input_state.raw_input] = i;
			action_to_state_mappings[input_state.mapped_name] = i;
		}
	}


	void InputContext::set_states(const std::vector<InputState>& states)
	{
		input_states = states;

		action_mappings = std::vector<bool>(input_states.size(), false);
		state_mappings = std::vector<bool>(input_states.size(), false);
		range_mappings = std::vector<float>(input_states.size(), 0.0f);

		for (int32_t i = 0; i < input_states.size(); ++i)
		{
			const InputState& input_state = input_states[i];
			raw_to_state_mappings[input_state.raw_input] = i;
			action_to_state_mappings[input_state.mapped_name] = i;
		}
	}

	void InputContext::set_state(int32_t input_state_index, bool b_new_state)
	{
		assert(input_state_index < state_mappings.size() && input_state_index >= 0);
		state_mappings[input_state_index] = b_new_state;
		if (b_new_state)
		{
			dirty_states.insert(input_state_index);
		}
	}

	void InputContext::set_action(int32_t input_state_index, bool b_new_action)
	{
		assert(input_state_index < action_mappings.size() && input_state_index >= 0);
		const bool b_action_fired = !action_mappings[input_state_index] && b_new_action;
		action_mappings[input_state_index] = b_new_action;
		if (b_action_fired)
		{
			dirty_states.insert(input_state_index);
		}
	}

	void InputContext::set_range(int32_t input_state_index, float new_range)
	{
		assert(input_state_index < range_mappings.size() && input_state_index >= 0);
		if (0.0f != new_range)
		{
			range_mappings[input_state_index] = new_range;
			input_states[input_state_index].range_value = new_range;
			dirty_states.insert(input_state_index);
		}
	}

	void InputContext::visit_dirty_states(std::function<void(InputState&)> visitor)
	{
		if (!visitor)
		{
			return;
		}

		for (auto it = dirty_states.cbegin(); it != dirty_states.cend(); ++it)
		{
			const int32_t index = *it;
			assert(index < input_states.size() && index >= 0);
			InputState& state = input_states[index];
			visitor(state);
		}

		dirty_states.clear();
	}
}
