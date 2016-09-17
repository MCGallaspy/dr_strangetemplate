#include <iostream>
#include <type_traits>


using std::cout;
using std::endl;


class JustBeforeReturnEvent {
    // ...
};

class CoutShouter {
public:
    static void handle(const JustBeforeReturnEvent& evt) {
        cout << "Goodbye!\n";
    }
};

class QuietGuy {};

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


template <typename List>
class has_tail_1 {
    template <typename X>
    static std::true_type test(typename X::tail* x = 0);
    
    template <typename X>
    static std::false_type test(...);
    
public:    
    static constexpr bool value = decltype(test<List>(0))();
};


template <typename...>
using void_t = void;

template <typename, typename = void>
struct has_tail_2 : std::false_type {};

template <typename T>
struct has_tail_2<T, void_t<typename T::tail>> : std::true_type {};

template <typename T, typename = void>
struct count : std::integral_constant<int, 1> {};

template <typename T>
struct count<T, void_t<typename T::tail>> :
    std::integral_constant<int, 1 + count<typename T::tail>()>
{};

template <typename Listeners>
class Dispatcher {
    template <typename Handler, typename Evt, typename = void>
    struct has_handler : std::false_type {};
    
    template <typename Handler, typename Evt>
    struct has_handler<Handler, Evt, decltype( Handler::handle( std::declval<const Evt&>() ) )> : 
        std::true_type {};
    
    template <typename Evt, typename List, bool HasTail, bool HasHandler>
    struct post_impl;
    
    // Case 1: Has tail, has a handler
    template <typename Evt, typename List>
    struct post_impl<Evt, List, true, true>
    {
        static void call(const Evt& evt) {
            List::head::handle(evt);
            using Tail = typename List::tail;
            constexpr bool has_handler_v = has_handler<typename Tail::head, Evt>::value;
            post_impl<Evt, Tail, has_tail_2<Tail>::value, has_handler_v>::call(evt);
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
            constexpr bool has_handler_v = has_handler<typename Tail::head, Evt>::value;
            post_impl<Evt, Tail, has_tail_2<Tail>::value, has_handler_v>::call(evt);
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
        constexpr bool has_handler_v = has_handler<typename Listeners::head, Evt>::value;
        post_impl<Evt, Listeners, has_tail_2<Listeners>::value, has_handler_v>::call(t);
    }
};

int main() {    
    using listeners = type_list<CoutShouter, QuietGuy>;
    using listeners_2 = type_list<CoutShouter, CoutShouter>;
    using listeners_3 = type_list<QuietGuy, QuietGuy, QuietGuy, QuietGuy>;

    cout << has_tail_1<listeners>::value << endl;
    cout << has_tail_2<listeners_2>() << endl;
    
    cout << count<listeners>() << endl;
    cout << count<listeners_2>() << endl;
    
    cout << "listeners 1: ";
    Dispatcher<listeners>::post(JustBeforeReturnEvent{});
    
    cout << "listeners_2: ";
    Dispatcher<listeners_2>::post(JustBeforeReturnEvent{});
    
    cout << "listeners_3: ";
    Dispatcher<listeners_3>::post(JustBeforeReturnEvent{});
    return 0;
}