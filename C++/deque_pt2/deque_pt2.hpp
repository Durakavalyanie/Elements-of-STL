#pragma once
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>

template <typename T, typename Allocator = std::allocator<T>>
class Deque {
 public:
  template <bool IsConst>
  class Iterator;

  using iterator = Deque::Iterator<false>;
  using const_iterator = Deque::Iterator<true>;
  using reverse_iterator = std::reverse_iterator<Deque::Iterator<false>>;
  using const_reverse_iterator = std::reverse_iterator<Deque::Iterator<true>>;

  using value_type = T;
  using allocator_type = Allocator;

  iterator begin() { return iterator(arr_, start_); }

  iterator end() { return iterator(arr_, buckets_ == 0 ? end_ : end_ + 1); }

  const_iterator begin() const { return iterator(arr_, start_); }

  const_iterator end() const {
    return iterator(arr_, buckets_ == 0 ? end_ : end_ + 1);
  }

  const_iterator cbegin() { return const_iterator(begin()); }

  const_iterator cend() { return const_iterator(end()); }

  std::reverse_iterator<iterator> rbegin() { return reverse_iterator(end()); }

  std::reverse_iterator<iterator> rend() { return reverse_iterator(begin()); }

  std::reverse_iterator<const_iterator> crbegin() {
    return const_reverse_iterator(end());
  }

  std::reverse_iterator<const_iterator> crend() {
    return const_reverse_iterator(begin());
  }

  Deque() = default;

  Deque(const Allocator& alloc);

  Deque(const Deque& other);

  Deque(size_t count, const T& value, const Allocator& alloc = Allocator());

  Deque(size_t count, const Allocator& alloc = Allocator());

  Deque(Deque&& other);

  Deque(std::initializer_list<T> init, const Allocator& alloc = Allocator());

  ~Deque();

  allocator_type get_allocator() const { return allocator_; }

  Deque<T, Allocator>& operator=(const Deque& other);

  Deque<T, Allocator>& operator=(Deque&& other);

  size_t size() const;

  bool empty() const;

  T& operator[](size_t index) {
    return arr_[(start_ + index) / kSizeOfBucket]
               [(start_ + index) % kSizeOfBucket];
  };

  const T& operator[](size_t index) const {
    return arr_[(start_ + index) / kSizeOfBucket]
               [(start_ + index) % kSizeOfBucket];
  };

  T& at(size_t index);

  const T& at(size_t index) const;

  void push_back(const T& elem) { emplace_back(elem); }

  void push_back(T&& elem) { emplace_back(std::move(elem)); }

  template <class... Args>
  void emplace_back(Args&&... args);

  void pop_back();

  void push_front(const T& elem) { emplace_front(elem); }

  void push_front(T&& elem) { emplace_front(std::move(elem)); }

  template <class... Args>
  void emplace_front(Args&&... args);

  void pop_front();

  void insert(iterator ins_iter, const T& elem) { emplace(ins_iter, elem); }

  void insert(iterator ins_iter, T&& elem) {
    emplace(ins_iter, std::move(elem));
  }

  template <class... Args>
  void emplace(iterator ins_iter, Args&&... args);

  void erase(iterator er_iter);

 private:
  void first_bucket();

  void realloc();

  using alloc_traits = std::allocator_traits<Allocator>;
  using ptr_alloc = typename alloc_traits::template rebind_alloc<T*>;
  using ptr_alloc_traits = std::allocator_traits<ptr_alloc>;

  Allocator allocator_;
  ptr_alloc ptr_allocator_;
  T** arr_ = nullptr;
  size_t buckets_ = 0;
  const static size_t kSizeOfBucket = 32;
  size_t start_ = 0;
  size_t end_ = 0;
};

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(const Allocator& alloc)
    : allocator_(alloc), ptr_allocator_(alloc) {}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(const Deque& other)
    : allocator_(alloc_traits::select_on_container_copy_construction(
          other.allocator_)),
      ptr_allocator_(allocator_) {
  buckets_ = (other.size() + kSizeOfBucket - 1) / kSizeOfBucket;
  if (buckets_ != 0) {
    arr_ = ptr_alloc_traits::allocate(ptr_allocator_, buckets_);
  }
  T* innovation_bucket;
  try {
    for (size_t i = 0; i < buckets_; ++i) {
      innovation_bucket = alloc_traits::allocate(allocator_, kSizeOfBucket);
      arr_[i] = innovation_bucket;
      try {
        for (size_t j = 0; j < kSizeOfBucket; ++j) {
          if (i * kSizeOfBucket + j == other.size()) {
            break;
          }
          alloc_traits::construct(allocator_, arr_[i] + j,
                                  other[i * kSizeOfBucket + j]);
          ++end_;
        }
      } catch (...) {
        --end_;
        this->~Deque();
        throw;
      }
    }
  } catch (...) {
    this->~Deque();
    throw;
  }
  if (end_ > 0) {
    --end_;
  }
  start_ = 0;
};

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(size_t count, const T& value, const Allocator& alloc)
    : allocator_(alloc), ptr_allocator_(alloc) {
  buckets_ = (count + kSizeOfBucket - 1) / kSizeOfBucket;
  arr_ = ptr_alloc_traits::allocate(ptr_allocator_, buckets_);
  T* innovation_bucket;
  try {
    for (size_t i = 0; i < buckets_; ++i) {
      innovation_bucket = alloc_traits::allocate(allocator_, kSizeOfBucket);
      arr_[i] = innovation_bucket;
      try {
        for (size_t j = 0; j < kSizeOfBucket; ++j) {
          if (i * kSizeOfBucket + j == count) {
            break;
          }
          alloc_traits::construct(allocator_, arr_[i] + j, value);
          ++end_;
        }
      } catch (...) {
        --end_;
        this->~Deque();
        throw;
      }
    }
  } catch (...) {
    this->~Deque();
    throw;
  }
  if (end_ > 0) {
    --end_;
  }
  start_ = 0;
};

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(size_t count, const Allocator& alloc)
    : allocator_(alloc), ptr_allocator_(alloc) {
  buckets_ = (count + kSizeOfBucket - 1) / kSizeOfBucket;
  arr_ = ptr_alloc_traits::allocate(ptr_allocator_, buckets_);
  T* innovation_bucket;
  try {
    for (size_t i = 0; i < buckets_; ++i) {
      innovation_bucket = alloc_traits::allocate(allocator_, kSizeOfBucket);
      arr_[i] = innovation_bucket;
      try {
        for (size_t j = 0; j < kSizeOfBucket; ++j) {
          if (i * kSizeOfBucket + j == count) {
            break;
          }
          alloc_traits::construct(allocator_, arr_[i] + j);
          ++end_;
        }
      } catch (...) {
        --end_;
        this->~Deque();
        throw;
      }
    }
  } catch (...) {
    this->~Deque();
    throw;
  }
  if (end_ > 0) {
    --end_;
  }
  start_ = 0;
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(Deque&& other)
    : allocator_(other.allocator_),
      ptr_allocator_(other.ptr_allocator_),
      arr_(other.arr_),
      buckets_(other.buckets_),
      start_(other.start_),
      end_(other.end_) {
  other.arr_ = nullptr;
  other.buckets_ = other.start_ = other.end_ = 0;
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(std::initializer_list<T> init,
                           const Allocator& alloc)
    : allocator_(alloc), ptr_allocator_(alloc) {
  buckets_ = (init.size() + kSizeOfBucket - 1) / kSizeOfBucket;
  arr_ = ptr_alloc_traits::allocate(ptr_allocator_, buckets_);
  T* innovation_bucket;
  auto iter = init.begin();
  try {
    for (size_t i = 0; i < buckets_; ++i) {
      innovation_bucket = alloc_traits::allocate(allocator_, kSizeOfBucket);
      arr_[i] = innovation_bucket;
      try {
        for (size_t j = 0; j < kSizeOfBucket; ++j) {
          if (i * kSizeOfBucket + j == init.size()) {
            break;
          }
          alloc_traits::construct(allocator_, arr_[i] + j, *iter);
          ++end_;
          ++iter;
        }
      } catch (...) {
        --end_;
        this->~Deque();
        throw;
      }
    }
  } catch (...) {
    this->~Deque();
    throw;
  }
  if (end_ > 0) {
    --end_;
  }
  start_ = 0;
}

template <typename T, typename Allocator>
Deque<T, Allocator>::~Deque() {
  bool is_first_bucket = true;
  if (buckets_ == 0) {
    return;
  }
  for (size_t i = 0; i < buckets_; ++i) {
    if (arr_[i] != nullptr) {
      for (size_t j = (is_first_bucket) ? start_ % kSizeOfBucket : 0;
           j < kSizeOfBucket; ++j) {
        if (i * kSizeOfBucket + j > end_) {
          break;
        }
        alloc_traits::destroy(allocator_, arr_[i] + j);
      }
      is_first_bucket = false;
      alloc_traits::deallocate(allocator_, arr_[i], kSizeOfBucket);
    }
  }
  ptr_alloc_traits::deallocate(ptr_allocator_, arr_, buckets_);
  buckets_ = 0;
};

template <typename T, typename Allocator>
Deque<T, Allocator>& Deque<T, Allocator>::operator=(const Deque& other) {
  Deque copy(allocator_);
  if (alloc_traits::propagate_on_container_copy_assignment::value) {
    copy.allocator_ = other.allocator_;
    copy.ptr_allocator_ = other.ptr_allocator_;
  }
  for (const T& elem : other) {
    copy.push_back(elem);
  }
  std::swap(copy, *this);
  return *this;
};

template <typename T, typename Allocator>
Deque<T, Allocator>& Deque<T, Allocator>::operator=(Deque&& other) {
  if (&other == this) {
    return *this;
  }
  for (size_t i = 0; i < buckets_; ++i) {
    if (arr_[i] != nullptr) {
      for (size_t j = 0; j < kSizeOfBucket; ++j) {
        alloc_traits::destroy(allocator_, arr_[i] + j);
      }
      alloc_traits::deallocate(allocator_, arr_[i], kSizeOfBucket);
    }
  }
  ptr_alloc_traits::deallocate(ptr_allocator_, arr_, buckets_);

  if (alloc_traits::propagate_on_container_copy_assignment::value) {
    allocator_ = other.allocator_;
    ptr_allocator_ = other.ptr_allocator_;
  }

  if (allocator_ == other.allocator_) {
    arr_ = other.arr_;
    other.arr_ = nullptr;
    start_ = other.start_;
    end_ = other.end_;
    buckets_ = other.buckets_;
    other.buckets_ = other.start_ = other.end_ = 0;
  } else {
    for (auto& elem : other) {
      emplace_back(std::move(elem));
    }
  }
  return *this;
}

template <typename T, typename Allocator>
size_t Deque<T, Allocator>::size() const {
  if (buckets_ == 0) {
    return 0;
  }
  return end_ - start_ + 1;
};

template <typename T, typename Allocator>
bool Deque<T, Allocator>::empty() const {
  if (buckets_ == 0) {
    return true;
  }
  return (start_ > end_);
};

template <typename T, typename Allocator>
T& Deque<T, Allocator>::at(size_t index) {
  if (index + start_ > end_) {
    throw std::out_of_range("inner mongolia");
  }
  return arr_[(start_ + index) / kSizeOfBucket]
             [(start_ + index) % kSizeOfBucket];
};

template <typename T, typename Allocator>
const T& Deque<T, Allocator>::at(size_t index) const {
  if (index + start_ > end_) {
    throw std::out_of_range("inner mongolia");
  }
  return arr_[(start_ + index) / kSizeOfBucket]
             [(start_ + index) % kSizeOfBucket];
};

template <typename T, typename Allocator>
template <class... Args>
void Deque<T, Allocator>::emplace_back(Args&&... args) {
  if (buckets_ == 0) {
    first_bucket();
    try {
      alloc_traits::construct(allocator_,
                              arr_[end_ / kSizeOfBucket] + end_ % kSizeOfBucket,
                              std::forward<Args>(args)...);
    } catch (...) {
      this->~Deque();
      throw;
    }
    return;
  }
  if ((end_ + 1) % kSizeOfBucket == 0) {
    if (end_ + 1 == buckets_ * kSizeOfBucket) {
      realloc();
    }
    if (arr_[(end_ + 1) / kSizeOfBucket] == nullptr) {
      T* innovation_bucket = nullptr;
      try {
        innovation_bucket = alloc_traits::allocate(allocator_, kSizeOfBucket);
        arr_[(end_ + 1) / kSizeOfBucket] = innovation_bucket;
      } catch (...) {
        alloc_traits::deallocate(allocator_, innovation_bucket, kSizeOfBucket);
        throw;
      }
    }
  }
  ++end_;
  try {
    alloc_traits::construct(allocator_,
                            arr_[end_ / kSizeOfBucket] + end_ % kSizeOfBucket,
                            std::forward<Args>(args)...);
  } catch (...) {
    --end_;
    throw;
  }
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::pop_back() {
  alloc_traits::destroy(allocator_,
                        arr_[end_ / kSizeOfBucket] + end_ % kSizeOfBucket);
  --end_;
}

template <typename T, typename Allocator>
template <class... Args>
void Deque<T, Allocator>::emplace_front(Args&&... args) {
  if (buckets_ == 0) {
    first_bucket();
    --end_;
  }
  if (start_ % kSizeOfBucket == 0) {
    if (start_ == 0) {
      realloc();
    }
    if (arr_[(start_ - 1) / kSizeOfBucket] == nullptr) {
      T* innovation_bucket = nullptr;
      try {
        innovation_bucket = alloc_traits::allocate(allocator_, kSizeOfBucket);
        arr_[(start_ - 1) / kSizeOfBucket] = innovation_bucket;
      } catch (...) {
        alloc_traits::deallocate(allocator_, innovation_bucket, kSizeOfBucket);
        throw;
      }
    }
  }
  --start_;
  try {
    alloc_traits::construct(
        allocator_, arr_[start_ / kSizeOfBucket] + start_ % kSizeOfBucket,
        std::forward<Args>(args)...);
  } catch (...) {
    ++start_;
    throw;
  }
};

template <typename T, typename Allocator>
void Deque<T, Allocator>::pop_front() {
  alloc_traits::destroy(allocator_,
                        arr_[start_ / kSizeOfBucket] + start_ % kSizeOfBucket);
  ++start_;
}

template <typename T, typename Allocator>
template <class... Args>
void Deque<T, Allocator>::emplace(iterator ins_iter, Args&&... args) {
  size_t dif = end() - ins_iter;
  emplace_back(args...);
  for (iterator iter = end() - 1; dif > 0; --iter, --dif) {
    std::swap(*iter, *(iter - 1));
  }
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::erase(iterator er_iter) {
  for (iterator iter = er_iter; iter < end(); ++iter) {
    std::swap(*iter, *(iter + 1));
  }
  pop_back();
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::first_bucket() {
  buckets_ = 1;
  arr_ = ptr_alloc_traits::allocate(ptr_allocator_, buckets_);
  T* innovation_bucket = alloc_traits::allocate(allocator_, kSizeOfBucket);
  arr_[0] = innovation_bucket;
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::realloc() {
  T** new_arr = ptr_alloc_traits::allocate(ptr_allocator_, buckets_ * 3);
  for (size_t i = 0; i < 3 * buckets_; ++i) {
    if (i >= buckets_ && i < 2 * buckets_) {
      new_arr[i] = arr_[i - buckets_];
    } else {
      new_arr[i] = nullptr;
    }
  }
  ptr_alloc_traits::deallocate(ptr_allocator_, arr_, buckets_);
  arr_ = new_arr;
  start_ += buckets_ * kSizeOfBucket;
  end_ += buckets_ * kSizeOfBucket;
  buckets_ *= 3;
}

template <typename T, typename Allocator>
template <bool IsConst>
class Deque<T, Allocator>::Iterator {
 public:
  using iterator_category = std::random_access_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using cond_type = std::conditional_t<IsConst, const T, T>;
  using value_type = cond_type;
  using pointer = cond_type*;
  using reference = cond_type&;

  Iterator(T** arr, size_t pos) : arr_(arr), pos_(pos) {}

  Iterator(const Iterator&) = default;

  Iterator& operator=(const Iterator&) = default;

  operator Iterator<true>() const { return Iterator<true>(arr_, pos_); }

  reference operator*() const {
    return arr_[pos_ / kSizeOfBucket][pos_ % kSizeOfBucket];
  }

  pointer operator->() const {
    return arr_[(pos_) / kSizeOfBucket] + pos_ % kSizeOfBucket;
  }

  Iterator& operator-=(int n) {
    pos_ -= n;
    return *this;
  }

  Iterator& operator+=(int n) {
    pos_ += n;
    return *this;
  }

  Iterator operator-(int n) const {
    auto copy = *this;
    copy.pos_ -= n;

    return copy;
  }

  Iterator operator+(int n) const {
    auto copy = *this;
    copy.pos_ += n;

    return copy;
  }

  Iterator operator++(int) {
    Iterator tmp = *this;
    ++(this->pos_);
    return tmp;
  }

  Iterator& operator++() {
    pos_++;
    return *this;
  }

  Iterator operator--(int) {
    Iterator tmp = *this;
    --(this->pos_);
    return tmp;
  }

  Iterator& operator--() {
    pos_--;
    return *this;
  }

  bool operator==(const Iterator& second) const { return pos_ == second.pos_; }

  bool operator!=(const Iterator& second) const { return pos_ != second.pos_; }

  bool operator<(const Iterator& second) const { return pos_ < second.pos_; }

  bool operator>(const Iterator& second) const { return pos_ > second.pos_; }

  difference_type operator-(Iterator second) const {
    return pos_ - second.pos_;
  }

  bool operator<=(const Iterator& second) const { return pos_ <= second.pos_; }

  bool operator>=(const Iterator& second) const { return pos_ >= second.pos_; }

 private:
  T** arr_ = nullptr;
  size_t pos_ = 0;
};