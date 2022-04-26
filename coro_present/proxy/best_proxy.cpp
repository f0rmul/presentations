#include <iostream>
#include <array>
#include <chrono>
#include <asio.hpp>
#include <asio/experimental/as_tuple.hpp>
#include <asio/experimental/awaitable_operators.hpp>

constexpr auto use_nothrow_awaitable = asio::experimental::as_tuple(asio::use_awaitable);
using namespace std::chrono_literals;
using namespace asio::experimental::awaitable_operators;
asio::awaitable<void> timeout(std::chrono::steady_clock::duration dur)
{
	asio::steady_timer timer{ co_await asio::this_coro::executor };
	timer.expires_after(dur);
	co_await timer.async_wait(use_nothrow_awaitable);
}


asio::awaitable<void> transfer(asio::ip::tcp::socket& from, asio::ip::tcp::socket& to)
{
	std::array<char, 1024> buffer;

	for (;;)
	{
		auto result_1 = co_await(from.async_read_some(asio::buffer(buffer), use_nothrow_awaitable)
			|| timeout(15s));
		if (result_1.index() == 1)
			co_return;

		auto [err_code, bytes1] = std::get<0>(result_1);

		if (err_code) break;

		auto result_2 = co_await(asio::async_write(to, asio::buffer(buffer, bytes1), use_nothrow_awaitable)
			|| timeout(15s));

		if (result_2.index() == 1)
			co_return;
		auto [err_code1, bytes2] = std::get<0>(result_2);

		if (err_code1) break;
	}
}

asio::awaitable<void> proxy(asio::ip::tcp::socket client, asio::ip::tcp::endpoint target)
{
	asio::ip::tcp::socket server{ client.get_executor() };

	auto [err_code] = co_await server.async_connect(target, use_nothrow_awaitable);

	if (!err_code)
	{
		co_await
		(
			transfer(client, server) || transfer(server, client)
			);
	}
}

asio::awaitable<void> listen(asio::ip::tcp::acceptor& acceptor, asio::ip::tcp::endpoint target)
{
	for (;;)
	{
		auto [err_code, client] = co_await acceptor.async_accept(use_nothrow_awaitable);
		if (err_code)break;
		std::cout << "New connection!\n";
		asio::co_spawn(client.get_executor(), proxy(std::move(client), target), asio::detached);
	}
}
int main(int argc, char** argv)
{
	try
	{
		if (argc != 5)
		{
			std::cerr << "Usage: proxy";
			std::cerr << " <listen_address> <listen_port>";
			std::cerr << " <target_address> <target_port>\n";
			return 1;
		}

		asio::io_context ctx;

		auto listen_endpoint =
			*asio::ip::tcp::resolver(ctx).resolve(
				argv[1],
				argv[2],
				asio::ip::tcp::resolver::passive
			);

		auto target_endpoint =
			*asio::ip::tcp::resolver(ctx).resolve(
				argv[3],
				argv[4]
			);

		asio::ip::tcp::acceptor acceptor(ctx, listen_endpoint);

		asio::co_spawn(ctx, listen(acceptor, target_endpoint), asio::detached);

		ctx.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
}