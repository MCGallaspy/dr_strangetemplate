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
templates: *they help you write less code*. Moreover with modern C++ features, template code is as readable and
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

And so on, and so on. This leads to our first take on templates: *templates are functions of functions*. So how do we
write something like this? We'll start by implementing a basic `apiExec` template and gradually add more bells and
whistles to it.
