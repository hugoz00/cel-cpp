#include "cel/expr/syntax.pb.h" // 包含 Expr 和 Constant 的定义
#include <iostream>

// 引入正确的命名空间
using ::cel::expr::Expr;
using ::cel::expr::Constant;

int main() {
    // 1. 创建根节点 (root)，它是一个函数调用。
    Expr root;
    root.set_id(1); // AST 中的每个节点都应该有一个唯一的 ID
    
    // 2. 获取 root 节点内部的 "call_expr" 结构体
    Expr::Call* call = root.mutable_call_expr();
    call->set_function("_+_"); // 设置函数名为 "+" 的内部名称

    // 3. 添加第一个参数 (子节点)：常量 1
    Expr* arg1 = call->add_args(); // add_args() 会创建一个新的空 Expr 并返回指针
    arg1->set_id(2);
    // 获取 arg1 内部的 "const_expr" 结构体并设置值
    arg1->mutable_const_expr()->set_int64_value(1);

    // 4. 添加第二个参数 (子节点)：常量 2
    Expr* arg2 = call->add_args();
    arg2->set_id(3);
    arg2->mutable_const_expr()->set_int64_value(2);


    // 5. 打印我们手动构建的 AST
    std::cout << "--- Manually built AST for '1 + 2' ---\n";
    std::cout << root.DebugString();
    std::cout << "---------------------------------------\n";

    return 0;
}
