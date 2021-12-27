#include <iostream>
#include <coroutine>
#include <chrono>
#include <thread>

template <typename T>
struct task
{
	struct promise_type
	{

		std::suspend_never initial_suspend() { return{}; }
		std::suspend_always final_suspend() noexcept { return {}; }

		void return_void() {}

		void unhandled_exception() { std::terminate(); }

		task get_return_object() { return std::coroutine_handle<promise_type>::from_promise(*this); }
	};
	task(std::coroutine_handle<promise_type> h) : handle{ h } {}

private:
	std::coroutine_handle<promise_type> handle;
};

struct Sleeper
{
	constexpr bool await_ready() const noexcept 
	{
		return dur == dur.zero();
	}

	void await_suspend(std::coroutine_handle<> handle) const 
	{
		std::this_thread::sleep_for(dur);
		handle.resume();
	}

	void await_resume() const noexcept {}
	std::chrono::seconds dur;
};

task<void> coro()
{
	std::cout << "coro() : start sleeping!\n";
	co_await Sleeper{ std::chrono::seconds(3)};
	std::cout << "coro() : stop sleeping\n";
}

int main()
{
	auto caller = coro();
}

