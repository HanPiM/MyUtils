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

// 用于初始化的中间对象
template<typename _basic_type>
class _obj_for_init
{
public:

	using _objs_t = std::vector<_obj_for_init>;

	_obj_for_init() {}
	_obj_for_init(const _obj_for_init& x) { assign(x); }
	_obj_for_init(const _basic_type& x) :_element(x) {}
	_obj_for_init(const _objs_t& x) :
		_is_obj(true), _objs(x) {}

	template<typename ..._ts>
	_obj_for_init(const _ts& ...args)
	{
		_objs = { _basic_type(args)... };
		if (_objs.size() > 1)_is_obj = true;
		else
		{
			_element = _objs[0]._element;
			_objs.clear();
		}
	}

	_obj_for_init(std::initializer_list<_obj_for_init<_basic_type> > x)
		:_is_obj(true)
	{
		for (auto& it : x)
		{
			if (it.is_obj())
			{
				_objs.push_back(it._objs);
			}
			else _objs.push_back(it._element);
		}
	}

	constexpr bool is_obj()const { return _is_obj; }

	_obj_for_init& operator=(const _obj_for_init& x)
	{
		assign(x);
		return *this;
	}

	void assign(const _obj_for_init& x)
	{
		if (x.is_obj()) _objs = x._objs;
		else _element = x._element;
		_is_obj = x._is_obj;
	}

//private:

	bool _is_obj = false;

	_basic_type _element;
	_objs_t _objs;
};

//template<typename _basic_type>
//class _initializer
//{
//public:
//
//	template<typename ..._ts>
//	_initializer(const _ts& ...args)
//	{
//		_objs = { _obj_t(args)... };
//	}
//
//	_initializer(std::initializer_list<_initializer> x)
//	{
//		for (auto& it : x)
//		{
//			if (it._objs.size() == 1)
//			{
//				_objs.push_back(it._objs[0]._element);
//			}
//			else
//			_objs.push_back(it._objs);
//		}
//	}
//
////private:
//	using _obj_t = _obj_for_init<_basic_type>;
//
//	bool _is
//	std::vector<_obj_t> _objs;
//
//};

int main()
{
	_obj_for_init<_base_value_ref> xx = {
		{"sss"},
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

	for (auto& it : xx._objs)
	{
		if (it.is_obj())printf("{...}:%d\n", it._objs.size());
		else puts(it._element.str_val().c_str());
	}

	printf("%s", xx._objs[7]._objs[0]._element.str_val().c_str());

	return 0;
}