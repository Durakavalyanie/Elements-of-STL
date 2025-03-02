#include "iostream.hpp"

namespace stdlike {

ostream cout;
istream cin;

void ostream::flush() {
    if (end_ == 0) {
        return;
    }
    int wrote = write(1, buf_, end_);
    fail_ = (wrote == -1);
    end_ = 0;
}

void ostream::put(char sym) {
    buf_[end_++] = sym;
    if (end_ == size_) {
        flush();
    }
}

ostream& ostream::operator<<(bool b) {
    put(b ? '1' : '0');
    return *this;
}

ostream& ostream::operator<<(char sym) {
    put(sym);
    return *this;
}

ostream& ostream::operator<<(const char* str) {
    for (const char* cur = str; *cur != '\0'; ++cur) {
        put(*cur);
    }
    return *this;
}

ostream& ostream::operator<<(const void* ptr) {
    fail_ = ptr == nullptr;
    if (fail_) {
        return *this;
    }

    u_int64_t address = reinterpret_cast<u_int64_t>(ptr);
    char buffer[20];
    int ind = 0;
    put('0');
    put('x');

    do {
        int digit = address % 16;
        if (digit < 10) {
            buffer[ind++] = '0' + digit;
        } else {
            buffer[ind++] = 'a' + (digit - 10);
        }
        address /= 16;
    } while (address > 0);

    for (int i = ind - 1; i >= 0; i--) {
        put(buffer[i]);
    }
    return *this;
}

ostream::~ostream() {
    flush();
}

char istream::get() {
    if (end_ == readed_) {
        return '\0';
    }

    if (end_ == size_) {
        readed_ = read(0, buf_, size_);
        fail_ = (readed_ == 0);
        end_ = 0;
    }

    return buf_[end_++];
}

char istream::peek() {
    if (end_ == size_) {
        read(0, buf_, size_);
        fail_ = (readed_ == 0);
    }

    return buf_[end_];
}

template <typename T>
T istream::GetInt() {
    u_int64_t res = 0;
    char cur = get();
    for (; cur == ' ' || cur == '\n'; cur = get()) {
    }

    fail_ = ((cur - '0' < 0 || cur - '0' > 9) && cur != '-' && cur != '.');

    if (fail_) {
        return T();
    }

    char int_buffer[20];
    int ind = 0;

    T sign = 1;
    if (cur == '-') {
        sign = -1;
        cur = get();
    }

    for (; cur - '0' <= 9 && cur - '0' >= 0; cur = get()) {
        int_buffer[ind++] = cur;
    }

    for (T x = 1; ind > 0; --ind) {
        res += (int_buffer[ind - 1] - '0') * x;
        if (ind > 1) {
            x *= 10;
        }
    }

    return res * sign;
}

template <typename T>
T istream::GetFloat() {
    T res = GetInt<T>();

    if (fail_) {
        return T();
    }

    long double t = 10;

    T sign = (res < 0) ? -1 : 1;

    char cur = peek();
    if (cur - '0' <= 9 && cur - '0' >= 0) {
        cur = get();
    }

    for (; cur - '0' <= 9 && cur - '0' >= 0; cur = get()) {
        res += sign * (cur - '0') / t;
        t *= 10;
    }

    return res;
}

istream& istream::operator>>(bool& b) {
    cout.flush();
    b = (GetInt<int>() == 0) ? false : true;
    return *this;
}

istream& istream::operator>>(char& sym) {
    cout.flush();
    for (sym = get(); sym == ' ' || sym == '\n'; sym = get()) {
    }
    return *this;
}

istream& istream::operator>>(short& num) {
    cout.flush();
    num = GetInt<short>();
    return *this;
}
istream& istream::operator>>(unsigned short& num) {
    cout.flush();
    num = GetInt<unsigned short>();
    return *this;
}
istream& istream::operator>>(int& num) {
    cout.flush();
    num = GetInt<int>();
    return *this;
}
istream& istream::operator>>(unsigned int& num) {
    cout.flush();
    num = GetInt<unsigned int>();
    return *this;
}

istream& istream::operator>>(long long& num) {
    cout.flush();
    num = GetInt<int64_t>();
    return *this;
}
istream& istream::operator>>(unsigned long long& num) {
    cout.flush();
    num = GetInt<u_int64_t>();
    return *this;
}

istream& istream::operator>>(float& num) {
    cout.flush();
    num = GetFloat<float>();
    return *this;
}
istream& istream::operator>>(double& num) {
    cout.flush();
    num = GetFloat<double>();
    return *this;
}
istream& istream::operator>>(long double& num) {
    cout.flush();
    num = GetFloat<long double>();
    return *this;
}

}  // namespace stdlike