#include <iostream>

#include "case_study_3.hpp"

using std::cout;
using std::endl;


int main() {
    using my_tuple = tuple<char, int, char>;
    
    using elt_0 = at<0, my_tuple>;
    cout << std::is_same<elt_0, char>::value << endl; // false
    
    using elt_1 = at<1, my_tuple>;
    cout << std::is_same<elt_1, char>::value << endl; // true
    
    /* Ah-ha! This is ambiguous.
    struct ambiguous : my_tuple, element<2, double> {};
    cout << std::is_same<at<2, ambiguous>, char>::value << endl;
    */
    
    return 0;
}