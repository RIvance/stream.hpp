# stream.hpp
Java-like stream in C++20

## Example

```cpp
std::vector<int> vector;
for (int i = 0; i < 80; i++) { vector.emplace_back(i); }
auto set = Stream(vector)
  .filter([](int x) { return x % 2 != 0; })
  .map<double>([](int x) { return (double) x / 2; })
  .take(10).takeWhile([](int x) { return x < 8; })
  .collect<std::set<double>>();
Stream(set).forEachIndexed([](int i, double x) {
    std::cout << i << ": " << x << std::endl;
});
```
Result:
```
0: 0.5
1: 1.5
2: 2.5
3: 3.5
4: 4.5
5: 5.5
6: 6.5
7: 7.5
```

## Usage

Put `stream.hpp` into your C++ project, then include it.
