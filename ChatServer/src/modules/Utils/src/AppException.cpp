#include "AppException.h"
namespace exception
{
    /* コンストラクタ */
    AppException::AppException(const std::string& message)
        : ExceptionBase(message)
    {
        /* Nothing to do */
    }
    /* コンストラクタ */
    AppException::AppException(const std::string& message, const std::string& file, const std::string& func, const int line)
        : ExceptionBase(message, file, func, line)
    {
        /* Nothing to do */
    }
    /* デストラクタ */
    AppException::~AppException()
    {
        /* Nothing to do */
    }
}