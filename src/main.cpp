#include <iostream>
#include <cstdlib> 
#include <ctime>
#include "Controller.hpp"

using namespace std;

int main() {
    srand(static_cast<unsigned int>(time(0)));

    try {
        // Створити контролер із 24 несупутникових вузлів та 2 супутникових вузла
        Controller controller(24, 2);
        
        // Запустити інтерактивний цикл команд
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