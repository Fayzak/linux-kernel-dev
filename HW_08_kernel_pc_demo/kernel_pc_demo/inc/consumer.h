#ifndef CONSUMER_H
#define CONSUMER_H

#include <linux/workqueue_types.h>

void tasklet_consumer(unsigned long data);
void work_consumer(struct work_struct *work);

#endif
