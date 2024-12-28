#include <stdint.h>
#include <string.h>
#include <stddef.h>

#include "config.h"

#define BUFFER_OVERFLOW -1

#define NO_BUFFER -2

#define ERROR -1

int8_t insert_fn_and_cl2buffer(char *buffer, int32_t buffer_size, const char *file_name, int32_t content_long)
{
	if (buffer == NULL) {
		return NO_BUFFER;
	}
	int buffer_len = strlen(buffer);
	int fn_insert_len = strlen(file_name);
	if (buffer_size < buffer_len + 48 || content_long < 0) {
		return ERROR;
	}

	int dest = FILE_NAME_INSERT_LOCA + fn_insert_len;
	int src = FILE_NAME_INSERT_LOCA;
	int len = buffer_len - FILE_NAME_INSERT_LOCA + 1;
	memmove(&buffer[dest], &buffer[src], len);
	memcpy(&buffer[src], file_name, fn_insert_len);

	char content_str[16];
	int cnt_ptr = 15;
	do {
		content_str[cnt_ptr] = (content_long % 10) + '0';
		content_long /= 10;
		cnt_ptr -= 1;
	} while (content_long != 0);
	cnt_ptr += 1;

	buffer_len = strlen(buffer);

	dest = FILE_CONTENT_INSERT_LOCA + fn_insert_len + (16 - cnt_ptr);
	src = FILE_CONTENT_INSERT_LOCA + fn_insert_len;
	len = buffer_len - (FILE_CONTENT_INSERT_LOCA + fn_insert_len) + 1;
	memmove(&buffer[dest], &buffer[src], len);
	len = 16 - cnt_ptr;
	memcpy(&buffer[src], &content_str[cnt_ptr], len);
	return 0;
}
