/// @file shared_storage.hh
/// @brief template for shared objects
/// @author René Müller
/// @version 0.0.1
/// @date 2019-12-30

#ifndef _SHARED_STORAGE_HH_
#define _SHARED_STORAGE_HH_

#include <functional>                   // for std::hash<>
#include <memory>                       // for unique_ptr for find
#ifdef USE_THREADS                      // be prepared
#   include <mutex>
#endif

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
/// - operator== (both compare with other shared_storage as well as
///                 objects of type T)
/// - operator!= (both compare with other shared_storage as well as
///                 objects of type T)
///
/// member functions:
/// - size_t ref_count(void)
///     returns the reference count for a given shared object
///     (reference count of "null object" is always 0)
/// - bool is_protected(void)
///     returns true for shared objects with reference count "overflow"
///
/// static member functions:
/// - size_t size(void)
///     returns amount of distinct shared objects in storage
/// - size_t unref_size(void)
///     returns amount of unreferenced shared objects in storage
///     (unreferenced shared objects for which an reference count
///     "overflow" has happened are not counted!)
/// - size_t prot_size(void)
///     returns amount of protected shared objects in storage
///     ("null object" doesn't count)
/// - status_t const &get_status(void|bool)
///     returns status info as member type
///     The signature depends on class T:
///     - size_t T.size(void) does not exist:
///         only basic information about the count of all, unreferenced
///         and protected values can be asked
///     - size_t T.size(void) does exist:
///         get_status(false) returns the basic info as above
///         get_status(true)  returns additional data about storage sizes
/// - size_t set_max_ref(size_t)
///     set's the reference count from which on a shared object is seen as
///     protected, returns the old setting
/// - size_t cleanup(void)
///     removes shared objects with reference count 0 from storage
///     returns amount of removed shared objects
///     (unreferenced shared objects for which an reference count
///     "overflow" has happened are not removed!)
/// - std::unique_ptr<shared_storage<T>> find(T) const
///     returns unique pointer
///         - to shared object representing argument if found
///         - nullptr otherwise
///
/// @tparam T       type of shared objects
///                 needs to be:
///                 - default constructible
///                 - copy constructible
///                 - comparable for equal
///
///                 if T has a public member function size get_status
///                 might be extended by defining HAS_SIZE prior
///                 inclusion of this header!
/// @tparam hash    hash for given type (defaults to: "std::hash<T>")
//
/// @attention
/// depending on compile time settings (USE_THREADS (un)defined)
/// shared_storage is multi thread aware!
// --------------------------------------------------------------------------
template <typename T, class hash = std::hash<T>>
class shared_storage {
    private:
        // check requirements for given type T
        BOOST_STATIC_ASSERT_MSG(boost::is_default_constructible<T>::value, "shared_storage<T>: T needs default constructor");
        BOOST_STATIC_ASSERT_MSG(boost::is_copy_constructible<T>::value, "shared_storage<T>: T needs copy constructor");
        BOOST_STATIC_ASSERT_MSG(boost::has_equal_to<T>::value, "shared_storage<T>: T needs operator==");

        template<typename TT>
        struct has_size
        {
            /* SFINAE size-has-correct-sig :) */
            template<typename A>
            static std::true_type test(size_t (A::*)() const) {
                return std::true_type();
            }

            /* SFINAE size-exists :) */
            template <typename A>
            static decltype(test(&A::size))
            test(decltype(&A::size),void *) {
            /* What about sig? */
                typedef decltype(test(&A::size)) return_type;
                return return_type();
            }

            /* SFINAE game over :( */
            template<typename A>
            static std::false_type test(...) {
                return std::false_type();
            }

            /* This will be either `std::true_type` or `std::false_type` */
            typedef decltype(test<TT>(0,0)) type;

            static const bool value = type::value; /* Which is it? */
        };

        /// @brief structure holding only basic status informations
        template<typename A, bool = (has_size<A>::value)>
        struct int_status_t {
            size_t size;                ///< @brief container size (stored string count)
            size_t unref;               ///< @brief unreferenced string count
            size_t max_ref;             ///< @brief threshold at which point a string becomes protected
            size_t prot;                ///< @brief proteted string count
        };
        /// @brief structure holding additional status informations
        template<typename A>
        struct int_status_t<A, true> {
            size_t size;                ///< @brief container size (stored string count)
            size_t unref;               ///< @brief unreferenced string count
            size_t max_ref;             ///< @brief threshold at which point a string becomes protected
            size_t prot;                ///< @brief proteted string count

            size_t data_size;           ///< @brief size of stored data (only updated when asking for verbose status)
            size_t unref_size;          ///< @brief size of stored but unregerenced data (only updated when asking for verbose status)
            size_t prot_size;           ///< @brief size of protected data (only updated when asking for verbose status)
        };

        using b_status_t    = int_status_t<T, false>;
        using v_status_t    = int_status_t<T, true>;

    public:
        using status_t = int_status_t<T>;

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
        using storage_t     = boost::container::multimap<size_t, shared_storage_vt, std::less<size_t>, boost::container::new_allocator<value_t>, avl_tree_t>;

        // value represented by *this
        shared_storage_vt    * val;

        // storage object
        static storage_t &storage(void);

        /// @brief status info, needs to be initialized as '{0,0,MAX_REF,0}'
        static status_t status;

#ifdef USE_THREADS
        // we want to he multi thread save => we need locking
        static std::mutex & mtx(void);
#endif

        // an empty object
        static const T & empty(void);

        // internally used only constructor
        // NEVER USES LOCKING!!!!
        shared_storage(shared_storage_vt * const);

    public:

        /// @brief default constructor
        ///
        /// represents empty object
        shared_storage(void);
        shared_storage(const T &arg);
        shared_storage(const shared_storage &);
        shared_storage(      shared_storage &&);
       ~shared_storage(void);

                shared_storage  &   operator=       (const shared_storage &);
                shared_storage  &   operator=       (      shared_storage &&);

                T const         &   operator*       (void) const;
                T const         *   operator->      (void) const;

                bool                operator==      (const shared_storage &) const;
                bool                operator==      (const T &) const;
                bool                operator!=      (const shared_storage &) const;
                bool                operator!=      (const T &) const;

                                    operator T      (void) const;

                size_t              ref_count       (void) const;
                bool                is_protected    (void) const;

        static  size_t              size            (void);
        static  size_t              unref_size      (void);
        static  size_t              prot_size       (void);
        template<class Q=T>
        static  auto                get_status      (bool verbose = false) -> typename std::enable_if<has_size<Q>::value, v_status_t const &>::type;
        template<class Q=T>
        static  auto                get_status      (void) -> typename std::enable_if<!has_size<Q>::value, b_status_t const &>::type;
        static  size_t              set_max_ref     (size_t);

        static  size_t              cleanup         (void);

        static  std::unique_ptr<shared_storage>
                                    find            (T);
};

#include <templates/impl/shared_storage.cc>

#endif  // _SHARED_STORAGE_HH_
