#ifndef LIBCURLPP_ASYNC_HANDLE_H_
#define LIBCURLPP_ASYNC_HANDLE_H_

/*
 *  CURLPP-Async Handle
 *  12/29/19 16:11
 */

// CURLPP-Async
#include <curlpp-async/WebClient.h>

// CURL
#include <curl/curl.h>

// STL
#include <mutex>
#include <unordered_map>

namespace CURLPPAsync
{
    // Handle allows the spawning of WebClients which asynchronously perform web requests
    class Handle
    {
    public:
        // Creates an empty handle
        Handle() noexcept;

        Handle(Handle&& other) noexcept;
        Handle& operator=(Handle&& other) noexcept;

        Handle(const Handle& other) = delete;
        Handle& operator=(const Handle& other) = delete;

        ~Handle() noexcept;

        // Runs all queued asynchronous operations, and returns when they are all complete
        void Run();
    private:
        using CallbackMap_t = std::unordered_map<CURL*, WebClient::RecvCallback_t>;

        CURLM* m_multi;

        CallbackMap_t m_callbacks;

        static std::mutex s_curlMutex;

        static size_t s_refCount;
        static std::mutex s_refCountMutex;

        friend WebClient::WebClient(Handle&) noexcept;
        friend WebClient::WebClient(WebClient&&) noexcept;
        friend WebClient& WebClient::operator=(WebClient&&) noexcept;
        friend WebClient::~WebClient() noexcept;
        friend CURLcode WebClient::GET(const std::string&, const std::vector<WebClient::Header>&);
        friend void WebClient::AsyncGET(std::string, std::vector<WebClient::Header>, WebClient::RecvCallback_t);
        friend void WebClient::AsyncPOST(std::string, std::string, std::vector<WebClient::Header>, WebClient::RecvCallback_t);
    };
}

#endif