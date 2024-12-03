#include <iostream>
#include <vector>
#include <string>
using namespace std;

class s {
public:
    int a;
    int b;
    string c;
    double d;

    s() {
        this->a = 1;
    }
    s(int a, int b, string c, double d) : a(a), b(b), c(c), d(d) {}
};

int main() {
    s ss;
    cout << ss.a << " " << ss.b << " " << ss.c << " " << ss.d << endl;
    vector<s> vec;
    vec.push_back(ss);
    vec[0].a = 3;
    cout << ss.a << " " << ss.b << " " << ss.c << " " << ss.d << endl;
    return 0;
}