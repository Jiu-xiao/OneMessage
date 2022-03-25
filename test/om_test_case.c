#include "om.h"
#include "om_test.h"

static char str1[] = "This is str1";
static char str2[] = "This is str2";
static char str3[] = "This is str3";

static bool msg_new = false, filter = false, deploy = false, get = false;

static int call_counter = 0;

static om_status_t new_fun(om_msg_t* msg, void* arg) {
  OM_UNUSED(arg);

  call_counter++;
  if (msg_new)
    return OM_OK;
  else
    return OM_ERROR;
}

static om_status_t get_fun(om_msg_t* msg, void* arg) {
  OM_UNUSED(arg);

  get = true;
  msg->buff = str1;
  msg->size = sizeof(str1);
  return OM_OK;
}

static char str_tmp[20];

static om_status_t deploy_fun(om_msg_t* msg, void* arg) {
  OM_UNUSED(arg);

  deploy = true;
  strncpy(str_tmp, msg->buff, 20);
  return OM_OK;
}

static om_status_t filter_fun(om_msg_t* msg, void* arg) {
  OM_UNUSED(arg);

  filter = true;
  if (!strncmp(str1, msg->buff, 20) || !strncmp(str2, msg->buff, 20))
    return OM_OK;
  else
    return OM_ERROR;
}

static float pub_freq = 12.5;

START_TEST(publish) {
  om_init();
  om_status_t res = OM_OK;
  om_topic_t* topic =
      om_config_topic(NULL, "fdna", "topic", filter_fun, NULL, deploy_fun, NULL,
                      new_fun, NULL, get_fun, NULL);
  om_topic_t* topic2 = om_config_topic(NULL, "la", "topic2", topic);
  ck_assert_msg(topic, "topic 指针为 NULL.");
  ck_assert_msg(topic2, "topic2 指针为 NULL.");

  for (uint32_t i = 0; i < 100; i++) om_sync(false);
  ck_assert_msg(call_counter == 100, "new_fun调用了%d次，应为100次",
                call_counter);

  ck_assert_msg(!deploy, "意外调用了deploy函数。");
  ck_assert_msg(!filter, "意外调用了filter函数。");
  msg_new = true;

  for (uint32_t i = 0; i < 100; i++) om_sync(false);
  ck_assert_msg(deploy, "未调用deploy函数。");
  ck_assert_msg(filter, "未调用filter函数。");
  ck_assert_msg(!strncmp(str_tmp, str1, 20), "sync数据损坏。");
  msg_new = false;

  ck_assert_msg(om_find_topic("topic", 0) == topic, "无法根据名称寻找话题。");

  om_publish(om_find_topic("topic2", 0), str2, sizeof(str2), true, false);
  ck_assert_msg(!strncmp(str_tmp, str2, 20), "publish数据损坏。");

  om_publish(om_find_topic("topic2", 0), str3, sizeof(str3), true, false);
  ck_assert_msg(strncmp(str_tmp, str3, 20), "filter函数未生效。");

  res = om_deinit();

  fail_if(res);
}
END_TEST

char str_log[] = {"Log test."};

START_TEST(om_log) {
  om_init();
  char buff[100] = {0};
  om_topic_t* topic_log = om_get_log_handle();

  om_suber_t* sub = om_subscript(topic_log, buff, sizeof(buff));

  ck_assert_msg(topic_log, "获取不到log话题。");
  om_print_log("init", OM_LOG_DEFAULT, true, false, "%s", str_log);
  om_suber_dump(sub, false);
  ck_assert_msg(!strcmp(buff, "[Default][init]Log test.\r\n"), "LOG数据错误:%s",
                buff);
  om_deinit();
}
END_TEST

typedef struct {
  double res;
  uint32_t range;
  uint8_t list[10];
  float decompose;
} om_afl_test_t;

START_TEST(om_afl) {
  om_init();

  om_delay_ms(1000);

  om_afl_test_t test = {0}, ans1 = {0}, ans2 = {0}, ans3 = {0};
  uint8_t template[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

  om_topic_t* source = om_config_topic(NULL, NULL, "source");
  om_topic_t* list = om_config_topic(NULL, NULL, "list");
  om_topic_t* range = om_config_topic(NULL, NULL, "range");
  om_topic_t* decompose = om_config_topic(NULL, NULL, "decompose");

  om_suber_t* list_sub = om_subscript(list, &(ans1), sizeof(ans1));
  om_suber_t* range_sub = om_subscript(range, &(ans2), sizeof(ans2));
  om_suber_t* decompose_sub =
      om_subscript(decompose, &(ans3.decompose), sizeof(ans3.decompose));

  om_config_filter(source, "LDR", list, OM_PRASE_STRUCT(om_afl_test_t, list),
                   template, decompose,
                   OM_PRASE_STRUCT(om_afl_test_t, decompose), range,
                   OM_PRASE_STRUCT(om_afl_test_t, range), 213, 100);

  memcpy(&test.list, template, sizeof(template));
  om_publish(source, &test, sizeof(test), true, false);
  om_suber_dump(list_sub, false);
  ck_assert_msg(!memcmp(&test.list, &ans1.list, sizeof(ans1.list)),
                "过滤器list模式数据错误");
  test.list[1] = 0;
  om_publish(source, &test, sizeof(test), true, false);
  om_suber_dump(list_sub, false);
  ck_assert_msg(memcmp(&test.list, &ans1.list, sizeof(ans1.list)),
                "过滤器list模式失效");

  for (uint32_t i = 0; i <= 100; i++) {
    test.range = 213 + i;
    om_publish(source, &test, sizeof(test), true, false);
    om_suber_dump(range_sub, false);
    ck_assert_msg(test.range == ans2.range,
                  "过滤器range模式数据错误在%d,应为%d,实际为%d", i, test.range,
                  ans2.range);
  }

  for (uint32_t i = 1000; i <= 2000; i++) {
    test.range = 213 + i;
    om_publish(source, &test, sizeof(test), true, false);
    om_suber_dump(range_sub, false);
    ck_assert_msg(test.range != ans2.range,
                  "过滤器range模式失效在%d,应为%d,实际为%d", i, test.range,
                  ans2.range);
  }

  test.decompose = 5.63f;
  om_publish(source, &test, sizeof(test), true, false);
  om_suber_dump(decompose_sub, false);
  ck_assert_msg(test.decompose == ans3.decompose,
                "过滤器decompose模式数据错误");

  om_deinit();
}
END_TEST

Suite* make_om_suite(void) {
  Suite* om = suite_create("OneMessage单元测试");
  TCase* tc_publish = tcase_create("订阅发布测试");
  TCase* tc_log = tcase_create("日志测试");
  TCase* tc_afl = tcase_create("过滤器测试");
  suite_add_tcase(om, tc_publish);
  suite_add_tcase(om, tc_log);
  suite_add_tcase(om, tc_afl);
  tcase_add_test(tc_publish, publish);
  tcase_add_test(tc_log, om_log);
  tcase_add_test(tc_afl, om_afl);
  return om;
}
