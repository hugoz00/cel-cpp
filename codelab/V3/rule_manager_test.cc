// codelab/V3/rule_manager_test.cc
#include "codelab/V3/RuleManager.h" // (路径已修正)

#include <memory>
#include <string>

// 包含 CelRule 以检查其内部状态 (路径已修正)
#include "codelab/V3/CelRule.h"

// 包含 GoogleTest 和 GMock 相关的测试宏
#include "internal/testing.h"

namespace {

using ::testing::HasSubstr;
using ::testing::IsEmpty;
using ::testing::NotNull;

// 使用 GoogleTest 框架定义一个测试套件
class RuleManagerTest : public ::testing::Test {
 protected:
  RuleManagerTest() : manager_(RuleManager::GetInstance()) {
    // 首次调用 GetInstance() 会初始化 RuleManager 及其 CEL 环境
    std::cout << "RuleManagerTest setup. Initializing instance." << std::endl;
  }

  RuleManager& manager_;
};

// 测试：添加一个语法和类型都正确的规则
TEST_F(RuleManagerTest, AddValidRule) {
  const std::string expr = "1 + 2 == 3";
  manager_.CompileRule("valid_rule", expr); // (重命名)

  std::shared_ptr<CelRule> rule = manager_.GetRule("valid_rule");

  // 验证规则已成功获取
  ASSERT_THAT(rule, NotNull());
  EXPECT_EQ(rule->GetExpression(), expr);

  // 验证编译状态
  EXPECT_TRUE(rule->IsCompiled());
  EXPECT_EQ(rule->GetStatus(), CelRule::COMPILED_OK);
  EXPECT_THAT(rule->GetCompileError(), IsEmpty());
  EXPECT_THAT(rule->GetCompiledExpr(), NotNull());
}

// 测试：添加一个有语法错误的规则
TEST_F(RuleManagerTest, AddRuleWithSyntaxError) {
  // "1 + + 2" 是一个语法错误
  const std::string expr = "1 + + 2";
  manager_.CompileRule("syntax_error_rule", expr); // (重命名)

  std::shared_ptr<CelRule> rule = manager_.GetRule("syntax_error_rule");

  // 验证规则已成功获取
  ASSERT_THAT(rule, NotNull());
  EXPECT_EQ(rule->GetExpression(), expr);

  // 验证编译状态
  EXPECT_FALSE(rule->IsCompiled());
  EXPECT_EQ(rule->GetStatus(), CelRule::COMPILE_ERROR);
  EXPECT_THAT(rule->GetCompiledExpr(), nullptr);

  // 验证错误信息
  std::string error = rule->GetCompileError();
  EXPECT_FALSE(error.empty());
  // 检查是否有预期的错误子字符串
  EXPECT_THAT(error, HasSubstr("Syntax error"));
}

// 测试：添加一个有类型检查错误的规则
TEST_F(RuleManagerTest, AddRuleWithTypeError) {
  // 语法正确，但 'string' + 'int' 是一个类型错误
  const std::string expr = "'hello' + 1";
  manager_.CompileRule("type_error_rule", expr); // (重命名)

  std::shared_ptr<CelRule> rule = manager_.GetRule("type_error_rule");

  // 验证规则已成功获取
  ASSERT_THAT(rule, NotNull());
  EXPECT_EQ(rule->GetExpression(), expr);

  // 验证编译状态
  EXPECT_FALSE(rule->IsCompiled());
  EXPECT_EQ(rule->GetStatus(), CelRule::COMPILE_ERROR);
  EXPECT_THAT(rule->GetCompiledExpr(), nullptr);

  // 验证错误信息
  std::string error = rule->GetCompileError();
  EXPECT_FALSE(error.empty());
  // 检查是否有预期的错误子字符串
  EXPECT_THAT(error, HasSubstr("no matching overload"));
}

// 测试：移除规则
TEST_F(RuleManagerTest, RemoveRule) {
  manager_.CompileRule("rule_to_remove", "true"); // (重命名)
  
  // 确认规则存在
  ASSERT_THAT(manager_.GetRule("rule_to_remove"), NotNull());

  // 移除规则
  EXPECT_TRUE(manager_.RemoveRule("rule_to_remove"));

  // 确认规则不再存在
  EXPECT_THAT(manager_.GetRule("rule_to_remove"), nullptr);

  // 尝试移除不存在的规则
  EXPECT_FALSE(manager_.RemoveRule("non_existent_rule"));
}

}  // namespace
