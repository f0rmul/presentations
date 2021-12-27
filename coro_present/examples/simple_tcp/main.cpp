
#include <iostream>
#include <thread>
#include "hard_way.hpp"
//#include "easy_way.hpp"
#define HARD
//#define EASY
int main()
{
	try
	{
#ifdef HARD
		asio::io_context context;
		auto thread_count = 3;
		auto address = asio::ip::make_address("192.168.1.42");
		auto ep = asio::ip::tcp::endpoint(address, 1234);

		auto serv = server(context, ep);

		std::vector<std::jthread> pool(thread_count);
		while (--thread_count > 0) {
			pool.emplace_back([&context] { context.run(); });
		}

		context.run();
		
#endif // HARD


#ifdef EASY
		asio::io_context context;
		auto thread_count = 3;
		auto address = asio::ip::make_address("192.168.1.42");
		auto ep = asio::ip::tcp::endpoint(address, 1234);

		asio::co_spawn(context.get_executor(), server(context, ep), asio::detached);
		
		std::vector<std::jthread> pool(thread_count);
		while (--thread_count > 0) {
			pool.emplace_back([&context] { context.run(); });
		}

		context.run();



#endif // EASY
	}
	catch (const std::exception& e)
	{
		std::cout << e.what();
	}
}

