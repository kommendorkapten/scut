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
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

/* Unit tests */
int test1(void);
int test_float(void);
int test_sig_die(void);
int test_sig_catch(void);

/* Test helpers */
int incr(int);

int main(int argc, char** argv)
{
        int verbose = 0;

        for (int i = 0; i < argc; ++i)
        {
                if (strncmp("-v", argv[i], 2) == 0)
                {
                        verbose = 1;
                }
        }

        scut_create("Example suite (will fail)");

        SCUT_ADD(test1);
        SCUT_ADD(test_float);
        SCUT_ADD(test_sig_die);
        SCUT_ADD(test_sig_catch);
        
        if (verbose)
        {
                scut_run(SCUT_VERBOSE);
        }
        else
        {
                scut_run(0);
        }

        scut_destroy();
}

int test1(void)
{
        SCUT_ASSERT_IE(2, incr(1));
        SCUT_ASSERT_TRUE(4 == incr(3));
        SCUT_ASSERT_FALSE(1 == incr(1));

        if (3 == incr(1))
        {
                SCUT_FAIL("This is not right");
        }

        return 0;
}

int test_float(void)
{
        float a = 0.01f;
        float b = 0.02f;

        SCUT_ASSERT_TRUE((b - a) < 1.0f);

        return 0;
}

int test_sig_die(void)
{
        printf("This will kill me...\n");

        kill(getpid(), SIGILL);
        
        return 0;
}

int test_sig_catch(void)
{
        printf("I shall survive this!\n");
        SCUT_EXPECT_SIG(SIGILL);

        kill(getpid(), SIGILL);

        SCUT_ASSERT_SIG(SIGILL);

        return 0;
}

int incr(int i)
{
        return i + 1;
}
