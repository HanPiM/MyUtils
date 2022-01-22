#pragma once

#include <stdarg.h>
#include <time.h>
#include <string>
#include <vector>
#include <unordered_map>

namespace string_tools
{
	int _cformat(std::string& str, const char* format, va_list args);
	int cformat(std::string& str, const char* format, ...);
	int _cformat(std::wstring& str, const wchar_t* format, va_list args);
	int cformat(std::wstring& str, const wchar_t* format, ...);

	std::string _cformat(const char* format, va_list args);
	std::string cformat(const char* format, ...);
	std::wstring _cformat(const wchar_t* format, va_list args);
	std::wstring cformat(const wchar_t* format, ...);

	std::string random(size_t len,
		const std::string& table = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"
	);

	void _get_tm(tm& out);

	enum
	{
		TM_SEC = 0,
		TM_MIN,
		TM_HOUR,
		TM_MDAY, // 月中的第几天
		TM_MON,
		TM_YEAR,
		TM_WDAY, // 周中的第几天
		TM_YDAY  // 年中的第几天
	};

	std::string get_time(
		const char* format = "%02d:%02d:%02d",
		const std::vector<int>& indexs = { TM_HOUR,TM_MIN,TM_SEC }
	);
};
