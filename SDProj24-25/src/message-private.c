
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include "../include/message-private.h"
#include "../include/htmessages.pb-c.h"

size_t write_all(int sock, const void *buf, size_t len)
{
	size_t buffersize = len;
	while (len > 0)
	{
		size_t res = write(sock, buf, len);
		if (res < 0)
		{
			if (errno == EINTR)
				continue;
			//printf("Erro ao escrever no wtire_all\n ");
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
			//printf("Erro ao ler no read_all\n ");
			return result;
		}

		buf += result;
		len -= result;
	}

	return buffersize;
}
