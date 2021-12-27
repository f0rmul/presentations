
#include <iostream>
#include <coroutine>
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

	bool resume() { return handle ? (handle.resume(), !handle.done()) : false; }
private:
	std::coroutine_handle<promise_type> handle;
};



task<void> simple_await()
{
	std::cout << "Hello coro ";
	co_await std::suspend_always{};
	std::cout << "world!";
}


int main()
{
	auto caller = simple_await();
	caller.resume();
}

