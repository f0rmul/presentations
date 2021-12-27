#include <iostream>
#include <coroutine>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;
template <typename T>
struct task
{
	struct Sleeper;
	struct promise_type
	{
		std::suspend_never initial_suspend() { return{}; }
		std::suspend_always final_suspend() noexcept { return {}; }
		void return_void() {}
		void unhandled_exception() { std::terminate(); }
		task get_return_object() { return std::coroutine_handle<promise_type>::from_promise(*this); }

	};
	task(std::coroutine_handle<promise_type> h) : handle{ h } {}

	bool resume() { return handle ? (handle.resume(), !handle.done()) : false; }

	
	
private:
	std::coroutine_handle<promise_type> handle;
};

auto operator co_await(std::chrono::seconds sec)
{
	struct  Sleeper
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

	return Sleeper{ sec };
}


task<void> coro()
{
	std::cout << "Hello coro ";
	co_await std::chrono::seconds(2);
	std::cout << "world!";
}

int main()
{
	auto caller = coro();
}