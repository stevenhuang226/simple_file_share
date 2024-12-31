#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "config.h"

void decode_req(char *buffer)
{
	if (buffer == NULL) {
		return;
	}

	int buffer_len = strlen(buffer);
	for (int i = 2; i < buffer_len; i += 1) {
		if (buffer[i-2] == '%' && buffer[i-1] == '2' && buffer[i] == '0') {
			buffer[i-2] = ' ';
			memcpy(&buffer[i-1], &buffer[i+1], buffer_len - i);
		}
	}
}
