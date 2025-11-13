#ifndef CEL_RULE_H_
#define CEL_RULE_H_

#include <iostream>
#include <string>
#include <memory>   // 包含 std::unique_ptr 和 std::move
#include <chrono>   // 包含 std::chrono::system_clock

// 包含 CEL 可执行表达式的定义
#include "eval/public/cel_expression.h"

/**
 * @brief 封装单个 CEL 规则表达式及其编译状态。
 *
 * 这个类的对象将被 std::shared_ptr 管理。
 */
class CelRule {
  // 授权 RuleManager 在编译后更新此对象的内部状态
  friend class RuleManager;

 public:
  /**
   * @brief 编译状态枚举
   */
  enum CompileStatus {
    NOT_COMPILED,
    COMPILED_OK,
    COMPILE_ERROR
  };

  /**
   * @brief 构造函数，接受 CEL 表达式。
   *
   * 我们在此打印日志，以观察对象的创建。
   * @param expression 规则的 CEL 表达式字符串。
   */
  CelRule(const std::string& expression)
      : expression_(expression), status_(NOT_COMPILED) {
    std::cout << "    [CelRule CONSTRUCTOR]: Rule object created for expression: "
              << expression_ << std::endl;
  }

  /**
   * @brief 析构函数。
   *
   * 我们在此打印日志，以观察对象的销毁。
   * 这将由 std::shared_ptr 在引用计数降为 0 时自动调用。
   */
  ~CelRule() {
    std::cout << "    [CelRule DESTRUCTOR]: Rule object destroyed for expression: "
              << expression_ << std::endl;
  }

  /**
   * @brief 获取规则的原始表达式字符串。
   */
  std::string GetExpression() const { return expression_; }

  /**
   * @brief 检查规则是否已成功编译。
   */
  bool IsCompiled() const { return status_ == COMPILED_OK; }

  /**
   * @brief 获取当前的编译状态。
   */
  CompileStatus GetStatus() const { return status_; }

  /**
   * @brief 获取编译错误信息（如果编译失败）。
   */
  std::string GetCompileError() const { return compile_error_; }

  /**
   * @brief 获取指向已编译的可执行表达式的 const 指针。
   * 如果未编译，则返回 nullptr。
   */
  const google::api::expr::runtime::CelExpression* GetCompiledExpr() const {
    return compiled_expr_.get();
  }

 private:
  /**
   * @brief (供 RuleManager 调用) 设置为编译成功状态。
   */
  void SetCompiled(
      std::unique_ptr<google::api::expr::runtime::CelExpression> expr) {
    compiled_expr_ = std::move(expr);
    status_ = COMPILED_OK;
    compile_error_.clear();
    last_compile_time_ = std::chrono::system_clock::now();
  }

  /**
   * @brief (供 RuleManager 调用) 设置为编译失败状态。
   */
  void SetError(const std::string& error_message) {
    compiled_expr_.reset(); // 释放任何旧的计划
    status_ = COMPILE_ERROR;
    compile_error_ = error_message;
    last_compile_time_ = std::chrono::system_clock::now();
  }

  // 核心字段
  std::string expression_;

  // 编译状态字段
  CompileStatus status_;
  std::string compile_error_;
  std::chrono::system_clock::time_point last_compile_time_;

  // 存储编译后的可执行计划
  std::unique_ptr<google::api::expr::runtime::CelExpression> compiled_expr_;
};

#endif  // CEL_RULE_H_
