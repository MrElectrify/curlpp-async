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

        // Runs all queued asynchronous operations, and returns when they are all complete
        void Run();
        // Runs all queued asynchronous operations, adding operations from a mutex-protected 
        // queue if they appear before the transfer or mid-transfer. Returns when all are complete 
		// and the queue is empty
        // MidTransferOperationQueue example: std::queue<std::function<void()>>
        // A queue of void returning functions, with front() and pop()
        // Mutex_t example: std::mutex
        // A mutex with lock() and unlock()
        template<typename MidTransferOperationQueue_t, typename Mutex_t>
        void Run(MidTransferOperationQueue_t& midTransferOperationQueue, Mutex_t& mutex)
        {
			int still_running = 0;
			while (true)
			{
				// empty out the operation queue
				{
					std::lock_guard opGuard(mutex);
					while (midTransferOperationQueue.empty() == false)
					{
						midTransferOperationQueue.front()();
						midTransferOperationQueue.pop();
					}
				}

				curl_multi_perform(m_multi, &still_running);

				timeval timeout;
				fd_set fdread;
				fd_set fdwrite;
				fd_set fdexcep;
				int maxfd = -1;

				long curl_timeo;

				curl_multi_timeout(m_multi, &curl_timeo);
				if (curl_timeo < 0)
					curl_timeo = 1000;

				timeout.tv_sec = curl_timeo / 1000;
				timeout.tv_usec = (curl_timeo % 1000) * 1000;

				FD_ZERO(&fdread);
				FD_ZERO(&fdwrite);
				FD_ZERO(&fdexcep);

				// get file descriptors from the transfers 
				CURLMcode mc = curl_multi_fdset(m_multi, &fdread, &fdwrite, &fdexcep, &maxfd);
				if (mc != CURLM_OK)
				{
					// fdset failed, call all handlers with the error
					for (CallbackMap_t::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it)
					{
						// tell them to try again
						const CallbackMap_t::value_type::second_type callback = it->second;
						it = m_callbacks.erase(it);
						callback(CURLE_AGAIN);
					}
					return;
				}

				int rc;
				if (maxfd == -1) {
#ifdef _WIN32
					Sleep(100);
					rc = 0;
#else
					/* Portable sleep for platforms other than Windows. */
					struct timeval wait = { 0, 100 * 1000 }; /* 100ms */
					rc = select(0, NULL, NULL, NULL, &wait);
#endif		
				}
				else
					rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);

				if (rc == -1)
				{
					// select failed, call all handlers with the error
					for (CallbackMap_t::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it)
					{
						// tell them to try again
						const CallbackMap_t::value_type::second_type callback = it->second;
						it = m_callbacks.erase(it);
						callback(CURLE_AGAIN);
					}
					break;
				}

				// timeout or readable/writable sockets
				curl_multi_perform(m_multi, &still_running);

				// check the transfers and see how they went
				int msgs_left;
				CURLMsg* msg;
				while ((msg = curl_multi_info_read(m_multi, &msgs_left)) != nullptr)
				{
					if (msg->msg == CURLMSG_DONE)
					{
						// find the callback (which should always be present), call it, and remove it
						const CallbackMap_t::iterator callbackIt = m_callbacks.find(msg->easy_handle);
						const CallbackMap_t::value_type::second_type callback = callbackIt->second;
						m_callbacks.erase(callbackIt);

						// remove the handle
						curl_multi_remove_handle(m_multi, msg->easy_handle);

						callback(msg->data.result);

						// a callback added a task, keep the loop going
						if (m_callbacks.size() > 0 &&
							still_running == 0)
							++still_running;
					}
				}

				{
					std::lock_guard opGuard(mutex);
					while (midTransferOperationQueue.empty() == false)
					{
						midTransferOperationQueue.front()();
						midTransferOperationQueue.pop();
						++still_running;
					}
				}
				
				if (still_running == 0)
					break;
			}
        }
    private:
        using CallbackMap_t = std::unordered_map<CURL*, WebClient::RecvCallback_t>;
        
        template<typename Callback_t>
        bool RegisterHandle(CURL* pCurl, Callback_t&& callback)
        {
            const bool res = m_callbacks.emplace(pCurl, std::forward<Callback_t>(callback)).second;
            
            // add the cURL handle to multi
            curl_multi_add_handle(m_multi, pCurl);

            return res;
        }
        void UnregisterHandle(CURL* pCurl);
        CallbackMap_t::const_iterator FindHandle(CURL* pCurl) const;

        CURLM* m_multi;

        CallbackMap_t m_callbacks;

        std::mutex m_callbackMutex;
        std::mutex m_multiMutex;

		static std::mutex& GetCURLMutex();
		static std::atomic_size_t& GetRefCount();

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