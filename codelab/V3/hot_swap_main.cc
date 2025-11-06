#include <atomic>
#include <chrono>  // 用于 std::chrono::milliseconds
#include <iostream>
#include <string>
#include <thread>  // 包含 std::thread
#include <vector>

#include "codelab/RuleSetManager.h" // 包含我们的管理器

// 一个全局原子标志，用于通知所有线程停止
std::atomic<bool> g_running(true);
// 一个全局原子标志，用于报告错误
std::atomic<int> g_errors_found(0);

/**
 * @brief 模拟“读线程”（如规则匹配）
 */
void ReaderThread(RuleSetManager& manager) {
  long read_count = 0;
  while (g_running.load(std::memory_order_relaxed)) {
    // 1. 高性能、无锁读取
    std::shared_ptr<const RuleSet> rules = manager.GetCurrentRuleSet();

    // 2. 验证数据一致性
    //    我们从规则集读取两条数据
    std::string rule_a = rules->GetRule("rule_a");
    std::string rule_b = rules->GetRule("rule_b");

    // 3. 核心验证：
    //    这两条规则必须 *永远* 相同（即来自同一版本）。
    //    如果它们不相同，意味着我们读取到了一个
    //    正在被写入的“中间状态”或“撕裂”的数据集。
    if (rule_a != rule_b) {
      g_errors_found++;
      std::cout << "!!!!!!!!!!! 数据竞争错误 !!!!!!!!!!!" << std::endl;
      std::cout << "  读到 rule_a: " << rule_a << std::endl;
      std::cout << "  读到 rule_b: " << rule_b << std::endl;
    }
    
    read_count++;
    
    // 释放 CPU，让其他线程有机会运行
    std::this_thread::yield(); 
  }
  std::cout << "[读线程] 停止。总共执行了 " << read_count << " 次一致性检查。"
            << std::endl;
}

/**
 * @brief 模拟“写线程”（如配置更新）
 */
void WriterThread(RuleSetManager& manager) {
  int version = 1;
  while (g_running.load(std::memory_order_relaxed)) {
    // 模拟每 20 毫秒更新一次配置
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    // 执行热替换
    manager.UpdateRuleSet(version++);
  }
  std::cout << "[写线程] 停止。总共发布了 " << (version - 1) << " 个版本。"
            << std::endl;
}

int main() {
  RuleSetManager manager; // 此时已创建 v0

  std::cout << "开始多线程热替换测试..." << std::endl;
  std::cout << "启动 1 个写线程和 4 个读线程，运行 3 秒钟..." << std::endl;

  // 启动 1 个写线程
  std::thread writer(WriterThread, std::ref(manager));

  // 启动 4 个读线程
  std::vector<std::thread> readers;
  for (int i = 0; i < 4; ++i) {
    readers.emplace_back(ReaderThread, std::ref(manager));
  }

  // 让测试运行 3 秒
  std::this_thread::sleep_for(std::chrono::seconds(3));

  // 3. 发送停止信号
  std::cout << "\n正在发送停止信号..." << std::endl;
  g_running.store(false);

  // 4. 等待所有线程退出 (Join)
  writer.join();
  for (auto& reader : readers) {
    reader.join();
  }

  std::cout << "\n--- 测试完成 ---" << std::endl;
  if (g_errors_found.load() == 0) {
    std::cout << "结果: 成功 (PASSED)" << std::endl;
    std::cout << "验证：在所有读取中，规则集始终保持了内部一致性。"
              << std::endl;
    std::cout << "读线程从未读取到“中间状态”的数据。" << std::endl;
  } else {
    std::cout << "结果: 失败 (FAILED)" << std::endl;
    std::cout << "错误：检测到 " << g_errors_found.load() << " 次数据竞争！"
              << std::endl;
  }
  
  std::cout << "\n[主线程] 退出。程序将清理所有剩余的 RuleSet 对象。" << std::endl;
  return 0;
}
