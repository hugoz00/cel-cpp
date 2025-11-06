#ifndef RULE_MANAGER_H_
#define RULE_MANAGER_H_

#include <iostream>
#include <map>
#include <memory>  // 包含 std::shared_ptr
#include <mutex>
#include <optional>
#include <string>

// 包含我们新定义的规则类
#include "codelab/CelRule.h"

class RuleManager {
 public:
  static RuleManager& GetInstance();

  RuleManager(const RuleManager&) = delete;
  RuleManager& operator=(const RuleManager&) = delete;

  /**
   * @brief 添加或更新一条规则。
   *
   * 这将创建一个新的 CelRule 对象，并由 std::shared_ptr 管理。
   */
  void AddRule(const std::string& name, const std::string& expression);

  /**
   * @brief 移除一条规则。
   *
   * 这将从 map 中移除 std::shared_ptr。
   * 如果引用计数降为 0，CelRule 对象将被自动销毁。
   */
  bool RemoveRule(const std::string& name);

  /**
   * @brief 获取一个指向规则对象的共享指针。
   *
   * @param name 要获取的规则名称。
   * @return 一个 std::shared_ptr<CelRule>。
   * 如果未找到，返回的 shared_ptr 将为 nullptr。
   */
  std::shared_ptr<CelRule> GetRule(const std::string& name);

  void PrintAllRules();

 private:
  RuleManager() = default;
  ~RuleManager() = default;

  // -----------------------------------------------------------------
  // 核心变更：
  // 存储类型从 std::string 变为 std::shared_ptr<CelRule>
  // -----------------------------------------------------------------
  std::map<std::string, std::shared_ptr<CelRule>> rules_;

  std::mutex mtx_;
};

#endif  // RULE_MANAGER_H_
