#include "om.h"
#include "om_test.h"

START_TEST(_MALLOC) {
  void* ptr = NULL;

  ptr = om_malloc(sizeof(int));
  ck_assert_msg(ptr, "内存分配失败");
  om_free(ptr);
}
END_TEST

START_TEST(_MUTEX) {
  om_mutex_t ptr;

  om_mutex_init(&ptr);

  om_mutex_lock(&ptr);
  om_mutex_unlock(&ptr);

  if ((om_mutex_trylock(&ptr)) == OM_OK) {
    om_mutex_unlock(&ptr);
  } else {
    ck_assert_msg(false, "解锁失败");
  }

  om_mutex_lock(&ptr);

  if (om_mutex_trylock(&ptr) != OM_OK) {
    om_mutex_unlock(&ptr);
  } else {
    ck_assert_msg(false, "加锁失败");
  }

  om_mutex_delete(&ptr);
}
END_TEST

Suite* make_om_base_suite(void) {
  Suite* om_base = suite_create("底层API测试");

  TCase* tc_malloc = tcase_create("内存分配测试");
  suite_add_tcase(om_base, tc_malloc);
  tcase_add_test(tc_malloc, _MALLOC);

  TCase* tc_mutex = tcase_create("互斥锁测试");
  suite_add_tcase(om_base, tc_mutex);
  tcase_add_test(tc_mutex, _MUTEX);

  return om_base;
}
