#include <iostream>


#include "case_study_2.hpp"


using std::cout;
using std::endl;


class JustBeforeReturnEvent {
    // ...
};

class CoutShouter {
public:
    static void handle(const JustBeforeReturnEvent& evt) {
        cout << "Goodbye!\n";
    }
};

class QuietGuy {};

int main() {    
    using listeners = type_list<CoutShouter, QuietGuy>;
    using listeners_2 = type_list<CoutShouter, CoutShouter>;
    using listeners_3 = type_list<QuietGuy, QuietGuy, QuietGuy, QuietGuy>;

    cout << has_tail<listeners_2>() << endl;
    
    cout << count<listeners>() << endl;
    cout << count<listeners_2>() << endl;
    
    cout << "listeners 1: ";
    Dispatcher<listeners>::post(JustBeforeReturnEvent{});
    
    cout << "listeners_2: ";
    Dispatcher<listeners_2>::post(JustBeforeReturnEvent{});
    
    cout << "listeners_3: ";
    Dispatcher<listeners_3>::post(JustBeforeReturnEvent{});
    return 0;
}