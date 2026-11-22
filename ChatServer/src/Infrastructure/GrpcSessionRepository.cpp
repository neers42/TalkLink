#include "Infrastructure/GrpcSessionRepository.hpp"
#include <grpcpp/grpcpp.h>
#include "LogHandler.hpp"

namespace {
// ISessionRepository::delete_keyは完成済みのキー全体("session:"+hash)を
// 受け取る(RedisAdapter::delete_keyと同じ意味論)。gRPC越しではハッシュ値
// のみを送るため、ここでプレフィックスを剥がす。
constexpr const char* kSessionKeyPrefix = "session:";

std::string strip_session_prefix(const std::string& key) {
    const std::string prefix(kSessionKeyPrefix);
    if (key.rfind(prefix, 0) == 0) {
        return key.substr(prefix.size());
    }
    return key;
}
} // namespace

GrpcSessionRepository::GrpcSessionRepository(const std::shared_ptr<grpc::Channel>& channel)
    : _stub(talklink::auth::SessionService::NewStub(channel))
{
}

GrpcSessionRepository::~GrpcSessionRepository() = default;

std::optional<std::string> GrpcSessionRepository::get_session(const std::string& session_token_hash)
{
    talklink::auth::ValidateSessionRequest request;
    request.set_session_token_hash(session_token_hash);

    talklink::auth::ValidateSessionResponse response;
    grpc::ClientContext context;

    const grpc::Status status = _stub->ValidateSession(&context, request, &response);
    if (!status.ok()) {
        LOG_ERROR("[GrpcSessionRepository] ValidateSession RPC failed: {}", status.error_message());
        return std::nullopt;
    }
    if (!response.valid()) {
        return std::nullopt;
    }
    return response.uid();
}

bool GrpcSessionRepository::delete_key(const std::string& key)
{
    talklink::auth::DeleteSessionRequest request;
    request.set_session_token_hash(strip_session_prefix(key));

    talklink::auth::DeleteSessionResponse response;
    grpc::ClientContext context;

    const grpc::Status status = _stub->DeleteSession(&context, request, &response);
    if (!status.ok()) {
        LOG_ERROR("[GrpcSessionRepository] DeleteSession RPC failed: {}", status.error_message());
        return false;
    }
    return response.success();
}
