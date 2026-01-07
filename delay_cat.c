// gcc -O2 -Wall delay_cat.c -o delay_cat
// vi: se ft=4 sw=4

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <time.h>

typedef struct node {
    struct timespec emit;
    char *line;
    struct node *next;
} node_t;

static node_t *head = NULL;

/* timespec comparison */
static int ts_cmp(struct timespec a, struct timespec b) {
    if (a.tv_sec != b.tv_sec)
        return a.tv_sec - b.tv_sec;
    return a.tv_nsec - b.tv_nsec;
}

/* timespec add seconds (double) */
static struct timespec ts_add(struct timespec t, double sec) {
    struct timespec r = t;
    r.tv_sec  += (time_t)sec;
    r.tv_nsec += (long)((sec - (time_t)sec) * 1e9);
    if (r.tv_nsec >= 1000000000L) {
        r.tv_sec++;
        r.tv_nsec -= 1000000000L;
    }
    return r;
}

/* insert sorted */
static void queue_push(struct timespec emit, char *line) {
    node_t **p = &head;
    while (*p && ts_cmp((*p)->emit, emit) <= 0)
        p = &(*p)->next;

    node_t *n = malloc(sizeof(*n));
    n->emit = emit;
    n->line = line;
    n->next = *p;
    *p = n;
}

/* pop and print ready lines */
static int flush_ready(struct timespec now) {
    int cnt = 0;
    while (head && ts_cmp(head->emit, now) <= 0) {
        fputs(head->line, stdout);
        fflush(stdout);
        free(head->line);
        node_t *old = head;
        head = head->next;
        free(old);
        cnt++;
    }
    return cnt;
}

/* arm timerfd for earliest emit */
static void arm_timer(int tfd) {
    if (!head)
        return;

    struct itimerspec it = {0};
    it.it_value = head->emit;
    timerfd_settime(tfd, TFD_TIMER_ABSTIME, &it, NULL);
}

int main(int argc, char **argv) {
    double delay = 0.5;

    /* parse -d / --delay */
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--delay")) {
            if (++i >= argc) {
                fprintf(stderr, "missing value for %s\n", argv[i-1]);
                return 1;
            }
            delay = atof(argv[i]);
        }
    }

    int ep = epoll_create1(0);
    int tfd = timerfd_create(CLOCK_MONOTONIC, 0);

    struct epoll_event ev = {0};

    ev.events = EPOLLIN;
    ev.data.fd = STDIN_FILENO;
    epoll_ctl(ep, EPOLL_CTL_ADD, STDIN_FILENO, &ev);

    ev.events = EPOLLIN;
    ev.data.fd = tfd;
    epoll_ctl(ep, EPOLL_CTL_ADD, tfd, &ev);

    char *line = NULL;
    size_t cap = 0;

    while (1) {
        struct epoll_event events[2];
        int n = epoll_wait(ep, events, 2, -1);

        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);

        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == STDIN_FILENO) {
                ssize_t r = getline(&line, &cap, stdin);
                if (r == -1)
                    goto eof;

                struct timespec emit = ts_add(now, delay);
                queue_push(emit, strdup(line));
                arm_timer(tfd);
            } else if (events[i].data.fd == tfd) {
                uint64_t x;
                // read(tfd, &x, sizeof(x)); /* clear */
                ssize_t r = read(tfd, &x, sizeof(x));
                if (r != sizeof(x)) {
                  perror("timerfd read");
                }
            }
        }

        flush_ready(now);
        arm_timer(tfd);
    }

eof:
    /* flush remaining */
    while (head) {
        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        flush_ready(now);
        arm_timer(tfd);
    }

    return 0;
}


