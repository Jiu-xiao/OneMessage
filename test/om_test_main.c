#include <stdlib.h>

#include "om_test.h"

int main(void) {
  int n;
  SRunner *sr;
  sr = srunner_create(make_om_suite());
  srunner_run_all(sr, CK_VERBOSE);
  n = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (n == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}