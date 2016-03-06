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
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>

/* Test helper functions */
int test_1(void);
int test_2(void);
int test_3(void);
int test_sig(void);
int test_sig_catch(void);
int test_fail(void);
int test_ie_ok(void);
int test_ie_fail(void);
int test_true_ok(void);
int test_true_fail(void);
int test_false_ok(void);
int test_false_fail(void);

/* Various suites */
int test_success(void);
int test_failed(void);
int test_m_add(void);
int test_m_fail(void);
int test_m_assert_ie(void);
int test_m_assert_true(void);
int test_m_assert_false(void);
int test_sig_fault_no_catch(void);
int test_sig_fault_catch(void);

int stdoutdup;

int main(void)
{
        int ret = 0;
        stdoutdup = 1;

        if (test_success())
        {
                char* msg = "test_success failed\n";
                write(stdoutdup, msg, strlen(msg));
                ret = 1;
        }
        
        write(1, "\n", 1);
        if (test_failed())
        {
                char* msg = "test_failed failed\n";
                write(stdoutdup, msg, strlen(msg));
                ret = 1;
        }

        write(1, "\n", 1);
        if (test_m_add())
        {
                char* msg = "test_m_add failed\n";
                write(stdoutdup, msg, strlen(msg));
                ret = 1;
        }

        write(1, "\n", 1);
        if (test_m_fail())
        {
                char* msg = "test_m_fail failed\n";
                write(stdoutdup, msg, strlen(msg));
                ret = 1;
        }

        write(1, "\n", 1);
        if (test_m_assert_ie())
        {
                char* msg = "test_m_ie failed\n";
                write(stdoutdup, msg, strlen(msg));
                ret = 1;
        }

        write(1, "\n", 1);
        if (test_m_assert_true())
        {
                char* msg = "test_m_assert_true failed\n";
                write(stdoutdup, msg, strlen(msg));
                ret = 1;
        }

        write(1, "\n", 1);
        if (test_m_assert_false())
        {
                char* msg = "test_m_assert_false failed\n";
                write(stdoutdup, msg, strlen(msg));
                ret = 1;
        }

        write(1, "\n", 1);
        if (test_sig_fault_no_catch())
        {
                char* msg = "test_sig_fault_no_catch failed\n";
                write(stdoutdup, msg, strlen(msg));
                ret = 1;
        }
// Run signal test two times to verify the signal handling works
        write(1, "\n", 1);
        if (test_sig_fault_no_catch())
        {
                char* msg = "test_sig_fault_no_catch failed\n";
                write(stdoutdup, msg, strlen(msg));
                ret = 1;
        }

        write(1, "\n", 1);
        if (test_sig_fault_catch())
        {
                char* msg = "test_sig_fault_catch failed\n";
                write(stdoutdup, msg, strlen(msg));
                ret = 1;
        }

        if (ret == 0)
        {
                char* msg = "\ntest_scut: All tests passed\n";
                write(stdoutdup, msg, strlen(msg));
        }
        else
        {
                char* msg = "\ntest_scut: Tests FAILED\n";
                write(stdoutdup, msg, strlen(msg));                
        }

        return ret;
}

int test_success(void)
{
        int ret = 0;

        scut_create("Test suite success");
        
        if (scut_add(&test_1, "Test 1"))
        {
                ret = 1;
                printf("Failed to add test\n");
        }

        if (scut_add(&test_2, "Test 2"))
        {
                ret = 1;
                printf("Failed to add test\n");
        }

        if (scut_run(SCUT_VERBOSE))
        {
                ret = 1;
        }

        scut_destroy();

        return ret;        
}

int test_failed(void)
{
        int ret = 0;
        int scut_ret;

        scut_create("Test suite shall fail");
        
        if (scut_add(&test_1, "Test 1"))
        {
                ret = 1;
                printf("Failed to add test\n");
        }

        if (scut_add(&test_3, "Test 3"))
        {
                ret = 1;
                printf("Failed to add test\n");
        }

        scut_ret = scut_run(0);
        if (scut_ret != 1)
        {
                ret = 1;
        }
        scut_destroy();

        return ret;        
}

int test_m_add(void)
{
        int ret = 0;

        scut_create("Test suite m_add");

        SCUT_ADD(test_1);
        
        ret += scut_run(0);
        if (scut_num_tests() != 1)
        {
                ret = 1;
        }

        scut_destroy();

        return ret;
}

int test_m_fail(void)
{
        int ret;

        scut_create("Test suite m_fail");

        SCUT_ADD(test_fail);
        ret = scut_run(0);
        scut_destroy();

        return ret == 0;
}

int test_m_assert_ie(void)
{
        int ret;

        scut_create("Test suite m_ie_ok");

        // Test ok
        SCUT_ADD(test_ie_ok);
        ret = scut_run(0);
        if (ret)
        {
                return 1;
        }
        scut_destroy();

        // test fail
        scut_create("Test suite m_ie_fail");
        SCUT_ADD(test_ie_fail);
        ret = scut_run(0);
        if (ret != 1 || scut_num_tests() != 1)
        {
                return 1;
        }

        scut_destroy();
        return 0;        
}

int test_m_assert_true(void)
{
        int ret;

        scut_create("Test suite m_true_ok");

        // Test ok
        SCUT_ADD(test_true_ok);
        ret = scut_run(0);
        scut_destroy();
        if (ret)
        {
                return 1;
        }

        // test fail
        scut_create("Test suite m_true_fail");
        SCUT_ADD(test_true_fail);
        ret = scut_run(0);
        if (ret != 1 || scut_num_tests() != 1)
        {
                scut_destroy();
                return 1;
        }
        scut_destroy();

        return 0;
}

int test_m_assert_false(void)
{
        int ret;

        scut_create("Test suite m_false_ok");

        // Test ok
        SCUT_ADD(test_false_ok);
        ret = scut_run(0);
        scut_destroy();
        if (ret)
        {
                return 1;
        }

        // test fail
        scut_create("Test suite m_false_fail");
        SCUT_ADD(test_false_fail);
        ret = scut_run(0);
        if (ret != 1 || scut_num_tests() != 1)
        {
                scut_destroy();
                return 1;
        }
        scut_destroy();

        return 0;
}

int test_sig_fault_no_catch(void)
{
        int ret;
        
        scut_create("Sig fault no catch");

        SCUT_ADD(test_sig);
        ret = scut_run(0);
        scut_destroy();

        if (ret == 1)
        {
                return 0;
        }
        return 1;
}

int test_sig_fault_catch(void)
{
        int ret;
        
        scut_create("Sig fault, catch");

        SCUT_ADD(test_sig_catch);
        ret = scut_run(0);
        scut_destroy();

        return ret;
}

/* Various test methods */

int test_1(void)
{
        printf("In test_1\n");
        return 0;
}

int test_2(void)
{
        printf("In test_2\n");
        return 0;
}

int test_3(void)
{
        printf("In test_3\n");
        return 1;
}

int test_fail(void)
{
        SCUT_FAIL("a message");
}

int test_ie_ok(void)
{
        SCUT_ASSERT_IE(1, 1);

        return 0;
}

int test_ie_fail(void)
{
        SCUT_ASSERT_IE(1, 0);

        return 0;
}

int test_true_ok(void)
{
        SCUT_ASSERT_TRUE(1);

        return 0;
}

int test_true_fail(void)
{
        SCUT_ASSERT_TRUE(0);

        return 0;
}

int test_false_ok(void)
{
        SCUT_ASSERT_FALSE(0);

        return 0;
}

int test_false_fail(void)
{
        SCUT_ASSERT_FALSE(1);

        return 0;
}

int test_sig(void)
{
        kill(getpid(), SIGUSR1);

        return 0;
}

int test_sig_catch(void)
{
        SCUT_EXPECT_SIG(SIGUSR1);
        
        test_sig();

        SCUT_ASSERT_SIG(SIGUSR1);

        return 0;
}
