#ifndef APPEXCEPTION_H_
#define APPEXCEPTION_H_

#include "ExceptionBase.h"
#include <vector>
namespace exception
{
    /* Application Exceptionクラス宣言 */
    class AppException : public ExceptionBase
    {
    public:
        /* コンストラクタ */
        AppException(const std::string& message);
        /* コンストラクタ */
        AppException(const std::string& message, const std::string& file, const std::string& func, const int line);
        /* デストラクタ */
        virtual ~AppException();
    };
}
/* Application Exceptionのthrow */
#define THROW_APP_EXCEPTION(message) \
    throw exception::AppException(message, __FILE__, __FUNCTION__, __LINE__)


#endif // !APPEXCEPTION_H_

