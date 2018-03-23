/*
 * wsemu.c
 *
 *  Created on: Dec 28, 2014
 *      Author: Nicola La Gloria, Kynetics LLC

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

#define _XOPEN_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <time.h>
#include "helpers.h"

// Caught a signal which indicates we should quit
static volatile int quit_requested = 0;
// Caught sigwinch. Start at 1 so we send the SIGWINCH at the beginning
static volatile int sigwinch_received = 1;

// Pools for data to read and then srite to stdout

static int poll_pts(struct pollfd *pfd, int json_len) {
    
	unsigned char buf[json_len];
    ssize_t blksz;

    if (pfd->revents & (POLLERR | POLLHUP | POLLNVAL)) {
        return 1;
    }

    if (!(pfd->revents & POLLIN)) return 0;

    // Read the data
    blksz = read(pfd->fd, buf, json_len);
    if (blksz == -1) {
        perror("Error reading from PTS device");
        return -1;
    }

    // EOF
    if (blksz == 0) return 1;

    // Write the data
    return 0;
	
	// write_to_fd(STDOUT_FILENO, buf, blksz);
}

// Polls stdin for data, and prompty writes it to the tty
static int write_data_to_pts(int pts_fd, char *json_mesg) {
    
    ssize_t blksz;

    blksz = strlen(json_mesg);
    // EOF
    if (blksz == 0) return 1;
    //usleep(millis);
    sleep(5);
    return write_to_fd(pts_fd, json_mesg, blksz);
}

// Handle a signal which should result in termination
static void handle_quit_signals(int sig) {
    quit_requested = 1;
}

// Handle a sigwinch
static void handle_sigwinch(int sig) {
    sigwinch_received = 1;
}

// Reads the current window size and
// passes it along to the PTS device
/*
static void update_winsize(int src_fd, int dst_fd) {
    struct winsize w;

    // Read
    if (ioctl(src_fd, TIOCGWINSZ, &w) == -1) {
        perror("update_winsize: Error getting window size!\n");
        return;
    }

    // Write
    if (ioctl(dst_fd, TIOCSWINSZ, &w) == -1) {
        perror("update_winsize: Error setting window size!\n");
        return;
    }
}
*/
// Installs the relevant signal handlers
// Returns -1 on failure, 0 on success
static int init_signals(void) {
    // Null terminated list of signals which result in termination
    int quit_signals[] = { SIGALRM, SIGHUP, SIGPIPE, SIGQUIT, SIGTERM, SIGINT, 0 };

    struct sigaction act;
    int i;

    memset(&act, '\0', sizeof(act));
    act.sa_flags = SA_RESTART;

    // Install the termination handlers
    act.sa_handler = &handle_quit_signals;
    for (i = 0; quit_signals[i]; i++) {
        if (sigaction(quit_signals[i], &act, NULL) < 0) {
            perror("Error installing signal handler");
            return -1;
        }
    }

    // Catch SIGWINCH
    act.sa_handler = &handle_sigwinch;
    if (sigaction(SIGWINCH, &act, NULL) < 0) {
        perror("Error installing signal handler");
        return -1;
    }

    return 0;
}


// Create a PTS device
int pts_wrap(int pts_fd) {

	struct pollfd fds[1];
	int json_len;
	int ret;
	// PTS file descriptor
    fds[0].fd = pts_fd;
    fds[0].events = POLLIN;
    fds[0].revents = 0;
	
	// Define Json message form the board
	char *json_mesg;
	
	json_mesg = "{\"Pressure\": 103}\n{\"Humidity\": 74}\n{\"Temperature\": 22}\n{\"Ambient_Light\": 40}\n";
			
	printf("Sending JSON message\n%s",json_mesg);
	
	json_len = strlen(json_mesg);
	
	
    // Install signal handlers
    if (init_signals() != 0) return -1;
	
	ret = fd_set_blocking(pts_fd, 1);
	
    while (!quit_requested) {
      

        // Half a second timeout so we get a chance to respond to
        // SIGWINCH if nothing new is printed or the user doesn't
        // press anything on the keyboard
        
        poll(fds, 1, 500);

        ret = poll_pts(&fds[0],json_len);
        if (ret == 1 || ret != 0) break;


        ret = write_data_to_pts(pts_fd, json_mesg);
		if (ret == 1 || ret != 0) break;

		/* if (sigwinch_received) {
            sigwinch_received = 0;
            update_winsize(STDOUT_FILENO, pts_fd);
        } */
    }

    return 0;
}

// Main application entry point
int main(int argc, char *argv[]) {
    char pts_name[256];
    int pts_fd, ret;

    // Open the PTS device
    pts_fd = pts_open(pts_name, 256);
    if (pts_fd < 0) {
        perror("Error opening PTS device");
        return -1;
    }

    printf("Create PTS device: %s\n", pts_name);
	printf("Device ready, connect your application\n");
	
    ret = pts_wrap(pts_fd);

    printf("\nexited\n");
    return ret;
}
