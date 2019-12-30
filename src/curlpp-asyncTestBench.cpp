#include <curlpp-async/Handle.h>
#include <curlpp-async/WebClient.h>

#include <iostream>
#include <memory>

using CURLPPAsync::Handle;
using CURLPPAsync::WebClient;

int main()
{
	Handle handle;
	WebClient webClient(handle);
	WebClient webClient2(handle);

	webClient.SetOpt(CURLOPT_FOLLOWLOCATION, true);
	webClient.AsyncGET("https://google.com/",
		{}, [&webClient](const CURLcode res)
		{
			if (res != CURLE_OK)
			{
				std::cerr << "Async Res: " << res << '\n';
				return;
			}

			// GetInfo is not guaranteed to return a valid pointer, but let's try it
			std::cout << "Got google asynchronously, response code: " << webClient.GetInfo<uint32_t>(CURLINFO_RESPONSE_CODE) << ", effective URL: " << webClient.GetInfo<const char*>(CURLINFO_EFFECTIVE_URL) << '\n';
		});

	// Start Async operation
	handle.Run();

	// now do the same thing, synchronously
	webClient2.SetOpt(CURLOPT_FOLLOWLOCATION, true);
	CURLcode res = webClient2.GET("https://google.com/", {});
	if (res != CURLE_OK)
	{
		std::cerr << "Sync Res: " << res << '\n';
		return 1;
	}

	// GetInfo is not guaranteed to return a valid pointer, but let's try it
	std::cout << "Got google synchronously, response code: " << webClient2.GetInfo<uint32_t>(CURLINFO_RESPONSE_CODE) << ", effective URL: " << webClient2.GetInfo<const char*>(CURLINFO_EFFECTIVE_URL) << '\n';
	
	return 0;
}