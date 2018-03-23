/*
 * Copyright 2013, Tan Chee Eng (@tan-ce)
 * 
 * Copyright 2014, Kyentics LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * Collection of helper functions
 */

#ifndef _HELPERS_H_
#define _HELPERS_H_

#ifndef SA_RESTART
#define SA_RESTART   0x10000000
#endif

#include <stddef.h>
#include <termios.h>

#define PATH_PREFIX    "/data/pts"

// Nulls a '\n' and ensures a buffer is null terminated
void terminate_buf(char *buf, size_t buf_len);

// Simply write to a given file descriptor
int write_to_fd(int fd, char *buf, ssize_t bufsz);

// Read the contents of a file
ssize_t load_file(char *file, char *buf, size_t buf_len);

// Get & open a file. If the file exists and contains data,
// its contents are placed into buf, otherwise it is created. 
// In all cases a file pointer is returned. Unless something
// died, in which case you get NULL, and errno is set.
// 
// On buf_size contains the size of buf, and in turn, is set to 
// the number of bytes read into buf
//
// buf and buf_size may be NULL, in which case nothing is read
//
// The file is never truncated
FILE *get_file(const char *path, char *buf, size_t *buf_size);

// Checks whether the path exists.
// If not, attempt to create it.
int check_path(const char *path);

// Daemonizes the process

/**
 * pts_open
 *
 * open a pts device and returns the name of the slave tty device.
 *
 * arguments
 * slave_name       the name of the slave device
 * slave_name_size  the size of the buffer passed via slave_name
 *
 * return values
 * on failure either -2 or -1 (errno set) is returned.
 * on success, the file descriptor of the master device is returned.
 */
int pts_open(char *slave_name, size_t slave_name_size);

int fd_set_blocking(int fd, int blocking);

#endif 
