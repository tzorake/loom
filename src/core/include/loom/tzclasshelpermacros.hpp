#ifndef TZCLASSHELPERMACROS_HPP
#define TZCLASSHELPERMACROS_HPP

template<typename T>
inline T *tzGetPtrHelper(T *ptr) noexcept
{
    return ptr;
}
template<typename Ptr>
inline auto tzGetPtrHelper(Ptr &ptr) noexcept -> decltype(ptr.get())
{
    static_assert(noexcept(ptr.get()),
                  "Smart d pointers for TZ_DECLARE_PRIVATE must have noexcept get()");
    return ptr.get();
}

#define TZ_DISABLE_COPY(Class) \
    Class(const Class &) = delete; \
    Class &operator=(const Class &) = delete;

#define TZ_DISABLE_COPY_MOVE(Class) \
    TZ_DISABLE_COPY(Class) \
    Class(Class &&) = delete; \
    Class &operator=(Class &&) = delete;

#define TZ_DISABLE_COPY_X(Class, reason) \
    Class(const Class &) TZ_DECL_ETZ_DELETE_X(reason); \
    Class &operator=(const Class &) TZ_DECL_ETZ_DELETE_X(reason);

#define TZ_DISABLE_COPY_MOVE_X(Class, reason) \
    TZ_DISABLE_COPY_X(Class, reason) \
    Class(Class &&) TZ_DECL_ETZ_DELETE_X(reason); \
    Class &operator=(Class &&) TZ_DECL_ETZ_DELETE_X(reason);

#define TZ_DECLARE_PRIVATE(Class) \
    inline Class##Private *d_func() noexcept \
    { \
        return reinterpret_cast<Class##Private *>(tzGetPtrHelper(d_ptr)); \
    } \
    inline const Class##Private *d_func() const noexcept \
    { \
        return reinterpret_cast<const Class##Private *>(tzGetPtrHelper(d_ptr)); \
    } \
    friend class Class##Private;

#define TZ_DECLARE_PRIVATE_D(Dptr, Class) \
    inline Class##Private *d_func() noexcept \
    { \
        return reinterpret_cast<Class##Private *>(tzGetPtrHelper(Dptr)); \
    } \
    inline const Class##Private *d_func() const noexcept \
    { \
        return reinterpret_cast<const Class##Private *>(tzGetPtrHelper(Dptr)); \
    } \
    friend class Class##Private;

#define TZ_DECLARE_PUBLIC(Class) \
    inline Class *q_func() noexcept \
    { \
        return static_cast<Class *>(q_ptr); \
    } \
    inline const Class *q_func() const noexcept \
    { \
        return static_cast<const Class *>(q_ptr); \
    } \
    friend class Class;

#define TZ_D(Class) Class##Private *const d = d_func()
#define TZ_Q(Class) Class *const q = q_func()

#endif // TZCLASSHELPERMACROS_HPP
