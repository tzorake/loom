#ifndef TZOBJCUTILS_HPP
#define TZOBJCUTILS_HPP

#include <objc/objc.h>
#include <objc/runtime.h>
#include <objc/message.h>

using ObjcObject = objc_object*;
using ObjcClass  = objc_class*;
using ObjcMethodImpl = void (*)(void);

template<typename R = void, typename... Args>
inline R sendMessage(ObjcObject obj, const char* sel, Args... args)
{
    return reinterpret_cast<R(*)(ObjcObject, objc_selector*, Args...)>(objc_msgSend)
        (obj, sel_registerName(sel), args...);
}

template<typename R = void, typename... Args>
inline R sendClassMessage(ObjcClass cls, const char* sel, Args... args)
{
    return reinterpret_cast<R(*)(ObjcClass, objc_selector*, Args...)>(objc_msgSend)
        (cls, sel_registerName(sel), args...);
}

inline ObjcClass getClass(const char* name)
{
    return objc_getClass(name);
}

#endif // TZOBJCUTILS_HPP
