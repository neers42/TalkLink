#include "Infrastructure/SessionGrpcAdapter.hpp"
#include "Domain/Interfaces/IRedisAdapter.hpp"
#include "LogHandler.hpp"

SessionGrpcAdapter::SessionGrpcAdapter(std::shared_ptr<IRedisAdapter> redis_adapter)
    : _redis_adapter(std::move(redis_adapter))
{
    if (!_redis_adapter) {
        throw std::runtime_error("Failed to initialize SessionGrpcAdapter dependencies");
    }
}

grpc::Status SessionGrpcAdapter::ValidateSession(
    grpc::ServerContext* /*context*/,
    const talklink::auth::ValidateSessionRequest* request,
    talklink::auth::ValidateSessionResponse* response)
{
    try {
        auto uid = _redis_adapter->get_session(request->session_token_hash());
        response->set_valid(uid.has_value());
        response->set_uid(uid.value_or(""));
        return grpc::Status::OK;
    } catch (const std::exception& e) {
        LOG_ERROR("[SessionGrpcAdapter] Exception in ValidateSession: {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status SessionGrpcAdapter::DeleteSession(
    grpc::ServerContext* /*context*/,
    const talklink::auth::DeleteSessionRequest* request,
    talklink::auth::DeleteSessionResponse* response)
{
    try {
        // IRedisAdapter::delete_keyは完成済みのキー全体を受け取る
        // (ChatServer側のISessionRepository::delete_keyと同じ意味論)。
        const std::string session_key = "session:" + request->session_token_hash();
        response->set_success(_redis_adapter->delete_key(session_key));
        return grpc::Status::OK;
    } catch (const std::exception& e) {
        LOG_ERROR("[SessionGrpcAdapter] Exception in DeleteSession: {}", e.what());
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}
