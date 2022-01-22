#include "logger.h"

void logger::_open_file(const std::string& filename)
{
#ifdef _WIN32
    fopen_s(&_fp, filename.c_str(), "w");
#else
    _fp = fopen(filename.c_str(), "w");
#endif
}

logger::logger(out_type ot, size_t maxlen, const std::string& filename)
{
    _fp = NULL;
    _maxlen = maxlen;
    _filename = filename;
    switch (ot)
    {
    case out_type::file:
        _open_file(filename);
        break;
    case out_type::cerr:
    case out_type::cout:
    case out_type::buffer:
        break;
    }
    _ot = ot;
    _buffer.reserve(maxlen);
}
logger::logger(func_t func, void* exdata)
{
    _func = func;
    _exdata = exdata;
    _ot = out_type::func;
}
logger::~logger()
{
    write();
    if (_ot == out_type::file)fclose(_fp);
}

void logger::write()
{
    if (_ot == out_type::buffer)return;
    if (_buffer.empty())return;
    if (_ot == out_type::func)
    {
        _func(_exdata, _buffer);
        _buffer.clear();
        return;
    }
    FILE* p = _fp;
    switch (_ot)
    {
    case out_type::cerr:
        p = stderr;
        break;
    case out_type::cout:
        p = stdout;
        break;
    }
    if (p != NULL)fprintf(p, "%s", _buffer.c_str());
    _buffer.clear();
}

void logger::_add(const std::string& str)
{
    _buffer += str;
    if (_ot == out_type::buffer)return;
    if (_buffer.size() >= _maxlen)write();
}

void logger::_out(bool need_write, const char* format, va_list args)
{
    std::string str;
    string_tools::_cformat(str, format, args);
    _add(str);
    if (need_write)write();
}
void logger::out(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    _out(_needwrite, format, args);
    va_end(args);
}
void logger::outline(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    _out(false, format, args);
    va_end(args);
    _add("\n");
    if (_needwrite)write();
}

void logger::save_file()
{
    if (_ot != out_type::file)return;
    write();
    fclose(_fp);
    _open_file(_filename);
}