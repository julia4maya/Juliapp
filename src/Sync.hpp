#pragma once

#include "GenericString.hpp"

#include <julia.h>
#include <map>
#include <string>
#include <typeindex>

namespace jl
{

namespace impl
{

std::type_index* synced_cpp_types;
jl_datatype_t** synced_jl_types;
std::size_t num_synced_types;

template<typename CppT>
jl_datatype_t* find_synced_jl_type()
{
    std::type_index cpp_type{typeid(CppT)};
    for (std::size_t i = 0; i < num_synced_types; ++i)
    {
        if (synced_cpp_types[i] == cpp_type)
            return synced_jl_types[i];
    }

    return nullptr;
}

template<typename CppT>
bool verify_synced_cpp_type(jl_datatype_t* julia_type_)
{
    for (std::size_t i = 0; i < num_synced_types; ++i)
    {
        if (synced_jl_types[i] == julia_type_)
            return synced_cpp_types[i] == typeid(CppT);
    }

    return false;
}

} // namespace impl

template<typename CppT>
struct type
{
    using cpp_type = CppT;
    type(generic_string type_name_) : julia_type{type_name_} {}

    const char* julia_type;
};

template<typename... TypeTs>
void sync(TypeTs&&... ts_)
{
    assert(impl::num_synced_types == 0 && "You may sync types only once");
    impl::num_synced_types = sizeof...(ts_);
    impl::synced_cpp_types = new std::type_index[sizeof...(ts_)]{
        typeid(typename TypeTs::cpp_type)...};
    // TODO: improve performance by exchanging `jl_eval_string`
    impl::synced_jl_types = new jl_datatype_t* [sizeof...(ts_)] {
        reinterpret_cast<jl_datatype_t*>(jl_eval_string(ts_.julia_type))...
    };

    jl_function_t* sizeof_func{jl_eval_string("sizeof")};
    JL_GC_PUSH1(sizeof_func);

    assert("Synced datatype sizes do not match"
           && ((sizeof(typename TypeTs::cpp_type)
                == static_cast<std::size_t>(jl_unbox_int64(
                       jl_call1(sizeof_func, jl_eval_string(ts_.julia_type)))))
               && ...));
    JL_GC_POP();
}

} // namespace jl
