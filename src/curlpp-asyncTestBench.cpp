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

	webClient.AsyncPOST("https://postman-echo.com/post", "TEST DATA", {},
		[&webClient](const CURLcode code)
		{
			if (code != CURLE_OK)
			{
				std::cerr << "Error: " << code << '\n';
				return;
			}

			std::cout << webClient.GetData() << '\n';
		});

	handle.Run();

	return 0;
}