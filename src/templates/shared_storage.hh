/// @file shared_storage.hh
/// @brief template for shared objects
/// @author René Müller
/// @version 0.0.0
/// @date 2016-08-04

#ifndef _SHARED_STORAGE_HH_
#define _SHARED_STORAGE_HH_

#include <functional>                   // for std::hash<>
#include <memory>                       // for unique_ptr for find

#include <boost/type_traits.hpp>
#include <boost/container/map.hpp>

// --------------------------------------------------------------------------
/// @brief shared_storage
///
/// this is a pointer type class!
///
/// supplied constructors:
/// - default (for "null" value)
/// - typecast from type T
/// - copy
/// - move
///
/// supplied operators:
/// - operator=  (both: copy and move)
/// - operator*  (dereferencing to get stored object as
///                 <const T &>)
/// - operator-> (dereferencing, analogue to operator*)
/// - operator T (implicit type conversion to type T)
///
/// member functions:
/// - size_t ref_count(void)
///     returns the reference count for a given shared object
///     (reference count of "null object" is always 0)
/// - bool is_protected(void)
///     returns true for shared objects with reference count "overflow"
///
/// static members:
/// - size_t size(void)
///     returns amount of distinct shared objects in storage
/// - size_t cleanup(void)
///     removes shared objects with reference count 0 from storage
///     returns amount of removed shared objects
///     (unreferenced shared objects for which an reference count
///     "overflow" has happened are not removed!)
/// - size_t unref_size(void)
///     returns amount of unreferenced shared objects in storage
///     (unreferenced shared objects for which an reference count
///     "overflow" has happened are not counted!)
/// - size_t prot_size(void)
///     returns amount of protected shared objects in storage
///     ("null object" doesn't count)
/// - size_t set_max_ref(size_t)
///     set's the reference count from which on a shared object is seen as
///     protected, returns the old setting
/// - find(T)
///     returns shared object representing argument if found, nullptr
///     otherwise
///
/// @tparam T       type of shared objects
///                 needs to be:
///                 - default constructible
///                 - copy constructible
///                 - comparable for equal
/// @tparam hash    hash for given type (defaults to: "std::hash<T>")
//
/// @attention
/// depending on compile time settings shared_storage is multi thread aware!
// --------------------------------------------------------------------------
template <typename T, class hash = std::hash<T>>
class shared_storage {
    // check requirements for given type T
    BOOST_STATIC_ASSERT_MSG(boost::is_default_constructible<T>::value, "shared_storage<T>: T needs default constructor");
    BOOST_STATIC_ASSERT_MSG(boost::is_copy_constructible<T>::value, "shared_storage<T>: T needs copy constructor");
    BOOST_STATIC_ASSERT_MSG(boost::has_equal_to<T>::value, "shared_storage<T>: T needs operator==");

    private:
        //  shared object value type
        struct shared_storage_vt {
            uint_fast32_t   ref_count;
            bool            flag;               // overflow flag for ref_count
            T               val;

            shared_storage_vt(T arg) :
                ref_count   {1},
                flag        {false},
                val         {arg}
            {}

            ~shared_storage_vt(void) = default;
        };

        // the storage type for the shared objects
        // !!! ATTENTION !!!
        // pointers to stored values MUST NOT be invalidated by resizing the container !!!
        //
        // we use an avl-tree based multimap.
        // avl-tree based since most likely most of our accesses will be
        // retrievals and it doesn't have problems with possible multi threaded
        // environments
        using avl_tree_t    = boost::container::tree_assoc_options<boost::container::tree_type<boost::container::avl_tree>>::type;
        using value_t       = std::pair<const size_t, shared_storage_vt>;
        typedef boost::container::multimap<size_t, shared_storage_vt,
            std::less<size_t>, boost::container::new_allocator<value_t>, avl_tree_t> storage_t;

        // value represented by *this
        shared_storage_vt    * val;

        // storage object
        static storage_t &storage(void);

#ifdef USE_THREADS
        // we want to he multi thread save => we need locking
        static boost::mutex & mtx(void);
#endif

        // amount of unreferenced shared objects in storage
        static size_t & unref(void);

        // amount of unreferenced shared objects in storage
        static size_t & prot(void);

        // maximum references when the shared object becomes protected
        static size_t & max_ref(void);

        // an empty string
        static const T & empty(void);

        // internal used only constructor
        // DOESN'T USE LOCKING!!!!
        shared_storage(shared_storage_vt * const);

    public:
        shared_storage(void);
        shared_storage(const T &);
        shared_storage(const shared_storage &);
        shared_storage(      shared_storage &&);
       ~shared_storage(void);

                shared_storage  &   operator=       (const shared_storage &);
                shared_storage  &   operator=       (      shared_storage &&);

        const   T               &   operator*       (void) const;
        const   T               *   operator->      (void) const;

                bool                operator==      (const shared_storage &) const;
                bool                operator!=      (const shared_storage &) const;

                                    operator T      (void) const;

                size_t              ref_count       (void) const;
                bool                is_protected    (void) const;

        static  size_t              size            (void);
        static  size_t              cleanup         (void);
        static  size_t              unref_size      (void);
        static  size_t              prot_size       (void);
        static  size_t              set_max_ref     (size_t);
        static  std::unique_ptr<shared_storage>
                                    find            (T);
};

#include <templates/impl/shared_storage.cc>

#endif  // _SHARED_STORAGE_HH_
