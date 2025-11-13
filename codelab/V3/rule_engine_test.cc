// codelab/V3/rule_engine_test.cc
#include "codelab/V3/RuleManager.h"

#include <map>
#include <string>

// 包含 GoogleTest 和 GMock 相关的测试宏
#include "internal/testing.h"
#include "eval/public/cel_value.h"
#include "eval/public/testing/matchers.h" // (新) For IsCel... matchers
#include "absl/status/status_matchers.h" // (新) For StatusIs

namespace {

using ::google::api::expr::runtime::CelValue;
using ::google::api::expr::runtime::test::IsCelBool;
using ::google::api::expr::runtime::test::IsCelInt64;
using ::testing::HasSubstr;
using ::absl_testing::StatusIs;


// 定义一个测试套件
class RuleEngineTest : public ::testing::Test {
 protected:
  // 在每个测试开始前，编译一组规则
  void SetUp() override {
    manager_ = &RuleManager::GetInstance();
    
    // 编译一个使用变量的有效规则
    manager_->CompileRule("var_rule", "request.size + 10");
    
    // 编译一个会导致运行时错误的规则
    manager_->CompileRule("runtime_error_rule", "1 / 0");

    // 编译一个编译时就失败的规则
    manager_->CompileRule("compile_error_rule", "1 + + 1");
  }

  RuleManager* manager_;
};

// 测试：成功执行一个带变量的规则
TEST_F(RuleEngineTest, EvaluateWithContext) {
  // 1. 创建执行上下文
  std::map<std::string, CelValue> context;
  context["request.size"] = CelValue::CreateInt64(5);

  // 2. 执行规则
  absl::StatusOr<CelValue> result = manager_->EvaluateRule("var_rule", context);

  // 3. 验证结果
  ASSERT_OK(result.status());
  EXPECT_THAT(result.value(), IsCelInt64(15));
}

// 测试：成功执行一个不带变量的规则
TEST_F(RuleEngineTest, EvaluateSimpleRule) {
  manager_->CompileRule("simple_rule", "'hello' == 'hello'");
  
  absl::StatusOr<CelValue> result = manager_->EvaluateRule("simple_rule", {});

  ASSERT_OK(result.status());
  EXPECT_THAT(result.value(), IsCelBool(true));
}

// 测试：尝试执行一个不存在的规则
TEST_F(RuleEngineTest, EvaluateRuleNotFound) {
  absl::StatusOr<CelValue> result = manager_->EvaluateRule("non_existent_rule", {});

  // 验证错误
  EXPECT_THAT(result.status(), StatusIs(absl::StatusCode::kNotFound));
  EXPECT_THAT(result.status().message(), HasSubstr("Rule not found"));
}

// 测试：尝试执行一个未能编译的规则
TEST_F(RuleEngineTest, EvaluateRuleNotCompiled) {
  absl::StatusOr<CelValue> result = manager_->EvaluateRule("compile_error_rule", {});

  // 验证错误
  EXPECT_THAT(result.status(), StatusIs(absl::StatusCode::kFailedPrecondition));
  EXPECT_THAT(result.status().message(), HasSubstr("Rule is not compiled"));
}

// 测试：执行一个导致运行时错误的规则
TEST_F(RuleEngineTest, EvaluateWithRuntimeError) {
  absl::StatusOr<CelValue> result = manager_->EvaluateRule("runtime_error_rule", {});

  // 验证错误
  EXPECT_THAT(result.status(), StatusIs(absl::StatusCode::kInternal));
  EXPECT_THAT(result.status().message(), HasSubstr("Runtime error"));
  EXPECT_THAT(result.status().message(), HasSubstr("divide by zero"));
}

}  // namespace
