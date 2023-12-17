// dllmain.cpp : Defines the entry point for the DLL application.

#if 0
#include <windef.h>
#include <Mochi/meta.hpp>
#include <Mochi/components.hpp>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }

    // Mochi::TypeStack<int, float, double, bool, long, std::string>
    using TestType1 = Mochi::TypeStack<>::Append<int>::Append<float>::Append<double>::Concat<Mochi::TypeStack<bool, long>>::Append<std::string>;
    static_assert(Mochi::IsSameType<TestType1, Mochi::TypeStack<int, float, double, bool, long, std::string>>,
        "Type mismatch. This is a bug!");
    
    // Mochi::TypeStack<bool, long, std::string>
    using TestType2 = TestType1::SplitAt<3>::RightStack;
    static_assert(Mochi::IsSameType<TestType2, Mochi::TypeStack<bool, long, std::string>>, "Type mismatch. This is a bug!");

    // std::string
    using TestType3 = TestType2::SplitAt<2>::RightStack::FirstType;
    static_assert(Mochi::IsSameType<TestType3, std::string>, "Type mismatch. This is a bug!");

    // Mochi::TypeStack<>
    using TestType4 = TestType1::SplitAt<0>::LeftStack;
    using TestType5 = TestType1::SplitAt<TestType1::TypeCount>::RightStack;
    static_assert(Mochi::IsSameType<TestType4, TestType5, Mochi::TypeStack<>>, "Type mismatch. This is a bug!");

    return TRUE;
}
#endif