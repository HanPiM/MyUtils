#include <stdio.h>
#include <chrono>
#include <random>
#include <ctype.h>

#include "string_tools.h"

static std::mt19937 _mtengine((unsigned int)std::chrono::steady_clock::now().time_since_epoch().count());
static size_t _mt19937rand(size_t l, size_t r)
{
	return std::uniform_int_distribution<size_t>(l, r)(_mtengine);
}

namespace string_tools
{

int _cformat(std::string& out_str, const char* format, va_list in_args)
{
	va_list args;
	va_copy(args, in_args);
	size_t need = vsnprintf(NULL, 0, format, args);
	va_end(args);
	out_str.resize(need);
	return vsnprintf((char*)out_str.data(), need + 1, format, in_args);
}
int _cformat(std::wstring& out_str, const wchar_t* format, va_list in_args)
{
	va_list args;
	va_copy(args, in_args);
	size_t need = vswprintf(NULL, 0, format, args);
	va_end(args);
	out_str.resize(need);
	return vswprintf((wchar_t*)out_str.data(), need + 1, format, in_args);
}
std::string _cformat(const char* format, va_list args)
{
	std::string res;
	_cformat(res, format, args);
	return res;
}
std::wstring _cformat(const wchar_t* format, va_list args)
{
	std::wstring res;
	_cformat(res, format, args);
	return res;
}

int cformat(std::string& str, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	int r = _cformat(str, format, args);
	va_end(args);
	return r;
}
int cformat(std::wstring& str, const wchar_t* format, ...)
{
	va_list args;
	va_start(args, format);
	int r = _cformat(str, format, args);
	va_end(args);
	return r;
}
std::string cformat(const char* format, ...)
{
	va_list args;
	std::string res;
	va_start(args, format);
	_cformat(res, format, args);
	va_end(args);
	return res;
}
std::wstring cformat(const wchar_t* format, ...)
{
	va_list args;
	std::wstring res;
	va_start(args, format);
	_cformat(res, format, args);
	va_end(args);
	return res;
}

std::string random(size_t len, const std::string& table)
{
	std::string res;
	res.reserve(len);
	for (size_t i = 0; i < len; ++i)
		res += table[_mt19937rand(0, table.size() - 1)];
	return res;
}

static void _get_tm(tm& out)
{
	auto now = std::chrono::system_clock::now();
	time_t tt = std::chrono::system_clock::to_time_t(now);
#ifdef _WIN32
	localtime_s(&out, &tt);
#else
	localtime_r(&tt, &out);
#endif
}

std::string get_time(const char* format, const std::vector<int>& indexs)
{
	tm t;
	_get_tm(t);
	int data[] =
	{
		t.tm_sec,
		t.tm_min,
		t.tm_hour,
		t.tm_mday,
		t.tm_mon,
		t.tm_year,
		t.tm_wday,
		t.tm_yday
	};
	int idxs[8];
	memset(idxs, 0, sizeof(idxs));
	memcpy(
		idxs, indexs.data(),
		std::min(size_t(8), indexs.size()) * sizeof(int)
	);
	return cformat(format,
		data[idxs[0]], data[idxs[1]], data[idxs[2]], data[idxs[3]],
		data[idxs[4]], data[idxs[5]], data[idxs[6]], data[idxs[7]]
	);
}

}