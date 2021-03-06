#pragma once

#include "Errors.hpp"
#include "Helpers.hpp"

#include <julia.h>

namespace jl
{

template<typename ElemT>
class array;

namespace impl
{

template<typename RetT>
RetT unbox(jl_value_t* arg_)
{
    if constexpr (std::is_floating_point_v<RetT>)
    {
        if (jl_typeis(arg_, jl_float32_type))
            return jl_unbox_float32(arg_);
        else if (jl_typeis(arg_, jl_float64_type))
            return jl_unbox_float64(arg_);

        throw result_type_error{
            "Incorrect result type. Floating-point type requested"};
    }
    else if constexpr (std::is_integral_v<RetT>)
    {
        if constexpr (std::is_signed_v<RetT>)
        {
            if (jl_typeis(arg_, jl_int8_type))
                return jl_unbox_int8(arg_);
            else if (jl_typeis(arg_, jl_int16_type))
                return jl_unbox_int16(arg_);
            else if (jl_typeis(arg_, jl_int32_type))
                return jl_unbox_int32(arg_);
            else if (jl_typeis(arg_, jl_int64_type))
                return jl_unbox_int64(arg_);
        }
        else
        {
            if (jl_typeis(arg_, jl_uint8_type))
                return jl_unbox_uint8(arg_);
            else if (jl_typeis(arg_, jl_uint16_type))
                return jl_unbox_uint16(arg_);
            else if (jl_typeis(arg_, jl_uint32_type))
                return jl_unbox_uint32(arg_);
            else if (jl_typeis(arg_, jl_uint64_type))
                return jl_unbox_uint64(arg_);
        }

        throw result_type_error{
            "Incorrect result type. Integral type requested"};
    }
    else if constexpr (std::is_same_v<RetT, bool>)
    {
        if (jl_typeis(arg_, jl_bool_type))
            return jl_unbox_bool(arg_);

        throw result_type_error{
            "Incorrect result type. Boolean type requested"};
    }
    else
    {
        //        jl_datatype_t* found{impl::find_synced_jl_type<RetT>()};
        //        assert(found && "Requested type not synced");
        return *reinterpret_cast<std::decay_t<RetT>*>(jl_data_ptr(arg_));
    }
}

template<typename ArgT>
jl_value_t* box(ArgT& arg_)
{
    if constexpr (std::is_convertible_v<ArgT, jl_value_t*>)
        return arg_;
    else if constexpr (std::is_same<ArgT, bool>())
        return jl_box_bool(arg_);
    else if constexpr (std::is_same<ArgT, std::int8_t>())
        return jl_box_int8(arg_);
    else if constexpr (std::is_same<ArgT, std::uint8_t>())
        return jl_box_uint8(arg_);
    else if constexpr (std::is_same<ArgT, std::int16_t>())
        return jl_box_int16(arg_);
    else if constexpr (std::is_same<ArgT, std::uint16_t>())
        return jl_box_uint16(arg_);
    else if constexpr (std::is_same<ArgT, std::int32_t>())
        return jl_box_int32(arg_);
    else if constexpr (std::is_same<ArgT, std::uint32_t>())
        return jl_box_uint32(arg_);
    else if constexpr (std::is_same<ArgT, std::int64_t>())
        return jl_box_int64(arg_);
    else if constexpr (std::is_same<ArgT, std::uint64_t>())
        return jl_box_uint64(arg_);
    else if constexpr (std::is_same<ArgT, float>())
        return jl_box_float32(arg_);
    else if constexpr (std::is_same<ArgT, double>())
        return jl_box_float64(arg_);
    else if constexpr (std::is_same<ArgT, void*>())
        return jl_box_voidpointer(arg_);
    else if constexpr (is_array<std::decay_t<ArgT>>{})
        return reinterpret_cast<jl_value_t*>(arg_.get_boxed_data());
    else
    {
        jl_datatype_t* found{impl::find_synced_jl_type<ArgT>()};
        assert(found && "Requested type not synced");
        jl_value_t* val{jl_new_struct_uninit(found)};
        *reinterpret_cast<ArgT*>(jl_data_ptr(val)) = arg_;
        return val;
    }
}

template<typename...>
struct make_arg_vec;

template<typename DatT, typename FirstArgT, typename... RestArgTs>
struct make_arg_vec<DatT, FirstArgT, RestArgTs...>
{
    static void make(DatT* vector_,
                     std::size_t index_,
                     FirstArgT first_,
                     RestArgTs... rest_)
    {
        if constexpr (std::is_same_v<DatT, jl_value_t*>)
        {
            jl_value_t* boxed_val{box(first_)};
            vector_[index_] = boxed_val;
        }
        else
            vector_[index_] = first_;

        make_arg_vec<DatT, RestArgTs...>::make(vector_, index_ + 1, rest_...);
    }
};

template<typename DatT>
struct make_arg_vec<DatT>
{

    static void make(DatT*, std::size_t) {}
};

} // namespace impl

} // namespace jl
