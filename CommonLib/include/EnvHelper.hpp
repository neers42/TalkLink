#ifndef ENVHELPER_HPP
#define ENVHELPER_HPP
#include <string>

// 環境変数を取得する。未設定の場合はstd::runtime_errorを投げる
std::string get_env(const char* name);

#endif // ENVHELPER_HPP
