#ifndef LIBCURLPP_ASYNC_WEBCLIENT_H_
#define LIBCURLPP_ASYNC_WEBCLIENT_H_

/*
 *  CURLPP-Async WebClient
 *  12/29/19 20:09
 */

// CURL
#include <curl/curl.h>

// STL
#include <functional>
#include <string>

namespace CURLPPAsync
{
    class Handle;

    // WebClient is a web client that wraps around a CURL easy handle, which has its own cookies and settings.
    // Despite the name, it can be instantiated and used on its own synchronously with WebClient::{METHOD}. The
    // internal buffer can be fetched with WebClient::GetBuffer, and options can be set with WebClient::SetOpt.
    // It must be created on the same thread as the handle.
    class WebClient
    {
    public:
        using RecvCallback_t = std::function<void(const CURLcode code)>;

        struct Header
        {
            std::string m_fieldName;
            std::string m_data;
        };

        // Creates a webclient with reference to a handle
        WebClient(Handle& handle) noexcept;

        WebClient(WebClient&& other) noexcept;
        WebClient& operator=(WebClient&& other) noexcept;

        WebClient(const WebClient& other) = delete;
        WebClient& operator=(const WebClient& other) = delete;

        // Cancels any async operations and calls the handler with
        ~WebClient() noexcept;

        // Starts a synchronous GET operation on a URL, and returns the result.
        // Note that there must not already be an in-progress operation on the object.
        // std::runtime_error will be thrown in this case.
        CURLcode GET(const std::string& url, const std::vector<Header>& customHeaders);

    private:
        void AsyncGETImpl(const std::string url, const std::vector<Header> customHeaders, const RecvCallback_t recvCallback);
    public:
        // Starts an Asynchronous GET operation on a URL, and calls recvCallback on completion.
        // Note that there must not already be an in-progress operation on the object.
        // std::runtime_error will be thrown in this case.
        template<typename string, typename headerVector, typename RecvCallback>
        void AsyncGET(string&& url, headerVector&& customHeaders, RecvCallback&& recvCallback)
        {
            AsyncGETImpl(std::forward<string>(url), std::forward<headerVector>(customHeaders), std::forward<RecvCallback>(recvCallback));
        }

        // Starts a synchronous POST operation on a URL, and returns the result.
        // Note that there must not already be an in-progress operation on the object.
        // std::runtime_error will be thrown in this case.
        CURLcode POST(const std::string& url, const std::string& postData, const std::vector<Header>& customHeaders);

    private:
        void AsyncPOSTImpl(const std::string url, const std::string postData, const std::vector<Header> customHeaders, const RecvCallback_t recvCallback);
    public:
        // Starts an Asynchronous POST operation on a URL, and calls recvCallback on completion.
        // Note that there must not already be an in-progress operation on the object.
        // std::runtime_error will be thrown in this case.
        template<typename string, typename string2, typename headerVector, typename RecvCallback>
        void AsyncPOST(string&& url, string2&& postData, headerVector&& customHeaders, RecvCallback&& recvCallback)
        {
            AsyncPOSTImpl(std::forward<string>(url), std::forward<string2>(postData), std::forward<headerVector>(customHeaders), std::forward<RecvCallback>(recvCallback));
        }

        // Sets an option
        template<typename T>
        void SetOpt(const CURLoption opt, const T& val)
        {
            curl_easy_setopt(m_curl, opt, val);
        }

        // Gets Info. T MUST be char*, long, curl_slist*, or double
        template<typename T>
        T GetInfo(const CURLINFO info)
        {
            T val;
            if (curl_easy_getinfo(m_curl, info, &val) != CURLE_OK)
                return 0;

            return val;
        }

        std::string GetData() const { return m_data; }
    private:
        CURL* m_curl;
        std::reference_wrapper<Handle> m_handle;

        std::string m_data;

        static size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata);
    };
}

#endif