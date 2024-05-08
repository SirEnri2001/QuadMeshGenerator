#include <iostream>
#include "Eigen/Core"
#include "../api/eigen.h"
using namespace std;

template <typename T> 
class MyTemplateClass
{
public:
    MyTemplateClass()
    {
        std::cout << std::endl << "   CONSTRUCTOR" << std::endl;
    }
    template <typename U> 
    const MyTemplateClass<T>& operator=(const MyTemplateClass<U>& that) 
    { std::cout << std::endl << "   OPERATOR" << std::endl; 
    }
    template <typename U> MyTemplateClass(const MyTemplateClass<U>& that)
    {
        std::cout << std::endl << "    COPY CONSTRUCTOR" << std::endl;
    }
};


MyTemplateClass<int> test(void)
{
    std::cout << std::endl << "      INSIDE " << std::endl; MyTemplateClass<int> a; return a;
}

int main() {
    Eigen::MatrixXd a{      // construct a 2x2 matrix
      {1, 2},     // first row
      {3, 4}      // second row
    };
    Eigen::MatrixXd b{
        {0, 1},     // first row
      {2, 3}      // second row
    };

    std::cout << solve(a, b);
}