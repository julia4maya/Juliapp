#pragma once

#include "Boxing.hpp"

#include <array>
#include <fstream>
#include <iostream>
#include <julia/julia.h>
#include <sstream>
#include <string>

namespace jl
{

namespace impl
{

void check_err()
{
    if (jl_exception_occurred())
        throw language_error{jl_typeof_str(jl_exception_occurred())};
}

template<typename ElemT>
jl_datatype_t* get_type()
{
    if constexpr (std::is_same_v<ElemT, std::int8_t>)
        return jl_int8_type;
    else if constexpr (std::is_same_v<ElemT, std::uint8_t>)
        return jl_uint8_type;
    else if constexpr (std::is_same_v<ElemT, std::int16_t>)
        return jl_int16_type;
    else if constexpr (std::is_same_v<ElemT, std::uint16_t>)
        return jl_uint16_type;
    else if constexpr (std::is_same_v<ElemT, std::int32_t>)
        return jl_int32_type;
    else if constexpr (std::is_same_v<ElemT, std::uint32_t>)
        return jl_uint32_type;
    else if constexpr (std::is_same_v<ElemT, std::int64_t>)
        return jl_int64_type;
    else if constexpr (std::is_same_v<ElemT, std::uint64_t>)
        return jl_uint64_type;
    else if constexpr (std::is_same_v<ElemT, float>)
        return jl_float32_type;
    else if constexpr (std::is_same_v<ElemT, double>)
        return jl_float64_type;
    else if constexpr (std::is_same_v<ElemT, bool>)
        return jl_bool_type;
    else
    {
        assert(false &&
               "jl - unsupported array type. "
               "Use boolean, floating point or integral types.");
    }
}

} // namespace impl

class array
{
public:
    template<typename ElemT>
    array(std::initializer_list<ElemT> elems_)
        : array{impl::get_type<ElemT>(), elems_.size()}
    {
        ElemT* arr_data = reinterpret_cast<ElemT*>(jl_array_data(_arr));
        std::copy(elems_.begin(), elems_.end(), arr_data);
    }

    array(jl_datatype_t* type_, std::size_t size_) : _size{size_}
    {
        jl_value_t* array_type{jl_apply_array_type(
            reinterpret_cast<jl_value_t*>(type_), dimensions)};

        _arr = jl_alloc_array_1d(array_type, size_);
    }

    std::size_t size() { return _size; }

private:
    static constexpr std::size_t dimensions{1};
    jl_array_t* _arr;
    const std::size_t _size;
};

class value
{
public:
    value(jl_value_t* boxed_value_) : _boxed_value{boxed_value_} {}

    value() = default;
    value(const value&) = default;
    value(value&&) = default;

    template<typename TargT>
    TargT get()
    {
        if constexpr (std::is_integral_v<TargT>)
            return static_cast<TargT>(impl::unbox<long>(_boxed_value));
        else if constexpr (std::is_floating_point_v<TargT>)
            return static_cast<TargT>(impl::unbox<double>(_boxed_value));
        else
            return impl::unbox<TargT>(_boxed_value);
    }

    template<typename TargT>
    operator TargT()
    {
        return impl::unbox<TargT>(_boxed_value);
    }

private:
    jl_value_t* _boxed_value;
};

value exec(const char* src_str_)
{
    jl_value_t* res{jl_eval_string(src_str_)};
    impl::check_err();
    return res;
}

value exec_from_file(const char* file_name_)
{
    std::ifstream file{file_name_};
    std::stringstream buffer;
    buffer << file.rdbuf();
    return exec(buffer.str().c_str());
}

template<typename... ArgTs>
value call(const char* fn_name_, ArgTs... args_)
{
    std::array<jl_value_t*, sizeof...(args_)> boxed_args{impl::box(args_)...};

    jl_value_t* func{jl_eval_string(fn_name_)};
    impl::check_err();
    jl_value_t* res{jl_call(func, boxed_args.data(), boxed_args.size())};
    impl::check_err();
    return res;
}

void init()
{
    jl_init();
}

void quit(int code_ = 0)
{
    jl_atexit_hook(code_);
}

void raise_error(const char* content_)
{
    jl_error(content_);
}

template<typename... ArgTs>
void raise_error(const char* content_, ArgTs... args_)
{
    jl_errorf(content_, args_...);
}

} // namespace jl
