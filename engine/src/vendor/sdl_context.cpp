#include <vendor/sdl_context.h>
#include <input/input_context.h>

#include <SDL_vulkan.h>

namespace Sunset
{
	bool B_SDL_INITIALIZED = false;
	SDL_Event sdl_event{ 0 };
	std::unordered_map<SDL_Keycode, InputKey> SDL_TO_SUNSET_KEY_MAP
	{
		{ SDLK_RETURN, InputKey::K_Return },
		{ SDLK_ESCAPE, InputKey::K_Escape },
		{ SDLK_BACKSPACE, InputKey::K_Backspace },
		{ SDLK_TAB, InputKey::K_Tab },
		{ SDLK_SPACE, InputKey::K_Space },
		{ SDLK_EXCLAIM, InputKey::K_Exclaim },
		{ SDLK_QUOTEDBL, InputKey::K_Quotedbl },
		{ SDLK_HASH, InputKey::K_Hash },
		{ SDLK_PERCENT, InputKey::K_Percent },
		{ SDLK_DOLLAR, InputKey::K_Dollar },
		{ SDLK_AMPERSAND, InputKey::K_Ampersand },
		{ SDLK_QUOTE, InputKey::K_Quote },
		{ SDLK_LEFTPAREN, InputKey::K_Leftparen },
		{ SDLK_RIGHTPAREN, InputKey::K_Rightparen },
		{ SDLK_ASTERISK, InputKey::K_Asterisk },
		{ SDLK_PLUS, InputKey::K_Plus },
		{ SDLK_COMMA, InputKey::K_Comma },
		{ SDLK_MINUS, InputKey::K_Minus },
		{ SDLK_PERIOD, InputKey::K_Period },
		{ SDLK_SLASH, InputKey::K_Slash },
		{ SDLK_COLON, InputKey::K_Colon },
		{ SDLK_SEMICOLON, InputKey::K_Semicolon },
		{ SDLK_LESS, InputKey::K_Less },
		{ SDLK_EQUALS, InputKey::K_Equals },
		{ SDLK_GREATER, InputKey::K_Greater },
		{ SDLK_QUESTION, InputKey::K_Question },
		{ SDLK_AT, InputKey::K_At },
		{ SDLK_LEFTBRACKET, InputKey::K_Leftbracket },
		{ SDLK_BACKSLASH, InputKey::K_Backslash },
		{ SDLK_RIGHTBRACKET, InputKey::K_Rightbracket },
		{ SDLK_CARET, InputKey::K_Caret },
		{ SDLK_UNDERSCORE, InputKey::K_Underscore },
		{ SDLK_BACKQUOTE, InputKey::K_Backquote },
		{ SDLK_0, InputKey::K_0 },
		{ SDLK_1, InputKey::K_1 },
		{ SDLK_2, InputKey::K_2 },
		{ SDLK_3, InputKey::K_3 },
		{ SDLK_4, InputKey::K_4 },
		{ SDLK_5, InputKey::K_5 },
		{ SDLK_6, InputKey::K_6 },
		{ SDLK_7, InputKey::K_7 },
		{ SDLK_8, InputKey::K_8 },
		{ SDLK_9, InputKey::K_9 },
		{ SDLK_a, InputKey::K_a },
		{ SDLK_b, InputKey::K_b },
		{ SDLK_c, InputKey::K_c },
		{ SDLK_d, InputKey::K_d },
		{ SDLK_e, InputKey::K_e },
		{ SDLK_f, InputKey::K_f },
		{ SDLK_g, InputKey::K_g },
		{ SDLK_h, InputKey::K_h },
		{ SDLK_i, InputKey::K_i },
		{ SDLK_j, InputKey::K_j },
		{ SDLK_k, InputKey::K_k },
		{ SDLK_l, InputKey::K_l },
		{ SDLK_m, InputKey::K_m },
		{ SDLK_n, InputKey::K_n },
		{ SDLK_o, InputKey::K_o },
		{ SDLK_p, InputKey::K_p },
		{ SDLK_q, InputKey::K_q },
		{ SDLK_r, InputKey::K_r },
		{ SDLK_s, InputKey::K_s },
		{ SDLK_t, InputKey::K_t },
		{ SDLK_u, InputKey::K_u },
		{ SDLK_v, InputKey::K_v },
		{ SDLK_w, InputKey::K_w },
		{ SDLK_x, InputKey::K_x },
		{ SDLK_y, InputKey::K_y },
		{ SDLK_z, InputKey::K_z },
	};

	std::unordered_map<Uint8, InputKey> SDL_TO_SUNSET_BUTTON_MAP
	{
		{ SDL_BUTTON_LEFT, InputKey::B_mouse_left },
		{ SDL_BUTTON_MIDDLE, InputKey::B_mouse_middle },
		{ SDL_BUTTON_RIGHT, InputKey::B_mouse_right }
	};

	void lazy_SDL_init()
	{
		if (!B_SDL_INITIALIZED)
		{
			B_SDL_INITIALIZED = true;
			SDL_Init(SDL_INIT_VIDEO);
		}
	}

	void WindowSDL::initialize(const char* title, const glm::ivec2& position, const glm::ivec2& extent)
	{
		lazy_SDL_init();

		SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

		this->extent = extent;
		this->position = position;

		window_handle = SDL_CreateWindow(
			title,
			position.x > 0 ? position.x : SDL_WINDOWPOS_CENTERED,
			position.y > 0 ? position.y : SDL_WINDOWPOS_CENTERED,
			extent.x,
			extent.y,
			window_flags
		);
	}

	void WindowSDL::poll()
	{
		SDL_PollEvent(&sdl_event);
	}

	bool WindowSDL::is_closing()
	{		
		return sdl_event.type == SDL_QUIT;
	}

	void WindowSDL::destroy()
	{
		SDL_DestroyWindow(window_handle);
	}

	void InputProcessorSDL::initialize()
	{
		lazy_SDL_init();
	}

	void InputProcessorSDL::update(InputContext* context)
	{
		// Update our underlying key-mapped bitset and axis values
		if (sdl_event.type == SDL_KEYDOWN)
		{
			if (SDL_TO_SUNSET_KEY_MAP.find(sdl_event.key.keysym.sym) != SDL_TO_SUNSET_KEY_MAP.end())
			{
				InputKey key = SDL_TO_SUNSET_KEY_MAP[sdl_event.key.keysym.sym];
				key_bitmap.set(static_cast<int16_t>(key));
			}
		}
		else if (sdl_event.type == SDL_KEYUP)
		{
			if (SDL_TO_SUNSET_KEY_MAP.find(sdl_event.key.keysym.sym) != SDL_TO_SUNSET_KEY_MAP.end())
			{
				InputKey key = SDL_TO_SUNSET_KEY_MAP[sdl_event.key.keysym.sym];
				key_bitmap.set(static_cast<int16_t>(key), 0);
			}
		}

		if (sdl_event.type == SDL_MOUSEBUTTONDOWN)
		{
			if (SDL_TO_SUNSET_BUTTON_MAP.find(sdl_event.button.button) != SDL_TO_SUNSET_BUTTON_MAP.end())
			{
				InputKey key = SDL_TO_SUNSET_BUTTON_MAP[sdl_event.button.button];
				key_bitmap.set(static_cast<int16_t>(key));
			}
		}
		else if (sdl_event.type == SDL_MOUSEBUTTONUP)
		{
			if (SDL_TO_SUNSET_BUTTON_MAP.find(sdl_event.button.button) != SDL_TO_SUNSET_BUTTON_MAP.end())
			{
				InputKey key = SDL_TO_SUNSET_BUTTON_MAP[sdl_event.button.button];
				key_bitmap.set(static_cast<int16_t>(key), 0);
			}
		}

		if (sdl_event.type == SDL_MOUSEMOTION)
		{
			ranges_array[static_cast<int16_t>(InputRange::M_x)] = static_cast<float>(sdl_event.motion.xrel);
			ranges_array[static_cast<int16_t>(InputRange::M_y)] = static_cast<float>(sdl_event.motion.yrel);
		}

		// Use the now updated bitset to update the passed in input context
		for (int32_t i = 0; i < context->input_states.size(); ++i)
		{
			const InputState& state = context->input_states[i];
			switch (state.input_type)
			{
				case InputType::State:
				{
					const bool b_input_is_active = key_bitmap.test(static_cast<int16_t>(state.raw_input));
					context->set_state(i, b_input_is_active);
					break;
				}
				case InputType::Action:
				{
					const bool b_input_is_active = key_bitmap.test(static_cast<int16_t>(state.raw_input));
					context->set_action(i, b_input_is_active);
					break;
				}
				case InputType::Range:
				{
					const float b_range_value = ranges_array[static_cast<int16_t>(state.raw_range)];
					context->set_range(i, b_range_value);
					break;
				}
				default: break;
			}
		}
	}
}