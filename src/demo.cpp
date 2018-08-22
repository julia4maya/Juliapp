#include "Julia.hpp"

int main()
{

    jl::init();

    jl::exec(R"(
        module Mod
          const a = 6.
          f(x) = a * x^3
        end    
    )");

    long res;
    try
    {
        res = jl::call("Mod.f", 2.);
        std::printf("%ld\n", res);
    }
    catch (jl::value_error&)
    {
        std::puts("Value error caught.");
    }

    try
    {
        jl::exec("nonexistent_function()");
    }
    catch (jl::language_error&)
    {
        std::puts("Language error caught.");
    }

    try
    {
        jl::call("nonexistent_function");
    }
    catch (jl::language_error&)
    {
        std::puts("Language error caught.");
    }

    jl::array arr{5, 8, 1};
    jl::call("reverse!", arr);
    jl::call("println", arr);

    // jl::raise_error("Test");

    jl::exec_from_file("sample-script.jl");

    std::printf("%d\n", jl::call("÷", 5, 2).get<int>());

    jl::quit();
}
