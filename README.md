# curlpp-async
cURLPP-Async was designed for cURL 7.67.0, and this is the only version with guaranteed support. It only performs `GET` and `POST` requests currently.

## CURLPPASYNC::Handle
`Handle` is a wrapper for `CURLM*`. It acts as the main actor in the proactor design for `curl_multi`.

### Functions
`Handle::Handle() noexcept`: Performs static initialization of libcurl, and creates a `multi` handle.

`void Handle::Run()`: Runs all of the queued asynchronous operations.

## CURLPPASYNC::WebClient
`WebClient` is a wrapper for `CURL*`. It acts as a standalone web client that can perform synchronous requests, or as an extension to `Handle` that will asynchronously run a web operation and call a callback on completion. Note that WebClient is not thread safe, and that only one operation is supported at a time per WebClient. All data will be written to an internal buffer and can be retrieved by calling `GetData()`.

### Structures/Types
```cpp
struct Header
{
    std::string m_fieldName;
    std::string m_data;
};
```
```cpp
using RecvCallback_t = std::function<void(const CURLcode code)>;
```

### Functions
`WebClient(Handle& handle) noexcept`: Creates a web client with reference to a handle

**Special note**: The move assignment operator will cancel any ongoing operations and discard the callback, as will the destructor.

`CURLcode GET(const std::string& url, const std::vector<Header>& customHeaders) noexcept`: Performs a synchronous GET request. Returns error code of the operation.

`void AsyncGET(std::string url, std::vector<Header> customHeaders, RecvCallback_t recvCallback) noexcept`: Starts an asynchronous GET request. Note that no asynchronous operations can be queued from the callback if `code` is `CURLE_AGAIN`, but may be started outside of the handler.

`CURLcode POST(const std::string& url, const std::string& postData, const std::vector<Header>& customHeaders) noexcept`: Performs a synchronous POST request. Returns error code of the operation.

`void AsyncPOST(std::string url, std::string postData, std::vector<Header> customHeaders, RecvCallback_t recvCallback) noexcept`: Starts an asynchronous POST request. Note that no asynchronous operations can be queued from the callback if `code` is `CURLE_AGAIN`, but may be started outside of the handler.

`void SetOpt(const CURLoption opt, const T& val) noexcept`: Sets a CURL option as if `curl_easy_setopt`. Must not be called while an operation is in progress.

`T GetInfo(const CURLINFO info) noexcept`: Gets CURLINFO as if `curl_easy_getinfo`. Must not be called while an operation is in progress, and per cURL documentation, T MUST be char*, long, curl_slist*, or double.

`const std::string& GetData() const noexcept`: Gets the internally stored data, that is cleared on each request.
