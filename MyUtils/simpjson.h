#pragma once

#include <string>
#include <vector>

#include <type_traits>

class _ref_union
{
public:
	_ref_union() :_data(nullptr) {};
	_ref_union(const _ref_union& x) :_data(x._data) {}
	template<typename T>
	inline constexpr void set(const T* x) { _data = (void*)x; };
	template<typename T>
	inline constexpr T* get() { return (T*)_data; }
	template<typename T>
	inline constexpr const T* get()const { return (T*)_data; }
private:
	void* _data;
}; 

class _fake_union
{
public:

	_fake_union() { _data = nullptr; }
	_fake_union(const _fake_union& x) { _assign(x); }
	_fake_union& operator=(const _fake_union& x)
	{
		_assign(x);
		return *this;
	}

	~_fake_union() { if (_data != nullptr)_destructor(_data); }

	constexpr bool empty()const { return _data == nullptr; }

	template<typename T>
	inline constexpr T& get()
	{
		return *((T*)_data);
	}
	template<typename T>
	inline constexpr T& as()
	{
		if (empty())set(T());
		return get<T>();
	}
	template<typename T>
	void set(const T& x)
	{
		_destroy_data();
		_destructor = [](void* p) {delete (T*)p; };
		_copy_constructor = [](void* x) {return new T(*((T*)x)); };
		_data = new T(x);
	}

private:

	void* _data = nullptr;
	std::function<void(void*)> _destructor;
	std::function<void* (void*)> _copy_constructor;

	void _destroy_data()
	{
		if (!empty())
			_destructor(_data);
		// 可以不设为空指针，因为该函数在内部使用，可确保调用后会赋值
	}

	void _assign(const _fake_union& x)
	{
		_destroy_data();
		_destructor = x._destructor;
		_copy_constructor = x._copy_constructor;
		if (x._data != nullptr)
			_data = _copy_constructor(x._data);
		else _data = nullptr;
	}
};

class _sjson_base
{
public:
	_sjson_base() = default;
//protected:
	using _data_type_id = int;
	
	_fake_union _data;
	_data_type_id _type = 0;
};

template<typename _node_t>
class _sjson_item;

class _sjson_node :private _sjson_base
{
public:
	enum
	{
		unknown,

		number,
		string,
		boolean,
		array,
		null
	};

	_sjson_node()
	{
		assign();
	}

	void assign()
	{
		_type = null;
	}

	template<typename T>
		typename std::enable_if<std::is_integral<T>::value, void>::type 
			assign(T x) { _type = number; _data.set(x); }

	void assign(bool x) { _type = boolean; _data.set(x); }
	void assign(const std::string& x) { _type = string; _data.set(x); }
	void assign(const std::vector<_sjson_item<_sjson_node> >& x)
	{ _type = array; _data.set(x); }

	template<typename T>
	T& get() { return _data.get<T>(); }
};

template<typename _node_t = _sjson_node>
class _sjson_item
{
public:

	using _map_t = std::unordered_map<std::string, _sjson_item>;

	_sjson_item()
	{
		_isobj = false;
	}
	_sjson_item(const _sjson_item& x)
	{
		_isobj = x._isobj;
		_data = x._data;
	}
	_sjson_item(const _map_t& x)
	{
		assign(x);
	}

	template<typename T>
	_sjson_item(const T& x)
	{
		assign(x);
	}

	const _map_t& operator=(const _map_t& x)
	{
		assign(x);
		return x;
	}

	template<typename T>
	const T& operator=(const T& x)
	{
		assign(x);
		return x;
	}

	template<typename T>
	operator T&()
	{
		return _data.get<_node_t>().get<T>();
	}

	_sjson_item& operator [](const char* key)
	{
		return this->operator[](std::string(key));
	}
	_sjson_item& operator [](const std::string& key)
	{
		if(!_isobj)
		{
			_data.set(_map_t());
			_isobj = true;
		}

		auto& items = _data.get<_map_t>();
		const auto& it = items.find(key);
		if (it == items.end())
		{
			auto res = items.insert({ key, _sjson_item() });
			return res.first->second;
		}
		else
		{
			return it->second;
		}
	}

private:

	bool _isobj = false;
	_fake_union _data;

	void assign(const _map_t& x)
	{
		_data.get<_map_t>() = x;
		_isobj = true;
	}

	template<typename T>
	void assign(const T& x)
	{
		_data.get<_node_t>().assign(x);
		_isobj = false;
	}
};