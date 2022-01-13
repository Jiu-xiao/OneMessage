#include "om.h"
#include "om_test.h"

static char str1[] = "This is str1";
static char str2[] = "This is str2";
static char str3[] = "This is str3";

static bool msg_new = false, filter = false, deploy = false, get = false;

static int call_counter = 0;

static om_status_t new_fun(om_msg_t *msg) {
  call_counter++;
  if (msg_new)
    return OM_OK;
  else
    return OM_ERROR;
}

static om_status_t get_fun(om_msg_t *msg) {
  get = true;
  msg->buff = str1;
  msg->size = sizeof(str1);
  return OM_OK;
}

static char str_tmp[20];

static om_status_t apply_fun(om_msg_t *msg) {
  deploy = true;
  strncpy(str_tmp, msg->buff, 20);
  return OM_OK;
}

static om_status_t filter_fun(om_msg_t *msg) {
  filter = true;
  if (!strncmp(str1, msg->buff, 20) || !strncmp(str2, msg->buff, 20))
    return OM_OK;
  else
    return OM_ERROR;
}

static om_suber_t *sub;
static om_puber_t *pub;
static om_topic_t *topic;
static om_topic_t *topic2;

float pub_freq = 12.5;

static om_config_t pub_config[] = {{OM_USER_FUN_NEW, new_fun},
                                   {OM_USER_FUN_GET, get_fun},
                                   {OM_PUB_FREQ, &pub_freq},
                                   {OM_CONFIG_END, NULL}};

static om_config_t sub_config[] = {{OM_USER_FUN_DEPLOY, apply_fun},
                                   {OM_CONFIG_END, NULL}};

static om_config_t topic_config1[] = {{OM_USER_FUN_FILTER, filter_fun},
                                      {OM_ADD_PUBER, &pub},
                                      {OM_ADD_SUBER, &sub},
                                      {OM_CONFIG_END, NULL}};

static om_config_t topic_config2[] = {
    {OM_LINK, &topic}, {OM_TOPIC_VIRTUAL, NULL}, {OM_CONFIG_END, NULL}};

START_TEST(publish) {
  om_init();
  om_status_t res = OM_OK;
  sub = om_create_suber(sub_config, NULL, 0);
  pub = om_create_puber(pub_config);
  ck_assert_msg(sub, "sub 指针为 NULL.");
  ck_assert_msg(sub, "pub 指针为 NULL.");
  topic = om_create_topic("topic", topic_config1);
  topic2 = om_create_topic("topic2", topic_config2);
  ck_assert_msg(topic, "topic 指针为 NULL.");
  ck_assert_msg(topic2, "topic2 指针为 NULL.");
  res += om_add_topic(topic);
  res += om_add_topic(topic2);
  ck_assert_msg(!res, "话题加入队列失败。");

  for (int i = 0; i < 10000; i++) om_sync();
  ck_assert_msg(call_counter == 125, "new_fun调用了%d次，应为125次",
                call_counter);

  ck_assert_msg(!get, "意外调用了apply函数。");
  ck_assert_msg(!filter, "意外调用了filter函数。");
  msg_new = true;

  for (int i = 0; i < 1000; i++) om_sync();
  ck_assert_msg(get, "未调用apply函数。");
  ck_assert_msg(filter, "未调用filter函数。");
  ck_assert_msg(!strncmp(str_tmp, str1, 20), "sync数据损坏。");
  msg_new = false;

  ck_assert_msg(om_find_topic("topic") == topic, "无法根据名称寻找话题。");

  om_publish(om_find_topic("topic2"), str2, sizeof(str2), true);
  ck_assert_msg(!strncmp(str_tmp, str2, 20), "publish数据损坏。");

  om_publish(om_find_topic("topic2"), str3, sizeof(str3), true);
  ck_assert_msg(strncmp(str_tmp, str3, 20), "filter函数未生效。");

  res = om_deinit();

  fail_if(res);
}
END_TEST

char str_log[] = {"Log test."};

START_TEST(om_log) {
  om_init();
  char buff[100];
  om_suber_t *sub = om_create_suber(NULL, buff, sizeof(buff));
  om_topic_t *topic_log = om_find_topic("om_log");

  ck_assert_msg(topic_log, "获取不到log话题。");
  om_topic_add_suber(topic_log, sub);
  om_print_log("init", OM_LOG_DEFAULT, "%s", str_log);
  ck_assert_msg(!strcmp(buff, "\033[0m[Default][init]Log test.\n"),
                "LOG数据错误:%s", buff);
  om_deinit();
}
END_TEST

Suite *make_om_suite(void) {
  Suite *om = suite_create("OneMessage单元测试");
  TCase *tc_publish = tcase_create("订阅发布测试");
  TCase *tc_log = tcase_create("日志测试");
  suite_add_tcase(om, tc_publish);
  suite_add_tcase(om, tc_log);
  tcase_add_test(tc_publish, publish);
  tcase_add_test(tc_log, om_log);
  return om;
}