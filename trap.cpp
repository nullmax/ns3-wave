#include <iostream>

using namespace std;

class test
{
private:
public:
    static const int attr1 = 0;
    static int attr2;
    test(/* args */){}
    ~test(){}
    void func(const int & a)
    {
        cout<< a <<endl;
    }
};

int test::attr2 = 1;

int main(int, char**) 
{
    test t;
    t.func(test::attr1);
    t.func(test::attr2);
}
