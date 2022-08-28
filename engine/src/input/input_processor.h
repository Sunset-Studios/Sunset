#pragma once

#include <common.h>
#include <input_types.h>

namespace Sunset
{
	template<class Policy>
	class GenericInputProcessor
	{
	public:
		GenericInputProcessor() = default;

		void initialize()
		{
			input_policy.initialize();
		}

		void update(class InputContext* context)
		{
			input_policy.update(context);
		}

	private:
		Policy input_policy;
	};

	class NoopInputProcessor
	{
	public:
		NoopInputProcessor() = default;

		void initialize()
		{ }

		void update(class InputContext* context)
		{ }
	};

#if USE_SDL_WINDOWING
	class InputProcessor : public GenericInputProcessor<InputProcessorSDL>
	{ };
#else
	class InputProcessor : public GenericInputProcessor<NoopInputProcessor>
	{ };
#endif

	class InputProcessorFactory
	{
	public:
		template<typename ...Args>
		static InputProcessor* create(Args&&... args)
		{
			InputProcessor* input = new InputProcessor;
			input->initialize(std::forward<Args>(args)...);
			return input;
		}
	};
}
