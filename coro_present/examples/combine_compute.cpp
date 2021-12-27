
#include <iostream>
#include <coroutine>
#include <thread>
#include <future>
#include <chrono>

using namespace std::chrono_literals;

template<typename T, typename ...Args>
struct std::coroutine_traits<std::future<T>, Args...>
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
	struct awaiter : std::future<T> {
		bool await_ready() const noexcept {
			using namespace std::chrono_literals;
			return this->wait_for(0s) != std::future_status::timeout;
		}
		void await_suspend(std::coroutine_handle<> cont) const {
			std::thread([this, cont] {
				this->wait();
				cont();
				}).detach();
		}
		T await_resume() { return this->get(); }
	};
	return awaiter{ std::move(future)};
}


int get_result()
{
	std::this_thread::sleep_for(500ms);
	return 21;
}

int process_result(int result)
{
	std::this_thread::sleep_for(1000ms);

	return result * 2;
}


int main() // без корутин
{
	auto start = std::chrono::high_resolution_clock::now();

	auto f1 = std::async(std::launch::async, get_result);

	std::this_thread::sleep_for(1000ms); // имитирует какую-то работу

	auto f2 = std::async(std::launch::async, process_result, f1.get());

	auto result = f2.get();

	auto end = std::chrono::high_resolution_clock::now();
	std::cout << "Result = " << result << '\n';
	std::cout << "Time : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
}			   

/*int main() // с корутинами
{	

	auto coro = []() ->std::future<int>
	{
		int result = co_await std::async(std::launch::async, get_result);
		std::cout << "awake 1\n";
		result = co_await std::async(std::launch::async, process_result, result);
		std::cout << "awake 2\n";
		co_return result;
	};


	auto start = std::chrono::high_resolution_clock::now();

	auto caller = coro();
	std::cout << "point 1\n";

	std::this_thread::sleep_for(1000ms); // имитирует какую-то работу

	std::cout << "point 2\n";

	auto result = caller.get();

	auto end = std::chrono::high_resolution_clock::now();
	std::cout << "Result = " << result << '\n';
	std::cout << "Time : " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
}  
*/