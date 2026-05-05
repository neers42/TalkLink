#include "Domain/Entity/Messages/InclimentalSearchMessage.hpp"


InclimentalSearchMessage::InclimentalSearchMessage(const MessageBase::CommonField& common_fields, InclimentalSearchMessage::SearchType s_type, const std::string &keyword)
    :MessageBase(common_fields, MessageBase::MessageType::INCLIMENTAL_SEARCH), _search_type{s_type}, _search_keyword{keyword}
{
}

InclimentalSearchMessage::SearchType InclimentalSearchMessage::get_search_type() const
{
    return this->_search_type;
}

void InclimentalSearchMessage::set_search_type(InclimentalSearchMessage::SearchType s_type)
{
    this->_search_type = s_type;
}

std::string InclimentalSearchMessage::get_search_keyword() const
{
    return this->_search_keyword;
}

void InclimentalSearchMessage::set_search_keyword(const std::string &keyword)
{
    this->_search_keyword = keyword;
}

bool InclimentalSearchMessage::validate_payload(const json &message) const
{
    return false;
}

std::unique_ptr<MessageBase> InclimentalSearchMessage::create_instance(const json &message) const
{
    return std::unique_ptr<MessageBase>();
}

bool InclimentalSearchMessage::validate_value() const
{
    return false;
}

InclimentalSearchMessage::~InclimentalSearchMessage()
{
}
