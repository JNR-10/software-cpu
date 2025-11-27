#include <cstddef>
#include <string>
#include <vector>
#include <iostream>

int main() {
    std::size_t n = 42;
    std::string s = "hello";
    std::vector<int> v = {1,2,3};
    std::cout << n << " " << s.size() << " " << v.size() << "\n";
    return 0;
}
