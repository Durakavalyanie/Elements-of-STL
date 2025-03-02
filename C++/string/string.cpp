#include "string.hpp"

String::String(size_t size, char character)
    : size_(size), ptr_(new char[size + 1]), capacity_(size) {
  for (size_t count = 0; count < size; ++count) {
    ptr_[count] = character;
  }
  ptr_[size] = '\0';
}

String::String(const char* ptr_input)
    : size_(strlen(ptr_input)), ptr_(new char[size_ + 1]), capacity_(size_) {
  std::copy(ptr_input, ptr_input + size_, ptr_);
  ptr_[size_] = '\0';
}

String::String(const String& other) : String(other.Data()) {}

String& String::operator=(const String& other) {
  if (&other == this) {
    return *this;
  }
  if (capacity_ >= other.size_) {
    size_ = other.size_;
    std::copy(other.ptr_, other.ptr_ + size_, ptr_);
    ptr_[size_] = '\0';
    return *this;
  }
  delete[] ptr_;
  size_ = other.size_;
  capacity_ = other.capacity_;
  ptr_ = new char[capacity_ + 1];
  std::copy(other.ptr_, other.ptr_ + size_, ptr_);
  ptr_[size_] = '\0';
  return *this;
}

String::~String() { delete[] ptr_; }

void String::Clear() {
  ptr_[0] = '\0';
  size_ = 0;
}

void String::PushBack(char character) {
  if (capacity_ == 0) {
    Reserve(1);
  } else {
    if (size_ == capacity_) {
      Reserve(capacity_ * 2);
    }
  }
  ptr_[size_] = character;
  ++size_;
  ptr_[size_] = '\0';
}

void String::PopBack() {
  if (size_ == 0) {
    return;
  }
  ptr_[size_ - 1] = '\0';
  --size_;
}

void String::Resize(size_t new_size) {
  if (new_size > capacity_) {
    char* new_ptr = new char[new_size + 1];
    std::copy(ptr_, ptr_ + size_, new_ptr);
    delete[] ptr_;
    ptr_ = new_ptr;
    capacity_ = new_size;
    size_ = new_size;
    return;
  }
  size_ = new_size;
}

void String::Resize(size_t new_size, char character) {
  Reserve(new_size);
  for (size_t count = size_; count < new_size; ++count) {
    ptr_[count] = character;
  }
  ptr_[new_size] = '\0';
  size_ = new_size;
}

void String::Reserve(size_t new_cap) {
  if (new_cap > capacity_) {
    NewCapacity(new_cap);
  }
}

void String::ShrinkToFit() {
  if (capacity_ > size_) {
    NewCapacity(size_);
  }
}

void String::NewCapacity(size_t new_cap) {
  char* ptr_new = new char[new_cap + 1];
  std::copy(ptr_, ptr_ + size_, ptr_new);
  delete[] ptr_;
  capacity_ = new_cap;
  ptr_ = ptr_new;
}

void String::Swap(String& other) {
  std::swap(ptr_, other.ptr_);
  std::swap(size_, other.size_);
  std::swap(capacity_, other.capacity_);
}

char& String::operator[](size_t index) { return ptr_[index]; }

const char& String::operator[](size_t index) const { return ptr_[index]; }

char& String::Front() { return ptr_[0]; }

const char& String::Front() const { return ptr_[0]; }

char& String::Back() { return ptr_[size_ - 1]; }

const char& String::Back() const { return ptr_[size_ - 1]; }

bool String::Empty() const { return (size_ == 0); }

size_t String::Size() const { return size_; }

size_t String::Capacity() const { return capacity_; }

const char* String::Data() const { return ptr_; }

char* String::Data() { return ptr_; }

String& String::operator+=(const String& other) {
  for (size_t count = 0; count < other.Size(); ++count) {
    this->PushBack(other[count]);
  }
  return *this;
}

String& String::operator*=(size_t num) {
  if (num == 0) {
    this->Clear();
  } else {
    String temp_str(*this);
    this->Reserve(num * this->Size());
    for (; num > 1; --num) {
      *this += temp_str;
    }
  }
  return *this;
}

std::vector<String> String::Split(const String& delim) {
  std::vector<String> arr;
  if (size_ < delim.Size()) {
    arr.push_back(*this);
    return arr;
  }
  bool flag = true;
  String str_for_arr = "";
  for (size_t count = 0; count < size_ - delim.Size() + 1; ++count) {
    flag = true;
    for (size_t ind = count; ind < delim.Size() + count; ++ind) {
      flag = (ptr_[ind] == delim[ind - count]);
      if (!flag) {
        str_for_arr.PushBack(ptr_[count]);
        break;
      }
    }
    if (flag) {
      arr.push_back(str_for_arr);
      str_for_arr.Clear();
      count += delim.Size() - 1;
    }
  }
  if (!flag) {
    for (size_t count = size_ - delim.Size() + 1; count < size_; ++count) {
      str_for_arr.PushBack(ptr_[count]);
    }
  }
  arr.push_back(str_for_arr);
  return arr;
}

String String::Join(const std::vector<String>& strings) const {
  String res = "";
  if (strings.empty()) {
    return res;
  }
  for (size_t count = 0; count < strings.size() - 1; ++count) {
    res = strings[count] + *this;
  }
  res += *(strings.end() - 1);
  return res;
}

bool operator<(const String& first, const String& second) {
  for (size_t count = 0; count < std::min(first.Size(), second.Size());
       ++count) {
    if (first[count] != second[count]) {
      return (first[count] < second[count]);
    }
  }
  return (first.Size() < second.Size());
}

bool operator==(const String& first, const String& second) {
  if (first.Size() != second.Size()) {
    return false;
  }
  for (size_t count = 0; count < first.Size(); ++count) {
    if (first[count] != second[count]) {
      return false;
    }
  }
  return true;
}

bool operator!=(const String& first, const String& second) {
  return !(first == second);
}

bool operator>(const String& first, const String& second) {
  return (second < first);
}

bool operator<=(const String& first, const String& second) {
  return !(first > second);
}

bool operator>=(const String& first, const String& second) {
  return !(first < second);
}

String operator+(const String& first, const String& second) {
  String new_string;
  new_string += first;
  new_string += second;
  return new_string;
}

String operator*(const String& str, size_t num) {
  String new_str(str);
  new_str *= num;
  return new_str;
}

std::ostream& operator<<(std::ostream& out, const String& str) {
  for (size_t count = 0; count < str.Size(); ++count) {
    out << str[count];
  }
  return out;
}

std::istream& operator>>(std::istream& input, String& str) {
  char buffer;
  input.get(buffer);
  while (buffer == ' ') {
    input.get(buffer);
  }
  while ((buffer != ' ') && (buffer != '\n')) {
    str.PushBack(buffer);
    if (!input.get(buffer)) {
      break;
    }
  }
  return input;
}