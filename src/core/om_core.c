#include "om_core.h"

LIST_HEAD(topic_list);

om_topic_t *om_core_topic_create(const char *name) {
  om_topic_t *topic = om_malloc(sizeof(om_topic_t));
  memset(topic, 0, sizeof(*topic));
  strncpy(topic->name, name, OM_TOPIC_MAX_NAME_LEN);
  INIT_LIST_HEAD(&topic->puber);
  INIT_LIST_HEAD(&topic->suber);
  INIT_LIST_HEAD(&topic->link);
  return topic;
}

om_status_t om_core_add_topic(om_topic_t *topic) {
  OM_ASSENT(topic);

  om_list_add_tail(&topic->self, &topic_list);
  return OM_OK;
}

om_suber_t *om_core_suber_create(om_topic_t *link) {
  om_suber_t *suber = om_malloc(sizeof(*suber));
  memset(suber, 0, sizeof(*suber));
  if (link) {
    suber->isLink = true;
    suber->target = link;
  }
  return suber;
}

om_status_t om_core_add_suber(om_topic_t *topic, om_suber_t *sub) {
  OM_ASSENT(topic);
  OM_ASSENT(sub);

  om_list_add_tail(&sub->self, &(topic->suber));
  return OM_OK;
}

om_puber_t *om_core_puber_create(float freq) {
  om_puber_t *puber = om_malloc(sizeof(*puber));
  memset(puber, 0, sizeof(*puber));
  puber->freq.reload = OM_CALL_FREQ / freq;
  puber->freq.counter = puber->freq.reload;

  return puber;
}

om_status_t om_core_add_puber(om_topic_t *topic, om_puber_t *pub) {
  OM_ASSENT(topic);
  OM_ASSENT(pub);

  om_list_add_tail(&pub->self, &(topic->puber));
  return OM_OK;
}

om_link_t *om_core_link_create(om_suber_t *sub) {
  om_link_t *link = om_malloc(sizeof(*link));
  link->source = sub;

  return link;
}

om_status_t om_core_add_link(om_topic_t *topic, om_link_t *link) {
  OM_ASSENT(topic);
  OM_ASSENT(link);

  om_list_add_tail(&link->self, &(topic->link));
  return OM_OK;
}

om_status_t om_core_link(om_topic_t *source, om_topic_t *target) {
  OM_ASSENT(source);
  OM_ASSENT(target);

  om_suber_t *sub = om_core_suber_create(target);
  om_core_add_suber(source, sub);
  om_link_t *link = om_core_link_create(sub);
  om_core_add_link(target, link);

  return OM_OK;
}

om_status_t om_core_delink(om_list_head_t *head) {
  OM_ASSENT(head);
  om_link_t *link = om_list_entry(head, om_link_t, self);

  om_list_del(&link->self);
  om_list_del(&link->source->self);

  om_free(link->source);
  om_free(link);

  return OM_OK;
}

om_status_t om_core_del_suber(om_list_head_t *head) {
  OM_ASSENT(head);
  om_suber_t *sub = om_list_entry(head, om_suber_t, self);

  om_list_del(&sub->self);
  if (sub->isLink) {
    OM_ASSENT(sub->target);
    om_list_head_t *pos;
    om_list_for_each(pos, &sub->target->link) {
      om_link_t *link = om_list_entry(pos, om_link_t, self);
      if (link->source == sub) {
        om_list_del(&link->self);
        om_free(link);
        break;
      }
    }
  }

  om_free(sub);

  return OM_OK;
}

om_status_t om_core_del_puber(om_list_head_t *head) {
  OM_ASSENT(head);
  om_puber_t *pub = om_list_entry(head, om_puber_t, self);

  om_list_del(&pub->self);
  free(pub);

  return OM_OK;
}

om_status_t om_core_del_topic(om_list_head_t *head) {
  OM_ASSENT(head);
  om_topic_t *topic = om_list_entry(head, om_topic_t, self);
  OM_CHECK((void *)topic != (void *)&topic_list);

  om_list_del(&topic->self);
  om_list_head_t *pos;
  om_del_all(pos, &topic->link, om_core_delink);
  om_del_all(pos, &topic->puber, om_core_del_puber);
  om_del_all(pos, &topic->suber, om_core_del_suber);
  if (!topic->virtual && topic->msg.buff) om_free(topic->msg.buff);
  om_free(topic);

  return OM_OK;
}

om_topic_t *om_core_find_topic(const char *name) {
  om_list_head_t *pos;
  om_list_for_each(pos, &topic_list) {
    om_topic_t *topic = om_list_entry(pos, om_topic_t, self);
    if (!strncmp(name, topic->name, OM_TOPIC_MAX_NAME_LEN)) return topic;
  }
  return NULL;
}

om_status_t om_core_set_dump_target(om_suber_t *suber, void *target,
                                    size_t max_size) {
  OM_ASSENT(target);
  OM_ASSENT(suber);

  suber->dump_target.max_size = max_size;
  suber->dump_target.address = target;
  suber->dump_target.enable = true;

  return OM_OK;
}

void om_error(const char *file, uint32_t line) {
  (void)(file);
  (void)(line);
  while (1) {
  };
}
