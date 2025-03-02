#pragma once

#include <unistd.h>
#include <type_traits>
#include <iostream>

namespace stdlike {

  class ostream {
    public:

      void flush();

      void put(char sym);

      bool fail() const { return fail_; }

      ostream& operator<<(bool b);
      ostream& operator<<(char sym);
      ostream& operator<<(const char* str);

      template <typename T, typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, int>::type = 0>
      ostream& operator<<(T num);

      template <typename T, typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value, int>::type = 0>
      ostream& operator<<(T num);

      template <typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
      ostream& operator<<(T num);
      ostream& operator<<(const void* ptr);

      ostream() = default;
      ~ostream();

    private:

    static const int size_ = 256; 

    char buf_[size_];

    int end_ = 0;

    bool fail_ = false;
  };

class istream {
  public:
    char get();

    char peek();

    bool fail() const { return fail_; }

    istream& operator>>(bool& b);
    istream& operator>>(char& sym);
    istream& operator>>(short& num);
    istream& operator>>(unsigned short& num);
    istream& operator>>(int& num);
    istream& operator>>(unsigned int& num);
    istream& operator>>(long long& num);
    istream& operator>>(unsigned long long& num);
    istream& operator>>(float& num);
    istream& operator>>(double& num);
    istream& operator>>(long double& num);

    istream() = default;
    ~istream() = default;

  private:

    template<typename T>
    T GetInt();

    template<typename T>
    T GetFloat();

    static const int size_ = 256;

    char buf_[size_];

    int readed_ = 1;

    int end_ = size_;

    bool fail_ = false;
  };

template <typename T, typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, int>::type>
ostream& ostream::operator<<(T num) {
    std::make_unsigned_t<T> abs;
    if (num < 0) {
        put('-');
        abs = static_cast<std::make_unsigned_t<T>>(-(num + 1)) + 1;
    } else {
      abs = num;
    }
    return *this << abs;
}

template <typename T, typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value, int>::type>
ostream& ostream::operator<<(T num) {
    char str[20];
    char* cur = str;

    do {
        *cur++ = static_cast<char>('0' + num % 10);
        num /= 10;
    } while (num > 0);

    do {
        put(*--cur);
    } while (cur != str);

    return *this;
}

template <typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type>
ostream& ostream::operator<<(T num) {
    if (num < 0) {
        put('-');
        num = -num;
    }

    int64_t int_part = static_cast<int64_t>(num);
    *this << int_part;

    put('.');

    T frac_part = num - int_part;
    int precision = 12;

    while (precision-- > 0) {
        frac_part *= 10;
        int digit = static_cast<int>(frac_part);
        put('0' + digit);
        frac_part -= digit;
    }

    return *this;
}

extern istream cin;
extern ostream cout;

}  // namespace stdlike