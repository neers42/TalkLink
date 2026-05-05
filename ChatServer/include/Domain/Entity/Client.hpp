#ifndef CLIENT_HPP
#define CLIENT_HPP
#include "Utils/common.hpp"
#include <ctime>

// セッション情報構造体
struct SessionInfo {
    std::string session_id;
    time_t created_at;
    time_t expires_at;
    std::map<std::string, std::string> metadata;
};

class Client
{
public:
	enum class ConnectState{
		ACTIVE,
		DISCONNECTING,
		DISCONNECT
	};

	//Client private変数（ドメイン情報のみ）
private:
	std::string _id;						//クライアントID
	std::string _name;						//クライアント名
	std::string _mail_address;				//クライアントのメールアドレス
	std::string _password;					//クライアントのパスワード（必要に応じて）
	std::atomic<int> _heart_beat_count{0};	//ハートビート回数
	std::atomic<bool> _auth_status{false};	//認証状態
	std::atomic<ConnectState> _connection_state{ConnectState::ACTIVE};	//接続状態
	std::atomic<time_t> _last_activity{0};	//最終アクティビティ時間

public:
	// ドメインエンティティのコンストラクタ（インフラ情報なし）
	explicit Client();
	explicit Client(const std::string& id, const std::string& name, const std::string& mail_addr);
	explicit Client(const std::string& id, const std::string& name, const std::string& mail_addr, const std::string& password);
	~Client();


	//ドメイン情報のgetter
	std::string get_id() const;
	std::string get_name() const;
	std::string get_mail_address() const;
	std::string get_password() const;
	int get_heart_beat_count() const;
	bool get_auth_status() const;
	ConnectState get_connect_state() const;
	time_t get_last_activity() const;

	//ドメイン情報のsetter
	void set_id(const std::string& id);
	void set_name(const std::string& name);
	void set_mail_address(const std::string& mail_address);
	void set_password(const std::string& password);
	void set_heart_beat_count(const int count);
	void set_auth_status(const bool status);
	
	//ドメインロジック
	void increment_heart_beat_count();
	void clear_heart_beat_count();
	void reset_heart_beat_count(); // clear_heart_beat_countのエイリアス
	void change_auth_status();
	void set_connect_state(ConnectState state);
	bool compare_and_set_state(ConnectState& expected, ConnectState desired);
	void update_last_activity(time_t timestamp);

	//その他のドメイン操作
	bool is_active() const noexcept { return _connection_state == ConnectState::ACTIVE; }
	bool is_authenticated() const noexcept { return _auth_status; }
	bool can_disconnect() const;
	bool operator==(const Client& other_client) const;
	Client& operator=(const Client& other_client);
};
#endif // !CLIENT_HPP



