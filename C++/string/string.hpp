#pragma once

#include <cstring>
#include <iostream>
#include <vector>

class String {
 public:
  String() = default;

  String(size_t size, char character);

  String(const char* ptr_input);

  String(const String& other);

  String& operator=(const String& other);

  ~String();

  void Clear();

  void PushBack(char character);

  void PopBack();

  void Resize(size_t new_size);

  void Resize(size_t new_size, char character);

  void Reserve(size_t new_cap);

  void ShrinkToFit();

  void Swap(String& other);

  char& operator[](size_t index);

  const char& operator[](size_t index) const;

  char& Front();

  const char& Front() const;

  char& Back();

  const char& Back() const;

  bool Empty() const;

  size_t Size() const;

  size_t Capacity() const;

  const char* Data() const;

  char* Data();

  String& operator+=(const String& other);

  String& operator*=(size_t num);

  std::vector<String> Split(const String& delim = " ");

  String Join(const std::vector<String>& strings) const;

 private:
  void NewCapacity(size_t new_cap);

  size_t size_ = 0;
  char* ptr_ = nullptr;
  size_t capacity_ = 0;
};

bool operator<(const String& first, const String& second);

bool operator==(const String& first, const String& second);

bool operator!=(const String& first, const String& second);

bool operator>(const String& first, const String& second);

bool operator<=(const String& first, const String& second);

bool operator>=(const String& first, const String& second);

String operator+(const String& first, const String& second);

String operator*(const String& str, size_t num);

std::ostream& operator<<(std::ostream& out, const String& str);

std::istream& operator>>(std::istream& input, String& str);