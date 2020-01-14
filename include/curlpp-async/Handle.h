#ifndef LIBCURLPP_ASYNC_HANDLE_H_
#define LIBCURLPP_ASYNC_HANDLE_H_

/*
 *  CURLPP-Async Handle
 *  12/29/19 16:11
 */

// CURLPP-Async
#include <curlpp-async/WebClient.h>

// CURL
#define CURL_STATICLIB
#include <curl/curl.h>

// STL
#include <atomic>
#include <mutex>
#include <queue>
#include <unordered_map>

namespace CURLPPAsync
{
    // Handle allows the spawning of WebClients which asynchronously perform web requests. Nothing is thread-safe
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

        // Runs all asynchronous operations, and returns when they are all complete
        void Run();
    private:
        using CallbackMap_t = std::unordered_map<CURL*, WebClient::RecvCallback_t>;
        
        bool RegisterHandle(CURL* pCurl, WebClient::RecvCallback_t callback);
        void UnregisterHandle(CURL* pCurl);

        CURLM* m_multi;

        CallbackMap_t m_callbacks;

        std::mutex m_callbackMutex;
        std::mutex m_multiMutex;

		static std::mutex& GetCURLMutex();
		static std::atomic_size_t& GetRefCount();

        friend WebClient& WebClient::operator=(WebClient&& other) noexcept;
        friend WebClient::WebClient(Handle&) noexcept;
        friend WebClient::~WebClient() noexcept;
        friend void WebClient::AsyncGET(std::string, std::vector<WebClient::Header>, WebClient::RecvCallback_t);
        friend void WebClient::AsyncPOST(std::string, std::string, std::vector<WebClient::Header>, WebClient::RecvCallback_t);
    };
}

#endif