#include <iostream>

using namespace std;

class test
{
private:
public:
    static const int attr = 0;

    test(/* args */){}
    ~test(){}
    void func(const int & a)
    {
        cout<< a <<endl;
    }
};

int main(int, char**) 
{
    test t;
    t.func(1);
    t.func(test::attr);
    t.func(t.attr);
}
