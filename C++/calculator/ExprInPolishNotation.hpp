#pragma once

#include <map>
#include <stack>
#include <string>
#include <vector>

#include "AbstractToken.hpp"
#include "BracketToken.hpp"
#include "InvalidExpr.hpp"
#include "OperandToken.hpp"
#include "OperatorToken.hpp"

template <typename T>
class ExprInPolishNotation {
 public:
  ExprInPolishNotation(const std::string& raw_input);

  const std::vector<AbstractToken*>& GetTokens() { return arr_; }

 private:
  void EndNotation(std::stack<char>& stack);

  void HandleCurPos(std::stack<char>& stack, const std::string& input,
                    size_t& pos);

  void Clear();

  std::string ClearInput(const std::string& input);

  T StrToNum(const std::string& input, size_t& pos);

  AbstractToken* Convert(char letter);

  std::vector<AbstractToken*> arr_;

  enum { ZEROPRIOR = 0, FIRSTPRIOR = 1, SECONDPRIOR = 2, THIRDPRIOR = 3 };
  std::map<char, int> priors_ = {{'(', ZEROPRIOR},   {'+', FIRSTPRIOR},
                                 {'-', FIRSTPRIOR},  {'*', SECONDPRIOR},
                                 {'/', SECONDPRIOR}, {'~', THIRDPRIOR}};
};

template <typename T>
ExprInPolishNotation<T>::ExprInPolishNotation(const std::string& raw_input) {
  std::string input = ClearInput(raw_input);
  std::stack<char> stack;
  for (size_t pos = 0; pos < input.length(); ++pos) {
    HandleCurPos(stack, input, pos);
  }
  EndNotation(stack);
}

template <typename T>
void ExprInPolishNotation<T>::EndNotation(std::stack<char>& stack) {
  while (!stack.empty()) {
    if (stack.top() == '(') {
      Clear();
      throw InvalidExpr();
    }
    arr_.push_back(Convert(stack.top()));
    stack.pop();
  }
}

template <typename T>
void ExprInPolishNotation<T>::HandleCurPos(std::stack<char>& stack,
                                           const std::string& input,
                                           size_t& pos) {
  char cur = input[pos];
  if (isdigit(cur) != 0) {
    OperandToken<T>* operand = new OperandToken<T>(StrToNum(input, pos));
    AbstractToken* token = operand;
    arr_.push_back(token);
  } else if (cur == '(') {
    stack.push(cur);
  } else if (cur == ')') {
    while (!stack.empty() && stack.top() != '(') {
      arr_.push_back(Convert(stack.top()));
      stack.pop();
    }
    if (stack.empty()) {
      Clear();
      throw InvalidExpr();
    }
    stack.pop();
  } else if (priors_.find(input[pos]) != priors_.end()) {
    if (pos == 0 || priors_.find(input[pos - 1]) != priors_.end()) {
      if (cur == '-') {
        cur = '~';
      }
    }
    while (!stack.empty() && (priors_[stack.top()] >= priors_[cur])) {
      arr_.push_back(Convert(stack.top()));
      stack.pop();
    }
    stack.push(cur);
  }
}

template <typename T>
void ExprInPolishNotation<T>::Clear() {
  for (AbstractToken* token : arr_) {
    delete token;
  }
}

template <typename T>
std::string ExprInPolishNotation<T>::ClearInput(const std::string& input) {
  std::string res;
  for (size_t i = 0; i < input.length(); ++i) {
    if ((input[i] == '+') &&
        ((res.empty()) || (priors_.find(res.back()) != priors_.end()))) {
      continue;
    }
    if (input[i] != ' ') {
      res.push_back(input[i]);
    }
  }
  return res;
}

template <typename T>
T ExprInPolishNotation<T>::StrToNum(const std::string& input, size_t& pos) {
  std::string number;
  while ((pos < input.length()) &&
         (isdigit(input[pos]) != 0 || input[pos] == '.')) {
    number += input[pos];
    ++pos;
  }
  --pos;
  T value;
  std::stringstream stream;
  stream << number;
  stream >> value;
  return value;
}

template <typename T>
AbstractToken* ExprInPolishNotation<T>::Convert(char letter) {
  std::string symb{letter};
  if (symb == "(" || symb == ")") {
    BracketToken* token = new BracketToken(symb);
    return token;
  }
  if (symb == "+") {
    OperatorPlusBin<T>* oper = new OperatorPlusBin<T>(symb);
    return oper;
  }
  if (symb == "-") {
    OperatorMinusBin<T>* oper = new OperatorMinusBin<T>(symb);
    return oper;
  }
  if (symb == "~") {
    OperatorMinusUn<T>* oper = new OperatorMinusUn<T>(symb);
    return oper;
  }
  if (symb == "/") {
    OperatorDivide<T>* oper = new OperatorDivide<T>(symb);
    return oper;
  }
  OperatorMultiply<T>* oper = new OperatorMultiply<T>(symb);
  return oper;
}