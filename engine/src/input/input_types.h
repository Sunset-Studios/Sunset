#pragma once

enum class InputKey : uint16_t
{
	K_Return = 0,
	K_Escape,
	K_Backspace,
	K_Tab,
	K_Space,
	K_Exclaim,
	K_Quotedbl,
	K_Hash,
	K_Percent,
	K_Dollar,
	K_Ampersand,
	K_Quote,
	K_Leftparen,
	K_Rightparen,
	K_Asterisk,
	K_Plus,
	K_Comma,
	K_Minus,
	K_Period,
	K_Slash,
	K_Colon,
	K_Semicolon,
	K_Less,
	K_Equals,
	K_Greater,
	K_Question,
	K_At,
	K_Leftbracket,
	K_Backslash,
	K_Rightbracket,
	K_Caret,
	K_Underscore,
	K_Backquote,
	K_0,
	K_1,
	K_2,
	K_3,
	K_4,
	K_5,
	K_6,
	K_7,
	K_8,
	K_9,
	K_a,
	K_b,
	K_c,
	K_d,
	K_e,
	K_f,
	K_g,
	K_h,
	K_i,
	K_j,
	K_k,
	K_l,
	K_m,
	K_n,
	K_o,
	K_p,
	K_q,
	K_r,
	K_s,
	K_t,
	K_u,
	K_v,
	K_w,
	K_x,
	K_y,
	K_z,
	NumKeys
};

constexpr size_t NUM_AVAILABLE_KEYS = static_cast<size_t>(InputKey::NumKeys);

enum class InputType : int8_t
{
	Action = 0,
	State,
	Range
};

struct InputState
{
	const char* mapped_name;
	InputKey raw_input;
	InputType input_type;
};
