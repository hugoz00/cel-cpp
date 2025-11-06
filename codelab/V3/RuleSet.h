#ifndef RULE_SET_H_
#define RULE_SET_H_

#include <iostream>
#include <map>
#include <string>

/**
 * @brief 一个只读的规则集。
 *
 * 这个对象被设计为“不可变的”（Immutable）。
 * 规则的更新是通过创建一个全新的 RuleSet 对象来完成的。
 */
class RuleSet {
 public:
  // 我们使用一个版本号，以便在测试中清晰可见
  RuleSet(int version) : version_(version) {
    std::cout << "    [RuleSet CONSTRUCTOR]: v" << version_
              << " 规则集对象被创建。" << std::endl;
  }

  ~RuleSet() {
    std::cout << "    [RuleSet DESTRUCTOR]: v" << version_
              << " 规则集对象被销毁。" << std::endl;
  }

  // (禁止拷贝和移动，我们只通过 shared_ptr 来管理它)
  RuleSet(const RuleSet&) = delete;
  RuleSet& operator=(const RuleSet&) = delete;

  /**
   * @brief (写操作) 仅在构建时调用：添加规则。
   */
  void AddRule(const std::string& name, const std::string& expression) {
    rules_[name] = expression;
  }

  /**
   * @brief (读操作) 供读取线程调用：获取规则。
   *
   * 这是一个 const 方法，保证不会修改任何成员，是线程安全的。
   */
  std::string GetRule(const std::string& name) const {
    auto it = rules_.find(name);
    if (it != rules_.end()) {
      return it->second;
    }
    return "NOT_FOUND";
  }

  int GetVersion() const { return version_; }

 private:
  std::map<std::string, std::string> rules_;
  int version_;
};

#endif  // RULE_SET_H_
