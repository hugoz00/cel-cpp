#include "codelab/V3/RuleManager.h"

#include <iostream>
#include <memory>  // 包含 std::make_shared
#include <mutex>
#include <string>

// 用于日志记录和状态检查
#include "absl/log/absl_log.h"
#include "absl/log/check.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h" // 用于构建错误消息

// 包含 CEL 编译器和运行时组件
#include "compiler/compiler_factory.h"
#include "compiler/standard_library.h"
#include "eval/public/activation.h" 
#include "eval/public/activation_bind_helper.h" // (新) 包含 BindProtoToActivation
#include "eval/public/builtin_func_registrar.h"
#include "eval/public/cel_expr_builder_factory.h"
#include "eval/public/cel_value.h" 
#include "google/protobuf/arena.h" 
#include "google/protobuf/descriptor.h"
#include "google/protobuf/message.h"
#include "google/rpc/context/attribute_context.pb.h" // (新) 包含 AttributeContext

// --- 包含声明变量所需 ---
#include "common/decl.h"
#include "common/type.h"
// ---------------------------------

// 包含我们在 V3 中已有的编译辅助函数 (路径已修正)
#include "codelab/V3/cel_compiler.h"

RuleManager& RuleManager::GetInstance() {
  static RuleManager instance;
  return instance;
}

// 构造函数：在单例首次创建时初始化 CEL 环境
RuleManager::RuleManager() {
  CHECK_OK(InitializeCelEnvironment())
      << "Failed to initialize CEL environment in RuleManager";
}

// 私有方法：设置 CEL 编译器和运行时
absl::Status RuleManager::InitializeCelEnvironment() {
  std::cout << "[RuleManager] Initializing CEL Environment..." << std::endl;
  
  // 1. 配置 CEL 运行时选项
  options_.enable_comprehension = true;
  options_.comprehension_max_iterations = 1000;
  options_.enable_regex = true;
  options_.regex_max_program_size = 1024;
  
  // 2. 创建 CEL 编译器
  auto compiler_builder_status =
      cel::NewCompilerBuilder(google::protobuf::DescriptorPool::generated_pool());
  if (!compiler_builder_status.ok()) {
    return compiler_builder_status.status();
  }
  auto compiler_builder = std::move(compiler_builder_status.value());

  // 添加标准 CEL 函数库 (e.g., '+', '==', 'size()', etc.)
  CEL_RETURN_IF_ERROR(compiler_builder->AddLibrary(cel::StandardCompilerLibrary()));

  // --- (修正点) 声明 Protobuf 上下文 ---
  // 链接 proto 描述符，使其对运行时可用
  google::protobuf::LinkMessageReflection<google::rpc::context::AttributeContext>();
  // 告诉编译器 "google.rpc.context.AttributeContext" 类型是可用的，
  // 它将被用作顶层变量（即 "request", "auth" 等字段的容器）。
  CEL_RETURN_IF_ERROR(compiler_builder->GetCheckerBuilder().AddContextDeclaration(
      "google.rpc.context.AttributeContext"));
  // ------------------------------------

  auto compiler_status = compiler_builder->Build();
  if (!compiler_status.ok()) {
    return compiler_status.status();
  }
  compiler_ = std::move(compiler_status.value());

  // 3. 创建 CEL 运行时表达式构建器
  builder_ = google::api::expr::runtime::CreateCelExpressionBuilder(
      google::protobuf::DescriptorPool::generated_pool(),
      google::protobuf::MessageFactory::generated_factory(), options_);

  // 4. 注册内置函数到运行时
  CEL_RETURN_IF_ERROR(google::api::expr::runtime::RegisterBuiltinFunctions(
      builder_->GetRegistry()));

  std::cout << "[RuleManager] CEL Environment Initialized." << std::endl;
  return absl::OkStatus();
}


// (CompileRule, RemoveRule, GetRule 保持不变)
void RuleManager::CompileRule(const std::string& name,
                              const std::string& expression) {
  std::lock_guard<std::mutex> lock(mtx_);
  std::cout << "Compiling/Updating rule: " << name << std::endl;

  auto rule = std::make_shared<CelRule>(expression);

  absl::StatusOr<cel::expr::CheckedExpr> checked_expr_status =
      cel_codelab::CompileToCheckedExpr(*compiler_, expression);

  if (!checked_expr_status.ok()) {
    std::string error =
        "Compile Check Error: " + checked_expr_status.status().ToString();
    std::cerr << "  ERROR: " << error << std::endl;
    rule->SetError(error);
  } else {
    absl::StatusOr<std::unique_ptr<google::api::expr::runtime::CelExpression>>
        build_status =
            builder_->CreateExpression(&(checked_expr_status.value()));

    if (!build_status.ok()) {
      std::string error =
          "Runtime Build Error: " + build_status.status().ToString();
      std::cerr << "  ERROR: " << error << std::endl;
      rule->SetError(error);
    } else {
      std::cout << "  Successfully compiled rule: " << name << std::endl;
      rule->SetCompiled(std::move(build_status.value()));
    }
  }
  rules_[name] = rule;
}

bool RuleManager::RemoveRule(const std::string& name) {
  std::lock_guard<std::mutex> lock(mtx_);
  size_t result = rules_.erase(name);
  if (result > 0) {
    std::cout << "Removed rule pointer from manager: " << name << std::endl;
    return true;
  } else {
    std::cout << "Rule not found, cannot remove: " << name << std::endl;
    return false;
  }
}

std::shared_ptr<CelRule> RuleManager::GetRule(const std::string& name) {
  std::lock_guard<std::mutex> lock(mtx_);
  auto it = rules_.find(name);
  if (it != rules_.end()) {
    return it->second;
  }
  return nullptr;
}

// --- (新) EvaluateRule (Proto) 实现 ---
absl::StatusOr<google::api::expr::runtime::CelValue> RuleManager::EvaluateRule(
    absl::string_view rule_name,
    const google::protobuf::Message& context_proto,
    google::protobuf::Arena& arena) {
  
  // 1. 创建 Activation
  google::api::expr::runtime::Activation activation;

  // 2. 将 Protobuf 消息绑定到 Activation
  //    (这将自动使 "request", "auth", "resource" 等字段可用)
  CEL_RETURN_IF_ERROR(google::api::expr::runtime::BindProtoToActivation(
      &context_proto, &arena, &activation));

  // 3. 调用底层重载
  return EvaluateRule(std::string(rule_name), activation, arena);
}


// (EvaluateRule (map) 保持不变)
absl::StatusOr<google::api::expr::runtime::CelValue> RuleManager::EvaluateRule(
    absl::string_view rule_name,
    const std::map<std::string, google::api::expr::runtime::CelValue>&
        context) {
  google::protobuf::Arena arena;
  google::api::expr::runtime::Activation activation;
  for (const auto& pair : context) {
    activation.InsertValue(pair.first, pair.second);
  }
  return EvaluateRule(rule_name, activation, arena);
}

// (EvaluateRule (Activation) 保持不变)
absl::StatusOr<google::api::expr::runtime::CelValue> RuleManager::EvaluateRule(
    absl::string_view rule_name,
    google::api::expr::runtime::Activation& activation,
    google::protobuf::Arena& arena) {
  std::shared_ptr<CelRule> rule = GetRule(std::string(rule_name));

  if (!rule) {
    return absl::NotFoundError(
        absl::StrCat("Rule not found: '", rule_name, "'"));
  }

  if (!rule->IsCompiled()) {
    return absl::FailedPreconditionError(
        absl::StrCat("Rule is not compiled: '", rule_name,
                     "', Error: ", rule->GetCompileError()));
  }

  const google::api::expr::runtime::CelExpression* expr =
      rule->GetCompiledExpr();
  if (!expr) {
    return absl::InternalError(
        absl::StrCat("Internal error: Rule '", rule_name,
                     "' is compiled but has no expression plan."));
  }

  absl::StatusOr<google::api::expr::runtime::CelValue> result_status =
      expr->Evaluate(activation, &arena);

  if (!result_status.ok()) {
    return absl::InternalError(
        absl::StrCat("Runtime error evaluating rule '", rule_name,
                     "': ", result_status.status().ToString()));
  }

  const auto& result_value = result_status.value();
  if (result_value.IsError()) {
    return absl::InternalError(
        absl::StrCat("Runtime error in rule '", rule_name,
                     "': ", result_value.ErrorOrDie()->ToString()));
  }

  return result_value;
}


// (PrintAllRules 保持不变)
void RuleManager::PrintAllRules() {
  std::lock_guard<std::mutex> lock(mtx_);
  std::cout << "--- Current Rules in Manager ---" << std::endl;
  if (rules_.empty()) {
    std::cout << "  (Manager is empty)" << std::endl;
  }
  for (const auto& pair : rules_) {
    std::cout << "  " << pair.first << " : "
              << pair.second->GetExpression() << std::endl;
    switch (pair.second->GetStatus()) {
      case CelRule::COMPILED_OK:
        std::cout << "    (Status: COMPILED_OK, Use Count: " 
                  << pair.second.use_count() << ")" << std::endl;
        break;
      case CelRule::COMPILE_ERROR:
        std::cout << "    (Status: COMPILED_ERROR, Error: " 
                  << pair.second->GetCompileError() << ")" << std::endl;
        break;
      case CelRule::NOT_COMPILED:
        std::cout << "    (Status: NOT_COMPILED)" << std::endl;
        break;
    }
  }
  std::cout << "--------------------------------" << std::endl;
}
