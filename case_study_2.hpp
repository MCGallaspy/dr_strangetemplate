#include <type_traits>


/* Type list */
template <typename... Args>
struct type_list;


template <typename Type>
struct type_list<Type> {
    using head = Type;
};


template <typename Head, typename... Tail>
struct type_list<Head, Tail...> : type_list<Head> {
    using tail = type_list<Tail...>;
};


/* OMG this awesome void_t metafunction will change your life */
template <typename...>
using void_t = void;


/* Type list metafunctions */

/* count */
template <typename T, typename = void>
struct count : std::integral_constant<int, 1> {};

template <typename T>
struct count<T, void_t<typename T::tail>> :
    std::integral_constant<int, 1 + count<typename T::tail>()> {};

/* Alternate implementation uses fewer template instantiations */
template <typename... Elts>
struct different_count;

template <typename... Elts>
struct different_count<type_list<Elts...>> : std::integral_constant<int, sizeof...(Elts)> {};
    
/* has_tail predicate */
template <typename T>
struct has_tail :    /*         predicate          */  /*  if true   */ /*  if false */
    std::conditional<(different_count<T>::value <= 1), std::false_type, std::true_type>::type {};


/* has_handler predicate */
template <typename Handler, typename Evt, typename = void>
struct has_handler : std::false_type {};

template <typename Handler, typename Evt>
struct has_handler<Handler, Evt, decltype( Handler::handle( std::declval<const Evt&>() ) )> : 
    std::true_type {};

    
/* Dispatcher */
template <typename Listeners>
class Dispatcher {
    template <typename Evt, typename List, bool HasTail, bool HasHandler>
    struct post_impl;

    // Case 1: Has tail, has a handler
    template <typename Evt, typename List>
    struct post_impl<Evt, List, true, true>
    {
        static void call(const Evt& evt) {
            List::head::handle(evt);
            
            using Tail = typename List::tail;
            
            constexpr bool has_tail_v = has_tail<Tail>::value;
            constexpr bool has_handler_v = has_handler<typename Tail::head, Evt>::value;
            post_impl<Evt, Tail, has_tail_v, has_handler_v>::call(evt);
        }
    };
    
    // Case 2: Has no tail, has a handler
    template <typename Evt, typename List>
    struct post_impl<Evt, List, false, true>
    {
        static void call(const Evt& evt) {
            List::head::handle(evt);
        }
    };

    // Case 3: Has tail, has no handler
    template <typename Evt, typename List>
    struct post_impl<Evt, List, true, false>
    {
        static void call(const Evt& evt) {
            using Tail = typename List::tail;
            
            constexpr bool has_tail_v = has_tail<Tail>::value;
            constexpr bool has_handler_v = has_handler<typename Tail::head, Evt>::value;
            post_impl<Evt, Tail, has_tail_v, has_handler_v>::call(evt);
        }
    };
    
    // Case 4: Has no tail, has no handler
    template <typename Evt, typename List>
    struct post_impl<Evt, List, false, false>
    {
        static void call(const Evt& evt) {
            // do nothing.
        }
    };
        
public:
    template <typename Evt>
    static void post(const Evt& t) {
        constexpr bool has_tail_v = has_tail<Listeners>::value;
        constexpr bool has_handler_v = has_handler<typename Listeners::head, Evt>::value;
        post_impl<Evt, Listeners, has_tail_v, has_handler_v>::call(t);
    }
};
