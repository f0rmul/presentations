#include <iostream>
#include <coroutine>
template<typename T>
class generator
{
public:
	struct promise_type;
	class iterator;
	class sentinel_iterator {};

	using value_type = T;
	using coro_handle = std::coroutine_handle<promise_type>;

	generator(coro_handle handle) : m_handle{ handle } {}

	generator(const generator&) = delete;
	generator& operator=(const generator&) = delete;

	generator(generator&& rhs)noexcept : m_handle{ rhs.m_handle }
	{
		rhs.m_handle = nullptr;
	}

	generator& operator=(generator&& rhs) noexcept
	{
		if (std::addressof(rhs) == this) return *this;
		m_handle = rhs.m_handle;
		rhs.m_handle = nullptr;
		return *this;
	}
	~generator() { if (m_handle) m_handle.destroy(); }

	bool go_next()
	{
		return m_handle ? (m_handle.resume(), !m_handle.done()) : false;
	}
	value_type get() const noexcept
	{
		return m_handle.promise().value;
	}

	iterator begin()
	{
		if (m_handle) m_handle.resume();
		return iterator{ m_handle };
	}
	sentinel_iterator end() noexcept { return{}; }

private:
	coro_handle m_handle;
};


template<typename T>
struct generator<T>::promise_type
{
	value_type value;
	std::exception_ptr exception;

	generator get_return_object() { return coro_handle::from_promise(*this); }

	std::suspend_never initial_suspend() { return{}; }
	std::suspend_always final_suspend() noexcept { return {}; }

	void unhandled_exception() { exception = std::current_exception(); }

	//void return_void() {}
	template<std::convertible_to<value_type> From>
	void return_value(From&& value_)
	{
		value = std::forward<From>(value_);
	}
	template<std::convertible_to<value_type> From>
	std::suspend_always yield_value(From&& value_)
	{
		value = std::forward<From>(value_);
		return {};
	}

};


template <typename T>
struct generator<T>::iterator
{
	coro_handle m_handle = nullptr;

	using iterator_category = std::input_iterator_tag;
	using difference_type = std::ptrdiff_t;
	using value_type = T;

	iterator() noexcept = default;

	iterator(const iterator&) = delete;


	iterator(iterator&& other) noexcept
		: m_handle(other.m_handle) {
		other.m_handle = nullptr;
	}

	iterator& operator=(iterator&& other) noexcept {
		m_handle = other.m_handle;
		other.m_handle = nullptr;
		return *this;
	}

	explicit iterator(coro_handle coroutine) noexcept
		: m_handle(coroutine) {}

	bool operator==(sentinel_iterator) const noexcept {
		return !m_handle || m_handle.done();
	}

	iterator& operator++()
	{
		m_handle.resume();
		return *this;
	}

	value_type operator *() noexcept
	{
		return m_handle.promise().value;
	}
};







generator<int> simple()
{   
    
	for (int i{ 0 }; i < 5;++i)
		co_yield i;
}


int main()
{   
 
       /* auto caller = simple();
        std::cout << caller.get() << " ";
        caller.go_next();
        std::cout << caller.get();
        */
        for (auto&& el : simple())
        {
            std::cout << el << " ";
        }

}
