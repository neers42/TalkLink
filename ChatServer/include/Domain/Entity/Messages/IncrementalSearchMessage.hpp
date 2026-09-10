#ifndef INCREMENTALSEARCHMESSAGE_HPP
#define INCREMENTALSEARCHMESSAGE_HPP
#include "Utils/common.hpp"
#include "MessageBase.hpp"
#include "IReceiveMessage.hpp"
class IncrementalSearchMessage :
    public MessageBase, public IReceiveMessage
{
public:
    enum class SearchType
    {
        BY_USER,  //ユーザ名検索
        BY_CHATROOM,    //チャットルーム名検索
        BOTH    //両方検索
    };

private:
    SearchType _search_type;
    std::string _search_keyword;
    
public:
    //コンストラクタ
    IncrementalSearchMessage() = delete;
    IncrementalSearchMessage(const MessageBase::CommonField& common_fields, SearchType s_type, const std::string& keyword);

    //各種アクセッサ
    SearchType get_search_type() const;
    void set_search_type(SearchType s_type);
    std::string get_search_keyword() const;
    void set_search_keyword(const std::string& keyword);

    bool validate_payload(const json& message) const override;
    std::unique_ptr<MessageBase> create_instance(const json& message) const override;
    bool validate_value() const override;
    


    //デストラクタ
    ~IncrementalSearchMessage();

};


#endif // !INCREMENTALSEARCHMESSAGE_HPP