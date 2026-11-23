#ifndef GRPCSESSIONREPOSITORY_HPP
#define GRPCSESSIONREPOSITORY_HPP

#include <memory>
#include <string>
#include "Domain/Interfaces/ISessionRepository.hpp"
#include "session.grpc.pb.h"

namespace grpc { class Channel; }

// ISessionRepositoryのgRPCクライアント実装。AuthenticationServiceが公開する
// SessionServiceを呼び出し、セッションの検証/削除を行う。ChatServiceは
// これ経由でのみセッション情報にアクセスし、Redisには直接アクセスしない
// (Redis自体はAuthenticationServiceの内部実装詳細として隠蔽される)。
//
// 注: RedisAdapterはIRoomSequenceRepository(ルームシーケンスカウンター)
// としては引き続き使われる。このクラスはISessionRepositoryの役割だけを
// RedisAdapterから引き継ぐ。
class GrpcSessionRepository : public ISessionRepository
{
public:
    explicit GrpcSessionRepository(const std::shared_ptr<grpc::Channel>& channel);
    ~GrpcSessionRepository() override;

    std::optional<std::string> get_session(const std::string& session_token_hash) override;
    bool delete_key(const std::string& key) override;

private:
    std::unique_ptr<talklink::auth::SessionService::Stub> _stub;
};

#endif // GRPCSESSIONREPOSITORY_HPP
