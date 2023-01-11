#pragma once

#include <minimal.h>

#include <utility/pattern/singleton.h>
#include <utility/strings.h>

namespace Sunset
{
	enum class CVarFlags : uint32_t
	{
		None = 0,
		NoEdit = 1 << 1,
		EditReadOnly = 1 << 2,
		Advanced = 1 << 3,

		EditCheckbox = 1 << 8,
		EditFloatDrag = 1 << 9
	};

	enum class CVarType : uint8_t
	{
		Int = 0,
		Float,
		String,
		Bool
	};

	class CVarParam
	{
		friend class CVarSystem;

	public:
		int32_t index;
		CVarType type;
		CVarFlags flags;
		const char* name;
		const char* description;
	};

	template<typename T>
	struct CVarStorage
	{
		T initial_value;
		T current_value;
		CVarParam* param;
	};

	template<typename T>
	struct CVarArray
	{
		CVarStorage<T>* cvars;
		size_t array_size{ 0 };
		int32_t last_cvar{ 0 };

		CVarArray(size_t size)
			: array_size(size)
		{
			cvars = new CVarStorage<T>[array_size]();
		}

		~CVarArray()
		{
			delete cvars;
		}

		T get_current_value(int32_t index)
		{
			assert(index >= 0 && index < array_size);
			return cvars[index].current_value;
		}

		T* get_current_value_ptr(int32_t index)
		{
			assert(index >= 0 && index < array_size);
			return &(cvars[index].current_value);
		}

		void set_current_value(const T& val, int32_t index)
		{
			assert(index >= 0 && index < array_size);
			cvars[index].current_value = val;
		}

		int32_t add(const T& default_value, const T& value, CVarParam* param)
		{
			int32_t index = last_cvar;

			cvars[index].current_value = value;
			cvars[index].initial_value = value;
			cvars[index].param = param;

			param->index = index;
			++last_cvar;

			return index;
		}
	};

	template<typename T>
	struct AutoCVar
	{
		int index;
		using Type = T;
	};

	struct AutoCVar_Int : AutoCVar<int32_t>
	{
		AutoCVar_Int(const char* name, const char* description, int32_t default_value, CVarFlags flags = CVarFlags::None);

		int32_t get();
		void set(int32_t value);
	};

	struct AutoCVar_Float : AutoCVar<double>
	{
		AutoCVar_Float(const char* name, const char* description, double default_value, CVarFlags flags = CVarFlags::None);

		double get();
		void set(double value);
	};

	struct AutoCVar_String : AutoCVar<std::string>
	{
		AutoCVar_String(const char* name, const char* description, const std::string& default_value, CVarFlags flags = CVarFlags::None);

		std::string get();
		void set(const std::string value);
	};

	struct AutoCVar_Bool : AutoCVar<bool>
	{
		AutoCVar_Bool(const char* name, const char* description, bool default_value, CVarFlags flags = CVarFlags::None);

		bool get();
		void set(bool value);
	};

	constexpr int MAX_INT_CVARS = 1024;
	constexpr int MAX_FLOAT_CVARS = 1024;
	constexpr int MAX_STRING_CVARS = 256;
	constexpr int MAX_BOOL_CVARS = 1024;

	class CVarSystem : public Singleton<CVarSystem>
	{
	friend class Singleton;
	
	public:
		void initialize() { }

		CVarParam* init_cvar(const char* name, const char* description);
		CVarParam* get_cvar(StringHash hash);

		CVarParam* create_int_cvar(const char* name, const char* description, int32_t default_value, int32_t current_value);
		CVarParam* create_float_cvar(const char* name, const char* description, double default_value, double current_value);
		CVarParam* create_string_cvar(const char* name, const char* description, const std::string& default_value, const std::string& current_value);
		CVarParam* create_bool_cvar(const char* name, const char* description, bool default_value, bool current_value);

		int32_t* get_int_cvar(StringHash hash);
		double* get_float_cvar(StringHash hash);
		std::string* get_string_cvar(StringHash hash);
		bool* get_bool_cvar(StringHash hash);

		void set_int_cvar(StringHash hash, int32_t value);
		void set_float_cvar(StringHash hash, double value);
		void set_string_cvar(StringHash hash, const std::string& value);
		void set_bool_cvar(StringHash hash, bool value);

		template<typename T>
		CVarArray<T>* get_cvar_array();

		template<>
		CVarArray<int32_t>* get_cvar_array()
		{
			return &int_cvars;
		}

		template<>
		CVarArray<double>* get_cvar_array()
		{
			return &float_cvars;
		}

		template<>
		CVarArray<std::string>* get_cvar_array()
		{
			return &string_cvars;
		}

		template<>
		CVarArray<bool>* get_cvar_array()
		{
			return &bool_cvars;
		}

		template<typename T>
		T* get_cvar_current_value(uint32_t name_hash)
		{
			CVarParam* param = get_cvar(name_hash);
			if (param == nullptr)
			{
				return nullptr;
			}
			return get_cvar_array<T>()->get_current_value_ptr(param->index);
		}

		template<typename T>
		void set_cvar_current_value(uint32_t name_hash, const T& value)
		{
			CVarParam* param = get_cvar(name_hash);
			if (param != nullptr)
			{
				return get_cvar_array<T>()->set_current_value(value, param->index);
			}
		}
		
	public:
		std::unordered_map<uint32_t, CVarParam> saved_cvar_params;

		CVarArray<int32_t> int_cvars{ MAX_INT_CVARS };
		CVarArray<double> float_cvars{ MAX_FLOAT_CVARS };
		CVarArray<std::string> string_cvars{ MAX_STRING_CVARS };
		CVarArray<bool> bool_cvars{ MAX_BOOL_CVARS };
	};
}
