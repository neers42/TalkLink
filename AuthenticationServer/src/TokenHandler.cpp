#include "TokenHandler.hpp"

namespace {
std::uint64_t get_exp_leeway_seconds() {
    constexpr std::uint64_t kDefaultLeewaySec = 60;
    const char* raw = std::getenv("JWT_EXP_LEEWAY_SEC");
    if (raw == nullptr) {
        return kDefaultLeewaySec;
    }

    try {
        return std::stoull(raw);
    } catch (...) {
        spdlog::warn("Invalid JWT_EXP_LEEWAY_SEC value: {}. Fallback to {} sec.", raw, kDefaultLeewaySec);
        return kDefaultLeewaySec;
    }
}
}

TokenHandler::TokenHandler() {
    // 必要なら初期化処理を書く
}

TokenHandler::~TokenHandler() {
    // 必要なら破棄処理を書く
}

bool TokenHandler::verify_token(const std::string &token, std::string &uid, std::uint64_t &expire)
{
    try {
        if(token.empty()) {
            LOG_ERROR("Token is empty.");
            return false; // Token is empty
        }
        // jwtトークンをデコード
        auto decoded_token = jwt::decode(token);
        std::string kid = decoded_token.get_key_id();
        spdlog::debug("Setting up JWT verifier with KID: {}", kid);
        std::string key_pass = get_key_pass(kid);
        if (key_pass.empty()) {
            LOG_ERROR("No PEM file found for KID: {}", kid);
            return false; // No PEM file found for the given KID
        }
        std::string rsa_pub_key = read_pem_file(key_pass);
        if (rsa_pub_key.empty()) {
            LOG_ERROR("Failed to read PEM file for KID: {}", kid);
            return false; // Failed to read the PEM file
        }else{
            spdlog::debug("PEM file content read successfully for KID: {}", kid);
        }
        const auto leeway_sec = get_exp_leeway_seconds();
        auto verify = jwt::verify()
                          .allow_algorithm(jwt::algorithm::rs256(rsa_pub_key, "", "", ""))
                          .with_issuer("https://securetoken.google.com/talklink-8b0d0")
                          .with_audience("talklink-8b0d0")
                          .leeway(leeway_sec);

        verify.verify(decoded_token); // Verify the token

        if(decoded_token.has_subject() == true) {
            uid = decoded_token.get_subject();
            spdlog::debug("Token subject (CID): {}", uid);
        }else{
            LOG_ERROR("Token does not have a subject.");
            return false; // Token does not have a subject
        }
        if(decoded_token.has_expires_at()) {
            auto tp = decoded_token.get_expires_at(); // system_clock::time_point 型
            expire = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count(); // UNIXエポックからのdurationを取得
        } else {
            LOG_ERROR("Token does not have an expiration time.");
            return false; // Token does not have an expiration time
        }
        return true;
    } catch (const std::exception& e) {
        try {
            auto decoded_token = jwt::decode(token);
            if (decoded_token.has_expires_at()) {
                const auto exp_tp = decoded_token.get_expires_at();
                const auto now_tp = std::chrono::system_clock::now();
                const auto exp_epoch = std::chrono::duration_cast<std::chrono::seconds>(exp_tp.time_since_epoch()).count();
                const auto now_epoch = std::chrono::duration_cast<std::chrono::seconds>(now_tp.time_since_epoch()).count();
                const auto delta_sec = now_epoch - exp_epoch;
                LOG_ERROR("JWT verify failed: {}, exp_epoch={}, now_epoch={}, now-exp={} sec", e.what(), exp_epoch, now_epoch, delta_sec);
            } else {
                LOG_ERROR("JWT verify failed: {}", e.what());
            }
        } catch (...) {
            LOG_ERROR("JWT verify failed: {}", e.what());
        }
        return false; // Token is invalid
    }
}

const std::string TokenHandler::get_key_pass(const std::string& kid) const {
    for(const auto& file : std::filesystem::directory_iterator(CERTS_DIR)) {
        if (file.path().filename() == kid + ".pem") {
            return file.path().string();
        }
    }
    return std::string();
}

const std::string TokenHandler::read_pem_file(const std::string &file_path) const
{
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open PEM file: " + file_path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
