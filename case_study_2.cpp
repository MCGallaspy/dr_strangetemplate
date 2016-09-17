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
    
    template <typename Evt, typename List, bool HasTail, typename = void>
    struct post_impl;
    
    // Case 1: no tail, has a handler
    template <typename Evt, typename List>
    struct post_impl<Evt, List, true, 
                     typename std::enable_if<
                        has_handler<typename List::head, Evt>::value
                     >::type>
    {
        static void call(const Evt& evt) {
            cout << "I've got a tail";
            List::head::handle(evt);
            using Tail = typename List::tail;
            post_impl<Evt, Tail, has_tail_2<Tail>::value>::call(evt);
        }
    };
    
    // Case 2: has no tail, has a handler
    template <typename Evt, typename List>
    struct post_impl<Evt, List, false,
                     typename std::enable_if<
                        has_handler<typename List::head, Evt>::value
                     >::type>
    {
        static void call(const Evt& evt) {
            cout << "I don't";
            List::head::handle(evt);
        }
    };
        
    // Case 3: has a tail, has no handler
    
public:
    template <typename Evt>
    static void post(const Evt& t) {
        post_impl<Evt, Listeners, has_tail_2<Listeners>::value>::call(t);
    }
};

int main() {    
    using listeners = type_list<CoutShouter>;
    using listeners_2 = type_list<CoutShouter, CoutShouter>;

    cout << has_tail_1<listeners>::value << endl;
    cout << has_tail_2<listeners_2>() << endl;
    
    cout << count<listeners>() << endl;
    cout << count<listeners_2>() << endl;
    
    cout << "listeners 1: ";
    Dispatcher<listeners>::post(JustBeforeReturnEvent{});
    
    cout << "listeners_2: ";
    Dispatcher<listeners_2>::post(JustBeforeReturnEvent{});
    return 0;
}