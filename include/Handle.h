#ifndef LIBCURLPP_ASYNC_HANDLE
#define LIBCURLPP_ASYNC_HANDLE

/*
 *  CurlPP-Async Handle
 *  12/29/19 16:11
 */

#include <curl/curl.h>

namespace CurlPPAsync
{
    // Handle allows the spawning of WebClients which asynchronously perform web requests
    class Handle
    {
    public:

    private:
        CURL* m_pCurl;
        CURLM* m_pMulti;
    };
}

#endif