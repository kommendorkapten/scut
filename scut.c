/* 
 * Copyright (C) 2016 Fredrik Skogman, skogman - at - gmail.com.
 * This file is part of Scut.
 *
 * The contents of this file are subject to the terms of the Common
 * Development and Distribution License (the "License"). You may not use this file
 * except in compliance with the License. You can obtain a copy of the License at
 * http://opensource.org/licenses/CDDL-1.0. See the License for the specific
 * language governing permissions and limitations under the License. When
 * distributing the software, include this License Header Notice in each file and
 * include the License file at http://opensource.org/licenses/CDDL-1.0.
 */

#include "scut.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <setjmp.h>

#define MAX_MSG 256
#define MAX_SIG 1024
#define BOLD "\x1b[1m"
#define BOLDOFF "\x1b[21m"

struct scut_test
{
        int (*test)(void);
        const char* name;
};

struct scut_suite
{
        const char* name;
        struct scut_test* tests;
        int cap;
        int count;
        int* sig_catched;
        int* sig_expected;
        int catch_sig_pos;
        int exp_sig_pos;
        jmp_buf env;
        sigset_t sigmask;
};

static int out;
static struct scut_suite* suite;

static void prepare_test(void);
static char* drain(int fd);
static void say(const char*);
static int sig_setup(void);
static void sig_trap(int);

void scut_create(const char* name)
{
        if (suite)
        {
                scut_destroy();
        }

        suite = malloc(sizeof(struct scut_suite));

        if (suite)
        {
                int cap = 64;
                suite->cap = 64;
                suite->count = 0;
                suite->name = name;
                suite->tests = malloc(sizeof(struct scut_test) * cap);
                suite->sig_catched = malloc(sizeof(int) * MAX_SIG);
                suite->sig_expected = malloc(sizeof(int) * MAX_SIG);
                suite->catch_sig_pos = 0;
                suite->exp_sig_pos = 0;
                if (!suite->tests)
                {
                        free(suite);
                        suite = NULL;
                }
                pthread_sigmask(SIG_SETMASK, NULL, &suite->sigmask);
        }
}

int scut_add(int (*test)(void), const char* name)
{
        if (suite->cap == suite->count)
        {
                printf("Can not add more tests\n");
                return 1;
        }
        
        if (test == NULL)
        {
                return 1;
        }

        if (name == NULL)
        {
                return 1;
        }

        suite->tests[suite->count].test = test;
        suite->tests[suite->count].name = name;
        suite->count++;

        return 0;
}

int scut_run(int flags)
{
        struct scut_test* test;
        char buf[MAX_MSG];
        int count = 0;
        int ret;
        int failed = 0;
        int fds[2];

        /* Disable buffering */
        setbuf(stdout, NULL);

        /* Capture stdout */
        out = 1;
        if (pipe(fds) == 0) 
        {
                out = dup(1);
                if (out > -1 && dup2(fds[1], 1) > -1)
                {
                        if (fcntl(fds[0], F_SETFL, O_NONBLOCK) == -1)
                        {
                                close(out);
                                close(fds[0]);
                                close(fds[1]);
                                out = 1;
                        }
                }
                else
                {
                        close(out);
                        close(fds[0]);
                        close(fds[1]);
                        out = 1;
                }
        }
        if (out == 1)
        {
                printf("Failed to capture stdout\n");
        }

        snprintf(buf, MAX_MSG, "> Running suite %s%s%s\n", BOLD, 
                 suite->name, 
                 BOLDOFF);
        say(buf);

        while (count < suite->count)
        {
                char* captured;
                int jmp;
                
                test = suite->tests + count++;
                snprintf(buf, MAX_MSG, "Running %16s: ", test->name);
                say(buf);

                prepare_test();
                jmp = setjmp(suite->env);
                if (jmp == 0)
                {
                        ret = test->test();
                }
                else
                {
                        // Must restore signal mask
                        sigprocmask(SIG_SETMASK, &suite->sigmask, NULL);

                        ret = 1;
                }

                captured = drain(fds[0]);

                if (ret)
                {
                        failed++;
                        snprintf(buf, MAX_MSG, BOLD "FAILED" BOLDOFF "\n");
                        say(buf);

                        if (jmp)
                        {
                                snprintf(buf, MAX_MSG, "> Killed by signal %d\n", jmp);
                                say(buf);
                        }
                }
                else
                {
                        snprintf(buf, MAX_MSG, BOLD "Ok" BOLDOFF "\n");
                        say(buf);
                }

                if ((ret || (flags & SCUT_VERBOSE)) && strlen(captured))
                {
                        snprintf(buf, MAX_MSG, ">>> Captured output <<<\n\n");
                        say(buf);
                        say(captured);
                        snprintf(buf, MAX_MSG, "\n>>> End of output <<<\n");
                        say(buf);
                }

                free(captured);
        }

        snprintf(buf, MAX_MSG, "\nResult: %d performed\n", count);
        say(buf);
        if (failed)
        {
                snprintf(buf, MAX_MSG, "Result: %d failed tests\n", failed);
                say(buf);

                snprintf(buf, MAX_MSG, "Suite %s%s FAILED%s\n", 
                         BOLD,
                         suite->name,
                         BOLDOFF);
                say(buf);
        }
        else 
        {
                snprintf(buf, MAX_MSG, "Suite %s%s SUCCESS%s\n",
                         BOLD,
                         suite->name,
                         BOLDOFF);
                say(buf);
        }

        // Restore stdout
        if (out != 1) 
        {
                close(fds[0]);
                close(fds[1]);
                dup2(out, 1);
                close(out);
        }

        return failed;
}

void scut_expect_sig(int signum)
{
        if (suite->exp_sig_pos >= MAX_SIG)
        {
                char buf[128];
                snprintf(buf, 
                         128, 
                         "Only %d signals can be catched\n",
                         MAX_SIG);
                exit(1);      
        }
        suite->sig_expected[suite->exp_sig_pos++] = signum;
}

int scut_assert_sig(int signum)
{
        for (int i = 0; i < suite->catch_sig_pos; ++i)
        {
                if (signum == suite->sig_catched[i])
                {
                        return 1;
                }
        }

        return 0;
}

int scut_num_tests(void)
{
        return suite->count;
}

void scut_destroy(void)
{
        free(suite->tests);
        free(suite);
        suite = NULL;
}

static void prepare_test(void)
{
        suite->catch_sig_pos = 0;
        suite->exp_sig_pos = 0;

        for( int i = 0; i < MAX_SIG; ++i)
        {
                suite->sig_catched[i] = -1;
                suite->sig_expected[i] = -1;
        }

        sig_setup();
}

static void say(const char* m)
{
        write(out, m, strlen(m));
}

static char* drain(int fd)
{
        ssize_t len = 1024 * 1024; // Start with 1MiB
        ssize_t br;
        char* msg = malloc(len);

        br = read(fd, msg, len);
        
        if (br == len)
        {
                br = len - 1;
        }
        if (br < 0)
        {
                br = 0;
        }

        msg[br] = 0;
        return msg;
}

static int sig_setup(void)
{
        int signals[] = {
                SIGABRT,
                SIGALRM,
                SIGBUS,
                SIGCHLD,
                SIGCONT,
                SIGFPE,
                SIGHUP,
                SIGILL,
                /* SIGINT, */
                /* SIGKILL, */
                SIGPIPE,
                SIGQUIT,
                SIGSEGV,
                /* SIGSTOP, */ 
                /* SIGTERM, */
                SIGTSTP,
                SIGTTIN,
                SIGTTOU,
                SIGUSR1,
                SIGUSR2,
#if (!defined __FreeBSD__)
                SIGPOLL,
#endif
                SIGPROF,
                SIGSYS,
                SIGTRAP,
                SIGURG,
                SIGVTALRM,
                SIGXCPU,
                SIGXFSZ
        };
        int s = sizeof(signals) / sizeof(int);
        struct sigaction sa;

        sa.sa_handler = &sig_trap;
        sigfillset(&sa.sa_mask);
        sa.sa_flags = 0;

        for (int i = 0; i < s; ++i)
        {
                if(sigaction(signals[i], &sa, NULL))
                {
                        perror("Failed to trap signal");
                        exit(1);
                }
        }

        return 0;
}

static void sig_trap(int signum)
{
        int exp = 0;

        for (int i = 0; i < suite->exp_sig_pos; ++i)
        {
                if (signum == suite->sig_expected[i])
                {
                        exp = 1;
                        break;
                }
        }

        if (exp)
        {
                if (suite->catch_sig_pos >= MAX_SIG)
                {
                        char buf[128];
                        snprintf(buf, 
                                 128, 
                                 "Maximum (%d) number of signals received, giving up\n",
                                 MAX_SIG);
                        exit(1);
                }
                suite->sig_catched[suite->catch_sig_pos++] = signum;
        }
        else 
        {
                longjmp(suite->env, signum);
        }
}
