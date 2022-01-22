#pragma once

#include <functional>

#include "string_tools.h"

class logger
{
public:
    enum class out_type
    {
        file, cerr, cout,
        buffer, func
    };
    using func_t = std::function<void(void*, const std::string&)>;

    logger(out_type ot, size_t buffer_maxlen = 1024, const std::string& filename = "");
    logger(func_t func, void* exdata = NULL);
    ~logger();

    void out(const char* format, ...);
    void outline(const char* format, ...);
    void write();

    bool needwrite()const { return _needwrite; }
    void set_needwrite(bool flag) { _needwrite = flag; }

    std::string get_buffer()const { return _buffer; }
    void clear_buffer() { _buffer = ""; }

    void save_file();

private:

    void _open_file(const std::string& filename);

    void _add(const std::string& str);
    void _out(bool need_write, const char* format, va_list args);

    out_type _ot;
    union
    {
        struct
        {
            std::string _filename;
            FILE* _fp;
        };
        struct
        {
            func_t _func;
            void* _exdata;
        };
    };

    size_t _maxlen = 1024; // buffer 模式下无效
    std::string _buffer;

    bool _needwrite = false;
};