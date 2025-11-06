#include <chrono>  // 用于 std::chrono::seconds
#include <iostream>
#include <memory>  // 包含 std::shared_ptr
#include <thread>  // 包含 std::thread

#include "codelab/RuleManager.h" // 包含你的头文件

/**
 * @brief 模拟一个长时间运行的工作线程。
 *
 * 它会获取一个规则，长时间持有它（模拟评估），
 * 在此期间，主线程将会更新这个规则。
 */
void WorkerThread() {
  std::cout << "[工作线程] 启动。准备获取 'admin_rule'..." << std::endl;
  RuleManager& manager = RuleManager::GetInstance();

  // 1. 工作线程获取规则 "admin_rule" (v1 版本)
  std::shared_ptr<CelRule> my_rule_ptr = manager.GetRule("admin_rule");

  if (!my_rule_ptr) {
    std::cout << "[工作线程] 错误：未能获取 'admin_rule'。" << std::endl;
    return;
  }

  // 此时，引用计数变为 2（1 个在 map 中, 1 个在 my_rule_ptr 中）
  std::cout << "[工作线程] 成功获取 'admin_rule' (v1)。" << std::endl;
  std::cout << "[工作线程] (v1 引用计数: " << my_rule_ptr.use_count()
            << ")" << std::endl;

  // 2. 模拟一个耗时的任务（2秒）
  //    在此期间，主线程会更新规则
  std::cout << "[工作线程] 开始模拟 2 秒的耗时工作..." << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(2));

  // 3. 任务完成，访问规则数据
  //    这是关键的验证点：
  //    即使主线程已经更新了 map，my_rule_ptr 仍然指向 v1 对象。
  std::cout << "[工作线程] 耗时工作完成。正在访问我持有的规则..." << std::endl;
  std::cout << "[工作线程] >> 我持有的规则是: "
            << my_rule_ptr->GetExpression() 
            << std::endl;

  std::cout << "[工作线程] 退出。" << std::endl;
  // 当此函数退出时，my_rule_ptr 被销毁。
  // v1 对象的析构函数将在此处被调用。
}

int main() {
  RuleManager& manager = RuleManager::GetInstance();

  // 1. 主线程：添加 "admin_rule" (v1 版本)
  std::cout << "[主线程] 添加 'admin_rule' (v1 版本)..." << std::endl;
  manager.AddRule("admin_rule", "request.auth.uid == 'admin'"); // v1

  // 2. 主线程：启动工作线程
  std::cout << "[主线程] 启动工作线程。" << std::endl;
  std::thread worker(WorkerThread);

  // 3. 主线程：等待 500 毫秒，确保工作线程已经获取了 v1 规则
  std::cout << "[主线程] 等待 500 毫秒，确保工作线程已拿到规则..."
            << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  // 4. 主线程：更新 "admin_rule" (v2 版本)
  //    这一步发生在工作线程的 2 秒睡眠期间
  std::cout << "[主线程] !!! 正在更新 'admin_rule' (v2 版本)... !!!"
            << std::endl;
  manager.AddRule("admin_rule", "resource.owner == 'admin'"); // v2
  // 此时 v1 的构造函数 *不应该* 被调用。
  // v1 对象的引用计数从 2 降为 1。

  // 5. 主线程：等待工作线程完成
  std::cout << "[主线程] 等待工作线程执行完毕..." << std::endl;
  worker.join(); // 等待 worker 线程结束

  std::cout << "[主线程] 工作线程已退出。" << std::endl;
  std::cout << "[主线程] 检查 RuleManager 最终状态：" << std::endl;
  manager.PrintAllRules(); // 应该只显示 v2 版本

  std::cout << "[主线程] 退出。" << std::endl;
  // 当 main 结束时，manager 被销毁，v2 对象的析构函数被调用。
  return 0;
}
