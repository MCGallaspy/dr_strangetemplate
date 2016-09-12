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
templates: **they help you write less code**. Moreover with modern C++ features, template code is as readable and
maintainable as any other code. Template code has other advantages and disadvantages -- a well-rounded C++ programmer
should be able to identify when templates will help and when they will hinder.

What follows is a guide on writing practical, maintainable C++ template code.
It is divided into case-studies of (more-or-less) real code that I have seen running freely in the wild, waiting to
be boldly elevated into template modernity. Each section provides a different take on how to use templates.
So let's begin:


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
instance if a call to ``delta`` succeeds then you want to handle it in another thread without blocking the calling
thread. And then a few weeks later your supervisor decides you should do this with ``gamma`` as well. And then your
supervisor decides that all API calls should be called and handled asynchronously, but reconsiders a few months
after you've added hundreds of calls and decides that *some* API calls should be handled asynchronously, oh and maybe
we can judiciously use ``std::future`` as well.

So you find yourself sweating laboriously over your keyboard, doing tedious and undignified copy-and-paste,
search-and-replace, and testing each change over and over again. But child... there is a better way!

.. code:: c++

    apiExec(alpha,
    /* on success */ [&foo](){
        actCasualWithMy(foo);
    },
    /* on error */ logIt,
    /* params */ foo, 4, 2);

    apiAsyncExec(beta,
    /* on success */ suchGreatFoo,  // foo is passed automatically to this
    /* on error */ logIt,
    /* params */ foo);

And so on, and so on. This leads to our first take on templates: **templates are functions of functions**. So how do we
write something like this? We'll start by implementing a basic ``apiExec`` template and gradually add more bells and
whistles to it.

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
    
    /* $ ./case_study_1.exe
    Much success.
    Got error: They really did it!!
    Got error: Mysterious unknown error!!
    Much success.
    */

There are two important things to note here:

#. ``apiExec`` is a variadic template.
#. The first parameter of ``apiExec`` is a function type.

Understanding variadic templates and function types unlocks the basic techniques in case study 1.

A **variadic template** is a template that takes a variable number of template parameters. If you've used templates
before you may know that a *template parameter* is a type[#]_ like ``int`` or ``MyCoolStruct``.
So a variadic template just takes some variable
number of types that you don't have to specify. A parameter pack can be expanded and used in a template as with 
``Args...`` and can be
named as with ``Args... args``. In this case ``Args...`` corresponds to the *types* of the parameters and ``args``
correponds to the actual *values* that we passed in. This leads to the second point.

A **function type** is how you pass functions as parameters in C++. In this case the parameter ``int func(Args...)``
means that we take a function that returns an ``int`` and takes the types denoted by ``Args...`` as parameters,
and we call this
function parameter ``func``. For instance, when we call ``apiExec(alpha, &foo, 1, 2)`` this type is expanded to
``int func(int, int)`` and when we call ``apiExec(gamma, &foo)`` it expands to ``int func(void)``.

.. [#] Not always, but sometimes we lie to ourselves for simplicity.
