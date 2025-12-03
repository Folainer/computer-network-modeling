#include <iostream>

#include "Global.hpp"
#include "Controller.hpp"

using namespace std;

int main()
{
    Controller controller(24, 2);
    controller.run();
}