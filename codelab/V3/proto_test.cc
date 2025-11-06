// codelab/proto_test.cc
#include <fstream>
#include <iostream>
#include <string>

// 1. 包含由 protoc *自动生成* 的头文件
// 这个路径是根据你的 .proto 文件包名和文件名自动生成的
#include "codelab/cel_rule_def.pb.h"

void RunProtoTest() {
  std::cout << "--- Protobuf 序列化测试开始 ---" << std::endl;

  // 2. 创建一个 RuleSetMessage C++ 对象
  codelab::protobuf::RuleSetMessage rule_set_v1;
  rule_set_v1.set_version(1);

  // 3. 向 RuleSet 中添加规则
  // 使用 mutable_rules() 获取 map 对象的指针
  // 并使用 [] 运算符创建新条目
  codelab::protobuf::CelRuleMessage* rule1 =
      &(*rule_set_v1.mutable_rules())["admin_rule"];
  
  // 设置 rule1 的字段
  rule1->set_rule_name("admin_rule");
  rule1->set_expression("request.auth.uid == 'admin'");
  rule1->set_version(1);

  // 添加第二条规则
  codelab::protobuf::CelRuleMessage* rule2 =
      &(*rule_set_v1.mutable_rules())["user_rule"];
  rule2->set_rule_name("user_rule");
  rule2->set_expression("request.auth.uid == resource.owner_id");
  rule2->set_version(1);

  std::cout << "创建的 C++ 对象 (v1):\n"
            << rule_set_v1.DebugString() << std::endl;

  // 4. 序列化 (C++ 对象 -> 二进制字符串)
  std::string serialized_data;
  if (!rule_set_v1.SerializeToString(&serialized_data)) {
    std::cerr << "序列化失败！" << std::endl;
    return;
  }

  std::cout << "序列化后的二进制数据大小: " << serialized_data.length()
            << " 字节。" << std::endl;

  // (可选：将二进制数据写入文件，这就是“持久化”)
  std::ofstream out_file("ruleset.bin", std::ios::binary);
  out_file.write(serialized_data.c_str(), serialized_data.length());
  out_file.close();
  std::cout << "已将序列化数据持久化到 'ruleset.bin'" << std::endl;

  // ----------------------------------------------------
  // 5. 反序列化 (二进制字符串 -> C++ 对象)
  // ----------------------------------------------------

  // (我们假装这是一个新程序，从文件或网络读取了数据)
  // std::string data_from_disk = ... (从 'ruleset.bin' 读取)

  codelab::protobuf::RuleSetMessage rule_set_read;

  if (!rule_set_read.ParseFromString(serialized_data)) {
    std::cerr << "反序列化失败！" << std::endl;
    return;
  }

  std::cout << "\n反序列化后的 C++ 对象:\n"
            << rule_set_read.DebugString() << std::endl;

  // 6. 验证数据
  if (rule_set_read.version() == 1 &&
      rule_set_read.rules().at("admin_rule").expression() ==
          "request.auth.uid == 'admin'") {
    std::cout << "验证成功：反序列化的数据与原始数据一致！" << std::endl;
  } else {
    std::cout << "验证失败！" << std::endl;
  }

  std::cout << "--- Protobuf 序列化测试结束 ---" << std::endl;
}

int main() {
  RunProtoTest();
  return 0;
}
