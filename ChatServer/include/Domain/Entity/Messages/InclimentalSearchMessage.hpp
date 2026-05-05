#ifndef INCLIMENTALSEARCH_HPP
#define INCLIMENTALSEARCH_HPP
#include "Utils/common.hpp"
#include "MessageBase.hpp"
#include "IReceiveMessage.hpp"
class InclimentalSearchMessage :
    public MessageBase, public IReceiveMessage
{
public:
    enum class SearchType
    {
        BY_CLIENT,  //ユーザ名検索
        BY_CHATROOM,    //チャットルーム名検索
        BOTH    //両方検索
    };

private:
    SearchType _search_type;
    std::string _search_keyword;
    
public:
    //コンストラクタ
    InclimentalSearchMessage() = delete;
    InclimentalSearchMessage(const MessageBase::CommonField& common_fields, SearchType s_type, const std::string& keyword);

    //各種アクセッサ
    SearchType get_search_type() const;
    void set_search_type(SearchType s_type);
    std::string get_search_keyword() const;
    void set_search_keyword(const std::string& keyword);

    bool validate_payload(const json& message) const override;
    std::unique_ptr<MessageBase> create_instance(const json& message) const override;
    bool validate_value() const override;
    


    //デストラクタ
    ~InclimentalSearchMessage();

};


#endif // !INCLIMENTALSEARCH_HPP