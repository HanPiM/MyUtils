#pragma once

#include <string>
#include <vector>

#include <unordered_map>

#include <functional>
#include <type_traits>

namespace sjson
{

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

	_fake_union() :_data(nullptr) {}
	_fake_union(const _fake_union& x)
	{
		_data = nullptr;
		_assign(x);
	}
	template<typename _t>
	_fake_union(const _t& x)
	{
		_data = nullptr;
		set(x);
	}
	const _fake_union& operator=(const _fake_union& x)
	{
		_assign(x);
		return x;
	}

	~_fake_union()
	{
		_destroy_data();
	}

	inline constexpr bool empty()const
	{
		return _data == nullptr;
	}
	inline void clear()
	{
		_destroy_data();
		_data = nullptr;
	}

	template<typename _t>
	inline constexpr _t& get()
	{
		return *((_t*)_data);
	}
	template<typename _t>
	inline constexpr const _t& get()const
	{
		return *((const _t*)_data);
	}
	// 如果原先为空则创建一个新对象再返回
	template<typename _t>
	inline constexpr _t& as()
	{
		if (empty())set(_t());
		return get<_t>();
	}
	template<typename _t>
	inline const _t& as()const
	{
		if (empty())
		{
			static _t x;
			return x;
		}
		return get<_t>();
	}

	template<typename _t>
	void set(const _t& x)
	{
		_destroy_data();
		_destructor = [](void* p) {delete (_t*)p; };
		_copy_constructor = [](void* x) {return new _t(*((_t*)x)); };
		_data = new _t(x);
	}

private:

	void* _data = nullptr;
	std::function<void(void*)> _destructor;
	std::function<void* (void*)> _copy_constructor;

	void _destroy_data()
	{
		if (!empty()) _destructor(_data);
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

enum class json_type
{
	value,
	array,
	object
};

class basic_value_type
{
public:

	enum
	{
		_empty,

		null,
		number_float,
		number_double,
		number_integer,
		boolean,
		string,

		_basic_type_end
	};

	basic_value_type() :_type(_empty) {}
	basic_value_type(nullptr_t) :_type(null) {}
	basic_value_type(bool x) :_type(boolean), _data(x) {}
	basic_value_type(float x) :_type(number_float), _data(x) {}
	basic_value_type(double x) :_type(number_double), _data(x) {}
	basic_value_type(int x) :_type(number_integer), _data(x) {}
	basic_value_type(const char* x) :_type(string), _data(std::string(x)) {}
	basic_value_type(const std::string& x) :_type(string), _data(x) {}

	operator float()const { return _data.as<float>(); }
	operator double ()const { return _data.as<double>(); }
	operator int ()const { return _data.as<int>(); }
	operator bool ()const { return _data.as<bool>(); }
	operator std::string()const { return _data.as<std::string>(); }

	template<typename _t> _t& as() { return _data.as<_t>(); }
	template<typename _t> const _t& as()const { return _data.as<_t>(); }

	std::string basic_to_string()const
	{
		switch (_type)
		{
		case number_float:
			return std::to_string(_data.as<float>());
		case number_double:
			return std::to_string(_data.as<double>());
		case number_integer:
			return std::to_string(_data.as<int>());
		case string:return _data.as<std::string>();
		case null:return "null";
		case boolean:return _data.as<bool>() ? "true" : "false";
		case _empty:return "_empty";
		}
		return "unknown";
	}
	virtual std::string to_string()const
	{
		return basic_to_string();
	}

	inline constexpr int type()const { return _type; }
	static const char* basic_type_name(int type)
	{
		switch (type)
		{
		case number_float:
		case number_double:
		case number_integer:
			return "number";
		case string:return "string";
		case null:return "null";
		case boolean:return "boolean";
		case _empty:return "_empty";
		}
		return "unknown";
	}
	virtual const char* type_name(int type)
	{
		return basic_type_name(type);
	}
	virtual std::string type_name()const
	{
		return basic_type_name(_type);
	}

private:

	int _type;
	_fake_union _data;
};

template<typename _value_type>
class json_base
{
private:
	class _my_initializer_list;
public:

	using object = std::unordered_map<std::string, json_base>;
	class array :public std::vector<json_base>
	{
	public:
		using _base_t = std::vector<json_base>;
		array() : _base_t() {}
		array(std::initializer_list<_my_initializer_list> x)
		{
			for (auto& it : x)
			{
				this->push_back(it.data());
			}
		}
	};

	json_base() :_type(json_type::value), _data(_value_type()) {}
	json_base(const json_base& x) { _assign(x); }
	json_base(const array& x) { _assign(x); }
	json_base(const object& x) { _assign(x); }
	json_base(std::initializer_list<_my_initializer_list> x)
	{
		_assign(_my_initializer_list(x));
	}
	template<
		typename _t,
		typename std::enable_if <
		std::is_constructible<_value_type, _t>::value, int
		>::type = 0
	>
		json_base(const _t& x)
	{
		_assign(_value_type(x));
	}

	const json_base& operator=(const json_base& x)
	{
		_assign(x); return x;
	}
	const array& operator=(const array& x)
	{
		_assign(x); return x;
	}
	const object& operator=(const object& x)
	{
		_assign(x); return x;
	}

	inline json_type type()const { return _type; }
	inline bool is_value()const { return _type == json_type::value; }
	inline bool is_array()const { return _type == json_type::array; }
	inline bool is_object()const { return _type == json_type::object; }

	inline array& as_array() { return _data.as<array>(); }
	inline object& as_object() { return _data.as<object>(); }
	inline _value_type& as_value() { return _data.as<_value_type>(); }

	inline const array& as_array()const { return _data.as<array>(); }
	inline const object& as_object()const { return _data.as<object>(); }
	inline const _value_type& as_value()const { return _data.as<_value_type>(); }

	template<typename _t>
	explicit operator const _t()const
	{
		return as_value();
	}

	json_base& operator [](size_t idx)
	{
		if (!is_array())
		{
			_type = json_type::array;
			_data.set(array());
		}
		return as_array()[idx];
	}
	const json_base& operator [](size_t idx)const { return as_array()[idx]; }

	json_base& operator [](const std::string& key)
	{
		if (!is_object())
		{
			_type = json_type::object;
			_data.set(object());
		}
		return as_object()[key];
	}
	const json_base& operator[](const std::string& key)const
	{
		return as_object()[key];
	}

	bool empty()const
	{
		if (is_array())return as_array().empty();
		if (is_object())return as_object().empty();
		return false;
	}
	inline void clear()
	{
		_data.clear();
	}


private:

	class _my_initializer_list
	{
	public:
		/*
		* {a,b,c} : 这种情况会对每一个元素调用该函数来初始化
		*  ^
		*/
		template<typename _t>
		_my_initializer_list(const _t& x) :_data(x) {}
		/*
		* (a,b,c) 或 {}
		* ^^^^^^^
		* 会调用该函数来初始化
		*/
		template<typename ..._ts>
		_my_initializer_list(const _ts& ...args) :
			_data(array({ json_base(args)... })) {}
		_my_initializer_list(std::initializer_list<_my_initializer_list> x)
		{
			/*
			* 形如 {{"xxx",...},{"yyy",...},...} 的数据会被视为 object
			*/
			bool is_object = true;
			for (auto& it : x)
			{
				const auto& node = it._data;
				if ((!node.is_array()) || node.as_array().size() != 2)
				{
					is_object = false;
					break;
				}
				const auto& first = node.as_array()[0];
				if (
					(!first.is_value())
					|| first.as_value().type() != _value_type::string
				){
					is_object = false;
					break;
				}
			}
			if (is_object)
			{
				_data = object();
				for (auto& it : x)
				{
					_data.as_object()[
						it._data.as_array()[0].as_value().as<std::string>()
					] = it._data.as_array()[1];
				}
				return;
			}
			_data = array();
			for (auto& it : x)
			{
				_data.as_array().push_back(it._data);
			}
		}

		json_base& data() { return _data; }
		const json_base& data()const { return _data; }

	private:

		json_base _data;
	};

	void _assign(const _my_initializer_list& obj)
	{
		auto& x = obj.data();
		if (x.is_array())_assign(x.as_array());
		else if (x.is_object())_assign(x.as_object());
		else if (x.is_value())_assign(x.as_value());
		else _type = json_type::value, _data = _value_type();
	}
	void _assign(const json_base& x)
	{
		_type = x._type;
		_data = x._data;
	}
	template<typename _t, json_type _t_val>
	void _assign(const _t& x)
	{
		if (_type == _t_val)_data.as<_t>() = x;
		else
		{
			_data.set(x);
			_type = _t_val;
		}
	}
	inline void _assign(const _value_type& x)
	{
		_assign<_value_type, json_type::value>(x);
	}
	inline void _assign(const array& x)
	{
		_assign<array, json_type::array>(x);
	}
	inline void _assign(const object& x)
	{
		_assign<object, json_type::object>(x);
	}

	friend class array;
	friend class _my_initializer_list;

	json_type _type;
	_fake_union _data;
};


using json = json_base<basic_value_type>;

}