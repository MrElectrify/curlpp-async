#include <curlpp-async/WebClient.h>

#include <curlpp-async/Handle.h>

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

CURLcode WebClient::GET(const std::string& url, 
	const std::vector<Header>& headers)
{
	curl_easy_setopt(m_curl, CURLOPT_URL, url.data());

	// clear data and set
	m_data.clear();

	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &WebClient::WriteCallback);
	curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &m_data);

	curl_easy_setopt(m_curl, CURLOPT_CAINFO, "cacert.pem");

	// add custom headers if they exist
	if (headers.size() != 0)
	{
		curl_slist* pList = nullptr;
		for (auto& header : headers)
		{
			// concatenate the headers and add them to a list
			std::string headerStr = header.m_fieldName + ": " + header.m_data;
			pList = curl_slist_append(pList, headerStr.c_str());
		}
		// set the custom header list
		curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, pList);
	}

	// add the handler
	if (m_handle.get().m_callbacks.find(m_curl) != m_handle.get().m_callbacks.end())
		throw std::runtime_error("Only one operation can be done at a time for a given client");

	return curl_easy_perform(m_curl);
}

void WebClient::AsyncGETImpl(const std::string url,
	const std::vector<Header> headers,
	const RecvCallback_t recvCallback)
{
	// make a copy of the url that will stay in scope
	curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());

	// clear data and set
	m_data.clear();

	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &WebClient::WriteCallback);
	curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &m_data);

	curl_easy_setopt(m_curl, CURLOPT_CAINFO, "cacert.pem");

	// add custom headers if they exist
	if (headers.size() != 0)
	{
		curl_slist* pList = nullptr;
		for (auto& header : headers)
		{
			// concatenate the headers and add them to a list
			std::string headerStr = header.m_fieldName + ": " + header.m_data;
			pList = curl_slist_append(pList, headerStr.c_str());
		}
		// set the custom header list
		curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, pList);
	}

	// add the handler, save url and headers
	if (m_handle.get().m_callbacks.emplace(m_curl, [url = std::move(url), headers = std::move(headers), recvCallback = std::move(recvCallback)](const CURLcode code) { recvCallback(code); }).second == false)
		throw std::runtime_error("Only one operation can be done at a time for a given client");

	// add ourselves to multi
	curl_multi_add_handle(m_handle.get().m_multi, m_curl);
}

CURLcode WebClient::POST(const std::string& url, 
	const std::string& postData,
	const std::vector<Header>& headers)
{
	curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, postData.c_str());

	return GET(url, headers);
}

void WebClient::AsyncPOSTImpl(const std::string url,
	const std::string postData,
	const std::vector<Header> headers,
	const RecvCallback_t recvCallback)
{
	curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, postData.c_str());

	// clear data and set
	m_data.clear();

	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &WebClient::WriteCallback);
	curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &m_data);

	curl_easy_setopt(m_curl, CURLOPT_CAINFO, "cacert.pem");

	// add custom headers if they exist
	if (headers.size() != 0)
	{
		curl_slist* pList = nullptr;
		for (auto& header : headers)
		{
			// concatenate the headers and add them to a list
			std::string headerStr = header.m_fieldName + ": " + header.m_data;
			pList = curl_slist_append(pList, headerStr.c_str());
		}
		// set the custom header list
		curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, pList);
	}

	// add the handler
	if (m_handle.get().m_callbacks.emplace(m_curl, [url = std::move(url), postData = std::move(postData), recvCallback = std::move(recvCallback)](const CURLcode code) { recvCallback(code); }).second == false)
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