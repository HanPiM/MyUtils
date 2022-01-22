#include "string_tools.h"
#include "logger.h"
#include "simpjson.h"
#include <array>

#include <tuple>

#include <iostream>

class _base_value_ref
{
public:
	enum
	{
		_empty,

		number_float,
		number_integer,
		boolean,
		cstring,
		string,
		null
	};
	_base_value_ref() { _type = _empty; }
	_base_value_ref(nullptr_t x)
	{
		_type = null;
	}
	_base_value_ref(const bool& x)
	{
		_type = boolean;
		_data.set(&x);
	}
	_base_value_ref(const double& x)
	{
		_type = number_float;
		_data.set(&x);
	}
	_base_value_ref(const int& x)
	{
		_type = number_integer;
		_data.set(&x);
	}
	_base_value_ref(const char* x)
	{
		_type = cstring;
		_data.set(x);
	}
	_base_value_ref(const std::string& x)
	{
		_type = string;
		_data.set(&x);
	}

	std::string str_val()const
	{
		switch (_type)
		{
		case number_float:
			return std::to_string(*_data.get<double>());
		case number_integer:
			return std::to_string(*_data.get<int>());
		case cstring:return _data.get<char>();
		case string:return *_data.get<std::string>();
		case null:return "null";
		case boolean:return *_data.get<bool>() ? "true" : "false";
		case _empty:return "_empty";
		}
		return "unknown";
	}

	std::string type_name()const
	{
		switch (_type)
		{
		case number_float:
		case number_integer:
			return "number";
		case cstring:
		case string:
			return "string";
		case null:return "null";
		case boolean:return "boolean";
		case _empty:return "_empty";
		}
		return "unknown";
	}

	int _type;
	_ref_union _data;
};

template<typename _basic_type>
class _initializer
{
public:

	template<typename _basic_type>
	friend class _obj_for_init;

	class value
	{
	public:
		constexpr static value empty_flag() { return value(-1); };

		constexpr value(int siz) :_siz(siz) {}
		constexpr value(_basic_type data) :_data(data) {}

		constexpr bool is_empty_flag()const { return _siz < 0; }
		constexpr bool is_size_flag()const { return _siz > 0; }
		constexpr bool is_data()const { return _siz == 0; }

		constexpr int get_size()const { return _siz; }
		constexpr _basic_type get_data()const { return _data; }
	private:
		int _siz = 0;
		_basic_type _data;
	};

	template<typename ..._ts>
	_initializer(const _ts& ...args)
	{
		_vals = { _basic_type(args)... };
	}

	_initializer(std::initializer_list<_initializer> x)
	{
		for (auto& it : x)
		{
			_vals.insert(_vals.end(), it._vals.begin(), it._vals.end());
			if (it._vals.empty())_vals.push_back(value::empty_flag());
		}
		_vals.push_back(value(x.size()));

	}
//private:
	std::vector<value> _vals;
};

// 用于初始化的中间对象
template<typename _basic_type>
class _obj_for_init
{
public:

	_obj_for_init()
	{
		_is_objs = false;
		_data.element = 0;
	}
	_obj_for_init(const _obj_for_init& x) { assign(x); }
	_obj_for_init(const _initializer<_basic_type>& x) :_is_objs(true)
	{
		_objs_t objs;
		//objs.push_back(_obj_for_init());

		for (int i = 0; i<int(x._vals.size()); ++i)
		{
			auto& it = x._vals[i];
			if (it.is_size_flag())
			{
				//int n = it.get_size();
				//_objs_t tmp;
				//tmp.resize(n);
				//std::copy(objs.end() - n, objs.end(), tmp.begin());
				//objs.erase(objs.end() - n, objs.end());
				//objs.push_back(_obj_for_init(tmp));
			}
			else if (it.is_empty_flag())
			{
				_objs_t a;
				objs.push_back(_obj_for_init(a));
			}
			else
			{
				objs.push_back(_obj_for_init(i));
			}
		}
	}

	_obj_for_init& operator=(const _obj_for_init& x)
	{
		assign(x);
		return *this;
	}

	void assign(const _obj_for_init& x)
	{
		_is_objs = x._is_objs;
		if (_is_objs) _data.objs = x._data.objs;
		else _data.element = x._data.element;
	}

private:

	using _objs_t = std::vector<_obj_for_init>;

	_obj_for_init(int idx) :_is_objs(false)
	{
		_data.element = idx;
	}
	_obj_for_init(const _objs_t& objs) :_is_objs(true)
	{
		_data.objs = objs;
	}

	bool _is_objs;
	struct _data_t
	{
		_data_t() {}
		~_data_t() {}
		int element = 0;
		_objs_t objs;
	}_data;
	
};

int main()
{
	_initializer<_base_value_ref> xx={
		{"pi", 3.141},
		{"happy", true},
		{"name", "Niels"},
		{"nothing", nullptr},
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
		{
			"list", {1, 0, 2}
		},
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
	
	//std::vector<int> a = { 1,2,3,4,5,6,7,8,9 }, b;
	//b.resize(5);
	//std::move(a.end()-5, a.end(), b.begin());
	//a.erase(a.end() - 5, a.end());
	//a.insert(a.begin(), b.begin(), b.end());

	//for (auto it : a)printf("%d ", it);

	for (auto& it : xx._vals)
	{
		if (it.is_size_flag())
			std::cout << it.get_size();
		else if (it.is_empty_flag())std::cout << "$empty";
		else std::cout << it.get_data().str_val();
		puts("");
	}

	_obj_for_init<_base_value_ref> x(xx);

	

	_ref_union uu;

	return 0;
}