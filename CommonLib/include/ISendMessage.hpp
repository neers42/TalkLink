#ifndef ISENDMESSAGE_HPP
#define ISENDMESSAGE_HPP
class ISendMessage {
public:
    //デストラクタ
    virtual ~ISendMessage() = default;
    //各メッセージの特有のシリアライズ(overrideで実装)
    virtual json serialize_specific_message_fields() const = 0;
    //メッセージインスタンスの内容検証(overrideで実装)
    virtual bool validate_specific_message_fields() const = 0;
};
#endif // ISENDMESSAGE_HPP