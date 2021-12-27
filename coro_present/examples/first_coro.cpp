
#include <iostream>
#include <coroutine>
#include <future>




struct task {
	struct promise_type
	{
			  std::suspend_never initial_suspend(){return{}; }
			  std::suspend_always final_suspend() noexcept{ return{}; }
			   void return_void(){}
			   void unhandled_exception() {}
			   task get_return_object() { return task{ std::coroutine_handle<promise_type>::from_promise(*this) }; }
	};
	task(std::coroutine_handle<promise_type> h_) : h{ h_ } {}
	private:
		std::coroutine_handle<promise_type> h;
 };




task g()
{	
	std::cout <<"Hello";
	co_return;
}


int main()
{
	g();
}