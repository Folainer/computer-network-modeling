#include <iostream>
#include <cstdlib> 
#include <ctime>
#include "Controller.hpp"

using namespace std;

int main() {
    srand(static_cast<unsigned int>(time(0)));

    try {
        // Create controller with 24 non-satellite nodes and 2 satellite nodes
        Controller controller(24, 2);
        
        // Run the interactive command loop
        controller.run();
    }
    catch (const exception& e) {
        cerr << "Fatal error: " << e.what() << endl;
        return 1;
    }
    catch (...) {
        cerr << "Unknown fatal error occurred" << endl;
        return 1;
    }
    
    return 0;
}