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
    apiExec(mandrake, &foo, 1, 2);
    apiExec(jack, &foo, 3, 4);
    apiExec(dmitri, &foo);
    apiExec(major, &foo, 5, 6, 7);
    
    // This is a compiler error:
    // apiExec(epsilon); 

    apiExec(mandrake, doNothing, print_error, &foo, 8, 9);
    apiExec(mandrake, useMyCoolStruct, print_error, &foo, 10, 11);
    
    apiExec(dmitri, doNothing, [](int err){
        printf("Calling all cool lambdas!\n");
    }, &foo);
    
    return 0;
}