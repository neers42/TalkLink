#ifndef IRECEIVEMESSAGE_HPP
#define IRECEIVEMESSAGE_HPP
class IReceiveMessage {

public:
    //デストラクタ
    virtual ~IReceiveMessage() = default;

    //各メッセージの特有のバリデーション(overrideで実装)
    virtual bool validate_specific_fields(const json& message) const = 0;
    //メッセージインスタンスの生成
    virtual std::unique_ptr<MessageBase> create_instance(const json& message) const = 0;
    //メッセージキーのチェック
    virtual bool validate_value() const = 0;

};
#endif // IRECEIVEMESSAGE_HPP