#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <sstream>
#include <cstring>
#include <concepts>

template<typename T>
concept HasModule = requires(T a) {
    { a.module() } -> std::convertible_to<double>;
};

namespace linalg {

template<typename T>
class Matrix {
    public:

    template<typename U>
    friend class Matrix;

    template<typename U, typename Y> 
    friend bool operator==(const Matrix<U>& first, const Matrix<Y>& second) noexcept;

    Matrix& operator=(const Matrix& other) { return operator=<T>(other); }

    Matrix() = default;

    Matrix(size_t rows);

    Matrix(size_t rows, size_t columns);

    Matrix(const volatile Matrix&) = delete;

    template<typename U>
    Matrix(const Matrix<U>& other);

    template<typename U>
    Matrix(Matrix<U>&& other) noexcept;

    template<typename U>
    Matrix(std::initializer_list<U> lst);

    template<typename U>
    Matrix(std::initializer_list<std::initializer_list<U>> lst);

    ~Matrix() noexcept;

    template<typename U>
    Matrix& operator=(const Matrix<U>& other);

    template<typename U>
    Matrix& operator=(Matrix<U>&& other) noexcept;

    T& operator()(size_t i, size_t j) noexcept { return m_ptr[i * m_columns + j]; }

    const T& operator()(size_t i, size_t j) const noexcept { return m_ptr[i * m_columns + j]; }

    template<typename U>
    Matrix& operator+=(const Matrix<U>& other);

    template<typename U>
    Matrix& operator-=(const Matrix<U>& other);

    template<typename U>
    Matrix& operator*=(const Matrix<U>& other);

    template<typename U>
    Matrix& operator*=(U num) noexcept;

    size_t rows() const noexcept { return m_rows; }

    size_t columns() const noexcept { return m_columns; }

    size_t size() const noexcept { return m_rows * m_columns; }

    size_t capacity() const noexcept { return m_capacity; }

    bool empty() const noexcept { return m_ptr == nullptr; }

    void reshape(size_t rows, size_t columns);

    void reserve(size_t new_cap) noexcept;

    void shrink_to_fit() noexcept;

    void clear() { m_columns = m_rows = 0; }

    void swap(Matrix& other) noexcept;

    T* begin() { return m_ptr; };

    T* end() { return m_ptr + (size()); }

    long double norm() const noexcept;

    T trace() const;

    T det() const;

    Matrix inversed() const;

    Matrix transposed() const;

    private:

    T* m_ptr = nullptr;
    size_t m_rows = 0;
    size_t m_columns = 0;
    size_t m_capacity = 0;

    template<typename U>
    void free(U*& ptr, size_t n) noexcept;

    T recursive_determinant(const T* ptr, size_t n) const;

    template<typename U, typename Y>
    static bool are_equal (const U& a, const Y& b) { return a == b; } 
 
    template<typename U>
    static bool are_equal (const double& a, const U& b, double epsilon = 1e-9) { 
        return std::fabs(a - b) < epsilon; 
    }

    template<typename U>
    static bool are_equal (const U& a, const double& b, double epsilon = 1e-9) { 
        return std::fabs(a - b) < epsilon; 
    }

    static double abs(const HasModule auto& value) {
        return value.module();
    }

    static double abs(const auto& value) {
        return std::abs(value);
    }
};

template<typename T>
void swap(Matrix<T>& first, Matrix<T>& second) noexcept { first.swap(second); }

template<typename T>
Matrix<T>::Matrix(size_t rows) : m_rows(rows), m_columns(1), m_capacity(rows) {
    m_ptr = reinterpret_cast<T*>(new std::byte[m_capacity * sizeof(T)]);

    size_t i = 0;

    try {
        for (; i < rows; ++i) {
            new (m_ptr + i) T();
        }
    } catch (...) {
        free<T>(m_ptr, i + 1);
        this->~Matrix();
        throw;
    }
}

template<typename T>
Matrix<T>::Matrix(size_t rows, size_t columns): m_rows(rows), m_columns(columns), m_capacity(rows * columns) {
    m_ptr = reinterpret_cast<T*>(new std::byte[m_capacity * sizeof(T)]);

    size_t i = 0;

    try {
        for (; i < rows * columns; ++i) {
            new (m_ptr + i) T();
        }
    } catch (...) {
        free<T>(m_ptr, i + 1);
        this->~Matrix();
        throw;
    }
}

template<typename T>
template<typename U>
Matrix<T>::Matrix(const Matrix<U>& other) : m_rows(other.m_rows), m_columns(other.m_columns), m_capacity(m_rows * m_columns) {
    m_ptr = reinterpret_cast<T*>(new std::byte[m_capacity * sizeof(T)]);

    size_t i = 0;

    try {
        for (0; i < m_rows * m_columns; ++i) {
            new (m_ptr + i) T((other.m_ptr[i]));
        }
    } catch(...) {
        free<T>(m_ptr, i + 1);
        this->~Matrix();
        throw;
    }
}

template<typename T>
template<typename U>
Matrix<T>::Matrix(Matrix<U>&& other) noexcept : m_ptr(reinterpret_cast<T*>(other.m_ptr)), m_rows(other.m_rows), m_columns(other.m_columns), m_capacity(other.m_capacity) {  
    other.m_ptr = nullptr;
    other.m_columns = other.m_rows = other.m_capacity = 0;
}

template<typename T>
template<typename U>
Matrix<T>::Matrix(std::initializer_list<U> lst) : m_rows(lst.size()), m_columns(1), m_capacity(lst.size()) {
    m_ptr = reinterpret_cast<T*>(new std::byte[m_capacity * sizeof(T)]);

    size_t i = 0;

    try {
        auto iter = lst.begin();
        for (; i < lst.size(); ++i) {
            new (m_ptr + i) T(*iter);
            ++iter;
        }
    } catch(...) {
        free<T>(m_ptr, i + 1);
        this->~Matrix();
        throw;
    }
}

template<typename T>
template<typename U> 
Matrix<T>::Matrix(std::initializer_list<std::initializer_list<U>> lst) : m_rows(lst.size()), m_columns(lst.begin()->size()), m_capacity(m_rows * m_columns) {
    m_ptr = reinterpret_cast<T*>(new std::byte[m_capacity * sizeof(T)]);

    size_t i = 0;

    try {
        for (const auto& row : lst) {
            for (auto& elem : row) {
                new (m_ptr + i) T(elem);
                ++i;
            }
        }
    } catch(...) {
        free<T>(m_ptr, i + 1);
        this->~Matrix();
        throw;
    }
}    

template<typename T>
Matrix<T>::~Matrix() noexcept { 
    if (m_ptr != nullptr) {
        free<T>(m_ptr, size());
    }
}

template<typename T>
template<typename U>
Matrix<T>& Matrix<T>::operator=(const Matrix<U>& other) {
    if (&other == this) {
        return *this;
    }

    if (m_capacity >= other.size()) {
        try { 
            for (size_t i = 0; i < size(); ++i) {
                m_ptr[i] = other.m_ptr[i];
            }
            for (size_t i = size(); i < other.size(); ++i) {
                new (m_ptr + i) T(other.m_ptr[i]);
            }
        } catch(...) {
            throw;
        }
        for (size_t i = other.size(); i < size(); ++i) {
            (m_ptr + i)->~T();
        }
        m_columns = other.m_columns;
        m_rows = other.m_rows;
        m_capacity = size();
        return *this;
    }

    try {
        Matrix<T> copy(other);
        ::linalg::swap(copy, *this);
    } catch (...) {
        throw;
    }

    return *this;
}

template<typename T>
template<typename U>
Matrix<T>& Matrix<T>::operator=(Matrix<U>&& other) noexcept {
    if (&other == this) {
        return *this;
    }

    for (size_t i = 0; i < size(); ++i) {
        (m_ptr + i)->~T();
    }

    delete[] reinterpret_cast<std::byte*>(m_ptr);
    m_rows = other.m_rows;
    m_columns = other.m_columns;
    m_ptr = reinterpret_cast<T*>(other.m_ptr);

    other.m_ptr = nullptr;
    other.m_columns = other.m_rows = 0;

    return *this;
}

template<typename T>
template<typename U>
Matrix<T>& Matrix<T>::operator+=(const Matrix<U>& other) {
    if (m_columns != other.m_columns || m_rows != other.m_rows) {
        throw std::runtime_error("Incompatible sizes");
    }

    for (size_t i = 0; i < size(); ++i) {
        m_ptr[i] += other.m_ptr[i];
    }

    return *this;
}

template<typename T>
template<typename U>
Matrix<T>& Matrix<T>::operator-=(const Matrix<U>& other) {
    if (m_columns != other.m_columns || m_rows != other.m_rows) {
        throw std::runtime_error("Incompatible sizes");
    }

    for (size_t i = 0; i < size(); ++i) {
        m_ptr[i] -= other.m_ptr[i];
    }

    return *this;
}

template<typename T>
template<typename U>
Matrix<T>& Matrix<T>::operator*=(const Matrix<U>& other) {
    if (m_columns != other.m_rows) {
        throw std::runtime_error("Incompatible sizes");
    }

    Matrix<T> res;
    try {
        res = Matrix<T>(m_rows, other.m_columns);
    } catch(...) {
        throw;
    }    

    for (size_t i = 0; i < m_rows; ++i) {
        for (size_t j = 0; j < other.m_columns; ++j) {
            for (size_t k = 0; k < m_columns; ++k) {
                res(i,j) += (*this)(i, k) * other(k,j);
            }
        }
    }

    *this = std::move(res);
    return *this;
}

template<typename T>
template<typename U>
Matrix<T>& Matrix<T>::operator*=(U num) noexcept {
    for (size_t i = 0; i < size(); ++i) {
        m_ptr[i] *= num;
    }

    return *this;
}

template<typename T>
void Matrix<T>::reshape(size_t rows, size_t columns) {
    if (rows * columns < size()) {
        for (size_t i = rows * columns; i < size(); ++i) {
            (m_ptr + i)->~T();
        }
    } else if (rows * columns > size()) {
        reserve(rows * columns);
        try {
            for (size_t i = size(); i < rows * columns; ++i) {
                new (m_ptr + i) T();
            }
        } catch (...) {
            this->~Matrix();
            throw;
        }
    }

    m_rows = rows;
    m_columns = columns;
}

template<typename T>
void Matrix<T>::reserve(size_t new_cap) noexcept {
    if (new_cap <= m_capacity) {
        return;
    }

    m_capacity = new_cap;
    T* new_ptr = reinterpret_cast<T*>(new std::byte[new_cap * sizeof(T)]);
    std::memcpy(new_ptr, m_ptr, size() * sizeof(T));
    delete[] reinterpret_cast<std::byte*>(m_ptr);
    m_ptr = new_ptr;
}

template<typename T>
void Matrix<T>::shrink_to_fit() noexcept {
    if (m_capacity == size()) {
        return;
    }

    m_capacity = size();
    T* new_ptr = reinterpret_cast<T*>(new std::byte[size() * sizeof(T)]);
    std::memcpy(new_ptr, m_ptr, size() * sizeof(T));
    delete[] reinterpret_cast<std::byte*>(m_ptr);
    m_ptr = new_ptr;
}


template<typename T>
void Matrix<T>::swap(Matrix<T>& other) noexcept {
    std::swap(m_rows, other.m_rows);
    std::swap(m_columns, other.m_columns);
    std::swap(m_ptr, other.m_ptr);
    std::swap(m_capacity, other.m_capacity);
}

template<typename T>
long double Matrix<T>::norm() const noexcept {
    double res = 0;
    for (size_t i = 0; i < size(); ++i) {
        res += m_ptr[i] * m_ptr[i];
    }

    return pow(res, 0.5);
}

template<typename T>
T Matrix<T>::trace() const {
    if (m_columns != m_rows || m_columns * m_rows == 0) {
        throw std::runtime_error("Not a square matrix");
    }

    T res = 0;
    
    for (size_t i = 0; i < m_rows; ++i) {
        res += m_ptr[i * m_columns + i];
    }
    
    return res;
}

template<typename T>
template<typename U>
void Matrix<T>::free(U*& ptr, size_t n) noexcept {
    for (size_t i = 0; i < n; ++i) {
        (ptr + i)->~U();
    }
    delete[] reinterpret_cast<std::byte*>(ptr);
    ptr = nullptr;
}

template<typename T>
T Matrix<T>::det() const {
    if (m_columns != m_rows || m_columns * m_rows == 0) {
        throw std::runtime_error("Not a square matrix");
    }
    return Matrix::recursive_determinant(m_ptr, m_rows);
}

template<typename T>
T Matrix<T>::recursive_determinant(const T* ptr, size_t n) const {
    if (n == 1) {
        return ptr[0];
    }

    T det = 0;

    try {
        std::vector<T> submatrix((n - 1) * (n - 1));
        for (int k = 0; k < n; ++k) {
            for (int i = 1; i < n; ++i) {
                int sub_i = i - 1;
                for (int j = 0, sub_j = 0; j < n; ++j) {
                    if (j == k) continue;
                    submatrix[sub_i * (n - 1) + sub_j] = ptr[i * n + j];
                    ++sub_j;
                }
            }

            T sign = (k % 2 == 0) ? 1 : -1;

            det += sign * ptr[k] * recursive_determinant(submatrix.data(), n - 1);
        }
    } catch(...) {
        throw;
    }

    return det;
}

template <typename T>
Matrix<T> Matrix<T>::inversed() const {
    size_t n = m_rows;
    if (n != m_columns) {
        throw std::runtime_error("Matrix is not square, cannot compute inverse.");
    }

    T det = this->det();
    if (det == T(0)) {
        throw std::runtime_error("Matrix is singular, cannot compute inverse.");
    }

    Matrix<T> augmented;
    try {
        augmented = Matrix<T>(n, 2 * n);
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                augmented(i, j) = operator()(i, j);
                augmented(i, j + n) = (i == j) ? T(1) : T(0); 
            }
        }
    } catch(...) {
        throw;
    }

    for (int i = 0; i < n; ++i) {
        int max_row = i;
        for (int j = i + 1; j < n; ++j) {
            if (abs(augmented(j, i)) > abs(augmented(max_row, i))) {
                max_row = j;
            }
        }

        for (int j = 0; j < 2 * n; ++j) {
            std::swap(augmented(i, j), augmented(max_row, j));
        }

        T pivot = augmented(i, i);
        if (pivot == T(0)) {
            throw std::invalid_argument("Matrix is singular, cannot compute inverse.");
        }

        for (int j = 0; j < 2 * n; ++j) {
            augmented(i, j) /= pivot;
        }

        for (int k = 0; k < n; ++k) {
            if (k != i) {
                T factor = augmented(k, i);
                for (int j = 0; j < 2 * n; ++j) {
                    augmented(k, j) -= factor * augmented(i, j);
                }
            }
        }
    }


    Matrix<T> inv;
    try {
        inv = Matrix<T>(n, n);
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                inv(i, j) = augmented(i, j + n);
            }
        }
    } catch(...) {
        throw;
    }

    return inv;
}

template <typename T>
Matrix<T> Matrix<T>::transposed() const {
    Matrix<T> transposed;

    try {
        transposed = Matrix<T>(m_columns, m_rows);

        for (size_t i = 0; i < m_rows; ++i) {
            for (size_t j = 0; j < m_columns; ++j) {
                transposed(j, i) = operator()(i, j);
            }
        }
    } catch(...) {
        throw;
    }

    return transposed;
}    

template<typename T, typename U> 
Matrix<std::common_type_t<T, U>> operator+(const Matrix<T>& first, const Matrix<U>& second) { 
    Matrix<std::common_type_t<T, U>> new_matr;
    try {
        new_matr = Matrix<std::common_type_t<T, U>>(first);
    } catch(...) {
        throw;
    }
    new_matr += second;
    return new_matr; 
}

template<typename T, typename U> 
Matrix<std::common_type_t<T, U>> operator-(const Matrix<T>& first, const Matrix<U>& second) { 
    Matrix<std::common_type_t<T, U>> new_matr;
    try {
        new_matr = Matrix<std::common_type_t<T, U>>(first);
    } catch(...) {
        throw;
    }
    new_matr -= second;
    return new_matr; 
}

template<typename T, typename U> 
Matrix<std::common_type_t<T, U>> operator*(const Matrix<T>& first, const Matrix<U>& second) { 
    Matrix<std::common_type_t<T, U>> new_matr;
    try {
        new_matr = Matrix<std::common_type_t<T, U>>(first);
    } catch(...) {
        throw;
    }
    new_matr *= second;
    return new_matr; 
}

template<typename T, typename U> 
Matrix<U> operator*(T num, const Matrix<U>& first) { 
    Matrix<std::common_type_t<T, U>> new_matr;
    try {
        new_matr = Matrix<std::common_type_t<T, U>>(first);
    } catch(...) {
        throw;
    }
    new_matr *= num;
    return new_matr; 
}

template<typename T, typename U> 
Matrix<T> operator*(const Matrix<T>& first, U num) {
    Matrix<std::common_type_t<T, U>> new_matr;
    try {
        new_matr = Matrix<std::common_type_t<T, U>>(first);
    } catch(...) {
        throw;
    }
    new_matr *= num;
    return new_matr;
}

template<typename T, typename U> 
bool operator==(const Matrix<T>& first, const Matrix<U>& second) noexcept {
    if (first.columns() != second.columns() || first.rows() != second.rows()) {
        return false;
    }

    for (size_t i = 0; i < first.rows(); ++i) {
        for(size_t j = 0; j < first.columns(); ++j) {
            if (!Matrix<T>::template are_equal<T, U>(first(i, j), second(i, j))) {
                return false;
            }
        }
    }

    return true;
}

template<typename T, typename U> 
bool operator!=(const Matrix<T>& first, const Matrix<U>& second) noexcept {
    return !(first == second);
}

template<typename T> 
std::ostream& operator<<(std::ostream& stream, const Matrix<T>& matr) noexcept {
    size_t rows = matr.rows();
    if (rows == 0) {
        return stream;
    }

    size_t cols = matr.columns();
    std::vector<int> col_widths(cols, 0);
    std::vector<std::vector<int>> all_widths(rows, std::vector<int>(cols));


    std::stringstream buf;
    std::string f;

    for (size_t j = 0; j < cols; ++j) {
        for (size_t i = 0; i < rows; ++i) {
            buf << matr(i, j);
            buf >> f;
            buf.clear();
            int width = f.size();
            all_widths[i][j] = width;
            if (width > col_widths[j]) {
                col_widths[j] = width;
            }
        }
    }

    for (size_t i = 0; i < rows; ++i) {
        stream << '|';
        for (size_t j = 0; j < cols; ++j) {
            stream << std::string(col_widths[j] - all_widths[i][j], ' ') << matr(i, j);
            if (j < cols - 1) {
                stream << ' ';
            }
        }
        stream << '|' << '\n';
    }

    return stream;
}

} // namespace linalg