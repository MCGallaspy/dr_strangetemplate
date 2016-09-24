#include <cstdlib>
#include <type_traits>
#include <utility>

#include <iostream>

using std::cout;
using std::endl;
using std::size_t;

template <typename Zero, typename One, typename Two>
struct three_tuple {
    using zero = Zero;
    using one = One;
    using two = Two;
};

template <size_t Index, typename Type>
struct element {
    using type = Type;
};

template <typename Indices, typename... Types>
struct tuple_impl;

template <std::size_t... Ns, typename... Types>
struct tuple_impl<std::index_sequence<Ns...>, Types...> : element<Ns, Types>... {};

template <typename... Types>
struct tuple : tuple_impl<std::make_index_sequence<sizeof...(Types)>, Types...> {};

template <std::size_t N, typename T>
typename element<N, T>::type get(const element<N, T> &);

template <std::size_t N, typename T>
using at = decltype(get<N>(std::declval<T>()));

int main() {
    using my_tuple = three_tuple<int, char, char*>;
    cout << std::is_same<my_tuple::two, char*>::value << endl;
    cout << std::is_same<my_tuple::one, char*>::value << endl;

    cout << std::is_same<element<3, char>::type, char>::value << endl;
    
    using my_other_tuple = tuple<char, int, char>;
    
    cout << std::is_same<decltype(get<0>(std::declval<my_other_tuple>())), char>::value << endl;
    cout << std::is_same<decltype(get<1>(std::declval<my_other_tuple>())), char>::value << endl;
    cout << std::is_same<decltype(get<2>(std::declval<my_other_tuple>())), char>::value << endl;

    /* Ah-ha! This is ambiguous.
    struct ambiguous : my_other_tuple, element<2, double> {};
    cout << std::is_same<decltype(get<2>(std::declval<ambiguous>())), char>::value << endl;
    */
    
    cout << std::is_same<decltype(static_cast<element<1, int>>(std::declval<my_other_tuple>()))::type, char>::value << endl;
    cout << std::is_same<decltype((element<1, int>)std::declval<my_other_tuple>())::type, char>::value << endl;
    cout << std::is_same<at<0, my_other_tuple>, char>::value << endl;

    using elt_0 = at<0, my_other_tuple>;
    cout << std::is_same<elt_0, char>::value; // false
    
    using elt_1 = at<1, my_other_tuple>;
    cout << std::is_same<elt_1, char>::value; // true
    
    //using maybe_char = static_cast<const element<3, char>&>(my_other_tuple)::type;
    //cout << std::is_same<maybe_char, char>::value << endl;
    return 0;
}