#include "simpjson.h"
#include <array>

#include <tuple>

#include <iostream>

using namespace sjson;




int main()
{

	std::allocator<char> void_alloc;

	//void_alloc.

	std::string s = "123ww";
	double d = 3.141;

	json str_test = "xx";

	str_test = s;
	std::cout << str_test.as_value().to_string()<< "\n";

	json arr_test = //json::array
	{
		{"pi", 3.141},
		{"happy", true},
		{"name", "Niels"},
		{"nothing", nullptr}
	};

	std::cout << int(arr_test.type()) << "________\n";
	std::cout << double(arr_test["pi"]);
	
	json obj_for_init_test =
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
	std::cout << obj_for_init_test[0][0][0].as_array().size() << '\n';

	json xx = 
	{
		1,2,3,4,d,
		s,
		arr_test,
		{
			{"pi", 3.141},
			{"happy", true},
			{"name", "Niels"},
			{"nothing", nullptr}
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
	//arr_test.push_back(114514);

	class A
	{
	public:
		A() {}
		class iter
		{
		public:
			iter(){}
			bool operator !=(const iter& x)const
			{
				return false;
			}
			iter operator++()
			{
				return *this;
			}
			int& operator*()
			{
				return *(new int());
			}
		};
		iter begin()
		{
			return iter();
		}
		iter end()
		{
			return iter();
		}
	};
	A iter_test;
	for (auto& it : iter_test)
	{

	}

	for (auto& it : xx.as_array())
	{
		if (it.is_array())printf("[...]:%d\n", it.as_array().size());
		else if (it.is_object())
		{
			printf("{obj}\n");
		}
		else if(it.is_value())printf(":'%s'\n",it.as_value().to_string().c_str());
		else
		{

		}
	}

	//printf("%s", xx._objs[7]._objs[0]._element.str_val().c_str());

	return 0;
}