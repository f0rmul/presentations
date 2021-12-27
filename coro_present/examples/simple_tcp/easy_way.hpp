#pragma once
#include <asio.hpp>
#include <iostream>

asio::awaitable<void> session(asio::ip::tcp::socket sock)
{
	sock.set_option(asio::ip::tcp::no_delay(true));
	std::vector<char> buff(128);

	for (;;)
	{	
		std::size_t n = co_await sock.async_read_some(asio::buffer(buff.data(), buff.size()), asio::use_awaitable);
		co_await asio::async_write(sock, asio::buffer(buff.data(), n), asio::use_awaitable);
	}
}

asio::awaitable<void> server(asio::io_context& io,asio::ip::tcp::endpoint ep)
{
	asio::ip::tcp::acceptor acc_(io.get_executor(),ep);
	acc_.listen();
	std::cout << "Server started...\n";
	for (;;)
	{
		asio::ip::tcp::socket sock= co_await acc_.async_accept(asio::use_awaitable);
		asio::co_spawn(io.get_executor(), session(std::move(sock)), asio::detached);
	}
}

