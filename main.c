#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>

#include "config.h"

#define FILE_NOT_EXISTS -2;
#define MALLOC_ERR -3;

char **file_array;
int32_t nums_files;
char *static_header_buffer;

int server_fd;

void cleanup(int sig);
int8_t file2buffer(char *buffer, int32_t buffer_size, const char *path);
int32_t req_find_file(const char *req);
int8_t res_file(const int *client_fd, const int *file_code);

int main()
{
	signal(SIGINT, cleanup);

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		goto exit_fail;
	}

	struct sockaddr_in address = {0};
	address.sin_family = AF_INET;
	address.sin_port = htons(PORT);
	address.sin_addr.s_addr = INADDR_ANY;

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0 ||
			listen(server_fd, 1) < 0) {
		goto close_srv_fd;
	}

	generate_file_array(&file_array, &nums_files, NULL);
	static_header_buffer = malloc(FILE_HEADER_BUFFER_SIZE * sizeof(char));
	if (static_header_buffer == NULL) {
		goto clean_fna;
	}
	if (file2buffer(static_header_buffer, FILE_HEADER_BUFFER_SIZE, FILE_HEADER) < 0) {
		goto clean_hdr;
	}

	for (;;) {
		int client_fd = accept(server_fd, NULL, NULL);
		if (client_fd < 0) {
			continue;
		}

		char *req_buffer;
		req_buffer = malloc(REQ_BUFFER_SIZE * sizeof(char));
		ssize_t req_bytes = recv(client_fd, req_buffer, REQ_BUFFER_SIZE -1, 0);
		if (req_bytes < 0) {
			free(req_buffer);
			close(client_fd);
			continue;
		}
		req_buffer[req_bytes] = 0;
		int32_t file_code = req_find_file(req_buffer);
		if (file_code > 0) {
			res_file(&client_fd, &file_code);
		}
		free(req_buffer);
		close(client_fd);
	}

clean_hdr:
	free(static_header_buffer);
clean_fna:
	for (int i = 0; i < nums_files; i += 1) {
		free(file_array[i]);
	}
	free(file_array);
close_srv_fd:
	close(server_fd);
exit_fail:
	return EXIT_FAILURE;

}

void cleanup(int sig)
{
	close(server_fd);
	for (int i = 0; i < nums_files; i += 1) {
		free(file_array[i]);
	}
	free(file_array);
	free(static_header_buffer);
	printf("exit...\n");
	exit(EXIT_SUCCESS);
}
int8_t file2buffer(char *buffer, int32_t buffer_size, const char *path)
{
	if (access(path, F_OK) < 0) {
		return FILE_NOT_EXISTS;
	}
	int file_fd = open(path, O_RDONLY);
	if (file_fd < 0) {
		return -1;
	}
	ssize_t read_bytes = read(file_fd, buffer, buffer_size - 1);
	close(file_fd);
	if (read_bytes <= 0) {
		return -1;
	}
	buffer[read_bytes] = '\0';
	return 0;
}
int32_t req_find_file(const char *req)
{
	if (req[5] == ' ') {
		return FILE_NOT_EXISTS;
	}
	int req_size = strlen(req);
	char *path;
	path = malloc(PATH_BUFFER_SIZE * sizeof(char));
	if (path == NULL) {
		return MALLOC_ERR;
	}

	int8_t get_space = 0;
	for ( int i = 5; i < req_size; i += 1) {
		if (req[i] == ' ') {
			memcpy(path, &req[5], i - 5);
			path[i-5] = '\0';
			get_space = 1;
			break;
		}
		else if (req[i-1] == '.' && req[i] == '.') {
			goto cleanup;
		}
	}
	if (get_space == 0) {
		goto cleanup;
	}
	for (int i = 0; i < nums_files; i += 1) {
		if (strcmp(path, file_array[i]) == 0) {
			free(path);
			return (int32_t)i;
		}
	}
cleanup:
	free(path);
	return FILE_NOT_EXISTS;
}
int8_t res_file(const int *client_fd, const int *file_code)
{
	char shd_path[] = SHARD_FILE_PATH;
	char path[strlen(shd_path) + strlen(file_array[*file_code]) + 1];
	strcpy(path, shd_path);
	strcpy(&path[strlen(shd_path)], file_array[*file_code]);

	struct stat file_stat;
	if (stat(path, &file_stat) != 0) {
		return FILE_NOT_EXISTS;
	}
	off_t file_size = file_stat.st_size;

	int file_fd = open(path, O_RDONLY);
	if (file_fd < 0) {
		return -1;
	}
	char *file_buffer;
	file_buffer = malloc(file_size * sizeof(char));
	if (file_buffer == NULL) {
		return MALLOC_ERR;
	}
	if (read(file_fd, file_buffer, file_size) <= 0) {
		free(file_buffer);
		return -1;
	}
	close(file_fd);

	char *header_buffer;
	header_buffer = malloc(FILE_HEADER_BUFFER_SIZE * sizeof(char));
	if (header_buffer == NULL) {
		free(file_buffer);
		return MALLOC_ERR;
	}
	strcpy(header_buffer, static_header_buffer);
	if (insert_fn_and_cl2buffer(header_buffer, FILE_HEADER_BUFFER_SIZE, file_array[*file_code], (int32_t)file_size) < 0) {
		free(file_buffer);
		free(header_buffer);
		return -1;
	}
	if (send(*client_fd, header_buffer, strlen(header_buffer), 0) < 0 ||
			send(*client_fd, file_buffer, file_size, 0) < 0) {
		free(file_buffer);
		free(header_buffer);
		return -4;
	}
	free(file_buffer);
	free(header_buffer);
	return 0;
}
