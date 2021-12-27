
#include <iostream>

#include <coroutine>
#include <optional>

template <typename T>
class task
{
public:
	struct promise_type;
	using coro_handle = std::coroutine_handle<promise_type>;
	using value_type = T;

	task(coro_handle handle) : m_handle{ handle } {}

	task(const task&) = delete;
	task& operator=(const task&) = delete;
	task(task&& rhs) : m_handle{ rhs.m_handle }
	{
		rhs.m_handle = nullptr;
	}
	task& operator=(task&& rhs)
	{
		if (std::addressof(rhs) == this) return *this;
		m_handle = rhs.m_handle;
		rhs.m_handle = nullptr;
		return *this;
	}

	~task() { if (m_handle) m_handle.destroy(); }

	std::optional<value_type> current_value() const
	{
		if (m_handle)
			return m_handle.promise().m_opt;
		return std::nullopt;
	}
private:
	struct awaiter;
	coro_handle m_handle;
};

template<typename T>
struct task<T>::promise_type
{
	std::optional<value_type> m_opt;

	std::suspend_never initial_suspend() { return {}; }
	std::suspend_always final_suspend() noexcept { return {}; }

	awaiter await_transform(std::optional<value_type> opt)
	{
		return awaiter{ opt };
	}
	void unhandled_exception() { std::terminate(); }
	auto get_return_object() { return coro_handle::from_promise(*this); }
	void return_value(value_type value) { m_opt = value; }
};


template<typename T>
struct task<T>::awaiter
{
	std::optional<value_type> m_opt;
	awaiter(std::optional<value_type> opt) : m_opt{ opt } {}
	bool await_ready() { return m_opt.has_value(); }
	void await_suspend(std::coroutine_handle<>) {}
	auto await_resume() { return m_opt.value(); }
};










std::optional<int> one() noexcept
{
    return  1;
}
std::optional<int> two()noexcept
{
    return 2;
}

std::optional<int> empty()
{
    return std::nullopt;
}

task<int> sum()
{
    auto a = co_await empty();
    auto b = co_await two();

    co_return a + b;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::optional<T>& opt)
{
    if (opt)
        std::cout << " sum() = " << opt.value() << std::endl;
    else
        std::cout << "empty state\n";
    return os;
}
int main()
{

    auto res = sum();
    std::cout << res.current_value();
}
