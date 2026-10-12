#ifndef MESSAGEBASE_HPP
#define MESSAGEBASE_HPP
#include "common.hpp"

using json = nlohmann::json;


class MessageBase
{
public :
    enum class MessageType
    {
        CHAT_MESSAGE,
        CHAT_ACK,
        CHAT_DELIVERY,
        CHATROOM_CONTROL,
        CHATROOM_CONTROL_ACK,
        CHATROOM_CONTROL_DELIVERY,
        HEARTBEAT_MESSAGE,
        AUTHENTICATION_REQUEST,
        AUTHENTICATION_RESPONSE,
        SESSION_REQUEST,
        SESSION_RESPONSE,
        INCLIMENTAL_SEARCH,
        USER_SETTING_CONTROL,
        USER_SEARCH_REQUEST,
        USER_SEARCH_RESPONSE,
        SYNC_REQUEST,
        SYNC_RESPONSE
    };

    struct CommonField
    {
        MessageBase::MessageType _message_type;  //メッセージタイプ
        std::string _correlation_id; //クライアントメッセージの識別ID
        std::string _timestamp;	//タイムスタンプ

        //コンストラクタ
        CommonField() : _message_type(MessageBase::MessageType::CHAT_MESSAGE), 
        _timestamp(ServerTime::get_current_time_to_milliseconds()),
         _correlation_id("") {}
        CommonField(const MessageBase::MessageType mt, const std::string& id)
        : _message_type(mt), _timestamp(ServerTime::get_current_time_to_milliseconds()), _correlation_id(id) {}
    };

    

protected:
    //変数宣言
    CommonField _common_fields;  // 共通フィールド

public:
    

    MessageBase();
    MessageBase(const MessageBase::MessageType mt);
    MessageBase(const MessageBase::MessageType mt, const std::string &id);
    MessageBase(const CommonField& common_fields);
    MessageBase(const CommonField& common_fields, const MessageBase::MessageType mt);
    virtual ~MessageBase();
    

    //アクセッサ
    MessageBase::CommonField get_common_fields() const {
        return this->_common_fields;
    }

    MessageBase::MessageType get_message_type() const {
        return this->_common_fields._message_type;
    }
    void set_message_type(const MessageBase::MessageType &mt) {
        this->_common_fields._message_type = mt;
    }
    std::string get_time_stamp() const {
        return this->_common_fields._timestamp;
    }
    void set_time_stamp(const std::string &ts) {
        this->_common_fields._timestamp = ts;
    }
    std::string get_correlation_id() const {
        return this->_common_fields._correlation_id;
    }
    void set_correlation_id(const std::string &id) {
        this->_common_fields._correlation_id = id;
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
            if (!message_instance.validate_payload(json_message))
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
            
            // 2. 派生クラス固有のフィールド変換
            json specific_fields = message_ptr->serialize_payload();
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
    static MessageBase::MessageType parse_message_type(const std::string& message_type_str);
    std::string message_type_to_string(const MessageBase::MessageType& message_type) const;

private:
    // 共通フィールドのバリデーション
    static bool validate_common_json_fields(const json& message);

    // 共通フィールドのシリアライゼーション
    static json serialize_common_message_fields(const MessageBase& message);
    // 共通フィールドのバリデーション（MessageBaseインスタンス版）
    static bool validate_common_message_fields(const MessageBase& message);

protected:
    CommonField parse_common_fields(const json& message) const;
    
    // タイムスタンプの正規化機能
    static std::string normalize_timestamp(const std::string& timestamp);
    static bool is_iso8601_format(const std::string& timestamp);

};

#endif // MESSAGEBASE_HPP
