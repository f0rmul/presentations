
#include <iostream>
#include <coroutine>
#include <future>
#include <chrono>
#include <thread>

template<typename T,typename ...Args>
struct std::coroutine_traits<std::future<T>,Args...>
{
	struct promise_type
	{
		std::promise<T> prom;

		std::future<T> get_return_object() { return prom.get_future(); }

		std::suspend_never initial_suspend() { return {}; }
		std::suspend_always	final_suspend() noexcept { return{}; }

		template<typename U>
		void return_value(U&& value)
		{
			prom.set_value(std::forward<U>(value));
		}

		void unhandled_exception() { prom.set_exception(std::current_exception()); }
	};
};


template <typename T>
auto operator co_await(std::future<T> future) noexcept
{
	struct awaiter : std::future<T>
	{
		bool await_ready() const noexcept
		{
			using namespace std::chrono_literals;
			return this->wait_for(0s) != std::future_status::timeout;
		}
		void await_suspend(std::coroutine_handle<> cont) const
		{
			std::thread([this, cont] {
				this->wait();
				cont();
				}).detach();
		}
		T await_resume() { return this->get(); }
	};
	return awaiter{ std::move(future) };
}


std::future<int> get_result()
{
	int result = co_await std::async(std::launch::async, [] {return 10; });

	co_return result;
}
int main()
{
   
	auto caller = get_result();

	std::cout << "Result = " << caller.get();
}


