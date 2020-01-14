#include <curlpp-async/WebClient.h>

#include <curlpp-async/Handle.h>

#include <memory>

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
	// clean up
	m_handle.get().UnregisterHandle(m_curl);
	curl_easy_cleanup(m_curl);

	m_curl = other.m_curl;
	m_handle = other.m_handle;

	other.m_curl = curl_easy_init();
	
	return *this;
}

WebClient::~WebClient() noexcept
{
	// clean up
	m_handle.get().UnregisterHandle(m_curl);
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

	return curl_easy_perform(m_curl);
}

void WebClient::AsyncGET(std::string url,
	std::vector<Header> headers,
	RecvCallback_t recvCallback)
{
	// make a copy of the url and headers that will stay in scope
	std::shared_ptr<std::string> pUrl = std::make_shared<std::string>(std::move(url));
	std::shared_ptr<std::vector<Header>> pHeaders = std::make_shared<std::vector<Header>>(std::move(headers));

	curl_easy_setopt(m_curl, CURLOPT_URL, pUrl->c_str());

	// clear data and set
	m_data.clear();

	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &WebClient::WriteCallback);
	curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &m_data);

	// add custom headers if they exist
	if (pHeaders->size() != 0)
	{
		curl_slist* pList = nullptr;
		for (auto& header : *pHeaders)
		{
			// concatenate the headers and add them to a list
			std::string headerStr = header.m_fieldName + ": " + header.m_data;
			pList = curl_slist_append(pList, headerStr.c_str());
		}
		// set the custom header list
		curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, pList);
	}

	// add the handler, save url and headers
	if (m_handle.get().RegisterHandle(m_curl, 
		[pUrl = std::move(pUrl), pHeaders = std::move(pHeaders), recvCallback = std::move(recvCallback)]
	(const CURLcode code) { recvCallback(code); }) == false)
		throw std::runtime_error("Only one operation can be done at a time for a given client");
}

CURLcode WebClient::POST(const std::string& url, 
	const std::string& postData,
	const std::vector<Header>& headers)
{
	curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, postData.c_str());

	return GET(url, headers);
}

void WebClient::AsyncPOST(std::string url,
	std::string postData,
	std::vector<Header> headers,
	RecvCallback_t recvCallback)
{
	// capture url, postData, and headers so they stay in scope
	std::shared_ptr<std::string> pUrl = std::make_shared<std::string>(std::move(url));
	std::shared_ptr<std::string> pPostData = std::make_shared<std::string>(std::move(postData));
	std::shared_ptr<std::vector<Header>> pHeaders = std::make_shared<std::vector<Header>>(std::move(headers));

	curl_easy_setopt(m_curl, CURLOPT_URL, pUrl->c_str());
	curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, pPostData->c_str());

	// clear data and set
	m_data.clear();

	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &WebClient::WriteCallback);
	curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &m_data);

	// add custom headers if they exist
	if (pHeaders->size() != 0)
	{
		curl_slist* pList = nullptr;
		for (auto& header : *pHeaders)
		{
			// concatenate the headers and add them to a list
			std::string headerStr = header.m_fieldName + ": " + header.m_data;
			pList = curl_slist_append(pList, headerStr.c_str());
		}
		// set the custom header list
		curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, pList);
	}

	// add the handler
	if (m_handle.get().RegisterHandle(m_curl, 
		[pUrl = std::move(pUrl), pPostData = std::move(pPostData), pHeaders = std::move(pHeaders), recvCallback = std::move(recvCallback)]
	(const CURLcode code) { recvCallback(code); }) == false)
		throw std::runtime_error("Only one operation can be done at a time for a given client");
}

size_t WebClient::WriteCallback(char* ptr, size_t size, size_t nmemb, void* userData)
{
	if (nmemb)
		reinterpret_cast<std::string*>(userData)->append(ptr, nmemb);
	return nmemb;
}