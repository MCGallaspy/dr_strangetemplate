#include <iostream>
#include <string>


#include "case_study_2.hpp"


using std::cout;
using std::endl;
using std::string;

/* Events */
class JustBeforeReturn {
    // ...
};

class InTheBeginning {};

struct ReadingComicBooks {
    string title;
};

/* Handlers */
class CoutShouter {
public:
    static void handle(const JustBeforeReturn& evt) {
        cout << "Goodbye!" << endl;
    }
    
    static void handle(const InTheBeginning& evt) {
        cout << "Hello!" << endl;
    }
    
    static void handle(const ReadingComicBooks& evt) {
        cout << "Comics!" << endl;
    }
};

class QuietGuy {};

class ComicBookNerd {
public:
    static void handle(const ReadingComicBooks& evt) {
        cout << "I love " + evt.title + "!" << endl;
    }
};


int main() {    
    using listeners_1 = type_list<QuietGuy>;
    string repr_1 = "type_list<QuietGuy>";
    using listeners_2 = type_list<CoutShouter, QuietGuy, ComicBookNerd>;
    string repr_2 = "type_list<CoutShouter, QuietGuy, ComicBookNerd>";
    
    using dispatcher_1 = Dispatcher<listeners_1>;
    using dispatcher_2 = Dispatcher<listeners_2>;
    
    dispatcher_1::post(InTheBeginning{});
    dispatcher_2::post(InTheBeginning{});
    cout << endl;
    
    cout << repr_1 + " has tail: " + (has_tail<listeners_1>() ? "true" : "false") << endl;
    cout << repr_2 + " has tail: " + (has_tail<listeners_2>() ? "true" : "false") << endl;
    cout << endl;

    ReadingComicBooks spiderman{"spiderman"};
    dispatcher_1::post(spiderman);
    dispatcher_2::post(spiderman);
    cout << endl;
    
    cout << repr_1 + " has count: " << count<listeners_1>::value << endl;
    cout << repr_2 + " has count: " << count<listeners_2>::value << endl;
    cout << endl;
    
    dispatcher_1::post(JustBeforeReturn{});
    dispatcher_2::post(JustBeforeReturn{});
    return 0;
}