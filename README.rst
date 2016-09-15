Dr. Strangetemplate
===================

Or How I Learned to Stop Worrying and Love C++ Templates
--------------------------------------------------------

Templates! Meta-programming! ``<``, ``>``, and ``typename``, oh my! The mere sight of these words/glyphs are enough
to strike fear in the hearts of the meek and ignite righteous anger in the bosoms of well-intentioned code
reviewers, who decry their use as too mysterious and arcane for good, plain production code. They may grudgingly
admit that templates and 
metaprogramming have their use in library code (as with the venerable ``std::vector`` and other containers) but
surely are a code smell anywhere else.

But no more! We live in a more enlightened age, and it's time to recognize the noble and simple truth of C++
templates: **they help you write less code**. Moreover with modern C++ features, template code is readable,
maintainable, sustainable [#]_, biodegradable [#]_, and fully embraceable [#]_!
A well-rounded C++ programmer should be able to identify when to use this powerful language feature.

What follows is a guide on writing practical, maintainable C++ template code.
It is divided into case-studies of (more-or-less) real code that I have seen running freely in the wild, waiting to
be boldly elevated into template modernity.
So let's begin:

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

    int err = dmitri(foo);
    if (err == 0) {
        suchGreatFoo(foo);
    } else if (err == 1) {
        log("dmitri returned ERR_IMPURE_FLUIDS!");
        await_the_inevitable();
    } else {
        log("dmitri returned %d, what could it mean?", err);
        ponder_the_mystery();
    }

And so on, and so on. You'll write tons of code like this. Sometimes you won't be able to do anything meaningful with
an error, so you just log it and move on. Maybe in some cases you'll want to change the behavior on success -- for
instance if a call to ``major`` succeeds then you want to handle it in another thread.
And then a few weeks later your supervisor decides you should do this with ``dmitri`` as well.
And then that all API calls should be handled asynchronously on success, but reconsiders a few weeks
after you've added hundreds of calls and decides that only API calls starting with the letter ``b`` should be
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
    foo); /* params */

Everything just described can be achieved with templates!
Easy refactoring, easily changeable success/error behavior, and the ability to select totally different behavior
by using a different template (perhaps something like ``api_async_exec``).
This leads to our first take on templates: **templates are functions that create more functions**.
By writing one template function we create a new ``api_exec`` for every api function that we have.
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

A **variadic template** is a template that takes a variable number of template parameters [#]_. If you've used templates
before you may know that a *template parameter* is a type [#]_ like ``int`` or ``MyCoolStruct``.
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

Abandon all hope, ye who enter here! a.k.a. an intermission
***********************************************************

Stop!

Variadic templates are a feature introduced in C++11 and they're really powerful, but they also introduce complexity.
So do the rest of the features considered below, because as it turns out C++ templates define a whole 'nother
programming language, one that's executed entirely at compile time and deals with types.

You can get a lot of mileage out of basic templates like above.
But if you understand metaprogramming techniques you can make good use of the standard library [#]_, libraries like
`boost::hana <http://www.boost.org/doc/libs/1_61_0/libs/hana/doc/html/index.html>`_,
and even write your own metafunctions for great profit.

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
intergral [#]_. This is our first example of *metaprogramming*! Turns out C++'s template system is a complete
programming language in itself. You can write programs evaluated at compile time that write your runtime program
for you [#]_!

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
``Args...`` [#]_.
Just like a metafunction's value can be accessed with ``::value``, by convention if it's the type we're interested in
we access it through ``::type`` as in ``std::result_of<Function(Args...)>::type``.
Finally we have to let the compiler know that an expression is a type and not a value, which you do with the keyword
``typename`` -- it's an unrelated double use of the keyword that appears in template parameter lists [#]_.

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

You don't need to understand the details of SFINAE to start using it [#]_. The standard library provides a metafunction
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

The ``::type`` of ``enable_if`` is ``void`` with the single-parameter version [#]_, so the signature of ``api_exec``
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

More case studies to come!

Who are you?
************

Michael Gallaspy, variously a professional software engineer, substitute teacher, Peace Corps volunteer,
whitewater raft guide, nature appreciater, enthusiastic exister, and enjoyer of Dr. Strangelove.

Resumes available upon request, and if you're reading this and you're my current employer consider giving me a
raise. ;)

.. [#] In a manner of speaking.

.. [#] Actually not.

.. [#] But this part is true.

.. [#] Kinda like regular variadic functions.

.. [#] Actually a template parameter can also be an integral type, e.g. ``template <int N>``, another template,
    and some other stuff too. Czech it out!

.. [#] The standard library provides metafunctions in the ``type_traits`` header, and support only gets better in
    C++14, C++17, and undoubtedly future versions as well.

.. [#] Like ``int`` or ``const int``.

.. [#] By generating code. It also turns out you can make a trade-off by turning some runtime computations into
    compile-time computations, although since C++11 it's much easier to do this with `constexpr` than with
    template metaprogramming.

.. [#] If ``Function`` is not actually a function then gcc will raise an error with C++11 and do some magic with
    SFINAE starting in C++14... we'll talk more about SFINAE later.
    
.. [#] Like ``template <typename Unrelated>``.

.. [#] Although it wouldn't hurt.
    
.. [#] The two-parameter version returns its second parameter as its ``::type``, e.g. 
    ``std::enable_if<true, int>::type`` is ``int``.
