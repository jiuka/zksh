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

START_TEST(test_join_to_root) {
  char *r = zksh_join_path("/", "yolo");
  ck_assert_str_eq(r, "/yolo");
}
END_TEST

START_TEST(test_join_to_node) {
    char *r = zksh_join_path("/foo", "bar");
      ck_assert_str_eq(r, "/foo/bar");
}
END_TEST

int main(void) {
    int number_failed;
    Suite *s;
    TCase *tc;
    SRunner *sr;

    s = suite_create("zksh_join_path");

    tc = tcase_create("main");
    tcase_add_test(tc, test_join_to_root);
    tcase_add_test(tc, test_join_to_node);

    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);

    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
