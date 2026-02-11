#ifndef ISENDMESSAGE_HPP
#define ISENDMESSAGE_HPP
class ISendMessage {
public:
    //デストラクタ
    virtual ~ISendMessage() = default;
    //各メッセージの特有のシリアライズ(overrideで実装)
    virtual json serialize_payload() const = 0;
    //メッセージインスタンスの内容検証(overrideで実装)
    virtual bool validate_payload() const = 0;
};
#endif // ISENDMESSAGE_HPP