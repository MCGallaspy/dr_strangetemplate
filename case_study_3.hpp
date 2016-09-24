#include <cstdlib>
#include <type_traits>
#include <utility>

/* We want the constant-time access provided by a fixed-length container,
   but without all that pesky typing. */
template <typename Zero, typename One, typename Two>
struct three_tuple {
    using zero = Zero;
    using one = One;
    using two = Two;
};

/* Class for the elements of our container.
   Just reflects its template paramter. */
template <std::size_t Index, typename Type>
struct element {
    using type = Type;
};

/* tuple_impl uses specialization and std::index_sequence to deduce the 
   length of the parameter pack Types... */
template <typename Indices, typename... Types>
struct tuple_impl;

/* ..and then expands the length as the parameter Ns... in order to
   create a sequence of base classes element<0, foo>, element<1, bar>, etc. */
template <std::size_t... Ns, typename... Types>
struct tuple_impl<std::index_sequence<Ns...>, Types...> : element<Ns, Types>... {};

/* Use sizeof... to make an index sequence that has the length of the 
   parameter pack. */
template <typename... Types>
struct tuple : tuple_impl<std::make_index_sequence<sizeof...(Types)>, Types...> {};

/* We don't really have to implement this -- we just
   want to use it to apply the rules of template argument deduction
   for function templates. */
template <std::size_t N, typename T>
typename element<N, T>::type get(const element<N, T> &);

/* Voila. */
template <std::size_t N, typename T>
using at = decltype(get<N>(std::declval<T>()));