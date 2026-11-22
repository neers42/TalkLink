#include <gtest/gtest.h>
#include <grpcpp/grpcpp.h>
#include <map>
#include <string>

#include "Infrastructure/GrpcSessionRepository.hpp"
#include "session.grpc.pb.h"

namespace {

// 本物のSessionGrpcAdapter(AuthenticationServer側)の代わりに、テスト専用の
// 最小実装を使う。auth_core(AuthenticationServer)とinfrastructure/domain
// (ChatServer)は共にグローバル名前空間に"RedisAdapter"等の同名クラスを
// 持つため同じテストバイナリにリンクできない。そのためここではGrpcSessionRepository
// (クライアント側/ワイヤーフォーマット)の実際の挙動を、ローカルの実gRPCサーバー
// 相手に検証する。
class TestSessionService final : public talklink::auth::SessionService::Service {
public:
    std::map<std::string, std::string> sessions; // hash -> uid
    std::string last_delete_requested_hash;

    grpc::Status ValidateSession(
        grpc::ServerContext*,
        const talklink::auth::ValidateSessionRequest* request,
        talklink::auth::ValidateSessionResponse* response) override
    {
        auto it = sessions.find(request->session_token_hash());
        if (it == sessions.end()) {
            response->set_valid(false);
            return grpc::Status::OK;
        }
        response->set_valid(true);
        response->set_uid(it->second);
        return grpc::Status::OK;
    }

    grpc::Status DeleteSession(
        grpc::ServerContext*,
        const talklink::auth::DeleteSessionRequest* request,
        talklink::auth::DeleteSessionResponse* response) override
    {
        last_delete_requested_hash = request->session_token_hash();
        const bool erased = sessions.erase(request->session_token_hash()) > 0;
        response->set_success(erased);
        return grpc::Status::OK;
    }
};

class GrpcSessionRepositoryTest : public ::testing::Test {
protected:
    void SetUp() override {
        grpc::ServerBuilder builder;
        builder.AddListeningPort("127.0.0.1:0", grpc::InsecureServerCredentials(), &bound_port);
        builder.RegisterService(&service);
        server = builder.BuildAndStart();
        ASSERT_NE(server, nullptr);
        ASSERT_NE(bound_port, 0);

        auto channel = grpc::CreateChannel("127.0.0.1:" + std::to_string(bound_port), grpc::InsecureChannelCredentials());
        repository = std::make_unique<GrpcSessionRepository>(channel);
    }

    void TearDown() override {
        if (server) {
            server->Shutdown();
        }
    }

    TestSessionService service;
    std::unique_ptr<grpc::Server> server;
    int bound_port = 0;
    std::unique_ptr<GrpcSessionRepository> repository;
};

TEST_F(GrpcSessionRepositoryTest, GetSessionReturnsUidForKnownHash) {
    service.sessions["hash-1"] = "uid-123";

    auto result = repository->get_session("hash-1");

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "uid-123");
}

TEST_F(GrpcSessionRepositoryTest, GetSessionReturnsNulloptForUnknownHash) {
    auto result = repository->get_session("no-such-hash");

    EXPECT_FALSE(result.has_value());
}

TEST_F(GrpcSessionRepositoryTest, DeleteKeyStripsSessionPrefixBeforeSendingOverWire) {
    service.sessions["hash-2"] = "uid-456";

    const bool deleted = repository->delete_key("session:hash-2");

    EXPECT_TRUE(deleted);
    EXPECT_EQ(service.last_delete_requested_hash, "hash-2");
    EXPECT_EQ(service.sessions.count("hash-2"), 0u);
}

TEST_F(GrpcSessionRepositoryTest, DeleteKeyReturnsFalseWhenSessionDoesNotExist) {
    const bool deleted = repository->delete_key("session:does-not-exist");

    EXPECT_FALSE(deleted);
}

} // namespace
