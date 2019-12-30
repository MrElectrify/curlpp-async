#include <WebClient.h>

#include <Handle.h>

using CURLPPAsync::Handle;
using CURLPPAsync::WebClient;

WebClient::WebClient(Handle& handle) noexcept
	: m_handle(handle) 
{
	m_curl = curl_easy_init();
}

WebClient::WebClient(WebClient&& other) noexcept
	: m_handle(other.m_handle)
{
	m_curl = other.m_curl;

	other.m_curl = curl_easy_init();
}

WebClient& WebClient::operator=(WebClient&& other) noexcept
{
	// remove ourselves from the handle and clean up
	m_handle.get().m_callbacks.erase(m_curl);
	curl_easy_cleanup(m_curl);

	m_curl = other.m_curl;
	m_handle = other.m_handle;

	other.m_curl = curl_easy_init();
	
	return *this;
}

WebClient::~WebClient() noexcept
{
	// clean up
	m_handle.get().m_callbacks.erase(m_curl);
	curl_easy_cleanup(m_curl);
}

CURLcode WebClient::GET(const std::string& url) noexcept
{
	curl_easy_setopt(m_curl, CURLOPT_URL, url.data());

	// clear data and set
	m_data.clear();

	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &WebClient::WriteCallback);
	curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &m_data);

	curl_easy_setopt(m_curl, CURLOPT_CAINFO, "cacert.pem");

	return curl_easy_perform(m_curl);
}

void WebClient::AsyncGET(const std::string& url,
	RecvCallback_t&& recvCallback)
{
	curl_easy_setopt(m_curl, CURLOPT_URL, url.data());

	// clear data and set
	m_data.clear();

	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &WebClient::WriteCallback);
	curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &m_data);

	curl_easy_setopt(m_curl, CURLOPT_CAINFO, "cacert.pem");

	// add the handler
	if (m_handle.get().m_callbacks.emplace(m_curl, recvCallback).second == false)
		throw std::runtime_error("Only one operation can be done at a time for a given client");

	// add ourselves to multi
	curl_multi_add_handle(m_handle.get().m_multi, m_curl);
}

size_t WebClient::WriteCallback(char* ptr, size_t size, size_t nmemb, void* userData)
{
	if (nmemb)
		reinterpret_cast<std::string*>(userData)->append(ptr, nmemb);
	return nmemb;
}