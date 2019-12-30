#ifndef LIBCURLPP_ASYNC_HANDLE
#define LIBCURLPP_ASYNC_HANDLE

/*
 *  CURLPP-Async Handle
 *  12/29/19 16:11
 */

#include <curl/curl.h>

#include <mutex>

namespace CURLPPAsync
{
    // Handle allows the spawning of WebClients which asynchronously perform web requests
    class Handle
    {
    public:
        Handle() noexcept;
        Handle(Handle&& other) noexcept;

        Handle(const Handle& other) = delete;
        Handle& operator=(const Handle& other) = delete;

        ~Handle() noexcept;
    private:
        CURLM* m_pMulti;

        static std::mutex s_curlMutex;

        static size_t s_refCount;
        static std::mutex s_refCountMutex;
    };
}

#endif