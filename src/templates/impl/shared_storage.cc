/// @file shared_storage.cc
/// @brief - implementation for shared_storage<T, hash>
/// @attention internal header, don't include or otherwise compile by itself
/// @author René Müller
/// @version 0.0.1
/// @date 2019-12-30

#ifndef _SHARED_STORAGE_IMPL_
#define _SHARED_STORAGE_IMPL_

#ifdef USE_THREADS
#   define LOCK(arg)   \
        std::lock_guard<std::mutex> lock(arg)
#endif

// every shared storage object either holds
//  - a null pointer
// or
//  - the pointed to value has at least one of:
//      - reference count != 0
//      - overflow flag == true
#define INVARIANT   (val != nullptr ? val->ref_count || val->flag : true)

// storage object
template <typename T, class hash>
inline typename shared_storage<T, hash>::storage_t &shared_storage<T, hash>::storage(void)
{
    static typename shared_storage<T, hash>::storage_t s {};

    return s;
}

#ifdef USE_THREADS                      // be prepared
// we want to he multi thread save => we need locking
template <typename T, class hash>
inline std::mutex & shared_storage<T, hash>::mtx(void)
{
    static std::mutex m {};

    return m;
}
#endif

// a "null object"
template <typename T, class hash>
inline const T & shared_storage<T, hash>::empty(void)
{
    static T e {};

    return e;
}

template <typename T, class hash>
shared_storage<T, hash>::shared_storage(void) :
    val {nullptr}
{
    BOOST_ASSERT(INVARIANT);
}

template <typename T, class hash>
shared_storage<T, hash>::shared_storage(shared_storage_vt * const arg) :
    val {arg}
{
    if(arg != nullptr)
    {
        if(!arg->flag && (arg->ref_count == 0))         // was unreferenced
        {
            status.unref--;
        }
        arg->ref_count++;                               // increase reference count
        if(!arg->flag &&                                // we had an "overflow" => we can't clean this value anymore
                (arg->ref_count >= status.max_ref))
        {
            arg->flag = true;
            status.prot++;
        }
    }
    BOOST_ASSERT(INVARIANT);
}

template <typename T, class hash>
shared_storage<T, hash>::shared_storage(const T &arg) :
    val {nullptr}
{
    if(!(arg == empty()))                                       // otherwise we're finished
    {
        size_t  key { hash()(arg) };                            // hash for given object as key for storage
#ifdef USE_THREADS
        LOCK(mtx());                                            // we might use multi threading
#endif
        auto    t   = storage().equal_range(key);               // search for hash in storage

        if(t.first != t.second)                                 // => we have at least one value with given hash
        {
            for(auto i = t.first; i != t.second; ++i)
            {
                auto &s = i->second;
                if(s.val == arg)                                // => we have this value already in storage
                {
                    if(!s.flag && (s.ref_count == 0))           // was unreferenced
                    {
                        status.unref--;
                    }
                    s.ref_count++;                              // increase reference count
                    if(!s.flag &&                               // we had an "overflow" => we can't clean this value anymore
                            (s.ref_count >= status.max_ref))
                    {
                        s.flag = true;
                        status.prot++;
                    }
                    val = &(i->second);                         // and remember value
                    BOOST_ASSERT(INVARIANT);
                    return;
                }
            }
        }
        // either fall through (hash collision) or new hash
        auto i = storage().emplace(key, shared_storage_vt {arg});   // insert new value

        val = &(i->second);                                     // and remember
    }
    BOOST_ASSERT(INVARIANT);
}

template <typename T, class hash>
shared_storage<T, hash>::shared_storage(const shared_storage &arg) :
    val {arg.val}
{
    BOOST_ASSERT(INVARIANT);
    if(val != nullptr)
    {
#ifdef USE_THREADS
        LOCK(mtx());
#endif
        val->ref_count++;
        if(!val->flag &&                               // we had an "overflow" => we can't clean this value anymore
                (val->ref_count >= status.max_ref))
        {
            val->flag = true;
            status.prot++;
        }
    }
    BOOST_ASSERT(INVARIANT);
}

template <typename T, class hash>
shared_storage<T, hash>::shared_storage(shared_storage &&arg) :
    val {arg.val}
{
    arg.val = nullptr;
    BOOST_ASSERT(INVARIANT);
}

template <typename T, class hash>
shared_storage<T, hash>::~shared_storage(void)
{
    BOOST_ASSERT(INVARIANT);
    if(val != nullptr)
    {
#ifdef USE_THREADS
        LOCK(mtx());
#endif
        val->ref_count--;
        if((val->ref_count == 0) && !val->flag)             // now unreferenced
        {
            status.unref++;
        }
    }
}

template <typename T, class hash>
shared_storage<T, hash> & shared_storage<T, hash>::operator=(const shared_storage &arg)
{
    BOOST_ASSERT(INVARIANT);
    // nothing to do for self assignment (decrease old reference count,
    // reassign old value, re increase old reference count
    // this is  true for either this == &arg or just assignment of one
    // shared object to another where by chance both hold the same value
    if(val != arg.val)                                  // non self assignment
    {
#ifdef USE_THREADS
        LOCK(mtx());
#endif
        if(val != nullptr)                              // old value looses one reference
        {
            val->ref_count--;
            if((val->ref_count == 0) && !val->flag)     // now unreferenced
            {
                status.unref++;
            }
        }
        val = arg.val;
        if(val != nullptr)                              // new value gains one reference
        {
            val->ref_count++;
            if(!val->flag &&                            // we had an "overflow" => we can't clean this value anymore
                    (val->ref_count >= status.max_ref))
            {
                val->flag = true;
                status.prot++;
            }
        }
        BOOST_ASSERT(INVARIANT);
    }
    return *this;
}

template <typename T, class hash>
shared_storage<T, hash> & shared_storage<T, hash>::operator=(shared_storage<T, hash> &&arg)
{
    BOOST_ASSERT(INVARIANT);

    // self assignment only through references or forced…
    if(this != &arg)
    {
        if(val != nullptr)                              // old value looses one reference
        {
#ifdef USE_THREADS
            LOCK(mtx());
#endif
            val->ref_count--;
            if((val->ref_count == 0) && !val->flag)     // now unreferenced
            {
                status.unref++;
            }
        }
        val     = arg.val;
        arg.val = nullptr;
        BOOST_ASSERT(INVARIANT);
    }
    return *this;
}

template <typename T, class hash>
const T & shared_storage<T, hash>::operator*(void) const
{
    BOOST_ASSERT(INVARIANT);
    if(val != nullptr)
        return val->val;
    else
        return empty();
}

template <typename T, class hash>
const T * shared_storage<T, hash>::operator->(void) const
{
    if(val != nullptr)
        return &(val->val);
    else
        return &(empty());
}

template <typename T, class hash>
bool shared_storage<T, hash>::operator==(const shared_storage<T, hash> &arg) const
{
    return val == arg.val;
}

template <typename T, class hash>
bool shared_storage<T, hash>::operator==(const T &arg) const
{
    return val->val == arg;
}

template <typename T, class hash>
bool shared_storage<T, hash>::operator!=(const shared_storage<T, hash> &arg) const
{
    return val != arg.val;
}

template <typename T, class hash>
bool shared_storage<T, hash>::operator!=(const T &arg) const
{
    return val->val != arg;
}

template <typename T, class hash>
shared_storage<T, hash>::operator T(void) const
{
    return T {val->val};
}

template <typename T, class hash>
size_t shared_storage<T, hash>::ref_count(void) const
{
    BOOST_ASSERT(INVARIANT);
    if(val != nullptr)
        return val->ref_count;
    else
        return 0;
}

template <typename T, class hash>
bool shared_storage<T, hash>::is_protected(void) const
{
    BOOST_ASSERT(INVARIANT);
    if(val != nullptr)
        return val->flag;
    else
        return true;                    // implicit protected
}

template <typename T, class hash>
size_t shared_storage<T, hash>::size(void)
{
    return storage().size();            // we don't need to update status.size here since it only got read by get_status, which updates it itself
}

template <typename T, class hash>
size_t shared_storage<T, hash>::unref_size(void)
{
    return status.unref;
}

template <typename T, class hash>
size_t shared_storage<T, hash>::prot_size(void)
{
    return status.prot;
}

template <typename T, class hash> template<class Q>
auto shared_storage<T, hash>::get_status(bool verbose) -> typename std::enable_if<has_size<Q>::value, v_status_t const &>::type
{
#ifdef USE_THREADS
    LOCK(mtx());                                            // we might use multi threading
#endif

    status.size             = storage().size();
    status.num_buckets      = storage().bucket_count();
    status.max_num_buckets  = storage().max_bucket_count();
    status.max_bucket_size  = storage().load_factor();
    status.av_bucket_size   = storage().load_factor();

    if(verbose)
    {
        status.data_size    =
        status.unref_size   =
        status.prot_size    = 0;

        for(auto i = storage().begin(); i != storage().end();)
        {
            auto   &s  = i->second;
            size_t len = s.val.size();

            status.data_size += len;

            if(s.flag)
            {
                status.prot_size += len;
            }
            else if(!s.ref_count)
            {
                status.unref_size += len;
            }
        }
    }

    return status;
}

template <typename T, class hash> template<class Q>
auto shared_storage<T, hash>::get_status(void) -> typename std::enable_if<!has_size<Q>::value, b_status_t const &>::type
{
#ifdef USE_THREADS
    LOCK(mtx());                                            // we might use multi threading
#endif

    status.size = storage().size();

    return status;
}

template <typename T, class hash>
size_t shared_storage<T, hash>::set_max_ref(size_t arg)
{
#ifdef USE_THREADS
    LOCK(mtx());
#endif
    size_t t {status.max_ref};

    status.max_ref = arg;
    return t;
}

template <typename T, class hash>
size_t shared_storage<T, hash>::cleanup(void)
{
    size_t count {0};

    if(!storage().size())
    {
        return 0;
    }

#ifdef USE_THREADS
    LOCK(mtx());
#endif
    for(auto i = storage().begin(); i != storage().end();)
    {
        auto &s = i->second;

        if(!s.ref_count && !s.flag)
        {
            i = storage().erase(i);
            count++;
            status.unref--;
        }
        else
        {
            ++i;
        }
    }
    return count;
}

template <typename T, class hash>
std::unique_ptr<shared_storage<T, hash>> shared_storage<T, hash>::find(T arg)
{
    std::unique_ptr<shared_storage<T, hash>> ret {nullptr}; // return value, nullptr => not found
    size_t  key { hash()(arg) };                            // hash for given object as key for storage
#ifdef USE_THREADS
    LOCK(mtx());                                            // we might use multi threading
#endif
    auto    t   = storage().equal_range(key);               // search for hash in storage

    if(t.first != t.second)                                 // => we have at least one value with given hash
    {
        for(auto i = t.first; i != t.second; ++i)
        {
            auto &s = i->second;

            if(s.val == arg)                                // => found
            {
                ret.reset(new shared_storage<T,hash> {&s});
                break;
            }
        }
    }
    return ret;
}

#undef INVARIANT
#ifdef USE_THREADS
#   undef LOCK
#endif // _SHARED_STORAGE_IMPL_

#endif // _SHARED_STORAGE_IMPL_
