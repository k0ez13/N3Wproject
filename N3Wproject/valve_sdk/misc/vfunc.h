#pragma once

template<typename FuncType>
__forceinline static FuncType CallVFunction(void* ppClass, int index)
{
    int* pVTable = *(int**)ppClass;
    int dwAddress = pVTable[index];
    return (FuncType)(dwAddress);
}

template<typename T>
__forceinline static T vfunc(void* base, int index)
{
	DWORD* vTabella = *(DWORD**)base;
	return (T)vTabella[index];
}

template<typename T, typename ...Args>
constexpr auto callVirtualMethod(void* classBase, int index, Args... args) noexcept
{
    return ((*reinterpret_cast<T(__thiscall***)(void*, Args...)>(classBase))[index])(classBase, args...);
}
