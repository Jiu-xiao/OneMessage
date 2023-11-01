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

START_TEST(_FIFO) {
  om_fifo_t fifo;
  char data[10], buff[10];

  om_fifo_create(&fifo, data, 10, sizeof(char));

  for (int i = 0; i < 8; i++) {
    om_fifo_write(&fifo, &i);
    om_fifo_pop(&fifo);
  }

  for (int i = 0; i < 4; i++) {
    om_fifo_write(&fifo, &i);
  }

  for (int i = 0; i < 4; i++) {
    char tmp = 0;
    om_fifo_jump_peek(&fifo, i, &tmp);
    ck_assert_msg(tmp == i, "数据损坏，应为%d，实际为%d", i, tmp);
  }

  void* tmp = NULL;
  for (int i = 0; i < 100; i++) {
    tmp = om_fifo_foreach_dist(&fifo, tmp);
    if (tmp == NULL) {
      break;
    }
    ck_assert_msg(*((char*)tmp) == i, "数据损坏，应为%d，实际为%d", i,
                  *((char*)tmp));
  }

  for (int i = 0; i < 4; i++) {
    om_fifo_read(&fifo, buff);
  }

  for (int i = 0; i < 10; i++) {
    char data = '0' + i;
    om_fifo_write(&fifo, &data);
  }

  for (int i = 0; i < 10; i++) {
    char data = '0' + i, tmp = 0;
    om_fifo_read(&fifo, &tmp);
    ck_assert_msg(data == tmp, "队列数据损坏。应为：%d 实际：%d", data, tmp);
  }

  for (int i = 0; i < 10; i++) {
    buff[i] = '0' + i;
  }

  om_fifo_writes(&fifo, buff, 10);

  for (int i = 0; i < 10; i++) {
    buff[i] = 0;
  }

  ck_assert_msg(fifo.is_full, "队列未满");

  ck_assert_msg(om_fifo_write(&fifo, "0") == OM_ERROR_FULL, "队列未满");

  om_fifo_reads(&fifo, buff, 10);

  for (int i = 0; i < 10; i++) {
    char data = '0' + i;
    ck_assert_msg(data == buff[i], "批量读写第%d个数据损坏。应为：%d 实际：%d",
                  i, data, buff[i]);
  }
}
END_TEST

START_TEST(_RBT) {
  RBT_ROOT(rbt);
  om_rbt_node_t node[50] = {};
  for (int i = 0; i < 50; i++) {
    char* tmp = malloc(50);
    memset(tmp, 0, 50);
    tmp[0] = i + 'A';
    node[i].key = tmp;
    om_rbtree_insert(&rbt, &node[i]);
  }

  om_rbt_node_t* tmp = NULL;

  for (int i = 0; i < 50; i++) {
    tmp = om_rbtree_foreach_disc(&rbt, tmp);
    ck_assert_msg(tmp->key[0] == 'A' + i, "遍历第%d个数据错误，为%d", i,
                  tmp->key[0] - 'A');
  }
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

  TCase* tc_fifo = tcase_create("队列测试");
  suite_add_tcase(om_base, tc_fifo);
  tcase_add_test(tc_fifo, _FIFO);

  TCase* tc_rbt = tcase_create("红黑树测试");
  suite_add_tcase(om_base, tc_rbt);
  tcase_add_test(tc_rbt, _RBT);

  return om_base;
}
