#ifndef CEL_RULE_H_
#define CEL_RULE_H_

#include <iostream>
#include <string>

/**
 * @brief 封装单个 CEL 规则表达式。
 *
 * 这个类的对象将被 std::shared_ptr 管理。
 */
class CelRule {
 public:
  /**
   * @brief 构造函数，接受 CEL 表达式。
   *
   * 我们在此打印日志，以观察对象的创建。
   * @param expression 规则的 CEL 表达式字符串。
   */
  CelRule(const std::string& expression) : expression_(expression) {
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
   * @brief 获取规则的表达式字符串。
   */
  std::string GetExpression() const { return expression_; }

 private:
  std::string expression_;
};

#endif  // CEL_RULE_H_
