#include <Handle.h>

using CURLPPAsync::Handle;

std::mutex Handle::s_curlMutex;

size_t Handle::s_refCount = 0;
std::mutex Handle::s_refCountMutex;

Handle::Handle() noexcept
{
	{
		// static initialization
		std::lock_guard refCountGuard(s_refCountMutex);
		if (s_refCount == 0)
		{
			std::lock_guard curlGuard(s_curlMutex);
			curl_global_init(CURL_GLOBAL_DEFAULT);
		}

		++s_refCount;
	}

	m_pMulti = curl_multi_init();
}

Handle::Handle(Handle&& other) noexcept
{
	m_pMulti = other.m_pMulti;
	other.m_pMulti = nullptr;

	{
		std::lock_guard refCountGuard(s_refCountMutex);
		++s_refCount;
	}
}

Handle::~Handle() noexcept
{
	if (m_pMulti != nullptr)
		curl_multi_cleanup(m_pMulti);

	{
		// static deinitialization
		std::lock_guard refCountGuard(s_refCountMutex);
		--s_refCount;

		if (s_refCount == 0)
		{
			std::lock_guard curlGuard(s_curlMutex);
			curl_global_cleanup();
		}
	}
}