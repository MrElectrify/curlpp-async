#include <Handle.h>

#include <iostream>

using CURLPPAsync::Handle;

int main()
{
	Handle handle;

	/*curl_global_init(CURL_GLOBAL_WIN32);
	
	CURL* const curl = curl_easy_init();
	CURL* const curl2 = curl_easy_init();
	CURLM* const multi = curl_multi_init();

	std::string resStr;
	std::string resStr2;

	curl_easy_setopt(curl, CURLOPT_URL, "https://www.google.com/");
	curl_easy_setopt(curl, CURLOPT_CAINFO, "C:\\Windows\\cacert.pem");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resStr);
	curl_easy_setopt(curl2, CURLOPT_URL, "https://goodaids.club/");
	curl_easy_setopt(curl2, CURLOPT_CAINFO, "C:\\Windows\\cacert.pem");
	curl_easy_setopt(curl2, CURLOPT_WRITEFUNCTION, &write_data);
	curl_easy_setopt(curl2, CURLOPT_WRITEDATA, &resStr2);

	curl_multi_add_handle(multi, curl);
	curl_multi_add_handle(multi, curl2);

	int still_running = 0;
	curl_multi_perform(multi, &still_running);
	int rc;
	while (still_running != 0)
	{
		timeval timeout;
		fd_set fdread;
		fd_set fdwrite;
		fd_set fdexcep;
		int maxfd = -1;

		long curl_timeo;

		curl_multi_timeout(multi, &curl_timeo);
		if (curl_timeo < 0)
			curl_timeo = 1000;

		timeout.tv_sec = curl_timeo / 1000;
		timeout.tv_usec = (curl_timeo % 1000) * 1000;

		FD_ZERO(&fdread);
		FD_ZERO(&fdwrite);
		FD_ZERO(&fdexcep);

		// get file descriptors from the transfers 
		CURLMcode mc = curl_multi_fdset(multi, &fdread, &fdwrite, &fdexcep, &maxfd);

		if (maxfd == -1) {
			Sleep(100);
			rc = 0;
		}
		else
			rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);

		switch (rc) {
		case -1:
			// select error
			break;
		case 0:
		default:
			// timeout or readable/writable sockets
			curl_multi_perform(multi, &still_running);
			break;
		}
	}

	std::cout << "Data: " << resStr << '\n';
	std::cout << "Data2: " << resStr2 << '\n';

	curl_multi_cleanup(multi);
	curl_easy_cleanup(curl2);
	curl_easy_cleanup(curl);

	curl_global_cleanup();*/
}