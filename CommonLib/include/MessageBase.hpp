#ifndef MESSAGEBASE_HPP
#define MESSAGEBASE_HPP
#include "common.hpp"

using json = nlohmann::json;

class MessageBase
{
public :
    enum class ContentType
    {
        TEXT,         // テキスト
        IMAGE,        // 画像
        VIDEO,        // 動画
        NOTIFICATION, // 通知
        HEARTBEAT,    // ハートビート
        AUTHENTICATION // 認証リクエスト
    };

protected:
    //変数宣言
    ContentType _content_type;	//メッセージコンテンツの内容
	std::string _message_type;	//メッセージタイプ
	std::string _time_stamp;	//タイムスタンプ
	uint64_t _id;	//メッセージID

public:
    

    MessageBase();
    MessageBase(ContentType c_type, const std::string &mt);
    MessageBase(int c_type, const std::string &mt);
    MessageBase(ContentType c_type, const std::string &m_type, const uint64_t id);
    MessageBase(ContentType c_type, const std::string &m_type, const uint64_t id, const std::string &time_stamp);
    MessageBase(int c_type, const std::string &m_type, const uint64_t id, const std::string &time_stamp);
    virtual ~MessageBase();

    //アクセッサ
    ContentType get_content_type() const {
        return this->_content_type;
    }
    void set_content_type(ContentType c_type) {
        this->_content_type = c_type;
    }
    std::string get_message_type() const {
        return this->_message_type;
    }
    void set_message_type(const std::string &mt) {
        this->_message_type = mt;
    }
    std::string get_time_stamp() const {
        return this->_time_stamp;
    }
    void set_time_stamp(const std::string &ts) {
        this->_time_stamp = ts;
    }
    uint64_t get_message_id() const {
        return this->_id;
    }
    void set_message_id(const uint64_t id) {
        this->_id = id;
    }

    // デシリアライズメソッド
    template <typename T>
    static std::unique_ptr<T> deserialize_message(const json &json_message)
    {
        // Tが抽象クラスでないことをコンパイル時にチェック
        static_assert(!std::is_abstract_v<T>, "T must be a concrete (non-abstract) class.");
        try
        {
            // 1. 共通のバリデーション
            if (!validate_common_json_fields(json_message))
            {
                throw std::runtime_error("Common field validation failed");
            }

            // 2. 派生クラス固有のバリデーション（仮想関数）
            T message_instance;
            if (!message_instance.validate_specific_fields(json_message))
            {
                throw std::runtime_error("Specific field validation failed.");
            }

            // 3. インスタンス生成（MessageBaseポインタとして返る）
            std::unique_ptr<MessageBase> base_ptr = message_instance.create_instance(json_message);
            if (!base_ptr)
            {
                throw std::runtime_error("create_instance returned nullptr.");
            }

            // 4. T型ポインタに安全にキャストして返す
            if (auto derived_ptr = dynamic_cast<T *>(base_ptr.release()))
            {
                //
                if (derived_ptr->validate_value() == false)
                {
                    throw std::runtime_error("Value validation failed.");
                }else if (derived_ptr == nullptr)
                {
                    throw std::runtime_error("Derived pointer is null.");
                }
                
                return std::unique_ptr<T>(derived_ptr);
            }
            else
            {
                throw std::runtime_error("Failed to cast MessageBase to the desired type T.");
            }
        }
        catch (const std::runtime_error &re)
        {
            LOG_ERROR("Runtime error : {}", re.what());
            return nullptr;
        }
        catch (const std::exception &e)
        {
            LOG_ERROR("Error serializing message: {}", e.what());
            return nullptr;
        }
    }

    // シリアライズメソッド
    template<typename T>
    static json serialize_message(const std::unique_ptr<T>& message_ptr) {
        static_assert(!std::is_abstract_v<T>, "T must be a concrete (non-abstract) class");
        
        if (!message_ptr) {
            spdlog::error("Message pointer is null");
            return json{};
        }
        
        try {
            // 1. 共通フィールドの変換
            json result = serialize_common_message_fields(*message_ptr);
            if (result.is_null()) {
                throw std::runtime_error("Failed to serialize common message fields");
            }
            
            // 2. 派生クラス固有のフィールド変換（仮想関数）
            json specific_fields = message_ptr->serialize_specific_message_fields();
            if (specific_fields.is_null()) {
                throw std::runtime_error("Failed to serialize specific message fields");
            }
            // 3. マージして返す
            result.merge_patch(specific_fields);
            if (result.is_null()) {
                throw std::runtime_error("Resulting JSON is null after merging specific fields");
            }
            return result;

        } catch (const std::runtime_error &re) {
            spdlog::error("Runtime error : {}", re.what());
            return json{};
        } 
        catch (const std::exception& e) {
            spdlog::error("Error deserializing message: {}", e.what());
            return json{};
        }
    }

private:
    // 共通フィールドのバリデーション
    static bool validate_common_json_fields(const json& message);

    // 共通フィールドのシリアライゼーション
    static json serialize_common_message_fields(const MessageBase& message);
    // 共通フィールドのバリデーション（MessageBaseインスタンス版）
    static bool validate_common_message_fields(const MessageBase& message);

protected:
    std::tuple<MessageBase::ContentType, std::string, uint64_t, std::string> parse_common_fields(const json& message) const;

};

#endif // MESSAGEBASE_HPP
