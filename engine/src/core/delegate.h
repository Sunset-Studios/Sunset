#pragma once

#include <exception>

namespace Sunset
{
	// Primary template intentionally left empty
	template<typename Signature>
	class Delegate;

	template<typename Signature>
	class MultiDelegate;

	class BadDelegateCall : public std::exception { };

	// Partial specialization so we can see the return type and arguments
	template<typename R, typename...Args>
	class Delegate<R(Args...)>
	{
	public:
		// Creates an unbound delegate
		Delegate() = default;

		// We want the Delegate to be copyable, since its lightweight
		Delegate(const Delegate& other) = default;
		auto operator=(const Delegate& other) -> Delegate& = default;

		// Call the underlying bound function
		template<typename...UArgs,
				 typename = std::enable_if_t<std::is_invocable_v<R(Args...), UArgs...>>>
		auto operator()(UArgs&&...args) const -> R
		{
			return stub(std::forward<Args>(args)...);
		}

		template<auto Function,
				 typename = std::enable_if_t<std::is_invocable_r_v<R, decltype(Function), Args...>>>
		auto bind() -> void
		{
			stub = [](Args...args) -> R {
				return std::invoke(Function, std::forward<Args>(args)...);
			};
		};

		template<typename Function>
		auto bind(Function func) -> void
		{
			stub = func;
		};

		template<auto MemberFunction, typename Class,
				 typename = std::enable_if_t<std::is_invocable_r_v<R, decltype(MemberFunction), const Class*, Args...>>>
		auto bind(const Class* cls) -> void
		{
			stub = [cls](Args...args) -> R {
				return std::invoke(MemberFunction, cls, std::forward<Args>(args)...);
			};
		}

		template<auto MemberFunction, typename Class,
				 typename = std::enable_if_t<std::is_invocable_r_v<R, decltype(MemberFunction), Class*, Args...>>>
		auto bind(Class* cls) -> void
		{
			stub = [cls](Args...args) -> R {
				return std::invoke(MemberFunction, cls, std::forward<Args>(args)...);
			};
		}

	private:
		using stub_function = std::function<R(Args...)>;

		stub_function stub = &stub_null;

	private:
		[[noreturn]]
		static auto stub_null(Args...) -> R
		{
			throw BadDelegateCall{};
		}
	};

	// Partial specialization so we can see the return type and arguments
	template<typename R, typename...Args>
	class MultiDelegate<R(Args...)>
	{
	public:
		// Creates an unbound delegate
		MultiDelegate() = default;

		// We want the Delegate to be copyable, since its lightweight
		MultiDelegate(const MultiDelegate& other) = default;
		auto operator=(const MultiDelegate& other) -> MultiDelegate& = default;

		// Call the underlying bound function
		template<typename...UArgs,
				 typename = std::enable_if_t<std::is_invocable_v<R(Args...), UArgs...>>>
		auto operator()(UArgs&&...args) const -> R
		{
			for (auto d : delegates)
			{
				d(std::forward<Args>(args)...);
			}
		}

		template<auto Function,
				 typename = std::enable_if_t<std::is_invocable_r_v<R, decltype(Function), Args...>>>
		auto bind() -> void
		{
			auto d = Delegate<R(Args...)>{};
			d.bind<Function>();
			delegates.push_back(d);
		};

		template<typename Function>
		auto bind(Function func) -> void
		{
			auto d = Delegate<R(Args...)>{};
			d.bind(func);
			delegates.push_back(d);
		};

		template<auto MemberFunction, typename Class,
				 typename = std::enable_if_t<std::is_invocable_r_v<R, decltype(MemberFunction), const Class*, Args...>>>
		auto bind(const Class* cls) -> void
		{
			auto d = Delegate<R(Args...)>{};
			d.bind<MemberFunction>(cls);
			delegates.push_back(d);
		}

		template<auto MemberFunction, typename Class,
				 typename = std::enable_if_t<std::is_invocable_r_v<R, decltype(MemberFunction), Class*, Args...>>>
		auto bind(Class* cls) -> void
		{
			auto d = Delegate<R(Args...)>{};
			d.bind<MemberFunction>(cls);
			delegates.push_back(d);
		}

	private:
		std::vector<Delegate<R(Args...)>> delegates;
	};
}