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
templates: **they help you write less code**. Moreover with modern C++ features, template code is as readable,
maintainable, sustainable [#]_, biodegradable [#]_, and fully embraceable [#]_!
A well-rounded C++ programmer should be able to identify when to use this powerful language feature.

What follows is a guide on writing practical, maintainable C++ template code.
It is divided into case-studies of (more-or-less) real code that I have seen running freely in the wild, waiting to
be boldly elevated into template modernity.
So let's begin:

.. [#] In a manner of speaking.

.. [#] Actually not.

.. [#] But this part is true.

Case Study 1: Consuming a C API
-------------------------------

Imagine that I have a library with several functions as such:

.. code:: c++

    int alpha(MyCoolStruct *input, int param1, int param2);
    int beta(MyCoolStruct *input, int param1, int param2);
    int gamma(MyCoolStruct *input);
    int delta(MyCoolStruct *input, int param1, int param2, int param3);

Each function returns an ``int`` error code -- ``0`` represents no error, and other integers indicate some
library-specific error which you can compare to a slew of macros from a header file. You might consume this API,
observing proper error handling and logging as follows:

.. code:: c++

    int err = alpha(foo, 4, 2);
    if (err != 0) {
        log("alpha returned error code %d!", err);
        panicAndCry();
    } else {
        actCasualWithMyFoo(foo);
    }

    // ... in some other file

    int err = gamma(foo);
    if (err == 0) {
        suchGreatFoo(foo);
    } else if (err == 1) {
        log("gamma returned ERR_THEY_REALLY_DID_IT!");
        awaitTheInevitable();
    } else {
        log("gamma returned %d, what could it mean?", err);
        ponderTheMystery();
    }

And so on, and so on. You'll write tons of code like this. Sometimes you won't be able to do anything meaningful with
an error, so you just log it and move on. Maybe in some cases you'll want to change the behavior on success -- for
instance if a call to ``delta`` succeeds then you want to handle it in another thread.
And then a few weeks later your supervisor decides you should do this with ``gamma`` as well.
And then that all API calls should be handled asynchronously on success, but reconsiders a few weeks
after you've added hundreds of calls and decides that only API calls starting with the letter ``b`` should be
handled synchronously.

So you find yourself sweating laboriously over your keyboard, doing tedious and undignified copy-and-paste,
search-and-replace, and testing each change over and over again. Dear beleaguered programmer... there is a better way!

.. code:: c++

    apiExec(alpha,
    [&foo](){ /* on success */ 
        actCasualWithMy(foo);
    },
    logIt, /* on error */
    foo, 4, 2); /* params */

    apiAsyncExec(beta,
    actCasualWithMyFoo,  /* on success - foo is passed automatically to this function */
    logIt, /* on error */
    foo); /* params */

And so on, and so on.
This leads to our first take on templates: **templates are functions that create more functions**.
By writing one template function we create a new ``apiExec`` for every api function that we have.
So how do we write something like this?
We'll start by implementing a basic ``apiExec`` template and gradually add more bells and whistles to it.

.. code:: c++

    // case_study_1.hpp
    
    template<typename... Args>
    void apiExec(int func(Args...), Args... args) {
        int err = func(args...);
        if (err == 0) {
            printf("Much success.\n");
        } else {
            printf("Got error: %s!\n",
                err == ERR_THEY_REALLY_DID_IT ? "They really did it!" :
                err == ERR_UNKNOWN ? "Mysterious unknown error!" : ""
            );
        }   
    }

    // case_study_1.cpp
    
    MyCoolStruct foo;
    apiExec(alpha, &foo, 1, 2);
    apiExec(beta, &foo, 3, 4);
    apiExec(gamma, &foo);
    apiExec(delta, &foo, 5, 6, 7);
    
    /* Output:
    Much success.
    Got error: They really did it!!
    Got error: Mysterious unknown error!!
    Much success.
    */

That's it! Note two things here:

#. ``apiExec`` is a variadic template.
#. The first parameter of ``apiExec`` is some weird function type.

A **variadic template** is a template that takes a variable number of template parameters [#]_. If you've used templates
before you may know that a *template parameter* is a type [#]_ like ``int`` or ``MyCoolStruct``.
So a variadic template just takes some variable number of types that you don't have to specify.
A variadic template's *parameter pack* can be expanded with ``Args...`` and used as a function parameter with 
``Args... args``. In this case ``Args...`` corresponds to the *types* of the parameters and ``args``
correponds to the actual *values* that we passed in.

.. [#] Kinda like regular variadic functions.

.. [#] Actually a template parameter can also be an integral type, e.g. ``template <int N>``, another template,
    and some other stuff too. Czech it out!

Regarding the second point, the first rule of weird function types is that *you shouldn't use a function type at
all if you don't have to*:

.. code:: c++

    template<typename Function, typename... Args>
    void apiExec(Function func, Args... args) {
        int err = func(args...);
        if (err == 0) {
            printf("Much success.\n");
        } else {
            printf("Got error: %s!\n",
                err == ERR_THEY_REALLY_DID_IT ? "They really did it!" :
                err == ERR_UNKNOWN ? "Mysterious unknown error!" : ""
            );
        }   
    }
    
Whoa. Your compiler can deduce the type of ``func`` automatically when you pass it in.
Let it! It's what compilers love to do.

Abandon all hope, ye who enter here! a.k.a. an intermission
***********************************************************

**Stop!**

Variadic templates are a feature introduced in C++11 and they're really powerful.
But they also introduce complexity.
So do the rest of the features considered below.
You can get a lot of mileage out of basic templates like above.
So stop here in your brave march towards template modernity, unless you want to learn about **metaprogramming**
and write *even less code*.

Back to your regular program
****************************

There is one caveat to our first example -- because built-in numeric types are implicitly convertible from one to
another, the compiler will quietly do stuff like this:

.. code:: c++

    double epsilon() {
        return 5.0;
    }

    apiExec(epsilon); // no error here!

This isn't always undesirable behavior -- but since our API *always* returns ``int`` anyway we may as well nip some
weird mistake in the bud by creating a compiler error when you try to do silly stuff like above:

.. code:: c++

    template<typename Function, typename... Args>
    void apiExec(Function func, Args... args) {
        // Guards against careless instantiations with functions that return double.
        typedef typename std::result_of<Function(Args...)>::type ReturnType;
        static_assert(std::is_integral<ReturnType>::value, "Please only call me with integral types!");
        
        int err = func(args...);
        if (err == 0) {
            printf("Much success.\n");
        } else {
            printf("Got error: %s!\n",
                err == ERR_THEY_REALLY_DID_IT ? "They really did it!" :
                err == ERR_UNKNOWN ? "Mysterious unknown error!" : ""
            );
        }   
    }

``static_assert`` will generate a compiler error if it's value is ``false``. It doesn't do anything at *all* at
runtime, so you should basically use it like it's going out of style to keep your code type-safe and readable.

More interesting is the expression ``std::is_integral<ReturnType>::value``.
``std::is_integral`` is a *metafunction* that returns ``true`` if the type ``ReturnType`` is (you guessed it) 
intergral [#]_. This is our first example of *metaprogramming*! Turns out C++'s template system is a complete
programming language in itself. You can write programs evaluated at compile time that write your runtime program
for you [#]_!

.. [#] Like ``int`` or ``const int``.

.. [#] By generating code. It also turns out you can make a trade-off by turning some runtime computations into
    compile-time computations, although since C++11 it's much easier to do this with `constexpr` than with
    template metaprogramming.

Metafunctions take template parameters and the result is a type.
Sometimes you are interested in the type itself, but in the case of ``is_integral`` we're actually interested in the
``bool`` value it returns.
By convention metafunction return values can be accessed by the static class variable ``value``:

.. code:: c++

    std::is_integral<int>::value; // true
    std::is_integral<double>::value; // false
    std::is_integral<int>; // this is actually a class, and not a valid statement.
 
Now consider the previous line:

.. code:: c++

    typedef typename std::result_of<Function(Args...)>::type ReturnType;

``typedef`` is the equivalent of assigning a variable in metaprogramming, and ``ReturnType`` is the type name we're 
assigning it to.
``std::result_of`` is a metafunction that returns the type of whatever ``Function`` would be if applied to 
``Args...`` [#]_.
Just like a metafunction's value can be accessed with ``::value``, by convention if it's the type we're interested in
we access it through ``::type``: ``std::result_of<Function(Args...)>::type``.
Finally we have to let the compiler know that an expression is a type and not a value, which you do with the keyword
``typename`` -- it's an unrelated double use of the keyword that appears in template parameter lists [#]_.

.. [#] If ``Function`` is not actually a function then gcc will raise an error with C++11 and do some magic with
    SFINAE starting in C++14... we'll talk more about SFINAE later.
    
.. [#] Like ``template <typename Unrelated>``.

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
    void apiExec(Function func, OnSuccess on_success, OnError on_error, Args... args) {
        typedef typename std::result_of<Function(Args...)>::type ReturnType;
        static_assert(std::is_integral<ReturnType>::value, "Please only call me with integral types!");
        
        int err = func(args...);
        if (err == 0) {
            on_success();
        } else {
            on_error(err);
        }
    }
