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

//// 用于初始化的中间对象
//class _obj_for_init
//{
//public:
//
//	using _objs_t = std::vector<_obj_for_init>;
//
//	// {} 会调用该析构函数
//	_obj_for_init() :_is_obj(true) {}
//	_obj_for_init(const _obj_for_init& x) { assign(x); }
//
//	template<typename ..._ts>
//	_obj_for_init(const _ts& ...args)
//	{
//		_objs = { _value_type(args)... };
//		if (_objs.empty())
//		{
//			_is_obj = true;
//		}
//		else if (_objs.size() > 1)
//		{
//			_is_obj = true;
//		}
//		else
//		{
//			_element = _objs[0]._element;
//			_objs.clear();
//		}
//	}
//
//	_obj_for_init(std::initializer_list<_obj_for_init> x)
//		:_is_obj(true)
//	{
//		for (auto& it : x)
//		{
//			if (it.is_obj())
//			{
//				_objs.push_back(it._objs);
//			}
//			else _objs.push_back(it._element);
//		}
//	}
//
//	constexpr bool is_obj()const { return _is_obj; }
//
//	_obj_for_init& operator=(const _obj_for_init& x)
//	{
//		assign(x);
//		return *this;
//	}
//
//	void assign(const _obj_for_init& x)
//	{
//		if (x.is_obj()) _objs = x._objs;
//		else _element = x._element;
//		_is_obj = x._is_obj;
//	}
//
//	//private:
//
//	bool _is_arr = false;
//	bool _is_obj = false;
//
//	_value_type _element;
//	_objs_t _objs;
//
//private:
//	_obj_for_init(const _value_type& x) :_element(x) {}
//	_obj_for_init(const _objs_t& x) :
//		_is_obj(true), _objs(x) {}
//};
