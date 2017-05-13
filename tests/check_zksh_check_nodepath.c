/*
 * Check: a unit test framework for C
 * Copyright (C) 2001, 2002 Arien Malec
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <stdlib.h>
#include <check.h>
#include "../src/zksh.h"

char *usage_description = "";

START_TEST(test_check_root) {
  ck_assert_int_eq(zksh_check_nodepath("/"), 0);
}
END_TEST

START_TEST(test_check_node) {
  ck_assert_int_eq(zksh_check_nodepath("/foo/bar"), 0);
}
END_TEST

START_TEST(test_check_invalid) {
  ck_assert_int_eq(zksh_check_nodepath("foo/bar"), 1);
}
END_TEST

int main(void) {
    int number_failed;
    Suite *s;
    TCase *tc;
    SRunner *sr;

    s = suite_create("zksh_check_nodepath");

    tc = tcase_create("main");
    tcase_add_test(tc, test_check_root);
    tcase_add_test(tc, test_check_node);
    tcase_add_test(tc, test_check_invalid);

    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);

    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
