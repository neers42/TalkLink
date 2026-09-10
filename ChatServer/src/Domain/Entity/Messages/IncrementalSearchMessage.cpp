#include "Domain/Entity/Messages/IncrementalSearchMessage.hpp"


IncrementalSearchMessage::IncrementalSearchMessage(const MessageBase::CommonField& common_fields, IncrementalSearchMessage::SearchType s_type, const std::string &keyword)
    :MessageBase(common_fields, MessageBase::MessageType::INCLIMENTAL_SEARCH), _search_type{s_type}, _search_keyword{keyword}
{
}

IncrementalSearchMessage::SearchType IncrementalSearchMessage::get_search_type() const
{
    return this->_search_type;
}

void IncrementalSearchMessage::set_search_type(IncrementalSearchMessage::SearchType s_type)
{
    this->_search_type = s_type;
}

std::string IncrementalSearchMessage::get_search_keyword() const
{
    return this->_search_keyword;
}

void IncrementalSearchMessage::set_search_keyword(const std::string &keyword)
{
    this->_search_keyword = keyword;
}

bool IncrementalSearchMessage::validate_payload(const json &message) const
{
    return false;
}

std::unique_ptr<MessageBase> IncrementalSearchMessage::create_instance(const json &message) const
{
    return std::unique_ptr<MessageBase>();
}

bool IncrementalSearchMessage::validate_value() const
{
    return false;
}

IncrementalSearchMessage::~IncrementalSearchMessage()
{
}
