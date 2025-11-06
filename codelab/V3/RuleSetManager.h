#ifndef RULE_SET_MANAGER_H_
#define RULE_SET_MANAGER_H_

#include <atomic>   // 仍然需要，因为它包含了 atomic_load/store 的非成员函数
#include <memory>   // 包含 std::shared_ptr 和 shared_ptr_atomic.h
#include <mutex>    // 包含 std::mutex (仅用于 *写* 操作)
#include <string>   // 包含 std::to_string

#include "codelab/RuleSet.h" // 包含 RuleSet

/**
 * @brief 管理规则集的热切换。
 *
 * 实现了无锁读取 (Lock-Free Read) 和
 * 低锁写入 (Low-Lock Write) 的热替换模式。
 */
class RuleSetManager {
 public:
  RuleSetManager() {
    // 1. 创建一个可变的 shared_ptr<RuleSet>
    auto initial_set_mutable = std::make_shared<RuleSet>(0);
    
    // 2. 修正：创建一个类型匹配的 shared_ptr<const RuleSet>
    std::shared_ptr<const RuleSet> initial_set_const = initial_set_mutable;

    // 3. 现在类型完全匹配：
    // std::atomic_store(std::shared_ptr<const RuleSet>*, 
    //                   std::shared_ptr<const RuleSet>)
    std::atomic_store(&current_ruleset_, initial_set_const);
  }

  /**
   * @brief (读操作) 获取当前激活的规则集。
   *
   * 这是一个高性能、无锁的原子操作。
   */
  std::shared_ptr<const RuleSet> GetCurrentRuleSet() {
    // 修正：使用非成员函数 std::atomic_load
    return std::atomic_load(&current_ruleset_);
  }

  /**
   * @brief (写操作) 创建一个新规则集并将其原子性地替换为当前规则集。
   */
  void UpdateRuleSet(int new_version) {
    // 1. 在后台创建和准备新的规则集
    auto new_set_mutable = std::make_shared<RuleSet>(new_version);

    std::string v = "v" + std::to_string(new_version);
    new_set_mutable->AddRule("rule_a", v);
    new_set_mutable->AddRule("rule_b", v);
    new_set_mutable->AddRule("rule_c", v);
    
    // 2. 修正：创建一个类型匹配的 shared_ptr<const RuleSet>
    std::shared_ptr<const RuleSet> new_set_const = new_set_mutable;

    // 3. 锁定 *写* 互斥锁
    std::lock_guard<std::mutex> lock(writer_mtx_);

    // 4. 实现“原子切换”
    std::cout << "[写线程] ---> 正在原子切换到 v" << new_version << "..."
              << std::endl;
              
    // 修正：使用非成员函数 std::atomic_store
    // 现在两个参数的类型都是 std::shared_ptr<const RuleSet>，
    // 编译器会找到正确的特化版本。
    std::atomic_store(&current_ruleset_, new_set_const);
    
    std::cout << "[写线程] ---> 切换完成。" << std::endl;
  }

 private:
  // 核心修正：
  // 变量是一个 *普通* 的 shared_ptr。
  // 对它的 *操作* (load/store) 将会是原子的。
  // -----------------------------------------------------------------
  std::shared_ptr<const RuleSet> current_ruleset_;
  // -----------------------------------------------------------------
  
  std::mutex writer_mtx_;
};

#endif  // RULE_SET_MANAGER_H_
