
#include <iostream>

#pragma once
#include <coroutine>


template <typename T>
class task final
{
public:
	struct promise_type;
	using custom_suspender_t = T;
	using coro_handle = std::coroutine_handle<promise_type>;

	task() = delete;
	task(coro_handle handle) : m_handle{ handle } {}

	task(const task&) = delete;
	task& operator=(const task&) = delete;

	task(task&& rhs) noexcept :m_handle{ rhs.m_handle }
	{
		rhs.m_handle = nullptr;
	}
	task& operator=(task&& rhs) noexcept
	{
		if (std::addressof(rhs) == this)
			return *this;
		m_handle = rhs.m_handle;
		rhs.m_handle = nullptr;
		return*this;
	}

	~task() { if (m_handle) m_handle.destroy(); }

	bool resume() { return m_handle ? (m_handle.resume(), !m_handle.done()) : false; }

	void cancell() { promise_type::is_cancelled = true; resume(); }

private:
	coro_handle m_handle;
};

template <typename T>
struct task<T>::promise_type
{
	static inline bool is_cancelled = false;

	constexpr std::suspend_always initial_suspend() const noexcept { return{}; }
	constexpr std::suspend_always final_suspend() const noexcept { return{}; }

	task get_return_object() { return coro_handle::from_promise(*this); }

	void return_void() const noexcept {}

	void unhandled_exception() { std::terminate(); } /* так делать нельзя(но да пох)*/

	custom_suspender_t await_transform(std::suspend_always) /*placeholder*/
	{
		return custom_suspender_t{ is_cancelled };
	}
};

struct custom_suspender final
{
	bool m_state;

	explicit custom_suspender(bool new_state) : m_state{ new_state } {}

	constexpr bool await_ready() const noexcept { return m_state; }
	constexpr bool await_suspend(std::coroutine_handle<>) { return !m_state; }

	void await_resume() const noexcept {}
};







task<custom_suspender> coro()
{
	for (int i{ 0 }; i < 10; ++i)
	{
		std::cout << i << "\n";
		co_await std::suspend_always{};
	}
}
int main()
{
	auto caller = coro();

	caller.resume();
	caller.resume();
	std::cout << "---- cancell ----\n";
	caller.cancell();
}


