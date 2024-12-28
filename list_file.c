#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>


#include "config.h"

int8_t generate_file_array(char ***file_array, int32_t *nums_file_ret, int32_t *all_str_len)
{
	*nums_file_ret = -1; // pre write for error exit

	int dir_fd = open(SHARD_FILE_PATH, O_RDONLY | O_DIRECTORY);
	if (dir_fd < 0) {
		perror("open");
		return -1;
	}
	DIR *shared_dir = fdopendir(dir_fd);
	if (shared_dir == NULL) {
		perror("fdopendir");
		return -1;
	}
	int fna_size = FILENAME_ARRAY_SIZE;
	*file_array = malloc(fna_size * sizeof(char *));
	if (*file_array == NULL) {
		perror("malloc");
		return -1;
	}
	int32_t str_len = 0;
	int fna_ptr = 0; // used in loop
	struct dirent *entry;
	while ((entry = readdir(shared_dir)) != NULL) {
		// if file name start with '.' ignore it
		if (entry->d_name[0] == '.') {
			continue;
		}
		// if reach to the file name array size, add more memory to it
		if (fna_ptr >= fna_size) {
			fna_size += 10;
			char **buff = realloc(*file_array, fna_size * sizeof(char *));
			if (buff == NULL) {
				goto free_fna;
			}
			*file_array = buff;
		}
		// assignment memory to (*file_array)[fna_ptr], preparation to copy file name to array
		(*file_array)[fna_ptr] = malloc((strlen(entry->d_name) + 1) * sizeof(char));
		if ((*file_array)[fna_ptr] == NULL) {
			goto free_fna;
		}
		strcpy((*file_array)[fna_ptr], entry->d_name);
		fna_ptr += 1;
		str_len += strlen(entry->d_name);
	}
	closedir(shared_dir);
	if (all_str_len != NULL) {
		*all_str_len = str_len;
	}
	*nums_file_ret = fna_ptr;
	return 0;
free_fna: // if get any error, free all of memory was assignment to file name array
	closedir(shared_dir);
	perror("fna memory");
	*nums_file_ret = -1;
	for (int i = 0; i < fna_ptr; i += 1) {
		free((*file_array)[i]);
	}
	free(*file_array);
	return -1;
}
