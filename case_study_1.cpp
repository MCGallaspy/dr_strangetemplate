#include <functional>

#include "case_study_1.hpp"

double epsilon() {
    return 5.0;
}

void doNothing() {}

void useMyCoolStruct(MyCoolStruct *foo) {
    printf("MyCoolStruct a: %d, b: %d, f: %f.2\n", foo->a, foo->b, foo->f);
}

int main() {
    
    MyCoolStruct foo;
    apiExec(alpha, &foo, 1, 2);
    apiExec(beta, &foo, 3, 4);
    apiExec(gamma, &foo);
    apiExec(delta, &foo, 5, 6, 7);
    
    // This is a compiler error:
    // apiExec(epsilon); 

    apiExec(alpha, doNothing, print_error, &foo, 8, 9);
    apiExec(alpha, useMyCoolStruct, print_error, &foo, 10, 11);
    
    apiExec(gamma, doNothing, [](int err){
        printf("Calling all cool lambdas!\n");
    }, &foo);
    
    return 0;
}