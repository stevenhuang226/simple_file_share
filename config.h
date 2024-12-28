#define PORT 8888
#define FILE_HEADER "file_headers" // this file will pre-write http header and script will insert file name and content-long into it.
#define SHARD_FILE_PATH "./"

#define FILE_NAME_INSERT_LOCA 0x62 // location to insert file name into FILE_HEADER
#define FILE_CONTENT_INSERT_LOCA 0x74 // location to isnert content-long into FILE_HEADER // must biger than FILE_NAME_INSERT_LOCA

#define FILENAME_ARRAY_SIZE 20 // default file array size, it would be dynamic adjust
#define REQ_BUFFER_SIZE 4096 // default request buffer size, 4096 should be enought
#define FILE_HEADER_BUFFER_SIZE 1024 // default header buffer size, depend on the FILE_HEADER size, you can manual adjust this value
#define PATH_BUFFER_SIZE 1024 // default path buffer size, if your path is very long, you can increase this value


int8_t generate_file_array(char ***file_array, int32_t *nums_file_ret, int32_t *all_str_len);
int8_t insert_fn_and_cl2buffer(char *buffer, int32_t buffer_size, const char *file_name, int32_t content_long);
