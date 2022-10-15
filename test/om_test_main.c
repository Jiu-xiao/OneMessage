#include <stdlib.h>

#include "om_test.h"

int main(void) {
  int n = 0;

  SRunner* base = srunner_create(make_om_base_suite());
  srunner_set_fork_status(base, CK_FORK);
  srunner_run_all(base, CK_VERBOSE);
  n += srunner_ntests_failed(base);
  srunner_free(base);

  SRunner* module = srunner_create(make_om_module_suite());
  srunner_set_fork_status(module, CK_FORK);
  srunner_run_all(module, CK_VERBOSE);
  n += srunner_ntests_failed(module);
  srunner_free(module);

  return (n == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}