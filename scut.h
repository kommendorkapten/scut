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

#ifndef __SCUT_H__
#define __SCUT_H__

#include <stdio.h>

#define SCUT_ADD(m) scut_add(&m, #m)
#define SCUT_FAIL(msg) do {printf("%s: %s+%d\n", msg, __FILE__, __LINE__); return 1;} while(0)
#define SCUT_ASSERT_IE(a, b) do {if((long)(a) != (long)(b)) {           \
                        printf("Assertion error, found %ld, expected %ld: %s+%d\n", (long)(a), (long)(b), __FILE__, __LINE__); \
                        return 1;}} while (0)
#define SCUT_ASSERT_TRUE(a) do {if(!(a)){                               \
                        printf("Assertion failed, expected true: %s+%d\n", __FILE__, __LINE__);return 1;}} while(0)
#define SCUT_ASSERT_FALSE(a) do {if((a)){                               \
                        printf("Assertion failed, expected false: %s+%d\n", __FILE__, __LINE__);return 1;}} while(0)
#define SCUT_EXPECT_SIG(s) scut_expect_sig((s))
#define SCUT_ASSERT_SIG(s) do {if(!scut_assert_sig((s))){               \
                        printf("Assertion error, signal %d was not caught: %s+%d\n", (s), __FILE__, __LINE__); return 1;}} while(0)

#define SCUT_VERBOSE 0x1
#define UNIT_TEST

/**
 * Creates a test suite. Tests can then be added to the suite for
 * execution. The suite is created as a static global object. Only
 * on suite can exist at a given time. Multiple calls to scut_create
 * will destroy any earlier created, and only the last created will 
 * we active.
 * @param The name of the suite.
 * @return void
 */
void scut_create(const char*);

/**
 * Add a single test to an existing suite. The test will not yet be 
 * executed.
 * @param the test to run. A test has the follwing signature:
 *        int (*f)(void). The test shall return 0 on success, and non zero
 *        upon failure.
 * @param The name of the test, this must be a null terminated string.
 * @return 0 if the test was successfully added.
 */
int scut_add(int (*test)(void), 
             const char*);

/**
 * Executes the tests in the provided suite.
 * Any output from a test will be captured, and not displayed unless the test
 * fails (this can be changed via flags).
 * @param On ore more flags, multiple flags can be "ored" (|) together.
 * @return The number of failed tests. 0 is returned if all tests were
 *         sucessfully executed.
 */
int scut_run(int);

/**
 * Announce that a signal is expected.
 * @param the signal to expect.
 * @return void.
 */
void scut_expect_sig(int);

/**
 * Verify that a signal has been generated.
 * @param the signal to expect.
 * @return 1 if the signal was caught.
 */
int scut_assert_sig(int);

/**
 * Returns the number of stored tests in a suite.
 * @return the number of tests stored in this suite.
 */
int scut_num_tests(void);

/**
 * Destroy a (and free any allocated memory) a suite.
 * @return void.
 */
void scut_destroy(void);

#endif /* __SCUT_H__ */
