#include "string_tools.h"
#include "logger.h"
#include "simpjson.h"
#include <array>

#include <tuple>

#include <iostream>

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
	inline constexpr const _t& as()const
	{
		if (empty())return std::move(_t());
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

class basic_value_type
{
public:

	enum
	{
		_empty,

		null,
		number_float,
		number_integer,
		boolean,
		string,

		_basic_type_end
	};

	basic_value_type() :_type(_empty) {}
	basic_value_type(nullptr_t x) :_type(null) {}
	basic_value_type(const bool x) :_type(boolean), _data(x) {}
	basic_value_type(const double x) :_type(number_float), _data(x) {}
	basic_value_type(const int x) :_type(number_integer), _data(x) {}
	basic_value_type(const char* x) :_type(string), _data(std::string(x)) {}
	basic_value_type(const std::string& x) :_type(string), _data(x) {}

	std::string basic_to_string()const
	{
		switch (_type)
		{
		case number_float:
			return std::to_string(_data.get<double>());
		case number_integer:
			return std::to_string(_data.get<int>());
		case string:return _data.get<std::string>();
		case null:return "null";
		case boolean:return _data.get<bool>() ? "true" : "false";
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
public:
	class object;
	class array;

	class data_t;
	using _map_t = std::unordered_map<std::string, data_t>;

	class data_t
	{
	public:

		data_t() :_type(val), _data(_value_type()) {}
		data_t(const data_t& x) { _assign(x); }
		data_t(const array& x) { _assign(x); }
		data_t(const object& x) { _assign(x); }

		template<
			typename _t,
			typename std::enable_if <
				std::is_constructible<_value_type, _t>::value, int
			>::type = 0
		>
		data_t(const _t& x)
		{
			_assign(_value_type(x));
		}

		const data_t& operator=(const data_t& x)
		{ _assign(x); return x; }
		const array& operator=(const array& x)
		{ _assign(x); return x; }
		const object& operator=(const object& x)
		{ _assign(x); return x; }

		enum
		{
			val,
			arr,
			obj,
			_maybe_obj
		};

		inline int type()const { return _type; }
		inline bool is_val()const { return _type == val; }
		inline bool is_arr()const { return _type == arr; }
		inline bool is_obj()const { return _type == obj; }

		inline array& as_arr() { return _data.as<array>(); }
		inline object& as_obj() { return _data.as<object>(); }
		inline _value_type& as_val() { return _data.as<_value_type>(); }

		inline const array& as_arr()const { return _data.as<array>(); }
		inline const object& as_obj()const { return _data.as<object>(); }
		inline const _value_type& as_val()const { return _data.as<_value_type>(); }

		data_t& operator [](size_t idx)
		{
			if (!is_arr())
			{
				_type = arr;
				_data.set(array());
			}
			return as_arr()[idx];
		}
		const data_t& operator [](size_t idx)const { return as_arr()[idx]; }

		data_t& operator [](const std::string& key)
		{
			if (!is_obj())
			{
				_type = obj;
				_data.set(object());
			}
			return as_obj()[key];
		}
		const data_t& operator[](const std::string& key)const
		{ return as_obj()[key]; }

		bool empty()const
		{
			if (is_arr())return as_arr().empty();
			if (is_obj())return as_obj().empty();
			return false;
		}
		inline void clear()
		{
			_data.clear();
		}


	private:

		void _assign(const data_t& x)
		{
			_type = x._type;
			_data = x._data;
		}
		template<typename _t, int _t_id>
		void _assign(const _t& x)
		{
			if (_type == _t_id)_data.as<_t>() = x;
			else
			{
				_data.set(x);
				_type = _t_id;
			}
		}
		inline void _assign(const _value_type& x)
		{ _assign<_value_type, val>(x); }
		inline void _assign(const array& x)
		{ _assign<array, arr>(x); }
		inline void _assign(const object& x)
		{ _assign<object, obj>(x); }

		friend class object;
		friend class array;
		friend class _obj_for_init;
		
		int _type;
		_fake_union _data;
	};

	class object :public _map_t
	{
	public:

	};

	class _obj_for_init
	{
	public:
		/*
		* {a,b,c} : 这种情况会对每一个元素调用该函数来初始化
		*  ^
		*/
		template<typename _t>
		_obj_for_init(const _t& x) :_data(x) {}
		/*
		* (a,b,c) 或 {}
		* ^^^^^^^
		* 会调用该函数来初始化
		*/
		template<typename ..._ts>
		_obj_for_init(const _ts& ...args)
		{
			auto& x = _data = array({ data_t(args)... });
		}
		_obj_for_init(std::initializer_list<_obj_for_init> x)
		{
			bool is_obj = true;
			for (auto& it : x)
			{
				const auto& node = it._data;
				if ((!node.is_arr()) || node.as_arr().size() != 2)
				{
					is_obj = false;
					break;
				}
				const auto& first = node.as_arr()[0];
				if ((!first.is_val()) || first.as_val().type() != _value_type::string)
				{
					is_obj = false;
					break;
				}
			}
			if (is_obj)
			{
				_data = object();
				for (auto& it : x)
				{
					_data.as_obj()[it._data.as_arr()[0].as_val().to_string()] = it._data.as_arr()[1];
				}
				return;
			}
			_data = array();
			for (auto& it : x)
			{
				_data.as_arr().push_back(it._data);
			}
		}

		data_t& data() { return _data; }
		const data_t& data()const { return _data; }

	private:

		data_t _data;
	};

	class array :public std::vector<data_t>
	{
	public:
		using _base_t = std::vector<data_t>;
		array() : _base_t() {}
		array(std::initializer_list<_obj_for_init> x)
		{
			for (auto& it : x)
			{
				this->push_back(it.data());
			}
		}
	};

private:

};

using basic_json = json_base<basic_value_type>;

int main()
{
	
	std::string s = "123ww";
	double d = 3.141;

	using json_t = basic_json;

	json_t::array arr_test(
	{
		{
			{
				{
					
					{
						1,2,3,4,5
					},
					1
				}
			}
		}
	});

	std::cout << arr_test.size() << "________\n";
	std::cout << arr_test[0][0].as_arr().size() << std::endl;

	json_t::_obj_for_init obj_for_init_test =
	{
		{
			{
				{
					1,2,3,4,5
				},
				1
			}
		}
	};
	std::cout << obj_for_init_test.data()[0][0].as_arr().size() << '\n';

	json_base<basic_value_type>::_obj_for_init xx = 
	{
		1,2,3,4,d,
		s,
		arr_test,
		{
			{"pi", 3.141},
			{"happy", true},
			{"name", "Niels"},
			{"nothing", nullptr},
		},
		{"answer",
			{
				{
					{"everything", 42},
					"ffff"
				},
				{
					1111
				},
				{},{},{},{},{}
			}
		},
		{"list", {1, 0, 2}},
		{
			"object",
			{
				{"currency", "USD"},
				{"value", 42.99}
			},
			"object2",
			{
				{"currency", "USD"},
				{"value", 42.99}
			},
			"ssss",
			"object3",
			{
				{"currency", "USD"},
				{"value", 42.99}
			},
			{
				{"fu((", "USD"},
				{"value", 42.99}
			},
			{
				{"thtttt", "USD"},
				{"value", 42.99}
			}
		}
	};
	
	s = "s";
	d = 999;
	arr_test.push_back(114514);

	for (auto& it : xx.data().as_arr())
	{
		if (it.is_arr())printf("[...]:%d\n", it.as_arr().size());
		else if (it.is_obj())
		{
			printf("{obj}\n");
		}
		else if(it.is_val())printf(":'%s'\n",it.as_val().to_string().c_str());
		else
		{

		}
	}

	//printf("%s", xx._objs[7]._objs[0]._element.str_val().c_str());

	return 0;
}