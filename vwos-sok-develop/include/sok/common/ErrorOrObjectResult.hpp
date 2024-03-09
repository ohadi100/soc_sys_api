/* Copyright (c) 2023 Volkswagen Group */

#ifndef ERROR_CODE_OR_OBJECT_HPP
#define ERROR_CODE_OR_OBJECT_HPP

#include <cassert>
#include <memory>
#include <string>

namespace sok
{
namespace common
{
template <typename T_ERR>
class ErrorTuple final
{
public:
    ErrorTuple() = default;

    ErrorTuple(T_ERR err)
      : mResultCode(err)
    {
    }

    ErrorTuple(T_ERR err, std::string const& info)
      : mResultCode(err)
      , mInformation(info)
    {
    }

    bool
    isSucceeded() const
    {
        return (T_ERR::kSuccess == mResultCode);
    }

    std::string const&
    getInformation() const
    {
        return mInformation;
    }

    T_ERR
    getResultCode() const { return mResultCode; }

private:
    /*!
     * \brief Result code of the create function of class T.
     */
    T_ERR mResultCode;

    /*!
     * \brief Additional information about the error code.
     */
    std::string mInformation;
};

/*!
 * \brief Generic class used as return type of functions in order to return either an object or an error code.
 *
 * \details
 * When an object is returned, the pointer mObject points to the object and mResultCode is T_ERR::kSuccess.
 * Otherwise, in case of an error, the pointer mObject points to nullptr and the mResulCode contains an error code.
 *
 */
template <typename T_ERR, typename T_RES>
class ErrorOrObjectResult final
{
public:
    /*!
     * \brief Creates an instance with result code success and sets a valid object.
     *
     * \param[in] object  Object instance (must not be nullptr).
     */
    explicit ErrorOrObjectResult(T_RES object)
      : ErrorOrObjectResult(ErrorTuple<T_ERR>(T_ERR::kSuccess, std::string{}), object){};

    /*!
     * \brief Creates an instance with a specific error code and information, and sets the object to nullptr.
     *
     * \param[in] code  Error code (must not be T_ERR::kSuccess).
     * \param[in] information   more data about the error code (set default empty string).
     */
    explicit ErrorOrObjectResult(T_ERR const code, std::string const& information = {})
      : ErrorOrObjectResult(ErrorTuple<T_ERR>(code, information)){};

    ~ErrorOrObjectResult() = default;

    /*!
     * \brief Checks if the operation did not fail.
     *
     * \return true   Operation succeeded.
     *                (getObject() will return valid object, getResultCode() will return T_ERR::kSuccess.)
     *         false  An error occurred.
     *                (getObject() will assert, use getResultCode() to identify the error)
     */
    inline bool isSucceeded() const;


    /*!
     * \brief Checks if the operation did fail.
     *
     * \return true     An error occurred.
     *                  (getObject() will assert, use getResultCode() to identify the error)
     *         false    Operation succeeded.
     *                  (getObject() will return valid object, getResultCode() will return T_ERR::kSuccess.)
     */
    inline bool isFailed() const;

    /*!
     * \brief Returns the internal object if the operation was successful, otherwise the function will assert
     *
     * \return object.
     */
    inline T_RES const& getObject();

    /*!
     * \brief Returns the information.
     *
     * \return string to internal object.
     */
    inline std::string const& getInformation() const;

    /*!
     * \brief Returns the result code set while creation of the object.
     *
     * \return Result code according to template
     */
    T_ERR getResultCode() const;

private:
    ErrorOrObjectResult(ErrorTuple<T_ERR> const& code, T_RES object)
      : mIsEmpty(false)
      , mError{code}
      , mObject{object} {};

    ErrorOrObjectResult(ErrorTuple<T_ERR> const& code)
      : mIsEmpty(true)
      , mError{code} {};


    /*!
     * \brief On failure mIsEmpty is true
     */
    bool mIsEmpty;

    /*!
     * \brief Result code of the create function of class T. The code is a value of enum class
     * and Additional information about the error code.
     */
    ErrorTuple<T_ERR> mError;

    /*!
     * \brief the contained object. On failure mObject is nullptr
     */
    T_RES mObject;
};

template <typename T_ERR, typename T_RES>
bool
ErrorOrObjectResult<T_ERR, T_RES>::isSucceeded() const
{
    return ((T_ERR::kSuccess == mError.getResultCode()) && (mIsEmpty != true));
}

template <typename T_ERR, typename T_RES>
bool
ErrorOrObjectResult<T_ERR, T_RES>::isFailed() const
{
    return !isSucceeded();
}

template <typename T_ERR, typename T_RES>
T_RES const&
ErrorOrObjectResult<T_ERR, T_RES>::getObject()
{
    assert(!mIsEmpty);
    return mObject;
}

template <typename T_ERR, typename T_RES>
std::string const&
ErrorOrObjectResult<T_ERR, T_RES>::getInformation() const
{
    return mError.getInformation();
}


template <typename T_ERR, typename T_RES>
T_ERR
ErrorOrObjectResult<T_ERR, T_RES>::getResultCode() const
{
    return mError.getResultCode();
}

}  // namespace common
}  // namespace sok

#endif  // ERROR_CODE_OR_OBJECT_HPP