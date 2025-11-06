#include "codelab/RuleManager.h"

#include <iostream>
#include <memory>  // 包含 std::make_shared
#include <mutex>

RuleManager& RuleManager::GetInstance() {
  static RuleManager instance;
  return instance;
}

void RuleManager::AddRule(const std::string& name,
                          const std::string& expression) {
  std::lock_guard<std::mutex> lock(mtx_);
  std::cout << "Adding/Updating rule: " << name << std::endl;

  // -----------------------------------------------------------------
  // 核心变更：
  // 使用 std::make_shared 在堆上创建 CelRule 对象，
  // 并将其封装在 std::shared_ptr 中，然后存入 map。
  // -----------------------------------------------------------------
  rules_[name] = std::make_shared<CelRule>(expression);
}

bool RuleManager::RemoveRule(const std::string& name) {
  std::lock_guard<std::mutex> lock(mtx_);

  size_t result = rules_.erase(name);

  if (result > 0) {
    std::cout << "Removed rule pointer from manager: " << name << std::endl;
    // 注意：此时 CelRule 对象的析构函数不一定被调用。
    // 只有当这是最后一个指向该对象的 shared_ptr 时，它才会被销毁。
    return true;
  } else {
    std::cout << "Rule not found, cannot remove: " << name << std::endl;
    return false;
  }
}

// 核心变更：返回类型现在是 std::shared_ptr<CelRule>
std::shared_ptr<CelRule> RuleManager::GetRule(const std::string& name) {
  std::lock_guard<std::mutex> lock(mtx_);

  auto it = rules_.find(name);
  if (it != rules_.end()) {
    // 返回 shared_ptr 的一个拷贝。
    // 这会将对象的引用计数 +1。
    return it->second;
  }

  // 没找到，返回一个空的 shared_ptr (等同于 nullptr)
  return nullptr;
}

void RuleManager::PrintAllRules() {
  std::lock_guard<std::mutex> lock(mtx_);
  std::cout << "--- Current Rules in Manager ---" << std::endl;
  if (rules_.empty()) {
    std::cout << "  (Manager is empty)" << std::endl;
  }
  for (const auto& pair : rules_) {
    // 核心变更：通过 ->GetExpression() 访问表达式
    std::cout << "  " << pair.first << " : "
              << pair.second->GetExpression() << std::endl
              // .use_count() 显示有多少个 shared_ptr 指向这个对象
              << "    (Use Count: " << pair.second.use_count() << ")" 
              << std::endl;
  }
  std::cout << "--------------------------------" << std::endl;
}
