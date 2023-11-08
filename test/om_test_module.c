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

START_TEST(_QUEUE) {
  om_topic_t* topic = om_config_topic(NULL, "a", "topic", 1u);
  om_fifo_t* queue = om_queue_add(topic, 10);
  for (uint8_t i = 0; i < 5; i++) {
    om_publish(topic, &i, sizeof(i), true, false);
  }
  OM_TOPIC_LOCK(topic);
  ck_assert_msg(om_fifo_readable_item_count(queue) == 5, "队列元素数量错误");

  for (uint8_t i = 0; i < 5; i++) {
    uint8_t tmp;

    om_fifo_read(queue, &tmp);
    ck_assert_msg(tmp == i, "队列元素数据损坏");
  }
  OM_TOPIC_UNLOCK(topic);
}
END_TEST

START_TEST(_QUEUE_STATIC) {
  om_topic_t topic;
  om_fifo_t queue;
  om_suber_t suber;
  uint8_t buff[10];
  om_create_topic_static(&topic, "static_topic", 1u);
  om_queue_init_fifo_static(&topic, &queue, buff, 10);
  om_queue_add_static(&topic, &suber, &queue);
  for (uint8_t i = 0; i < 5; i++) {
    om_publish(&topic, &i, sizeof(i), true, false);
  }
  OM_TOPIC_LOCK(&topic);
  ck_assert_msg(om_fifo_readable_item_count(&queue) == 5, "队列元素数量错误");

  for (uint8_t i = 0; i < 5; i++) {
    uint8_t tmp;

    om_fifo_read(&queue, &tmp);
    ck_assert_msg(tmp == i, "队列元素数据损坏");
  }
  OM_TOPIC_UNLOCK(&topic);
}
END_TEST

START_TEST(_LOG) {
  char str_log[] = {"Log test."};
  om_init();
  char buff[100] = {0};
  om_topic_t* topic_log = om_get_log_handle();

  om_suber_t* sub = om_subscribe(topic_log);

  ck_assert_msg(topic_log, "获取不到log话题。");
  om_print_log("init", OM_LOG_LEVEL_DEFAULT, true, false, "%s", str_log);
  om_suber_export(sub, buff, false);
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
  ck_assert_msg(last_event == EVENT_1, "触发了错误的事件,应为%d,实际为%d",
                EVENT_1, last_event);
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
  ck_assert_msg(last_event == EVENT_1, "触发了错误的事件,应为%d,实际为%d",
                EVENT_1, last_event);
}
END_TEST

typedef struct {
  uint32_t range;
  uint8_t list[10];
  float decompose;
} om_afl_test_t;

START_TEST(_FILTER) {
  om_init();

  om_afl_test_t test = {0}, ans1 = {0}, ans2 = {0}, ans3 = {0};
  uint8_t fl_template[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

  om_topic_t* source =
      om_config_topic(NULL, NULL, "source", sizeof(om_afl_test_t));
  om_topic_t* list = om_config_topic(NULL, NULL, "list", sizeof(om_afl_test_t));
  om_topic_t* range =
      om_config_topic(NULL, NULL, "range", sizeof(om_afl_test_t));
  om_topic_t* decompose =
      om_config_topic(NULL, NULL, "decompose", sizeof(om_afl_test_t));

  om_suber_t* list_sub = om_subscribe(list);
  om_suber_t* range_sub = om_subscribe(range);
  om_suber_t* decompose_sub = om_subscribe(decompose);

  om_config_filter(source, "LDR", list, OM_PRASE_STRUCT(om_afl_test_t, list),
                   fl_template, decompose,
                   OM_PRASE_STRUCT(om_afl_test_t, decompose), range,
                   OM_PRASE_STRUCT(om_afl_test_t, range), 213, 100);

  memcpy(&test.list, fl_template, sizeof(fl_template));
  om_publish(source, &test, sizeof(test), true, false);
  om_suber_export(list_sub, &ans1, false);
  ck_assert_msg(!memcmp(&test.list, &ans1.list, sizeof(ans1.list)),
                "过滤器list模式数据错误");
  test.list[1] = 0;
  om_publish(source, &test, sizeof(test), true, false);
  ck_assert_msg(memcmp(&test.list, &ans1.list, sizeof(ans1.list)),
                "过滤器list模式失效");

  for (uint32_t i = 0; i < 100; i++) {
    test.range = 213 + i;
    om_publish(source, &test, sizeof(test), true, false);
    om_suber_export(range_sub, &ans2, false);
    ck_assert_msg(test.range == ans2.range,
                  "过滤器range模式数据错误在%d,应为%d,实际为%d", i, test.range,
                  ans2.range);
  }

  for (uint32_t i = 1000; i < 2000; i++) {
    test.range = 213 + i;
    om_publish(source, &test, sizeof(test), true, false);
    om_suber_export(range_sub, &ans2, false);
    ck_assert_msg(test.range != ans2.range,
                  "过滤器range模式失效在%d,应为%d,实际为%d", i, test.range,
                  ans2.range);
  }

  test.decompose = 5.63f;
  om_publish(source, &test, sizeof(test), true, false);
  om_suber_export(decompose_sub, &ans3.decompose, false);
  ck_assert_msg(test.decompose == ans3.decompose,
                "过滤器decompose模式数据错误");
}
END_TEST

START_TEST(_FILTER_STATIC) {
  om_init();

  om_afl_test_t test = {0}, ans1 = {0}, ans2 = {0}, ans3 = {0};
  uint8_t fl_template[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

  om_topic_t source, list, range, decompose;

  om_create_topic_static(&source, "source", sizeof(om_afl_test_t));
  om_create_topic_static(&list, "list", sizeof(om_afl_test_t));
  om_create_topic_static(&range, "range", sizeof(om_afl_test_t));
  om_create_topic_static(&decompose, "decompose", sizeof(om_afl_test_t));

  om_suber_t* list_sub = om_subscribe(&list);
  om_suber_t* range_sub = om_subscribe(&range);
  om_suber_t* decompose_sub = om_subscribe(&decompose);

  om_afl_t afl;
  om_afl_filter_t f_list, f_range, f_decompose;

  om_config_filter_static(
      &source, "LDR", &afl, &f_list, &list,
      OM_PRASE_STRUCT(om_afl_test_t, list), fl_template, &f_decompose,
      &decompose, OM_PRASE_STRUCT(om_afl_test_t, decompose), &f_range, &range,
      OM_PRASE_STRUCT(om_afl_test_t, range), 213, 100);

  memcpy(&test.list, fl_template, sizeof(fl_template));
  om_publish(&source, &test, sizeof(test), true, false);
  om_suber_export(list_sub, &ans1, false);
  ck_assert_msg(!memcmp(&test.list, &ans1.list, sizeof(ans1.list)),
                "过滤器list模式数据错误");
  test.list[1] = 0;
  om_publish(&source, &test, sizeof(test), true, false);
  om_suber_export(list_sub, &ans1, false);
  ck_assert_msg(memcmp(&test.list, &ans1.list, sizeof(ans1.list)),
                "过滤器list模式失效");

  for (uint32_t i = 0; i < 100; i++) {
    test.range = 213 + i;
    om_publish(&source, &test, sizeof(test), true, false);
    om_suber_export(range_sub, &ans2, false);
    ck_assert_msg(test.range == ans2.range,
                  "过滤器range模式数据错误在%d,应为%d,实际为%d", i, test.range,
                  ans2.range);
  }

  for (uint32_t i = 1000; i < 2000; i++) {
    test.range = 213 + i;
    om_publish(&source, &test, sizeof(test), true, false);
    om_suber_export(range_sub, &ans2, false);
    ck_assert_msg(test.range != ans2.range,
                  "过滤器range模式失效在%d,应为%d,实际为%d", i, test.range,
                  ans2.range);
  }

  test.decompose = 5.63f;
  om_publish(&source, &test, sizeof(test), true, false);
  om_suber_export(decompose_sub, &ans3.decompose, false);
  ck_assert_msg(test.decompose == ans3.decompose,
                "过滤器decompose模式数据错误");
}
END_TEST

typedef struct {
  uint32_t test;
  char str[10];
} om_com_test_t;

static uint32_t com_pub_count = 0;

static om_status_t com_cb(om_msg_t* msg, void* arg) {
  OM_UNUSED(arg);

  com_pub_count++;

  return OM_OK;
}

START_TEST(_COM) {
  om_init();

  om_topic_t* source = om_config_topic(NULL, "cad", "source",
                                       sizeof(om_com_test_t), com_cb, NULL);

  om_topic_t* source1 =
      om_config_topic(NULL, "ca", "source1", sizeof(om_com_test_t));

  om_com_t com;

  om_com_test_t test_data = {.str = "test data1", .test = 0x12345678};

  om_com_create(&com, 128, 5, 128);

  om_com_add_topic_with_name(&com, "source1");

  om_com_add_topic(&com, source);

  OM_COM_TYPE(om_com_test_t) trans_buffer;

  om_publish(source, &test_data, sizeof(om_com_test_t), true, false);

  ck_assert(com_pub_count == 1);

  om_com_generate_pack(source, &trans_buffer);

  test_data = (om_com_test_t){.str = "test data2", .test = 0x12785634};

  om_publish(source, &test_data, sizeof(om_com_test_t), true, false);

  ck_assert(com_pub_count == 2);

  /* prase an entire packet of data */
  for (int i = 0; i < 100; i++) {
    com_pub_count = 0;
    om_com_prase_recv(&com, (uint8_t*)&trans_buffer,
                      OM_COM_TYPE_SIZE(om_com_test_t), true, false);

    ck_assert(com_pub_count == 1);
  }

  om_com_test_t* test_ans = (om_com_test_t*)source->msg.buff;

  ck_assert_msg(strncmp(test_ans->str, "test data1", 10) == 0, "数据损坏");
  ck_assert_msg(test_ans->test == 0x12345678, "数据损坏");

  /* prase packet by byte */
  for (int j = 0; j < 100; j++) {
    com_pub_count = 0;
    for (int i = 0; i < sizeof(trans_buffer); i++) {
      ck_assert(com_pub_count == 0);
      uint8_t* data_buff = (uint8_t*)&trans_buffer;
      om_com_prase_recv(&com, data_buff + i, 1, true, false);
    }
    ck_assert(com_pub_count == 1);
  }

  /* prase packet with invalid data */
  for (uint8_t j = 0; j <= 254; j++) {
    com_pub_count = 0;
    for (int t = 0; t < j; t++) {
      om_com_prase_recv(&com, &j, 1, true, false);
    }
    for (int i = 0; i < sizeof(trans_buffer); i++) {
      ck_assert(com_pub_count == 0);
      uint8_t* data_buff = (uint8_t*)&trans_buffer;
      om_com_prase_recv(&com, data_buff + i, 1, true, false);
    }
    ck_assert(com_pub_count == 1);
  }
}

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

  TCase* tc_queue = tcase_create("队列测试");
  suite_add_tcase(om_module, tc_queue);
  tcase_add_test(tc_queue, _QUEUE);

  TCase* tc_queue_static = tcase_create("静态队列测试");
  suite_add_tcase(om_module, tc_queue_static);
  tcase_add_test(tc_queue_static, _QUEUE);

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

  TCase* tc_com = tcase_create("通信收发器测试");
  suite_add_tcase(om_module, tc_com);
  tcase_add_test(tc_com, _COM);

  return om_module;
}
