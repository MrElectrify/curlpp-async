#include <curlpp-async/Handle.h>

using CURLPPAsync::Handle;

Handle::Handle() noexcept
{
	if (GetRefCount()++ == 0)
	{
		std::lock_guard curlGuard(GetCURLMutex());
		curl_global_init(CURL_GLOBAL_DEFAULT);
	}

	m_multi = curl_multi_init();
}

Handle::Handle(Handle&& other) noexcept
{
	m_multi = other.m_multi;
	other.m_multi = nullptr;

	++GetRefCount();
}

Handle& Handle::operator=(Handle&& other) noexcept
{
	// free multi handle if one already exists
	if (m_multi != nullptr)
		curl_multi_cleanup(m_multi);

	m_multi = other.m_multi;
	other.m_multi = nullptr;

	return *this;
}

Handle::~Handle() noexcept
{
	if (m_multi != nullptr)
		curl_multi_cleanup(m_multi);

	if (--GetRefCount() == 0)
	{
		std::lock_guard curlGuard(GetCURLMutex());
		curl_global_cleanup();
	}
}

void Handle::Run()
{
	int still_running = 0;
	// try to perform now
	curl_multi_perform(m_multi, &still_running);
	while (still_running != 0)
	{
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
	}
}

void Handle::UnregisterHandle(CURL* pCurl)
{
	curl_multi_remove_handle(m_multi, pCurl);
	m_callbacks.erase(pCurl);
}

Handle::CallbackMap_t::const_iterator Handle::FindHandle(CURL* pCurl) const
{
	return m_callbacks.find(pCurl);
}

std::mutex& Handle::GetCURLMutex()
{
	static std::mutex s_curlMutex;
	return s_curlMutex;
}

std::atomic_size_t& Handle::GetRefCount()
{
	static std::atomic_size_t s_refCount = 0;
	return s_refCount;
}