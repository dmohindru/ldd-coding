# lab 4

This lab will demo periodic clean of circular buffer message using

- delayed_work jiq.c (Ch 7)

Module Init periodic cleanup

```c
INIT_DELAYED_WORK(&dev->cleanup_dwork, cleanup_worker);

schedule_delayed_work(&dev->cleanup_dwork,
                      msecs_to_jiffies(interval_ms));

```

Inside worker

```c
static void cleanup_worker(struct work_struct *work)
{
    struct kmsgpipe_dev *dev =
        container_of(work, struct kmsgpipe_dev,
                     cleanup_dwork.work);

    mutex_lock(&dev->lock);
    kmsgpipe_cleanup_expired(&dev->buffer, ktime_get());
    mutex_unlock(&dev->lock);

    schedule_delayed_work(&dev->cleanup_dwork,
                          msecs_to_jiffies(dev->interval_ms));
}

```

Module exit

```c
cancel_delayed_work_sync(&dev->cleanup_dwork);

```
