#include <utility/cvar.h>

namespace Sunset
{
	Sunset::CVarParam* CVarSystem::init_cvar(const char* name, const char* description)
	{
		if (get_cvar(name) != nullptr)
		{
			return nullptr;
		}

		uint32_t name_hash = StringHash{ name };
		saved_cvar_params[name_hash] = CVarParam{};

		CVarParam& new_param = saved_cvar_params[name_hash];
		new_param.name = name;
		new_param.description = description;

		return &new_param;
	}

	Sunset::CVarParam* CVarSystem::get_cvar(StringHash hash)
	{
		if (auto it = saved_cvar_params.find(hash); it != saved_cvar_params.end())
		{
			return &(*it).second;
		}
		return nullptr;
	}

	Sunset::CVarParam* CVarSystem::create_int_cvar(const char* name, const char* description, int32_t default_value, int32_t current_value)
	{
		CVarParam* param = init_cvar(name, description);
		if (param == nullptr)
		{
			return nullptr;
		}

		param->type = CVarType::Int;

		get_cvar_array<int32_t>()->add(default_value, current_value, param);

		return param;
	}

	Sunset::CVarParam* CVarSystem::create_float_cvar(const char* name, const char* description, double default_value, double current_value)
	{
		CVarParam* param = init_cvar(name, description);
		if (param == nullptr)
		{
			return nullptr;
		}

		param->type = CVarType::Float;

		get_cvar_array<double>()->add(default_value, current_value, param);

		return param;
	}

	Sunset::CVarParam* CVarSystem::create_string_cvar(const char* name, const char* description, const std::string& default_value, const std::string& current_value)
	{
		CVarParam* param = init_cvar(name, description);
		if (param == nullptr)
		{
			return nullptr;
		}

		param->type = CVarType::String;

		get_cvar_array<std::string>()->add(default_value, current_value, param);

		return param;
	}

	Sunset::CVarParam* CVarSystem::create_bool_cvar(const char* name, const char* description, bool default_value, bool current_value)
	{
		CVarParam* param = init_cvar(name, description);
		if (param == nullptr)
		{
			return nullptr;
		}

		param->type = CVarType::Bool;

		get_cvar_array<bool>()->add(default_value, current_value, param);

		return param;
	}

	int32_t* CVarSystem::get_int_cvar(StringHash hash)
	{
		return get_cvar_current_value<int32_t>(hash);
	}

	double* CVarSystem::get_float_cvar(StringHash hash)
	{
		return get_cvar_current_value<double>(hash);
	}

	std::string* CVarSystem::get_string_cvar(StringHash hash)
	{
		return get_cvar_current_value<std::string>(hash);
	}

	bool* CVarSystem::get_bool_cvar(StringHash hash)
	{
		return get_cvar_current_value<bool>(hash);
	}

	void CVarSystem::set_int_cvar(StringHash hash, int32_t value)
	{
		set_cvar_current_value<int32_t>(hash, value);
	}

	void CVarSystem::set_float_cvar(StringHash hash, double value)
	{
		set_cvar_current_value<double>(hash, value);
	}

	void CVarSystem::set_string_cvar(StringHash hash, const std::string& value)
	{
		set_cvar_current_value<std::string>(hash, value);
	}

	void CVarSystem::set_bool_cvar(StringHash hash, bool value)
	{
		set_cvar_current_value<bool>(hash, value);
	}

	AutoCVar_Int::AutoCVar_Int(const char* name, const char* description, int32_t default_value, CVarFlags flags /*= CVarFlags::None*/)
	{
		CVarParam* cvar = CVarSystem::get()->create_int_cvar(name, description, default_value, default_value);
		cvar->flags = flags;
		index = cvar->index;
	}

	int32_t AutoCVar_Int::get()
	{
		return CVarSystem::get()->get_cvar_array<Type>()->get_current_value(index);
	}

	void AutoCVar_Int::set(int32_t value)
	{
		CVarSystem::get()->get_cvar_array<Type>()->set_current_value(value, index);
	}

	AutoCVar_Float::AutoCVar_Float(const char* name, const char* description, double default_value, CVarFlags flags /*= CVarFlags::None*/)
	{
		CVarParam* cvar = CVarSystem::get()->create_float_cvar(name, description, default_value, default_value);
		cvar->flags = flags;
		index = cvar->index;
	}

	double AutoCVar_Float::get()
	{
		return CVarSystem::get()->get_cvar_array<Type>()->get_current_value(index);
	}

	void AutoCVar_Float::set(double value)
	{
		CVarSystem::get()->get_cvar_array<Type>()->set_current_value(value, index);
	}

	AutoCVar_String::AutoCVar_String(const char* name, const char* description, const std::string& default_value, CVarFlags flags /*= CVarFlags::None*/)
	{
		CVarParam* cvar = CVarSystem::get()->create_string_cvar(name, description, default_value, default_value);
		cvar->flags = flags;
		index = cvar->index;
	}

	std::string AutoCVar_String::get()
	{
		return CVarSystem::get()->get_cvar_array<Type>()->get_current_value(index);
	}

	void AutoCVar_String::set(const std::string value)
	{
		CVarSystem::get()->get_cvar_array<Type>()->set_current_value(value, index);
	}

	AutoCVar_Bool::AutoCVar_Bool(const char* name, const char* description, bool default_value, CVarFlags flags /*= CVarFlags::None*/)
	{
		CVarParam* cvar = CVarSystem::get()->create_bool_cvar(name, description, default_value, default_value);
		cvar->flags = flags;
		index = cvar->index;
	}

	bool AutoCVar_Bool::get()
	{
		return CVarSystem::get()->get_cvar_array<Type>()->get_current_value(index);
	}

	void AutoCVar_Bool::set(bool value)
	{
		CVarSystem::get()->get_cvar_array<Type>()->set_current_value(value, index);
	}
}
