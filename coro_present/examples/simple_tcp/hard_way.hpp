#pragma once
#include <asio.hpp>
#include <memory>
#include <iostream>

class session
{
public:

	session(asio::io_context& io, asio::ip::tcp::socket s) : context{ io }, sock{ std::move(s) } {}

	void start()
	{
		sock.async_read_some(asio::buffer(buf.data(), buf.size()), [this](auto ec, auto n) { handle_read(ec, n); });
		return;
	}

	void handle_read(const asio::error_code& ec, std::size_t n)
	{
		if (!ec)
		{
			asio::async_write(sock, asio::buffer(buf.data(), buf.size()), [this](auto ec, auto) { handle_write(ec); });
		}
	}

	void handle_write(const asio::error_code& ec)
	{
		if (!ec)
		{
			sock.async_read_some(asio::buffer(buf.data(), buf.size()), [this](auto ec, auto n) {handle_read(ec, n); });
		}
	}


private:
	asio::io_context& context;
	asio::ip::tcp::socket sock;
	std::vector<char> buf;
};



class server
{
public:
	server(asio::io_context& io, asio::ip::tcp::endpoint ep) : context{ io }, acc(io, ep)
	{
		acc.listen();
		std::cout << "server started!\n";
		start_accept();
	}

	void start_accept()
	{
		acc.async_accept([this](auto ec, auto s) { handle_accept(ec, std::move(s)); });
	}

	void handle_accept(const asio::error_code& ec, asio::ip::tcp::socket s)
	{
		if (!ec)
		{
			std::unique_ptr<session> session_ = std::make_unique<session>(context, std::move(s));
			session_->start();
		}
		start_accept();
	}
private:

	asio::io_context& context;
	asio::ip::tcp::acceptor acc;
};