#include <functional>
#include <type_traits>

#include <stdio.h>


/* The hypothetical C API */

# define ERR_IMPURE_FLUIDS 6
# define ERR_UNKNOWN 13

struct MyCoolStruct {
    int a, b;
    double f;
};


int mandrake(MyCoolStruct *input, int param1, int param2) {
    input->a = param1*param1;
    input->b = param1*param2;
    return 0;
}

int jack(MyCoolStruct *input, int param1, int param2) {
    return ERR_IMPURE_FLUIDS;
}

int dmitri(MyCoolStruct *input) {
    input->f = 7.0*6.0;
    return ERR_UNKNOWN;
}

int major(MyCoolStruct *input, int param1, int param2, int param3) {
    input->b = param1 + param2 + param3;
    return 0;
}


/* Our code */

void print_success() {
        printf("Much success.\n");
}

void print_error(int err) {
    printf("Got error: %s!\n",
        err == ERR_IMPURE_FLUIDS ? "My life essence!" :
        err == ERR_UNKNOWN ? "Mysterious unknown error!" : ""
    );
}

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

template<
typename Function,
typename... Args>
void api_exec(Function func, Args... args) {
    typedef typename std::result_of<Function(Args...)>::type ReturnType;
    static_assert(std::is_integral<ReturnType>::value, "Please only call me with integral types!");
    
    int err = func(args...);
    if (err == 0) {
        print_success();
    } else {
        print_error(err);
    }   
}
