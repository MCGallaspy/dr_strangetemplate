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