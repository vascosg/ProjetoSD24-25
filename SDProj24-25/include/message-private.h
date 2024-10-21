#ifndef _MESSAGE_PRIVATE_H
#define _MESSAGE_PRIVATE_H

size_t write_all(int sock, const void *buf, size_t len);

size_t read_all(int sock, void *buf, size_t len);

#endif
