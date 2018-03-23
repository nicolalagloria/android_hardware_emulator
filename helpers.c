/*
 * Copyright 2013, Tan Chee Eng (@tan-ce)
 * Adapyted by Nicola La Gloria, Kyentics LLC
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
#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <signal.h>
#include "helpers.h"




// Nulls a '\n' and ensures a buffer is null terminated
void terminate_buf(char *buf, size_t buf_len) {
    int i;
    for (i = 0; i < buf_len; i++) {
        if (!buf[i] || buf[i] == '\n') {
            buf[i] = '\0';
            return;
        }
    }

    // No new line or NUL? Add one.
    buf[buf_len - 1] = '\0';
}

// Simply write to a given file descriptor
int write_to_fd(int fd, char *buf, ssize_t bufsz) {
    ssize_t ret, written;
    
    written = 0;
    do {
        ret = write(fd, buf + written, bufsz - written);
        if (ret == -1) {
            perror("Error writing to FD");
            return -1;
        }
        written += ret;
    } while (written < bufsz);

    return 0;
}

// Read the contents of a file
ssize_t load_file(char *file, char *buf, size_t buf_len) {
    FILE *fp;
    size_t ret;

    fp = fopen(file, "r");
    if (!fp) {
        perror("load_file()");
        return -1;
    }

    ret = fread(buf, 1, buf_len, fp);
    fclose(fp);

    return ret;
}

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
FILE *get_file(const char *path, char *buf, size_t *buf_size) {
    int fd;
    FILE *ret;

    // Open sesame
    fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd < 0) return NULL;

    // Get a stream
    ret = fdopen(fd, "r+");
    if (!ret) return NULL;

    // Do we want the contents of the file?
    if (buf && buf_size) {
        *buf_size = fread(buf, 1, *buf_size, ret);
    }

    return ret;
}

// Checks whether the app working path exists.
// If not, attempt to create it.
int check_path(const char *path) {
    DIR *dir;
    mode_t old_umask;
    int ret;

    dir = opendir(path);

    if (dir) {
        // Directory exists
        closedir(dir);
        // Ensure it has the right permissions
        if (chmod(path, 0701) < 0) {
            perror("Warning: Unable to set working directory mode");
        }
        return 0;
    } else if (errno == ENOENT) {

        // We need that execute bit on others, otherwise
        // the socket won't be accesible to others
        old_umask = umask(0);
        // Try to create the directory
        ret = mkdir(path, 0701);
        // Restore the old umask
        umask(old_umask);

        if (ret < 0) {
            perror("Unable to create working directory");
            return -1;
        } else {
            return 0;
        }
    } else {
        perror("Cannot access working directory");
        return -1;
    }
}



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
int pts_open(char *slave_name, size_t slave_name_size) {
    int fdm;
    char *sn_tmp;
        
    // Open master ptmx device
    fdm = open("/dev/ptmx", O_RDWR);
    if (fdm == -1) return -1;

    // Get the slave name
    sn_tmp = ptsname(fdm);
    if (!sn_tmp) {
        close(fdm);
        return -2;
    }

    strncpy(slave_name, sn_tmp, slave_name_size);
    slave_name[slave_name_size - 1] = '\0';

    // Grant, then unlock
    if (grantpt(fdm) == -1) {
        close(fdm);
        return -1;
    }
    if (unlockpt(fdm) == -1) {
        close(fdm);
        return -1;
    }

    return fdm;
}

int fd_set_blocking(int fd, int blocking){
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		return 0;

	if (blocking)
		flags &= ~O_NONBLOCK;
	else
		flags |= O_NONBLOCK;

	return fcntl(fd, F_SETFL, flags) != -1;
}
