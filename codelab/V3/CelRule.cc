#include "codelab/V3/CelRule.h" // 假设路径

#include "absl/strings/str_cat.h"
#include "absl/time/clock.h"

CelRule::CelRule(const std::string& expression)
    : expression_(expression), status_(CompileStatus::kUncompiled) {
  std::cout << "    [CelRule CONSTRUCTOR]: Rule object created for expression: "
            << expression_ << std::endl;
}

CelRule::~CelRule() {
  std::cout << "    [CelRule DESTRUCTOR]: Rule object destroyed for expression: "
            << expression_ << std::endl;
  // compiled_expr_ (unique_ptr) 会在这里被自动销毁
}

void CelRule::SetCompiled(
    std::unique_ptr<google::api::expr::runtime::CelExpression> expr) {
  status_ = CompileStatus::kCompiled;
  compiled_expr_ = std::move(expr);
  compile_timestamp_ = absl::Now();
  compile_error_ = "";
}

void CelRule::SetFailed(const std::string& error) {
  status_ = CompileStatus::kFailed;
  compiled_expr_ = nullptr;
  compile_timestamp_ = absl::Now();
  compile_error_ = absl::StrCat("Compile failed: ", error);
}
