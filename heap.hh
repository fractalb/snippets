#include <vector>
#include <algorithm>
#include <functional>
#include <cassert>

// WARNING: Won't work if the no. of items are greater than INT_MAX items
// The type T should be sortable for this to work. Maybe we need to use
// type_traits to ensure this.

namespace heap {
template <typename T, bool U = true>
class Heap {
 private:
  std::vector<T> v;

 public:
  Heap() : v() {
  }

  ssize_t size() const {
    return v.size();
  }

  void pop_back() {
    v.pop_back();
  }

  T& top() {
    return v.front();
  }

  T& back() {
    return v.back();
  }

  const T& top_of(const T& a, const T& b) {
    if (U)
      return a > b ? a : b;
    else
      return a < b ? a : b;
  }

  const T& top_of(const T& a, const T& b, const T& c) {
    return top_of(a, top_of(b, c));
  }

  void sink() {
    int n = v.size();
    int index = 0;
    int ch2 = 2 * (index + 1);
    int ch1 = ch2 - 1;

    while (ch2 < n && v[index] != top_of(v[index], v[ch2], v[ch1])) {
      if (v[ch1] == top_of(v[ch1], v[ch2])) {
        std::swap(v[index], v[ch1]);
        index = ch1;
      } else {
        std::swap(v[index], v[ch2]);
        index = ch2;
      }
      ch2 = 2 * (index + 1);
      ch1 = ch2 - 1;
    }
    if (ch1 < n && v[index] != top_of(v[index], v[ch1])) {
      std::swap(v[index], v[ch1]);
    }
  }

  void push(const T& elem) {
    v.push_back(elem);
    if (U) {
      push_heap(v.begin(), v.end(), std::less<T>());
    } else {
      push_heap(v.begin(), v.end(), std::greater<T>());
    }
  }

  void pop() {
    if (U)
      pop_heap(v.begin(), v.end(), std::less<T>());
    else
      pop_heap(v.begin(), v.end(), std::greater<T>());
    v.pop_back();
  }
  void remove(const T& elem) {
    // std::cout << "Removing: " << elem << '\n';
    auto it = find(v.begin(), v.end(), elem);
    if (it == v.end())
      assert(0);

    int n = v.size();
    int index, ch1, ch2;

    int parent;
    index = it - v.begin();
    while (index != 0) {
      parent = (index - 1) / 2;
      v[index] = v[parent];
      index = parent;
    }
    // The deleted element will be at top. Heap property is lost.
    // std::swap(v[index], v[n - 1]);
    // v.pop_back();
  }
};

template <typename T>
using MinHeap = Heap<T, false>;
template <typename T>
using MaxHeap = Heap<T, true>;

}  // namespace heap
