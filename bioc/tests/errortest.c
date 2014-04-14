/*
 * File:   errortest.c
 * Author: roberto
 *
 * Created on Apr 14, 2014, 11:53:14 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>

/*
 * CUnit Test Suite
 */

int init_suite(void) {
    return 0;
}

int clean_suite(void) {
    return 0;
}

void* checkPointerError(void* data, char* msg, char* file, int line, int exit_status);

void testCheckPointerError() {
    char* msg = "Testing memory allocation failed\n";
    void* result = checkPointerError(calloc(1, sizeof (int)), msg, __FILE__, __LINE__, -1);
    if (!result) {
        CU_ASSERT(0);
    }
}

void printLog(FILE* out, char* msg, char* file, int line, int exit_status);

void testPrintLog() {
    FILE* out = stdout;
    char* msg = "\tTesting the print out";
    printLog(out, msg, __FILE__, __LINE__, 0); 
}

int main() {
    CU_pSuite pSuite = NULL;

    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* Add a suite to the registry */
    pSuite = CU_add_suite("errortest", init_suite, clean_suite);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Add the tests to the suite */
    if ((NULL == CU_add_test(pSuite, "testCheckPointerError", testCheckPointerError)) ||
            (NULL == CU_add_test(pSuite, "testPrintLog", testPrintLog))) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /* Run all tests using the CUnit Basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}
