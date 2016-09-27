Dr. Strangetemplate
===================

Or How I Learned to Stop Worrying and Love C++ Templates
--------------------------------------------------------

Templates! Meta-programming! ``<``, ``>``, and ``typename``! The mere sight of these words/glyphs are enough
to strike fear in the hearts of the meek and ignite righteous anger in the bosoms of well-intentioned code
reviewers, who decry their use as too mysterious and arcane for good, plain production code. They may grudgingly
admit that templates and 
metaprogramming have their use in library code (as with the venerable ``std::vector`` and other containers) but
surely are a code smell anywhere else.

But no more! We live in a more enlightened age, and it's time to recognize the noble and simple truth of post-C++11
templates: **they help you write better code**. Templated code is type-safe, expressive, and can increase runtime
performance by both shifting computation to compile-time and enabling optimizations that can't be applied to
equivalent runtime code.

With modern C++ features, template code is readable,
maintainable, sustainable [1]_, biodegradable [2]_, and fully embraceable [3]_!
A well-rounded C++ programmer should be able to identify when to use this powerful language feature.

What follows is a guide on writing practical, maintainable C++ template code.
It is divided into case-studies of (more-or-less) real code that I have seen running freely in the wild, waiting to
be boldly elevated into template modernity.
So let's begin:

.. contents:: Table of Contents

Case Study 1: Consuming a C API
-------------------------------

Imagine that I have a library with several functions as such:

.. code:: c++

    int mandrake(MyCoolStruct *input, int param1, int param2);
    int jack(MyCoolStruct *input, int param1, int param2);
    int dmitri(MyCoolStruct *input);
    int major(MyCoolStruct *input, int param1, int param2, int param3);

Each function returns an ``int`` error code -- ``0`` represents no error, and other integers indicate some
library-specific error. You might consume this API, observing proper error handling and logging as follows:

.. code:: c++

    int err = mandrake(foo, 4, 2);
    if (err != 0) {
        log("mandrake returned error code %d!", err);
        enter_underground_shelter();
    } else {
        total_commitment_to_my_foo(foo);
    }

    // ... in some other file

    int err = jack(foo, 3, 6);
    if (err == 0) {
        preemptive_strike(foo);
    } else if (err == 1) {
        log("jack returned ERR_IMPURE_FLUIDS!");
        await_the_inevitable();
    } else {
        log("jack returned %d, what could it mean?", err);
        ponder_the_mystery();
    }

And so on, and so on. You'll write tons of code like this. Sometimes you won't be able to do anything meaningful with
an error, so you just log it and move on. Maybe in some cases you'll want to change the behavior on success -- for
instance if a call to ``major`` succeeds then you want to handle it in another thread.
And then a few weeks later your supervisor decides you should do this with ``dmitri`` as well.
And then that all API calls should be handled asynchronously on success, but reconsiders a few weeks
after you've added hundreds of calls and decides that only API functions starting with the letter ``b`` should be
handled synchronously.

So you find yourself sweating laboriously over your keyboard, doing tedious and undignified copy-and-paste,
search-and-replace, and testing each change over and over again. Dear beleaguered programmer... there is a better way!

.. code:: c++

    api_exec(mandrake,
    [&foo](){ /* on success */ 
        total_commitment_to_my_foo(foo);
    },
    log_it, /* on error */
    foo, 4, 2); /* params */

    api_exec(jack,
    total_commitment_to_my_foo,  /* on success - foo is passed automatically to this function */
    log_it, /* on error */
    foo, 3, 6); /* params */

Everything just described can be achieved with templates!
Easy refactoring, easily changeable success/error behavior, and the ability to select totally different behavior
by using a different template (perhaps something like ``api_async_exec``).
By writing one template function we create a new ``api_exec`` for every API function that we have.
We'll start by implementing a basic ``api_exec`` template function and gradually add more bells and whistles to it.

.. code:: c++

    // case_study_1.hpp
    
    template<typename... Args>
    void api_exec(int func(Args...), Args... args) {
        int err = func(args...);
        if (err == 0) {
            printf("Much success.\n");
        } else {
            printf("Got error: %s!\n",
                err == ERR_IMPURE_FLUIDS ? "My life essence!" :
                err == ERR_UNKNOWN ? "Mysterious unknown error!" : ""
            );
        }   
    }

    // case_study_1.cpp
    
    MyCoolStruct foo;
    api_exec(mandrake, &foo, 1, 2);
    api_exec(jack, &foo, 3, 4);
    api_exec(dmitri, &foo);
    api_exec(major, &foo, 5, 6, 7);
    
    /* Output:
    Much success.
    Got error: My life essence!!
    Got error: Mysterious unknown error!!
    Much success.
    */

That's it! Now you're generating code like a pro. Note two things here:

#. ``api_exec`` is a variadic template.
#. The first parameter of ``api_exec`` is some weird function type.

A **variadic template** is a template that takes a variable number of template parameters [4]_. If you've used templates
before you may know that a *template parameter* is a type [5]_ like ``int`` or ``MyCoolStruct``.
So a variadic template just takes some variable number of types that you don't have to specify.
A variadic template's *parameter pack* can be expanded with ``Args...`` and used as a function parameter with 
``Args... args``. In this case ``Args...`` corresponds to the *types* of the parameters and ``args``
correponds to the actual *values* that we passed in.

Regarding the second point, the first rule of weird function types is that *you shouldn't use a function type at
all if you don't have to*:

.. code:: c++

    template<typename Function, typename... Args>
    void api_exec(Function func, Args... args) {
        int err = func(args...);
        if (err == 0) {
            printf("Much success.\n");
        } else {
            printf("Got error: %s!\n",
                err == ERR_IMPURE_FLUIDS ? "My life essence!" :
                err == ERR_UNKNOWN ? "Mysterious unknown error!" : ""
            );
        }   
    }
    
Whoa. Your compiler can deduce the type of ``func`` automatically when you use it as a parameter.
Let it! It's what compilers love to do.

Abandon all hope, ye who enter here!
************************************

Variadic templates are a feature introduced in C++11 and they're really powerful, but they also introduce complexity.
So do the rest of the features considered below, because as it turns out C++ templates define a whole 'nother
programming language, one that's executed entirely at compile time and deals with types [6]_.

You can get a lot of mileage out of basic templates like above.
But if you understand metaprogramming techniques you can make good use of the standard library [7]_, libraries like
`boost::hana <http://www.boost.org/doc/libs/release/libs/hana/doc/html/index.html>`_,
and write your own metafunctions for great profit. So read on if you wish to continue the brave march into
template modernity!

Back to your regular program(ming)
**********************************

There is one caveat to our first example -- because built-in numeric types are implicitly convertible from one to
another, the compiler will quietly do stuff like this:

.. code:: c++

    double epsilon() {
        return 5.0;
    }

    api_exec(epsilon); // no error here!

This isn't always undesirable behavior -- but since our C API *always* returns ``int`` anyway we may as well nip some
weird mistake in the bud by creating a compiler error when you try to do silly stuff like above:

.. code:: c++

    template<typename Function, typename... Args>
    void api_exec(Function func, Args... args) {
        // Guards against careless instantiations with functions that return double.
        typedef typename std::result_of<Function(Args...)>::type ReturnType;
        static_assert(std::is_integral<ReturnType>::value, "Please only call me with integral types!");
        
        int err = func(args...);
        if (err == 0) {
            printf("Much success.\n");
        } else {
            printf("Got error: %s!\n",
                err == ERR_IMPURE_FLUIDS ? "My life essence!" :
                err == ERR_UNKNOWN ? "Mysterious unknown error!" : ""
            );
        }   
    }

``static_assert`` will generate a compiler error if its value is ``false``. It doesn't do anything at *all* at
runtime, so you should basically use it like it's going out of style to keep your code type-safe and readable.

More interesting is the expression ``std::is_integral<ReturnType>::value``.
``std::is_integral`` is a *metafunction* that returns ``true`` if the type ``ReturnType`` is (you guessed it) 
integral [8]_. This is our first example of metaprogramming!

Metafunctions take template parameters and the result is either another type or a constant value.
In the case of ``is_integral`` we're interested in the ``bool`` value it returns, which 
by the standard library's convention is accessed in the static class variable ``value``:

.. code:: c++

    std::is_integral<int>::value; // true
    std::is_integral<double>::value; // false
    std::is_integral<int>; // this is actually a class, and not a valid statement.
 
    // This works though.
    typedef typename std::is_integral<double> is_integral_t;
    is_integral_t::value; // false

Now consider the previous line:

.. code:: c++

    typedef typename std::result_of<Function(Args...)>::type ReturnType;

``typedef`` is the equivalent of assigning a variable in metaprogramming, and ``ReturnType`` is the type name we're 
assigning it to.
``std::result_of`` is a metafunction that returns the type of the result of ``Function`` if it was applied to 
``Args...`` [9]_.
Just like a metafunction's value can be accessed with ``::value``, by convention if it's the type we're interested in
we access it through ``::type`` as in ``std::result_of<Function(Args...)>::type``.
Finally we have to let the compiler know that an expression is a type and not a value, which you do with the keyword
``typename`` -- it's an unrelated double use of the keyword that appears in template parameter lists [10]_.

Whenever you use a template inside of another template, you generally have to help the compiler deduce that the
template is in fact a *type* by prefixing it with ``typename``. So basically if you don't call it with ``::value``
then you should use ``typename``.

My mother said SFINAE is not a polite word
******************************************

Finally let's write something that takes success and error callbacks:

.. code:: c++

    template<
    typename Function,
    typename OnSuccess,
    typename OnError,
    typename... Args>
    void api_exec(Function func, OnSuccess on_success, OnError on_error, Args... args) {
        typedef typename std::result_of<Function(Args...)>::type ReturnType;
        static_assert(std::is_integral<ReturnType>::value, "Please only call me with integral types!");
        
        int err = func(args...);
        if (err == 0) {
            on_success();
        } else {
            on_error(err);
        }
    }
    
    // example use
    api_exec(mandrake, do_nothing, print_error, &foo, 8, 9);

Simple! We just add two more template parameters representing our success and error functions. But a perceptive
reader might wonder what happens if you try to call this with an on_error function that doesn't take a single ``int``
parameter. Turns out it's a compile error.

Wait, weren't we promised an on_success callback that would automatically take the ``foo`` parameter we passed in?
Let's write an overloaded function to handle that!

.. code:: c++
    
    // WRONG CODE, THIS DOESN'T WORK!
    
    template<
    typename Function,
    typename OnSuccess,
    typename OnError,
    typename InputType,
    typename... Args>
    void api_exec(Function func, OnSuccess on_success, OnError on_error, InputType input, Args... args) {
        typedef typename std::result_of<Function(InputType, Args...)>::type ReturnType;
        static_assert(std::is_integral<ReturnType>::value, "Please only call me with integral types!");
        
        int err = func(input, args...);
        if (err == 0) {
            on_success(input);
        } else {
            on_error(err);
        }
    }

    template<
    typename Function,
    typename OnSuccess,
    typename OnError,
    typename... Args>
    void api_exec(Function func, OnSuccess on_success, OnError on_error, Args... args) {
        typedef typename std::result_of<Function(Args...)>::type ReturnType;
        static_assert(std::is_integral<ReturnType>::value, "Please only call me with integral types!");
        
        int err = func(args...);
        if (err == 0) {
            on_success();
        } else {
            on_error(err);
        }
    }

Ahh! This doesn't work. If you try to use it, then you'll get errors because the compiler has no way of knowing
which overloaded function to pick. It can't figure it out from the template parameters, because variadic parameters
"eat" up all the rest. In other words a parameter list like ``template <typename One, typename... TheRest>``
seems exactly the same as ``template <typename... SameAsTheLastOne>``. If only there was some way to specify the 
*metatype* of the types in template parameters, just like you declare the
types of variables in regular functions... And there is! But sadly in C++11 it's a bit clunky as you may infer from
its weird acronym-name (acroname?) SFINAE.

SFINAE stands for "substitution failure is not an error" and refers to
the rules of how C++ selects overloaded templates. Basically, in some circumstances if substituting a type would
result in an error otherwise, the compiler will quietly ignore the error and try to select another template for
overload resolution instead. SFINAE does *not* apply in function bodies -- we already saw this if you try to pass
in an on error function that doesn't take a single ``int`` parameter. However it does apply to the *return type* of a
template function.

You don't need to understand the details of SFINAE to start using it [11]_. The standard library provides a metafunction
called ``std::enable_if`` which takes one ``bool`` template parameter and one optional template parameter.
When its first parameter is ``false``, it simply results in a compiler error!
You can use it as the return type of a function along with the metafunctions in ``type_traits`` to create
overloaded templates that have constraints on their template parameters:

.. code:: c++

    template <typename Arg>
    using returns_void = typename std::is_same<typename std::result_of<Arg>::type, void>;

    template<
    typename Function,
    typename OnSuccess,
    typename OnError,
    typename InputType,
    typename... Args>
    typename std::enable_if<
        returns_void<OnSuccess(InputType)>::value
    >::type
    api_exec(Function func, OnSuccess on_success, OnError on_error, InputType input, Args... args) {
        typedef typename std::result_of<Function(InputType, Args...)>::type ReturnType;
        static_assert(std::is_integral<ReturnType>::value, "Please only call me with integral types!");
        
        int err = func(input, args...);
        if (err == 0) {
            on_success(input);
        } else {
            on_error(err);
        }
    }

Let's break it down. First we define a new metafunction ``returns_void`` from the ``type_traits`` metafunctions for
readability. It takes a single template parameter, and has a ``value`` member that's true if ``result_of`` applied to
its argument is ``void``. Next we replace the return type with ``std::enable_if``:

.. code:: c++

    typename std::enable_if<
        returns_void<OnSuccess(InputType)>::value
    >::type
    api_exec(Function func, OnSuccess on_success, OnError on_error, InputType input, Args... args) {

The ``::type`` of ``enable_if`` is ``void`` with the single-parameter version [12]_, so the signature of ``api_exec``
hasn't changed. However if the predicate ``returns_void`` is ``false`` then this function will be removed from
overload resolution because of SFINAE. We can define as many overloaded version as we want now!

.. code:: c++

    template<
    typename Function,
    typename OnSuccess,
    typename OnError,
    typename... Args>
    typename std::enable_if<
        returns_void<OnSuccess(void)>::value
    >::type
    api_exec(Function func, OnSuccess on_success, OnError on_error, Args... args) {
        typedef typename std::result_of<Function(Args...)>::type ReturnType;
        static_assert(std::is_integral<ReturnType>::value, "Please only call me with integral types!");
        
        int err = func(args...);
        if (err == 0) {
            on_success();
        } else {
            on_error(err);
        }
    }

This one will only be available to overload resolution if ``OnSuccess`` called with ``void`` returns ``true``.
    
Huzzah! Let's use it:

.. code:: c++

    // case_study_1.cpp
    
    #include "case_study_1.hpp"

    double epsilon() {
        return 5.0;
    }

    void do_nothing() {}

    void use_my_cool_struct(MyCoolStruct *foo) {
        printf("MyCoolStruct a: %d, b: %d, f: %f.2\n", foo->a, foo->b, foo->f);
    }

    int main() {
        
        MyCoolStruct foo;
        api_exec(mandrake, &foo, 1, 2);
        api_exec(jack, &foo, 3, 4);
        api_exec(dmitri, &foo);
        api_exec(major, &foo, 5, 6, 7);
        
        // This is a compiler error:
        // api_exec(epsilon); 

        api_exec(mandrake, do_nothing, print_error, &foo, 8, 9);
        api_exec(mandrake, use_my_cool_struct, print_error, &foo, 10, 11);
        
        api_exec(
        dmitri,
        [](const MyCoolStruct* foo){
            printf("Success!\n");
        },
        [](int err){
            printf("Calling all cool lambdas!\n");
        },
        &foo);
        
        api_exec(
        major,
        [](){
            printf("Yee-haw!\n");
        },
        [](int err){
            printf("Another cool lambda!\n");
        },
        &foo, 8, 9, 10);

        return 0;
    }
    
    /* output
    Much success.
    Got error: My life essence!!
    Got error: Mysterious unknown error!!
    Much success.
    MyCoolStruct a: 100, b: 110, f: 42.000000.2
    Calling all cool lambdas!
    Yee-haw!
    */
    
Case closed
***********

Example code for this case study is provided in ``case_study_1.hpp`` and ``case_study_1.cpp``.
Any typos or inaccuracies are my fault -- I would appreciate a PR!

A guide on metaprogramming would be remiss without mentioning
`C++ concepts <https://en.wikipedia.org/wiki/Concepts_(C%2B%2B)>`_,
which have been proposed to greatly simplify selecting template overloads instead of using SFINAE.
Concepts are currently availabe in `GCC <https://gcc.gnu.org/gcc-6/changes.html>`_.

You can use the fundamental techniques presented to start writing great metaprograms, but if you get deep into it
you'll probably want to use a library like 
the older `MPL <http://www.boost.org/doc/libs/1_61_0/libs/mpl/doc/index.html>`_
or the newer `boost::hana <http://www.boost.org/doc/libs/1_61_0/libs/hana/doc/html/index.html>`_.

Some readers have taken umbrage with this example -- see my thoughts in the issues.

Case Study 2: Building an awesome event interface
-------------------------------------------------

Templates can be used to create really great interfaces!
They allow you to manipulate types in ways that wouldn't otherwise be possible.
Consider the following pattern that I'll call *Do Something When X Happens*.
It's a very simple pattern: whenever some particular event occurs, then one or more listeners respond to that event!
An event occurring is realized as instantiating a class and providing it to a dispatcher.
Listeners are recognized by providing them to the dispatcher *and* defining an appropriate handler member function.
We'll start with an interface that we [13]_ want and work backwards to build it:

.. code:: c++

    class JustBeforeReturn {
        // ...
    }
    
    class CoutShouter {
    public:
        void handle(const JustBeforeReturn& evt) {
            std::cout << "Goodbye!\n";
        }
    }
    
    template <typename Listeners>    
    class Dispatcher {
    public:
        template <typename Evt>
        static void post(const Evt&);
    }
    
    using listeners = type_list<CoutShouter>;
    using dispatcher = Dispatcher<listeners>;
    
    int main() {
 
        dispatcher::post(JustBeforeReturn{});
        return 0;
    }

The *Do Something When X Happens* pattern involves three actors -- an event class (here ``JustBeforeReturn``) which
is instantiated and provided to a ``Dispatcher``. The ``Dispatcher`` in turn provides the event to a list of types
which handle it. In this case the list of types is really just one, ``CoutShouter``. Turns out this example will
involve some nontrivial metaprogramming! Let's start with:

type_list: A *metaclass* (if you will)
**************************************

Algorithms require data structures. The C++ standard library doesn't have data structures for types [14]_, so in
order to do anything other than trivial operations we'll have to define them. And it's actually really
simple, although perhaps a bit unfamiliar compared to runtime C++. Template metaprogramming is a
**pure functional language** [15]_, so data structures look a little different than their runtime counterparts since
runtime C++ is neither pure nor functional.
The upshot is that we have plenty of rich examples to draw from. For instance, a type list could be defined as
follows:

.. code:: c++

    // A forward declaration. This is required so that we can define specializations below.
    template <typename... Args>
    struct type_list;

    // A list of one element just provides us with that element again!
    // We can access it through the type alias head.
    template <typename Type>
    struct type_list<Type> {
        using head = Type;
    };

    // A list with *more* than one element has a head and a tail.
    // Here the head is provided through inheritance,
    // And the tail is defined as a list containing the rest of the elements!
    template <typename Head, typename... Tail>
    struct type_list<Head, Tail...> : type_list<Head> {
        using tail = type_list<Tail...>;
    };
    
    // Here's what it looks like in action.
    using my_cool_list = type_list<int, double, int, char>;

Since C++11 the `using keyword <http://en.cppreference.com/w/cpp/language/type_alias>`_
can be used as a more natural ``typedef``. Using metaclasses [16]_ effectively requires good use of SFINAE, so before
we go further I'm gonna let you in on a little trick [17]_ to make using SFINAE much less awkward:

.. code:: c++

    /* OMG awesome void_t metafunction will change your life */
    template <typename...>
    using void_t = void;

The metafunction ``void_t`` just maps any number of type parameters into ``void``. It seems unremarkable at first
until you realize that it can be used to invoke SFINAE since ``void_t``'s parameters must be well-formed! Here's
a ``type_list`` metafunction that makes use of it:

.. code:: c++

    // Template with default parameter
    template <typename T, typename = void>
    struct count : std::integral_constant<int, 1> {};

    // More specialized template will be chosen unless SFINAE removes it!
    template <typename T>
    struct count<T, void_t<typename T::tail>> :
        std::integral_constant<int, 1 + count<typename T::tail>()> {};
        
    using my_cool_list = type_list<int, double, int, char>;
    count<my_cool_list>::value; // 4;

This is a really great metaprogramming technique to have up your sleeves! The first template is the default case.
It has an unused (and thus unnamed) default template parameter -- meaning importantly that you can call it by passing
in *one* template parameter only, the type that you're interested in. The metafunction ``count`` will return ``1`` 
by default, but if the type passed in has a ``::tail`` like
our ``type_list``, then it will peel it off and recursively call ``count`` until it hits the default case, adding
one to its value each time.

The template specialization below it is where the magic happens. ``void_t<typename T::tail>`` will invoke SFINAE if
the template parameter ``T`` does not have a ``tail`` member! And since it is more specialized [18]_ the compiler
will always prefer it unless SFINAE removes it from overload resolution. Inheriting from ``std::integral_constant`` 
allows a type to be used in a numeric context, which we'll find very useful shortly.

The big takeaway here is that ``void_t`` can be used to really easily determine if a type has some particular member.
Along with ``enable_if`` (which can also be used for this purpose, but the implementation is much more verbose)
we can start building much more complex data structures and metafunctions.

Some readers have pointed out that ``count`` can be implemented with fewer template instantiations.
And they're right! So check out this alternate implementation that doesn't use SFINAE at all:

.. code:: c++

    /* Alternate implementation uses fewer template instantiations */
    template <typename... Elts>
    struct different_count;

    template <typename... Elts>
    struct different_count<type_list<Elts...>> : std::integral_constant<int, sizeof...(Elts)> {};

We only define a specialization here -- you can't instantiate ``different_count`` with anything other than a
``type_list``. This is an example of pattern matching, which we'll see used to good effect in the next example!
The interesting thing to note here is that matching the pattern ``type_list<Elts...>`` actually unpacks ``Elts`` so
that we can use it elsewhere in the template, namely as the argument of ``sizeof...``, which counts the number of
types in a parameter pack.
    
Here's another metafunction that we'll be using:

.. code:: c++

    template <typename T>
    struct has_tail :    /*    predicate     */  /*  if true   */ /*  if false */
        std::conditional<(count<T>::value == 1), std::false_type, std::true_type>::type {};

``has_tail`` uses ``std::conditional`` to inherit from either ``false_type`` or ``true_type`` depending on what the
predicate evaluates to. It's the functional equivalent of the ternary operator, choosing the first type if its
predicate is true, otherwise the second type. ``false_type`` and ``true_type`` are specializations of our friend
``integral_constant`` that allow a class to be used in a boolean context [19]_.

Dispatcher: The Dispatchening
*****************************

We've got nearly everything we need to write dispatcher. It looks like this:

.. code:: c++

    template <typename Handler, typename Evt, typename = void>
    struct has_handler : std::false_type {};

    template <typename Handler, typename Evt>
    struct has_handler<Handler, Evt, decltype( Handler::handle( std::declval<const Evt&>() ) )> : 
        std::true_type {};

    template <typename Listeners>
    class Dispatcher {
        template <typename Evt, typename List, bool HasTail, bool HasHandler>
        struct post_impl;
        
        /*
        We will fill this in with implementation details.
        */
        
    public:
        template <typename Evt>
        static void post(const Evt& t) {
            constexpr bool has_tail_v = has_tail<Listeners>::value;
            constexpr bool has_handler_v = has_handler<typename Listeners::head, Evt>::value;
            post_impl<Evt, Listeners, has_tail_v, has_handler_v>::call(t);
        }
    };

The ``has_handler`` metafunction determines if the parameter ``Handler`` has a static member function ``handle``
that takes
a ``const Evt&`` parameter. Note again the default template parameter in the primary definition -- 
that's a sign that we're
about to make use of SFINAE. And indeed, the specialization below it reveals one more SFINAE technique to add to our
collection. Since C++11 the keyword ``decltype`` can be used to determine the declared type of an expression
*without* evaluating that expression. You can use it to determine if a type has a member function (``handle`` here)
that takes some arbitrary parameters (here ``const Evt&``). If the expression inside of ``decltype`` is well-formed
then the result will be the function's return type. Otherwise SFINAE will be invoked!

``std::declval`` is another standard library metafunction that we can use to instantiate types inside of ``declval``.
The expression ``decltype( Handler::handle( const Evt& ) )`` will produce an error
because we need to call ``handle`` with an *instance* of
``const Evt&``. The expression ``std::declval<const Evt&>()`` gives us just that [20]_.

``Dispatcher::post`` defers its call to another template, ``post_impl`` which takes *four* parameters. Two of the
parameters (``HasTail`` and ``HasHandler``) are completely determined by the ``Listeners`` parameter. The
strategy for defining ``post_impl`` will be to write four template specializations that do different things depending
on the values of ``HasTail`` and ``HasHandler``, which tell us if the ``type_list`` has a tail we need to peel off
and whether ``type_list::head`` has an appropriate handler for ``Evt``, respectively.

Note that ``post_impl`` is actually a *type* and we're really calling its static member function ``call``.
That's because we rely on partial template specialization, which is *only* allowed with template classes and *not*
template functions. Wrapping such functions in a class is a work-around.
Here is the specialization for when both conditions are true:

.. code:: c++

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

If the ``::head`` of the list has an appropriate handler, then we call it!
If the list has a ``::tail``, then we peel it off and call ``post_impl`` on the tail.
We pass in conditions that allow the appropriate specializations to be chosen depending on whether the next element
in the list has a handler and whether it has a ``::tail`` or not. And that's it [21]_!

Wrapping up the Dispatcher example
**********************************

Here's some code that illustrates use of the *Do Something When X Happens* pattern:

.. code:: c++

    class CoutShouter {
    public:
        static void handle(const JustBeforeReturn& evt) {
            cout << "Goodbye!" << endl;
        }
        
        static void handle(const InTheBeginning& evt) {
            cout << "Hello!" << endl;
        }
        
        static void handle(const ReadingComicBooks& evt) {
            cout << "Comics!" << endl;
        }
    };

    class QuietGuy {};

    class ComicBookNerd {
    public:
        static void handle(const ReadingComicBooks& evt) {
            cout << "I love " + evt.title + "!" << endl;
        }
    };

    int main() {    
        using listeners = type_list<CoutShouter, QuietGuy, ComicBookNerd>;
        
        using dispatcher = Dispatcher<listeners>;
        
        dispatcher::post(InTheBeginning{});

        ReadingComicBooks spiderman{"spiderman"};
        dispatcher::post(spiderman);
        
        dispatcher::post(JustBeforeReturn{});
        return 0;
    }

With template code like this it's reasonable to expect that the compiler could inline all of the ``Dispatcher`` code. [22]_

Complete code examples can be found in ``case_study_2.hpp`` and ``case_study_2.cpp``.

Case study 3: A compile-time container metaclass
------------------------------------------------

The final case study for now is meant to give a taste of more advanced metaprogramming.
The ad-hoc ``type_list`` data structure
structure above is fine and dandy, but can we create an honest-to-goodness ``std::vector``-style random access
container for types?

Turns out the answer is not only **"Heck yes!"** but that lots of library writers have already done it!
So go use those in production code instead of writing your own.
But if you want to learn a little bit about how those implementations might work, read on. We'll use some
features from the C++14 ``std::`` namespace, like ``integer_sequence`` -- but they're features that *could* have been
implemented in C++11.

First we must note that with purely functional data structures the naive implementation of a random-access list has
really bad (linear) performance for accessing an arbitrary element. This might seem surprising since C-style arrays
trivially have constant-time access to arbitrary elements, so why shouldn't type lists? Indeed for a fixed-length
type list the obvious implementation is to reflect the arguments back for constant-time element access:

.. code:: c++

    template <typename Zero, typename One, typename Two>
    struct three_tuple {
        using zero = Zero;
        using one = One;
        using two = Two;
    };

In order to do this with an arbitrary number of type parameters, we'll make each *element* of the list a distinct
base class, and implement an accessor function that casts the tuple to the appropriate class [23]_. This is possible
due to the rules of template argument deduction, which we'll examine in more detail later.

.. code:: c++

    template <size_t Index, typename Type>
    struct element {
        using type = Type;
    };

    template <typename Indices, typename... Types>
    struct tuple_impl;

First we define an ``element``. Its ``Type`` is the payload, and the unused ``Index`` parameter will turn out to be
critical to the template argument deduction for the accessor function. The forward declaration of ``tuple_impl``
will match a ``std::index_sequence`` and its variadic parameter will become a pack of ``element``s.

.. code:: c++

    template <std::size_t... Ns, typename... Types>
    struct tuple_impl<std::index_sequence<Ns...>, Types...> : element<Ns, Types>... {};

This specialization is the heart of ``tuple_impl``. Note that the first parameter of the specialization is *not* the
parameter pack ``Ns``, but rather it's the ``std::index_sequence<Ns...>``. This demonstrates the purpose of
``std::index_sequence`` -- when you instantiate a template with an ``index_sequence`` then it will match a
specialization like above. Then through template argument deduction the *values* of the sequence will be *unpacked*
into the parameter ``Ns``. If you're familiar with other functional languages, this is a C++ template
metaprogramming technique for *pattern matching*!

Note also that ``tuple_impl`` inherits from some number of ``element`` specializations. The pack expansion will work
on ``Ns`` and ``Types`` simultaneously to produce a sequence like ``element<0, foo>``, ``element<1, bar>``, etc.

.. code:: c++

    template <typename... Types>
    struct tuple : tuple_impl<std::make_index_sequence<sizeof...(Types)>, Types...> {};

Now we actually define ``tuple``, which just takes a variadic parameter ``Types`` and constructs a ``tuple_impl``
from it. For instance ``tuple<int, int, char*>`` will have the base classes
``tuple_impl<std::index_sequence<0, 1, 2>, int, int, char*>``, ``element<0, int>``, ``element<1, int>``, and
``element<2, char*>``.

That's it! But if we try to access the ``::type`` of a tuple it's ambiguous since more than one base class defines
that type alias.
We have to cast a tuple to *one* of its base classes in order to unambiguously access it. If we know exactly what one
of the base classes is we could ``static_cast``, e.g. ``static_cast<element<1, int>>(my_tuple_instance)``. But that
defeats the purpose because we *don't* know what the second
template parameter of ``element`` will be. Here's where we'll rely
on template argument deduction. We'll *declare* a function that takes the base class
we want to select as a parameter and returns its ``::type``:

.. code:: c++

    template <std::size_t N, typename T>
    typename element<N, T>::type get(const element<N, T> &);

If we instantiate this template for example with ``get<1>(tuple<int, int, char*>())`` then 
we're telling the compiler to conisder calling ``get<1, T>(const element<1, T> &)``.
Since the argument is a class that is *derived* from ``element<1, T>``, then it can deduce unambiguously what
``T`` is -- in this case ``int``. And we don't actually have to supply a definition of ``get``, because
we're just using it in an unevaluated context for the template argument deduction:

.. code:: c++

    template <std::size_t N, typename Tuple>
    using at = decltype(get<N>(std::declval<Tuple>()));

The template ``at`` finds the return type of ``get`` if we supplied it with the parameters ``N`` and ``Tuple``.
Here's ``at`` in action:

.. code:: c++

    int main() {
        using my_tuple = tuple<int, char, char*>;
        
        using elt_0 = at<0, my_tuple>;
        std::is_same<elt_0, char>::value; // false
        
        using elt_1 = at<1, my_tuple>;
        std::is_same<elt_1, char>::value; // true

        return 0;
    }

Example code can be found in case_study_3.hpp and case_study_3.cpp.

Further reading
---------------

Metaprogramming is about computing with types in ways that aren't possible with runtime C++.

If you're wondering *why* computing with types is even necessary, outside of the performance benefits that
metaprogramming can enable, then here's a presentation on 
`using types effectively in C++. <http://www.elbeno.com/presentations/using-types-effectively/presentation.html#/slide-orgheadline82>`_

If you're thinking about metaprogramming in production code, or if you just want some great examples of 
metaprogramming in action, take a look at the 
`boost::hana user manual <http://www.boost.org/doc/libs/release/libs/hana/>`_.

Why did you write this guide?
-----------------------------

I wanted to learn about C++ template metaprogramming, but many of the resources I found were either:

* In reference to pre-C++11, which introduces a lot of new features for metaprogramming.
* Too simple. (Not another factorial example!)
* Too advanced.

My aim is for this guide to fill a gap between "too simple" and "too advanced" by showing short but non-trivial
applications of metaprogramming.

Who are you?
------------

Michael Gallaspy, variously a professional software engineer, substitute teacher, Peace Corps volunteer,
whitewater raft guide, nature appreciater, enthusiastic exister, and enjoyer of Dr. Strangelove.

Resumes available upon request, and if you're reading this and you're my current employer consider giving me a
raise. ;)

.. [1] In a manner of speaking.

.. [2] Actually not.

.. [3] But this part is true.

.. [4] Kinda like regular variadic functions.

.. [5] Actually a template parameter can also be an integral type, e.g. ``template <int N>``, another template,
    and some other stuff too. Check it out!

.. [6] It also turns out you can make a trade-off by turning some runtime computations into
    compile-time computations, although since C++11 it's much easier to do this with `constexpr` than with
    template metaprogramming.

.. [7] The standard library provides metafunctions in the ``type_traits`` header, and support only gets better in
    C++14, C++17, and undoubtedly future versions as well.

.. [8] Like ``int`` or ``const int``.

.. [9] If ``Function`` is not actually a function then gcc will raise an error with C++11.
    
.. [10] Like ``template <typename Unrelated>``.

.. [11] Although it wouldn't hurt.
    
.. [12] The two-parameter version returns its second parameter as its ``::type``, e.g. 
    ``std::enable_if<true, int>::type`` is ``int``.

.. [13] And by "we" of course I mean "I".

.. [14] Although libraries like boost::hana do!

.. [15] It really is **pure** in the sense that it has both no side-effects and its results are totally determined
    by the inputs, and not affected by user's runtime decisions, the weather, and other random occurrences.
    Other functional languages include Haskell and Scala.
    
.. [16] The term *metaclass* doesn't seem to be in common use among C++ template metaprogrammers. Which is a shame
    because it sounds cool. I use it here in an imprecise sense to mean template classes.
    
.. [17] Actually the trick is well-known and is in the
    `standard library <http://en.cppreference.com/w/cpp/types/void_t>`_ since C++17. See
    `this great talk <https://www.youtube.com/watch?v=a0FliKwcwXE>`_ by Walter E. Brown.
    
.. [18] C++ will pick the most specialized template that matches an invocation.

.. [19] This is an inefficient implementation. Better would be to use SFINAE to check for a ``::tail`` member
    directly. Here I use ``count`` only for compactness -- it becomes a one-liner!
    
.. [20] Strangely enough ``declval`` is undefined. Only its *declaration* exists.
    You can't use it to *actually* instantiate anything, it can only be used in unevaluated contexts.
    I know of no use for it outside of ``decltype`` expressions.
    
.. [21] The other specializations fall down similarly.

.. [22] This will be implementation dependent. But when I ``make assembly`` and inspect the generated assembly of
    ``<main>`` for case study 2, I only see one function call to a name-mangled
    ``ComicBookNerd::handle(ReadingComicBooks const&)``!
    Pretty compelling evidence that the rest was inlined.
    For me this is totally dependent on optimization levels --
    without ``-O3`` enabled it's clear from the assembly that *nothing* is inlined.

.. [23] I got this clever idea from the author of boost::hana!
