#pragma once
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>

template <typename T>
class Deque {
 public:
  template <bool IsConst>
  class Iterator;

  using iterator = Deque::Iterator<false>;
  using const_iterator = Deque::Iterator<true>;
  using reverse_iterator = std::reverse_iterator<Deque::Iterator<false>>;
  using const_reverse_iterator = std::reverse_iterator<Deque::Iterator<true>>;

  iterator begin() { return iterator(arr_, start_); }

  iterator end() { return iterator(arr_, buckets_ == 0 ? end_ : end_ + 1); }

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

  Deque(const Deque& other);

  Deque(size_t count) : Deque(count, T()){};

  Deque(size_t count, const T& value);

  ~Deque();

  Deque<T>& operator=(const Deque& other);

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

  void push_back(const T& elem);

  void pop_back() { --end_; };

  void push_front(const T& elem);

  void pop_front() { ++start_; };

  void insert(iterator ins_iter, const T& elem);

  void erase(iterator er_iter);

 private:
  void swap(Deque<T>& first, Deque<T>& second);

  void first_bucket();

  void realloc();

  T** arr_ = nullptr;
  size_t buckets_ = 0;
  const static size_t kSizeOfBucket = 32;
  size_t start_ = 0;
  size_t end_ = 0;
};

template <typename T>
Deque<T>::Deque(const Deque& other)
    : arr_(new T*[other.buckets_]),
      buckets_(other.buckets_),
      start_(other.start_),
      end_(other.end_) {
  if (other.buckets_ == 0) {
    return;
  }
  size_t counter;
  try {
    for (counter = other.start_ / kSizeOfBucket;
         counter <= other.end_ / kSizeOfBucket; ++counter) {
      arr_[counter] =
          reinterpret_cast<T*>(new std::byte[kSizeOfBucket * sizeof(T)]);
    }
  } catch (...) {
    this->~Deque();
    throw;
  }
  size_t count;
  try {
    for (count = other.start_; count <= other.end_; ++count) {
      new (arr_[count / kSizeOfBucket] + count % kSizeOfBucket)
          T(other.arr_[count / kSizeOfBucket][count % kSizeOfBucket]);
    }
  } catch (...) {
    size_t count2;
    for (count2 = other.start_; count2 <= count; ++count2) {
      (arr_[count / kSizeOfBucket] + count % kSizeOfBucket)->~T();
    }
    for (size_t k = other.start_ / kSizeOfBucket;
         k <= other.end_ / kSizeOfBucket; ++k) {
      delete[] reinterpret_cast<std::byte*>(arr_[k]);
    }
    delete[] arr_;
    throw;
  }
};

template <typename T>
Deque<T>::Deque(size_t count, const T& value) {
  buckets_ = (count + kSizeOfBucket - 1) / kSizeOfBucket;
  arr_ = new T*[buckets_];
  for (size_t i = 0; i < buckets_; ++i) {
    try {
      arr_[i] = reinterpret_cast<T*>(new std::byte[kSizeOfBucket * sizeof(T)]);
    } catch (...) {
      this->~Deque();
    }
    for (size_t j = 0; j < kSizeOfBucket; ++j) {
      if (i * kSizeOfBucket + j == count) {
        break;
      }
      try {
        new (arr_[i] + j) T(value);
      } catch (...) {
        this->~Deque();
        throw;
      }
    }
  }
  start_ = 0;
  end_ = count - 1;
};

template <typename T>
Deque<T>::~Deque() {
  for (size_t i = 0; i < buckets_; ++i) {
    if (arr_[i] != nullptr) {
      for (size_t j = 0; j < kSizeOfBucket; ++j) {
        (arr_[i] + j)->~T();
      }
      delete[] reinterpret_cast<std::byte*>(arr_[i]);
    }
  }
  delete[] arr_;
};

template <typename T>
Deque<T>& Deque<T>::operator=(const Deque& other) {
  Deque<T> copy(other);
  swap(copy, *this);
  return *this;
};

template <typename T>
size_t Deque<T>::size() const {
  if (buckets_ == 0) {
    return 0;
  }
  return end_ - start_ + 1;
};

template <typename T>
bool Deque<T>::empty() const {
  if (buckets_ == 0) {
    return true;
  }
  return (start_ > end_);
};

template <typename T>
T& Deque<T>::at(size_t index) {
  if (index + start_ > end_) {
    throw std::out_of_range("inner mongolia");
  }
  return arr_[(start_ + index) / kSizeOfBucket]
             [(start_ + index) % kSizeOfBucket];
};

template <typename T>
const T& Deque<T>::at(size_t index) const {
  if (index + start_ > end_) {
    throw std::out_of_range("inner mongolia");
  }
  return arr_[(start_ + index) / kSizeOfBucket]
             [(start_ + index) % kSizeOfBucket];
};

template <typename T>
void Deque<T>::push_back(const T& elem) {
  if (buckets_ == 0) {
    first_bucket();
    try {
      new (arr_[end_ / kSizeOfBucket] + end_ % kSizeOfBucket) T(elem);
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
      try {
        arr_[(end_ + 1) / kSizeOfBucket] =
            reinterpret_cast<T*>(new std::byte[kSizeOfBucket * sizeof(T)]);
      } catch (...) {
        delete[] reinterpret_cast<std::byte*>(arr_[(end_ + 1) / kSizeOfBucket]);
        throw;
      }
    }
  }
  ++end_;
  (arr_[end_ / kSizeOfBucket] + end_ % kSizeOfBucket)->~T();
  try {
    new (arr_[end_ / kSizeOfBucket] + end_ % kSizeOfBucket) T(elem);
  } catch (...) {
    --end_;
    throw;
  }
};

template <typename T>
void Deque<T>::push_front(const T& elem) {
  if (buckets_ == 0) {
    first_bucket();
    --end_;
  }
  if (start_ % kSizeOfBucket == 0) {
    if (start_ == 0) {
      realloc();
    }
    if (arr_[(start_ - 1) / kSizeOfBucket] == nullptr) {
      try {
        arr_[(start_ - 1) / kSizeOfBucket] =
            reinterpret_cast<T*>(new std::byte[kSizeOfBucket * sizeof(T)]);
      } catch (...) {
        delete[] reinterpret_cast<std::byte*>(
            arr_[(start_ - 1) / kSizeOfBucket]);
        throw;
      }
    }
  }
  --start_;
  (arr_[end_ / kSizeOfBucket] + end_ % kSizeOfBucket)->~T();
  try {
    new (arr_[start_ / kSizeOfBucket] + start_ % kSizeOfBucket) T(elem);
  } catch (...) {
    ++start_;
    throw;
  }
};

template <typename T>
void Deque<T>::insert(iterator ins_iter, const T& elem) {
  size_t dif = end() - ins_iter;
  push_back(elem);
  for (iterator iter = end() - 1; dif > 0; --iter, --dif) {
    std::swap(*iter, *(iter - 1));
  }
}

template <typename T>
void Deque<T>::erase(iterator er_iter) {
  for (iterator iter = er_iter; iter < end(); ++iter) {
    std::swap(*iter, *(iter + 1));
  }
  pop_back();
}

template <typename T>
void Deque<T>::swap(Deque<T>& first, Deque<T>& second) {
  std::swap(first.arr_, second.arr_);
  std::swap(first.start_, second.start_);
  std::swap(first.end_, second.end_);
  std::swap(first.buckets_, second.buckets_);
}

template <typename T>
void Deque<T>::first_bucket() {
  buckets_ = 1;
  arr_ = new T*[buckets_];
  arr_[0] = reinterpret_cast<T*>(new std::byte[kSizeOfBucket * sizeof(T)]);
}

template <typename T>
void Deque<T>::realloc() {
  T** new_arr = new T*[buckets_ * 3];
  for (size_t i = 0; i < 3 * buckets_; ++i) {
    if (i >= buckets_ && i < 2 * buckets_) {
      new_arr[i] = arr_[i - buckets_];
    } else {
      new_arr[i] = nullptr;
    }
  }
  delete[] arr_;
  arr_ = new_arr;
  start_ += buckets_ * kSizeOfBucket;
  end_ += buckets_ * kSizeOfBucket;
  buckets_ *= 3;
}

template <typename T>
template <bool IsConst>
class Deque<T>::Iterator {
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