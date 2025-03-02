#pragma once

#include <stack>
#include <string>
#include <vector>

#include "AbstractToken.hpp"
#include "BracketToken.hpp"
#include "ExprInPolishNotation.hpp"
#include "InvalidExpr.hpp"
#include "OperandToken.hpp"
#include "OperatorToken.hpp"

template <typename T>
class Calculator {
 public:
  static T CalculateExpr(const std::string& expr);

 private:
  void static Clear(
      std::stack<AbstractToken*, std::vector<AbstractToken*>>& stack);

  static OperandToken<T>* GetOperand(
      std::stack<AbstractToken*, std::vector<AbstractToken*>>& stack);
};

template <typename T>
T Calculator<T>::CalculateExpr(const std::string& expr) {
  ExprInPolishNotation<T> notation(expr);
  std::vector<AbstractToken*> arr = notation.GetTokens();
  std::stack<AbstractToken*, std::vector<AbstractToken*>> stack(arr);
  OperandToken<T>* operand = GetOperand(stack);
  if (!stack.empty()) {
    Clear(stack);
    delete operand;
    throw InvalidExpr();
  }
  T res = operand->GetValue();
  delete operand;
  return res;
}

template <typename T>
void Calculator<T>::Clear(
    std::stack<AbstractToken*, std::vector<AbstractToken*>>& stack) {
  while (!stack.empty()) {
    delete stack.top();
    stack.pop();
  }
}

template <typename T>
OperandToken<T>* Calculator<T>::GetOperand(
    std::stack<AbstractToken*, std::vector<AbstractToken*>>& stack) {
  if (stack.empty()) {
    throw InvalidExpr();
  }
  AbstractToken* token = stack.top();
  stack.pop();
  OperandToken<T>* operand = dynamic_cast<OperandToken<T>*>(token);
  if (operand != nullptr) {
    return operand;
  }
  OperandToken<T>* res;
  OperatorToken<T, false>* un_min =
      dynamic_cast<OperatorToken<T, false>*>(token);
  if (un_min != nullptr) {
    res = un_min->Calculate(GetOperand(stack));
    delete un_min;
    return res;
  }
  OperatorToken<T, true>* bin_oper =
      dynamic_cast<OperatorToken<T, true>*>(token);
  OperandToken<T>* first = GetOperand(stack);
  OperandToken<T>* second = GetOperand(stack);
  res = bin_oper->Calculate(first, second);
  delete bin_oper;
  return res;
}