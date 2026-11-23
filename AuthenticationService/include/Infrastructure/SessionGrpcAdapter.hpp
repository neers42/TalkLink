#ifndef SESSIONGRPCADAPTER_HPP
#define SESSIONGRPCADAPTER_HPP

#include <memory>
#include "session.grpc.pb.h"

class ISessionStore;

// ChatServiceに公開するセッション検証/削除APIのgRPCサーバー側実装。
// 読み書き自体はAuthenticationServiceが所有するRedisに対して行う
// (ISessionStore経由)。ChatServiceはこのAPI越しにしかセッション情報へ
// アクセスできない(Redisを直接見ることはない)。
class SessionGrpcAdapter final : public talklink::auth::SessionService::Service
{
public:
    explicit SessionGrpcAdapter(std::shared_ptr<ISessionStore> session_store);

    grpc::Status ValidateSession(
        grpc::ServerContext* context,
        const talklink::auth::ValidateSessionRequest* request,
        talklink::auth::ValidateSessionResponse* response) override;

    grpc::Status DeleteSession(
        grpc::ServerContext* context,
        const talklink::auth::DeleteSessionRequest* request,
        talklink::auth::DeleteSessionResponse* response) override;

private:
    std::shared_ptr<ISessionStore> _session_store;
};

#endif // SESSIONGRPCADAPTER_HPP
