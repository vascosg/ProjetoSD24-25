
#include <errno.h>
#include <unistd.h>
#include "../include/message-private.h"
#include "../include/htmessages.pb-c.h"


MessageT;

size_t write_all(int sock, const void *buf, size_t len)
{
	const char *p = buf;
	size_t buffersize = len;
	while (len > 0)
	{
		size_t res = write(sock, buf, len);
		if (res < 0)
		{
			if (errno == EINTR)
				continue;
			perror("write failed: ");
			return res;
		}
		buf += res;
		len -= res;
	}

	return buffersize;
}

size_t read_all(int sock, void *buf, size_t len)
{

	size_t buffersize = len;
	while (len > 0)
	{
		size_t result = read(sock, buf, len);

		if (result <= 0)
		{
			if (errno == EINTR)
				continue;
			perror("read failed: ");
			return result;
		}

		buf += result;
		len -= result;
	}

	return buffersize;
}
