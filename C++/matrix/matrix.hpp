#pragma once

#include <iostream>
#include <vector>

template <size_t N, size_t M, typename T = int64_t>

class Matrix {
 public:
  Matrix() : Matrix(T()) {}

  Matrix(const std::vector<std::vector<T>>& input_arr) : arr_(input_arr) {}

  Matrix(const T& elem) : arr_(N, std::vector<T>(M, elem)) {}

  Matrix<N, M, T>& operator+=(const Matrix<N, M, T>& other);

  Matrix<N, M, T>& operator-=(const Matrix<N, M, T>& other);

  T& operator()(size_t index1, size_t index2) { return arr_[index1][index2]; }

  const T& operator()(size_t index1, size_t index2) const {
    return arr_[index1][index2];
  }

  Matrix<M, N, T> Transposed();

  T Trace();

 private:
  std::vector<std::vector<T>> arr_;
};

template <size_t N, size_t M, typename T>
Matrix<N, M, T>& Matrix<N, M, T>::operator+=(const Matrix<N, M, T>& other) {
  for (size_t count1 = 0; count1 < N; ++count1) {
    for (size_t count2 = 0; count2 < M; ++count2) {
      arr_[count1][count2] += other.arr_[count1][count2];
    }
  }
  return *this;
}

template <size_t N, size_t M, typename T>
Matrix<N, M, T>& Matrix<N, M, T>::operator-=(const Matrix<N, M, T>& other) {
  for (size_t count1 = 0; count1 < N; ++count1) {
    for (size_t count2 = 0; count2 < M; ++count2) {
      arr_[count1][count2] -= other.arr_[count1][count2];
    }
  }
  return *this;
}

template <size_t N, size_t M, typename T>
Matrix<M, N, T> Matrix<N, M, T>::Transposed() {
  Matrix<M, N, T> new_matrix;
  for (size_t count1 = 0; count1 < M; ++count1) {
    for (size_t count2 = 0; count2 < N; ++count2) {
      new_matrix(count1, count2) = arr_[count2][count1];
    }
  }
  return new_matrix;
}

template <size_t N, size_t M, typename T>
T Matrix<N, M, T>::Trace() {
  static_assert(N == M, "Not square");
  T res = T();
  for (size_t count = 0; count < N; ++count) {
    res += arr_[count][count];
  }
  return res;
}

template <size_t N, size_t M, typename T = int64_t>
Matrix<N, M, T> operator*(const Matrix<N, M, T>& matrix, const T& elem) {
  Matrix<N, M, T> new_matrix = matrix;
  for (size_t count1 = 0; count1 < N; ++count1) {
    for (size_t count2 = 0; count2 < M; ++count2) {
      new_matrix(count1, count2) *= elem;
    }
  }
  return new_matrix;
}

template <size_t N, size_t M, typename T = int64_t>
Matrix<N, M, T> operator+(const Matrix<N, M, T>& first,
                          const Matrix<N, M, T>& second) {
  Matrix<N, M, T> new_matrix = first;
  new_matrix += second;
  return new_matrix;
}

template <size_t N, size_t M, typename T = int64_t>
Matrix<N, M, T> operator-(const Matrix<N, M, T>& first,
                          const Matrix<N, M, T>& second) {
  Matrix<N, M, T> new_matrix = first;
  new_matrix -= second;
  return new_matrix;
}

template <size_t N, size_t M, size_t R, typename T = int64_t>
Matrix<N, R, T> operator*(const Matrix<N, M, T>& first,
                          const Matrix<M, R, T>& second) {
  Matrix<N, R, T> new_matrix;
  for (size_t count1 = 0; count1 < N; ++count1) {
    for (size_t count2 = 0; count2 < R; ++count2) {
      new_matrix(count1, count2) = Sum(first, second, count1, count2);
    }
  }
  return new_matrix;
}

template <size_t N, size_t M, size_t R, typename T = int64_t>
T Sum(const Matrix<N, M, T>& first, const Matrix<M, R, T>& second, size_t ind1,
      size_t ind2) {
  T res = T();
  for (size_t count1 = 0; count1 < M; ++count1) {
    res += first(ind1, count1) * second(count1, ind2);
  }
  return res;
}

template <size_t N, size_t M, typename T = int64_t>
bool operator==(const Matrix<N, M, T>& first, const Matrix<N, M, T>& second) {
  for (size_t count1 = 0; count1 < N; ++count1) {
    for (size_t count2 = 0; count2 < M; ++count2) {
      if (first(count1, count2) != second(count1, count2)) {
        return false;
      }
    }
  }
  return true;
}