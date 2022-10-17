#include "om.h"
#include "om_test.h"

static char str1[] = "This is str1";
static char str2[] = "This is str2";
static char str3[] = "This is str3";

static bool filter = false, sub_callback = false;

static char str_tmp[20];

static om_status_t deploy_fun(om_msg_t* msg, void* arg) {
  OM_UNUSED(arg);

  sub_callback = true;
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

START_TEST(_PUBLISH) {
  om_init();
  om_status_t res = OM_OK;

  om_topic_t* topic = om_config_topic(NULL, "fdac", "topic", 100u, filter_fun,
                                      NULL, deploy_fun, NULL);
  om_topic_t* topic2 = om_config_topic(NULL, "la", "topic2", 100, topic);
  ck_assert_msg(topic, "topic 指针为 NULL.");
  ck_assert_msg(topic2, "topic2 指针为 NULL.");

  ck_assert_msg(om_find_topic("topic", 0) == topic, "无法根据名称寻找话题。");

  om_publish(om_find_topic("topic2", 0), str2, sizeof(str2), true, false);
  ck_assert_msg(!strncmp(str_tmp, str2, 20), "publish数据损坏。");

  om_publish(om_find_topic("topic2", 0), str3, sizeof(str3), true, false);
  ck_assert_msg(strncmp(str_tmp, str3, 20), "filter函数未生效。");

  fail_if(res);
}
END_TEST

START_TEST(_PUBLISH_STATIC) {
  om_init();
  om_status_t res = OM_OK;

  om_topic_t _topic, _topic2;

  om_create_topic_static(&_topic, "static_topic", 100u);
  om_create_topic_static(&_topic2, "static_topic2", 100);

  om_suber_t suber;
  om_link_t link;

  uint8_t topic_buff[100];

  om_topic_t* topic = om_config_topic(&_topic, "fdax", filter_fun, NULL,
                                      deploy_fun, NULL, topic_buff);
  om_topic_t* topic2 = om_config_topic(&_topic2, "ka", &suber, &link, topic);

  ck_assert_msg(topic, "topic 指针为 NULL.");
  ck_assert_msg(topic2, "topic2 指针为 NULL.");

  ck_assert_msg(om_find_topic("static_topic", 0) == topic,
                "无法根据名称寻找话题。");

  om_publish(om_find_topic("static_topic2", 0), str2, sizeof(str2), true,
             false);
  ck_assert_msg(!strncmp(str_tmp, str2, 20), "publish数据损坏:%s", str_tmp);

  om_publish(om_find_topic("static_topic2", 0), str3, sizeof(str3), true,
             false);
  ck_assert_msg(strncmp(str_tmp, str3, 20), "filter函数未生效。");

  fail_if(res);
}
END_TEST

START_TEST(_LOG) {
  char str_log[] = {"Log test."};
  om_init();
  char buff[100] = {0};
  om_topic_t* topic_log = om_get_log_handle();

  om_suber_t* sub = om_subscript(topic_log, buff, sizeof(buff));

  ck_assert_msg(topic_log, "获取不到log话题。");
  om_print_log("init", OM_LOG_LEVEL_DEFAULT, true, false, "%s", str_log);
  om_suber_export(sub, false);
  ck_assert_msg(!strcmp(buff, "[init]\033[mLog test.\r\n"), "LOG数据错误:%s",
                buff);
}
END_TEST

typedef enum { EVENT_1 = 0x21, EVENT_2 = 0x01 } test_event_t;

uint32_t last_event = 0, event_counter = 0;

void event_callback(uint32_t event, void* arg) {
  OM_UNUSED(arg);

  last_event = event;
  event_counter++;
}

START_TEST(_EVENT) {
  om_init();

  om_event_group_t* evt_group = om_event_create_group("test_group");

  om_event_register(evt_group, EVENT_1, OM_EVENT_END, event_callback, NULL);
  om_event_register(evt_group, EVENT_2, OM_EVENT_START, event_callback, NULL);

  om_event_active(evt_group, EVENT_1, true, false);

  ck_assert_msg(event_counter == 0, "事件误触发");
  ck_assert_msg(last_event == 0, "事件误触发");

  om_event_active(evt_group, EVENT_2, true, false);

  ck_assert_msg(event_counter == 2, "事件触发失败");
  ck_assert_msg(last_event == EVENT_2, "触发了错误的事件");
}
END_TEST

START_TEST(_EVENT_STATIC) {
  om_init();

  om_event_group_t group;

  om_event_create_group_static(&group, "test_group");

  om_event_t event1, event2;

  om_event_register_static(&event1, &group, EVENT_1, OM_EVENT_END,
                           event_callback, NULL);
  om_event_register_static(&event2, &group, EVENT_2, OM_EVENT_START,
                           event_callback, NULL);

  om_event_active(&group, EVENT_1, true, false);

  ck_assert_msg(event_counter == 0, "事件误触发");
  ck_assert_msg(last_event == 0, "事件误触发");

  om_event_active(&group, EVENT_2, true, false);

  ck_assert_msg(event_counter == 2, "事件触发失败");
  ck_assert_msg(last_event == EVENT_2, "触发了错误的事件");
}
END_TEST

typedef struct {
  uint32_t range;
  uint8_t list[10];
  float decompose;
} om_afl_test_t;

START_TEST(_FILTER) {
  om_init();

  om_delay_ms(1000);

  om_afl_test_t test = {0}, ans1 = {0}, ans2 = {0}, ans3 = {0};
  uint8_t fl_template[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

  om_topic_t* source =
      om_config_topic(NULL, NULL, "source", sizeof(om_afl_test_t));
  om_topic_t* list = om_config_topic(NULL, NULL, "list", sizeof(om_afl_test_t));
  om_topic_t* range =
      om_config_topic(NULL, NULL, "range", sizeof(om_afl_test_t));
  om_topic_t* decompose =
      om_config_topic(NULL, NULL, "decompose", sizeof(om_afl_test_t));

  om_suber_t* list_sub = om_subscript(list, &(ans1), sizeof(ans1));
  om_suber_t* range_sub = om_subscript(range, &(ans2), sizeof(ans2));
  om_suber_t* decompose_sub =
      om_subscript(decompose, &(ans3.decompose), sizeof(ans3.decompose));

  om_config_filter(source, "LDR", list, OM_PRASE_STRUCT(om_afl_test_t, list),
                   fl_template, decompose,
                   OM_PRASE_STRUCT(om_afl_test_t, decompose), range,
                   OM_PRASE_STRUCT(om_afl_test_t, range), 213, 100);

  memcpy(&test.list, fl_template, sizeof(fl_template));
  om_publish(source, &test, sizeof(test), true, false);
  om_suber_export(list_sub, false);
  ck_assert_msg(!memcmp(&test.list, &ans1.list, sizeof(ans1.list)),
                "过滤器list模式数据错误");
  test.list[1] = 0;
  om_publish(source, &test, sizeof(test), true, false);
  om_suber_export(list_sub, false);
  ck_assert_msg(memcmp(&test.list, &ans1.list, sizeof(ans1.list)),
                "过滤器list模式失效");

  for (uint32_t i = 0; i < 100; i++) {
    test.range = 213 + i;
    om_publish(source, &test, sizeof(test), true, false);
    om_suber_export(range_sub, false);
    ck_assert_msg(test.range == ans2.range,
                  "过滤器range模式数据错误在%d,应为%d,实际为%d", i, test.range,
                  ans2.range);
  }

  for (uint32_t i = 1000; i < 2000; i++) {
    test.range = 213 + i;
    om_publish(source, &test, sizeof(test), true, false);
    om_suber_export(range_sub, false);
    ck_assert_msg(test.range != ans2.range,
                  "过滤器range模式失效在%d,应为%d,实际为%d", i, test.range,
                  ans2.range);
  }

  test.decompose = 5.63f;
  om_publish(source, &test, sizeof(test), true, false);
  om_suber_export(decompose_sub, false);
  ck_assert_msg(test.decompose == ans3.decompose,
                "过滤器decompose模式数据错误");
}
END_TEST

START_TEST(_FILTER_STATIC) {
  om_init();

  om_delay_ms(1000);

  om_afl_test_t test = {0}, ans1 = {0}, ans2 = {0}, ans3 = {0};
  uint8_t fl_template[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

  om_topic_t source, list, range, decompose;

  om_create_topic_static(&source, "source", sizeof(om_afl_test_t));
  om_create_topic_static(&list, "list", sizeof(om_afl_test_t));
  om_create_topic_static(&range, "range", sizeof(om_afl_test_t));
  om_create_topic_static(&decompose, "decompose", sizeof(om_afl_test_t));

  om_suber_t* list_sub = om_subscript(&list, &(ans1), sizeof(ans1));
  om_suber_t* range_sub = om_subscript(&range, &(ans2), sizeof(ans2));
  om_suber_t* decompose_sub =
      om_subscript(&decompose, &(ans3.decompose), sizeof(ans3.decompose));

  om_afl_t afl;
  om_afl_filter_t f_list, f_range, f_decompose;

  om_config_filter_static(
      &source, "LDR", &afl, &f_list, &list,
      OM_PRASE_STRUCT(om_afl_test_t, list), fl_template, &f_decompose,
      &decompose, OM_PRASE_STRUCT(om_afl_test_t, decompose), &f_range, &range,
      OM_PRASE_STRUCT(om_afl_test_t, range), 213, 100);

  memcpy(&test.list, fl_template, sizeof(fl_template));
  om_publish(&source, &test, sizeof(test), true, false);
  om_suber_export(list_sub, false);
  ck_assert_msg(!memcmp(&test.list, &ans1.list, sizeof(ans1.list)),
                "过滤器list模式数据错误");
  test.list[1] = 0;
  om_publish(&source, &test, sizeof(test), true, false);
  om_suber_export(list_sub, false);
  ck_assert_msg(memcmp(&test.list, &ans1.list, sizeof(ans1.list)),
                "过滤器list模式失效");

  for (uint32_t i = 0; i < 100; i++) {
    test.range = 213 + i;
    om_publish(&source, &test, sizeof(test), true, false);
    om_suber_export(range_sub, false);
    ck_assert_msg(test.range == ans2.range,
                  "过滤器range模式数据错误在%d,应为%d,实际为%d", i, test.range,
                  ans2.range);
  }

  for (uint32_t i = 1000; i < 2000; i++) {
    test.range = 213 + i;
    om_publish(&source, &test, sizeof(test), true, false);
    om_suber_export(range_sub, false);
    ck_assert_msg(test.range != ans2.range,
                  "过滤器range模式失效在%d,应为%d,实际为%d", i, test.range,
                  ans2.range);
  }

  test.decompose = 5.63f;
  om_publish(&source, &test, sizeof(test), true, false);
  om_suber_export(decompose_sub, false);
  ck_assert_msg(test.decompose == ans3.decompose,
                "过滤器decompose模式数据错误");
}
END_TEST

Suite* make_om_module_suite(void) {
  Suite* om_module = suite_create("模块测试");

  TCase* tc_log = tcase_create("日志测试");
  suite_add_tcase(om_module, tc_log);
  tcase_add_test(tc_log, _LOG);

  TCase* tc_public = tcase_create("订阅发布测试");
  suite_add_tcase(om_module, tc_public);
  tcase_add_test(tc_public, _PUBLISH);

  TCase* tc_public_static = tcase_create("静态订阅发布测试");
  suite_add_tcase(om_module, tc_public_static);
  tcase_add_test(tc_public_static, _PUBLISH_STATIC);

  TCase* tc_event = tcase_create("事件触发测试");
  suite_add_tcase(om_module, tc_event);
  tcase_add_test(tc_event, _EVENT);

  TCase* tc_event_static = tcase_create("静态事件触发测试");
  suite_add_tcase(om_module, tc_event_static);
  tcase_add_test(tc_event_static, _EVENT_STATIC);

  TCase* tc_filter = tcase_create("高级过滤器测试");
  suite_add_tcase(om_module, tc_filter);
  tcase_add_test(tc_filter, _FILTER);

  TCase* tc_filter_static = tcase_create("静态高级过滤器测试");
  suite_add_tcase(om_module, tc_filter_static);
  tcase_add_test(tc_filter_static, _FILTER_STATIC);

  return om_module;
}
