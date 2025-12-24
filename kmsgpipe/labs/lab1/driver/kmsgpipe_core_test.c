#include <kunit/test.h>
#include <linux/slab.h>

#include "kmsgpipe_core.h"

#ifndef KBUILD_MODNAME
#define KBUILD_MODNAME "kmsgpipe"
#endif

/* Simple write → read test */
static void kmsgpipe_core_write_read_test(struct kunit *test)
{
    struct kmsgpipe_char_driver dev;
    char *buffer;
    char read_buf[16];
    const char write_buf[] = "hello";
    loff_t pos = 0;
    ssize_t ret;

    /* allocate fake device buffer */
    buffer = kmalloc(64, GFP_KERNEL);
    KUNIT_ASSERT_NOT_NULL(test, buffer);

    /* initialize core state */
    kmsgpipe_core_init(&dev, buffer, 64);

    /* write data */
    ret = kmsgpipe_write_core(&dev,
                              write_buf,
                              sizeof(write_buf) - 1,
                              &pos);
    KUNIT_ASSERT_EQ(test, ret, (ssize_t)(sizeof(write_buf) - 1));
    KUNIT_ASSERT_EQ(test, dev.data_len, (size_t)(sizeof(write_buf) - 1));

    /* reset position for read */
    pos = 0;
    memset(read_buf, 0, sizeof(read_buf));

    /* read data back */
    ret = kmsgpipe_read_core(&dev,
                             read_buf,
                             sizeof(read_buf),
                             &pos);
    KUNIT_ASSERT_EQ(test, ret, (ssize_t)(sizeof(write_buf) - 1));

    /* verify contents */
    KUNIT_EXPECT_MEMEQ(test,
                       read_buf,
                       write_buf,
                       sizeof(write_buf) - 1);

    kfree(buffer);
}

/* Register test cases */
static struct kunit_case kmsgpipe_core_test_cases[] = {
    KUNIT_CASE(kmsgpipe_core_write_read_test),
    {}};

static struct kunit_suite kmsgpipe_core_test_suite = {
    .name = "kmsgpipe-core",
    .test_cases = kmsgpipe_core_test_cases,
};

kunit_test_suite(kmsgpipe_core_test_suite);
