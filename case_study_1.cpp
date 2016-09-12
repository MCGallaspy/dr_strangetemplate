#include "case_study_1.hpp"

double epsilon() {
    return 5.0;
}

int main() {
    
    MyCoolStruct foo;
    apiExec(alpha, &foo, 1, 2);
    apiExec(beta, &foo, 3, 4);
    apiExec(gamma, &foo);
    apiExec(delta, &foo, 5, 6, 7);
    
    // This is a compiler error:
    // apiExec(epsilon); 
    
    return 0;
}