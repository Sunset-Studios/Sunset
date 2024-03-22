#pragma once

#include <vector>
#include <set>
#include <functional>
#include <input_types.h>
#include <phmap.h>

namespace Sunset
{
	class InputContext
	{
	public:
		InputContext() = default;
		InputContext(std::initializer_list<InputState> states);
		InputContext(const std::vector<InputState>& states);

		inline size_t num_states() const { return input_states.size(); }

		void set_states(const std::vector<InputState>& states);
		void set_state(int32_t input_state_index, bool b_new_state);
		void set_action(int32_t input_state_index, bool b_new_action);
		void set_range(int32_t input_state_index, float new_range);
		void visit_dirty_states(std::function<void(InputState&)> visitor);

	public:
		std::vector<InputState> input_states;

	protected:
		phmap::flat_hash_map<InputKey, int32_t> raw_to_state_mappings;
		phmap::flat_hash_map<const char*, int32_t> action_to_state_mappings;
		std::vector<bool> action_mappings;
		std::vector<bool> state_mappings;
		std::vector<float> range_mappings;
		std::set<int32_t> dirty_states;
	};
}
