#include <functional>
#include <type_traits>

#include <stdio.h>


/* The hypothetical C API */

# define ERR_THEY_REALLY_DID_IT 6
# define ERR_UNKNOWN 13

struct MyCoolStruct {
    int a, b;
    double f;
};


int alpha(MyCoolStruct *input, int param1, int param2) {
    input->a = param1*param1;
    input->b = param1*param2;
    return 0;
}

int beta(MyCoolStruct *input, int param1, int param2) {
    return ERR_THEY_REALLY_DID_IT;
}

int gamma(MyCoolStruct *input) {
    input->f = 7.0*6.0;
    return ERR_UNKNOWN;
}

int delta(MyCoolStruct *input, int param1, int param2, int param3) {
    input->b = param1 + param2 + param3;
    return 0;
}


/* Our code */

void print_success() {
        printf("Much success.\n");
}

void print_error(int err) {
    printf("Got error: %s!\n",
        err == ERR_THEY_REALLY_DID_IT ? "They really did it!" :
        err == ERR_UNKNOWN ? "Mysterious unknown error!" : ""
    );
}

template<
typename Function,
typename OnSuccess,
typename OnError,
typename InputType,
typename... Args>
typename std::enable_if<
    std::is_same<typename std::result_of<OnSuccess(InputType)>::type, void>::value
>::type
apiExec(Function func, OnSuccess on_success, OnError on_error, InputType input, Args... args) {
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
    std::is_same<typename std::result_of<OnSuccess(void)>::type, void>::value
>::type
apiExec(Function func, OnSuccess on_success, OnError on_error, Args... args) {
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
void apiExec(Function func, Args... args) {
    typedef typename std::result_of<Function(Args...)>::type ReturnType;
    static_assert(std::is_integral<ReturnType>::value, "Please only call me with integral types!");
    
    int err = func(args...);
    if (err == 0) {
        print_success();
    } else {
        print_error(err);
    }   
}
