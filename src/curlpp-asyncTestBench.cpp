#include <Handle.h>
#include <WebClient.h>

#include <iostream>
#include <memory>

using CURLPPAsync::Handle;
using CURLPPAsync::WebClient;

int main()
{
	Handle handle;
	WebClient webClient(handle);
	WebClient webClient2(handle);
	WebClient webClient3(handle);

	webClient.SetOpt(CURLOPT_FOLLOWLOCATION, true);
	webClient.AsyncGET("https://google.com/",
		[&webClient](const CURLcode res)
		{
			if (res != CURLE_OK)
			{
				std::cerr << "Res: " << res << '\n';
				return;
			}

			// GetInfo is not guaranteed to return a valid pointer, but let's try it
			std::cout << "Got google, response code: " << webClient.GetResponseCode() << ", effective URL: " << webClient.GetInfo<const char*>(CURLINFO_EFFECTIVE_URL) << '\n';
		});

	handle.Run();

	return 0;
}