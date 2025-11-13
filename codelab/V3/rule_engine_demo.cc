// codelab/V3/rule_engine_demo.cc

#include <chrono>  // 用于 std::chrono::seconds
#include <iostream>
#include <memory>  // 包含 std::shared_ptr
#include <string>
#include <thread>  // 包含 std::thread

#include "codelab/V3/RuleManager.h" // 包含我们的管理器
#include "absl/status/status.h"
#include "absl/log/check.h" // (*** 修正点 1 ***) 包含 CHECK_OK
#include "eval/public/cel_value.h"
#include "google/protobuf/arena.h"
#include "google/rpc/context/attribute_context.pb.h" // 包含上下文 Proto

// --- 包含 lambda 中所需的 include ---
#include "eval/public/activation_bind_helper.h"
#include "google/protobuf/struct.pb.h" // Value 定义
// ---------------------------------------------

// 辅助函数，用于打印评估结果
void PrintResult(const std::string& rule_name,
                 const absl::StatusOr<google::api::expr::runtime::CelValue>& result) {
  if (!result.ok()) {
    std::cerr << "  [EVAL FAILED] " << rule_name << ": " 
              << result.status().ToString() << std::endl;
    return;
  }
  std::cout << "  [EVAL SUCCESS] " << rule_name << ": "
            << result.value().DebugString() << std::endl;
}

/**
 * @brief 模拟一个长时间运行的工作线程。
 *
 * 它会获取一个规则 v1，长时间持有它（模拟评估），
 * 在此期间，主线程将会更新这个规则为 v2。
 * 它验证了它持有的 v1 规则不受 v2 更新的影响。
 */
void WorkerThread(google::rpc::context::AttributeContext context) {
  std::cout << "[WORKER] 启动。准备获取 'auth_rule' (v1)..." << std::endl;
  RuleManager& manager = RuleManager::GetInstance();
  google::protobuf::Arena arena;

  // 1. 工作线程获取规则 "auth_rule" (v1 版本)
  std::shared_ptr<CelRule> my_rule_ptr = manager.GetRule("auth_rule");

  if (!my_rule_ptr) {
    std::cout << "[WORKER] 错误：未能获取 'auth_rule'。" << std::endl;
    return;
  }

  std::cout << "[WORKER] 成功获取 'auth_rule' (v1)。" << std::endl;
  std::cout << "[WORKER] (v1 引用计数: " << my_rule_ptr.use_count()
            << ")" << std::endl;

  // 2. 模拟一个耗时的任务（2秒）
  std::cout << "[WORKER] 开始模拟 2 秒的耗时工作..." << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(2));

  // 3. 任务完成，访问规则数据
  //    这是关键的验证点：
  //    即使主线程已经更新了 map，my_rule_ptr 仍然指向 v1 对象。
  std::cout << "[WORKER] 耗时工作完成。正在使用我持有的 v1 规则..." << std::endl;
  
  // (我们使用底层的 Evaluate 接口来直接使用 CelRule 指针)
  auto result = my_rule_ptr->GetCompiledExpr()->Evaluate(
      [&]() {
        // 重新绑定 Proto 到一个新的 Activation
        google::api::expr::runtime::Activation activation;
        
        // (*** 修正点 1 ***) 检查 BindProtoToActivation 的返回值
        absl::Status bind_status = google::api::expr::runtime::BindProtoToActivation(
            &context, &arena, &activation);
        // 在演示/测试中，如果绑定失败，CHECK 是一种可接受的处理方式
        ABSL_CHECK_OK(bind_status) << "Worker failed to bind proto";
        
        return activation;
      }(),
      &arena);

  std::cout << "[WORKER] >> v1 规则 (" << my_rule_ptr->GetExpression() 
            << ") 评估结果: " << result.value().DebugString() << std::endl;

  std::cout << "[WORKER] 退出。" << std::endl;
  // 当此函数退出时，my_rule_ptr 被销毁。
  // v1 对象的析构函数将在此处被调用。
}

int main() {
  RuleManager& manager = RuleManager::GetInstance();
  google::protobuf::Arena arena;

  std::cout << "--- 任务 3.1: 创建 Protobuf 上下文和编译规则 ---" << std::endl;

  // (3.1.2) 创建对应的 Protobuf 消息作为执行上下文
  google::rpc::context::AttributeContext context;

  // --- Protobuf 字段填充 ---
  // "auth.uid" 映射到 "request.auth.principal"
  context.mutable_request()->mutable_auth()->set_principal("admin");

  // "auth.claims['group']" 映射到 "request.auth.claims"
  google::protobuf::Value val;
  val.set_string_value("prod");
  
  // (*** 修正点 2 ***) 必须调用 mutable_fields() 来访问 Struct 的内部 map
  context.mutable_request()->mutable_auth()->mutable_claims()->mutable_fields()->insert(
      {"group", val});
  // -----------------------------------

  context.mutable_request()->set_path("/admin/v1/items");
  context.mutable_request()->mutable_headers()->insert(
      {"x-token", "secret-token"});
  context.mutable_resource()->set_name("//db/items/123");

  std::cout << "[MAIN] 创建 Protobuf 上下文:\n" 
            << context.DebugString() << std::endl;

  // (3.1.1) 编写 3-5 个有代表性的 CEL 表达式测试用例
  // "auth.uid" -> "request.auth.principal"
  manager.CompileRule("auth_rule", "request.auth.principal == 'admin'");
  // "auth.claims" -> "request.auth.claims"
  manager.CompileRule("group_rule", "request.auth.claims['group'] == 'prod'");
  manager.CompileRule("path_rule", "request.path.startsWith('/admin')");
  manager.CompileRule("token_rule", "request.headers['x-token'] == 'secret-token'");
  manager.CompileRule("resource_rule", "resource.name == '//db/items/456'");
  
  std::cout << "\n[MAIN] 编译 5 个规则后的管理器状态：" << std::endl;
  manager.PrintAllRules();

  std::cout << "\n--- 任务 3.1.3: 验证规则执行结果的正确性 ---" << std::endl;
  
  // (3.1.3) 验证规则执行结果
  PrintResult("auth_rule", manager.EvaluateRule("auth_rule", context, arena));
  PrintResult("group_rule", manager.EvaluateRule("group_rule", context, arena));
  PrintResult("path_rule", manager.EvaluateRule("path_rule", context, arena));
  PrintResult("token_rule", manager.EvaluateRule("token_rule", context, arena));
  PrintResult("resource_rule", manager.EvaluateRule("resource_rule", context, arena));

  std::cout << "\n--- 任务 3.2: 演示规则生命周期和热更新 ---" << std::endl;

  // (3.2.3) 启动工作线程，它将获取 'auth_rule' 的 v1 版本
  std::cout << "[MAIN] 启动工作线程。" << std::endl;
  std::thread worker(WorkerThread, context);

  // (3.2.1) 等待 500 毫秒，确保工作线程已经获取了 v1 规则
  std::cout << "[MAIN] 等待 500 毫秒..." << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  // (3.2.1) 更新 "auth_rule" (v2 版本)
  std::cout << "[MAIN] !!! 正在热更新 'auth_rule' (v2 版本)... !!!" << std::endl;
  // "auth.uid" -> "request.auth.principal"
  manager.CompileRule("auth_rule", "request.auth.principal == 'root'"); // v2
  
  std::cout << "\n[MAIN] 检查管理器状态（v1 析构函数不应被调用）：" << std::endl;
  manager.PrintAllRules();

  // (3.2.1) 主线程使用 v2 规则进行评估
  std::cout << "[MAIN] 主线程使用 v2 规则评估：" << std::endl;
  PrintResult("auth_rule", manager.EvaluateRule("auth_rule", context, arena));

  // (3.2.1) 等待工作线程完成
  std::cout << "[MAIN] 等待工作线程执行完毕..." << std::endl;
  worker.join(); // 等待 worker 线程结束

  std::cout << "[MAIN] 工作线程已退出。" << std::endl;
  std::cout << "[MAIN] (此时 v1 析构函数应该已被调用)。" << std::endl;

  // (3.2.1) 移除规则
  std::cout << "\n[MAIN] 移除 'auth_rule' (v2)..." << std::endl;
  manager.RemoveRule("auth_rule");
  std::cout << "[MAIN] (此时 v2 析构函数应该已被调用)。" << std::endl;

  std::cout << "\n[MAIN] 最终管理器状态：" << std::endl;
  manager.PrintAllRules();

  std::cout << "\n[MAIN] 演示完成。退出。" << std::endl;
  return 0;
}
