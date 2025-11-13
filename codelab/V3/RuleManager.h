#ifndef RULE_MANAGER_H_
#define RULE_MANAGER_H_

#include <iostream>
#include <map>
#include <memory>  // 包含 std::shared_ptr
#include <mutex>
#include <optional>
#include <string>

// 包含我们新定义的规则类 (路径已修正)
#include "codelab/V3/CelRule.h"

// 包含 CEL 运行时选项
#include "eval/public/cel_options.h"

// --- (新) 包含 Evaluate 所需的组件 ---
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "eval/public/activation.h"
#include "eval/public/activation_bind_helper.h" // (新) 用于绑定 Proto
#include "eval/public/cel_value.h"
#include "google/protobuf/arena.h"
#include "google/protobuf/message.h" // (*** 修正点：必须包含此文件以实现正确的重载)
#include "google/rpc/context/attribute_context.pb.h" // (新) 包含 AttributeContext

// 前向声明 CEL 编译器和构建器类
namespace cel {
class Compiler;
}  // namespace cel
namespace google {
namespace api {
namespace expr {
namespace runtime {
class CelExpressionBuilder;
}  // namespace runtime
}  // namespace expr
}  // namespace api
}  // namespace google

class RuleManager {
 public:
  static RuleManager& GetInstance();

  RuleManager(const RuleManager&) = delete;
  RuleManager& operator=(const RuleManager&) = delete;

  /**
   * @brief (原 AddRule) 编译一个新规则并将其缓存在管理器中。
   */
  void CompileRule(const std::string& name, const std::string& expression);

  /**
   * @brief 移除一条规则。
   */
  bool RemoveRule(const std::string& name);

  /**
   * @brief 获取一个指向规则对象的共享指针。
   */
  std::shared_ptr<CelRule> GetRule(const std::string& name);

  /**
   * @brief (新) 执行一个已编译的规则 (Protobuf 上下文)。
   *
   * 这是一个高级辅助函数，它将一个 Protobuf 消息
   * 自动绑定到 Activation 以供 CEL 评估。
   *
   * @param rule_name 要执行的规则的名称。
   * @param context_proto 作为上下文的 Protobuf 消息。
   * @param arena 用于评估期间内存分配的 Arena。
   * @return 规则的评估结果 (CelValue)，如果出错则返回 Status。
   */
  absl::StatusOr<google::api::expr::runtime::CelValue> EvaluateRule(
      absl::string_view rule_name,
      const google::protobuf::Message& context_proto,
      google::protobuf::Arena& arena);

  /**
   * @brief 执行一个已编译的规则 (std::map 上下文)。
   */
  absl::StatusOr<google::api::expr::runtime::CelValue> EvaluateRule(
      absl::string_view rule_name,
      const std::map<std::string, google::api::expr::runtime::CelValue>&
          context);

  /**
   * @brief 执行一个已编译的规则 (底层 Activation)。
   */
  absl::StatusOr<google::api::expr::runtime::CelValue> EvaluateRule(
      absl::string_view rule_name,
      google::api::expr::runtime::Activation& activation,
      google::protobuf::Arena& arena);


  void PrintAllRules();

 private:
  // 构造函数现在是自定义的，用于初始化 CEL 环境
  RuleManager();
  ~RuleManager() = default;

  /**
   * @brief 初始化 CEL 编译器和运行时环境。
   * @return absl::OkStatus() on success.
   */
  absl::Status InitializeCelEnvironment();

  // 核心存储 (即编译缓存)
  std::map<std::string, std::shared_ptr<CelRule>> rules_;
  std::mutex mtx_;

  // --- CEL 环境 ---
  google::api::expr::runtime::InterpreterOptions options_;
  std::unique_ptr<cel::Compiler> compiler_;
  std::unique_ptr<google::api::expr::runtime::CelExpressionBuilder> builder_;
};

#endif  // RULE_MANAGER_H_
