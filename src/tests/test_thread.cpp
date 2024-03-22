#include <iostream>
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
    MyTemplateClass<double> a = test();

    return 0;
}