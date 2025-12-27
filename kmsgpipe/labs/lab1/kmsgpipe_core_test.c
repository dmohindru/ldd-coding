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

static void kmsgpipe_core_read_empty_test(struct kunit *test)
{
    struct kmsgpipe_char_driver dev;
    char *buffer;
    char read_buf[8];
    loff_t pos = 0;
    ssize_t ret;

    buffer = kmalloc(32, GFP_KERNEL);
    KUNIT_ASSERT_NOT_NULL(test, buffer);

    kmsgpipe_core_init(&dev, buffer, 32);

    ret = kmsgpipe_read_core(&dev, read_buf, sizeof(read_buf), &pos);

    KUNIT_EXPECT_EQ(test, ret, (ssize_t)0);
    KUNIT_EXPECT_EQ(test, pos, (loff_t)0);

    kfree(buffer);
}

static void kmsgpipe_core_write_overflow_test(struct kunit *test)
{
    struct kmsgpipe_char_driver dev;
    char *buffer;
    char big_buf[64];
    loff_t pos = 0;
    ssize_t ret;

    memset(big_buf, 'A', sizeof(big_buf));

    buffer = kmalloc(16, GFP_KERNEL);
    KUNIT_ASSERT_NOT_NULL(test, buffer);

    kmsgpipe_core_init(&dev, buffer, 16);

    ret = kmsgpipe_write_core(&dev, big_buf, sizeof(big_buf), &pos);

    KUNIT_EXPECT_EQ(test, ret, (ssize_t)-EFAULT);
    KUNIT_EXPECT_EQ(test, dev.data_len, (size_t)0);

    kfree(buffer);
}

static void kmsgpipe_core_partial_read_test(struct kunit *test)
{
    struct kmsgpipe_char_driver dev;
    char *buffer;
    char read_buf[4];
    const char write_buf[] = "abcdef";
    loff_t pos = 0;
    ssize_t ret;

    buffer = kmalloc(32, GFP_KERNEL);
    KUNIT_ASSERT_NOT_NULL(test, buffer);

    kmsgpipe_core_init(&dev, buffer, 32);

    kmsgpipe_write_core(&dev, write_buf, sizeof(write_buf) - 1, &pos);

    pos = 0;
    ret = kmsgpipe_read_core(&dev, read_buf, sizeof(read_buf), &pos);

    KUNIT_EXPECT_EQ(test, ret, (ssize_t)4);
    KUNIT_EXPECT_EQ(test, pos, (loff_t)4);
    KUNIT_EXPECT_MEMEQ(test, read_buf, "abcd", 4);

    kfree(buffer);
}

static void kmsgpipe_core_zero_write_test(struct kunit *test)
{
    struct kmsgpipe_char_driver dev;
    char *buffer;
    loff_t pos = 0;
    ssize_t ret;

    buffer = kmalloc(16, GFP_KERNEL);
    KUNIT_ASSERT_NOT_NULL(test, buffer);

    kmsgpipe_core_init(&dev, buffer, 16);

    ret = kmsgpipe_write_core(&dev, "", 0, &pos);

    KUNIT_EXPECT_EQ(test, ret, (ssize_t)0);
    KUNIT_EXPECT_EQ(test, dev.data_len, (size_t)0);

    kfree(buffer);
}

static void kmsgpipe_core_read_past_end_test(struct kunit *test)
{
    struct kmsgpipe_char_driver dev;
    char *buffer;
    char read_buf[16];
    const char write_buf[] = "hi";
    loff_t pos = 0;
    ssize_t ret;

    buffer = kmalloc(16, GFP_KERNEL);
    KUNIT_ASSERT_NOT_NULL(test, buffer);

    kmsgpipe_core_init(&dev, buffer, 16);
    kmsgpipe_write_core(&dev, write_buf, sizeof(write_buf) - 1, &pos);

    pos = 10; /* past data_len */
    ret = kmsgpipe_read_core(&dev, read_buf, sizeof(read_buf), &pos);

    KUNIT_EXPECT_EQ(test, ret, (ssize_t)0);

    kfree(buffer);
}

/* Register test cases */
static struct kunit_case kmsgpipe_core_test_cases[] = {
    KUNIT_CASE(kmsgpipe_core_write_read_test),
    KUNIT_CASE(kmsgpipe_core_read_empty_test),
    KUNIT_CASE(kmsgpipe_core_write_overflow_test),
    KUNIT_CASE(kmsgpipe_core_partial_read_test),
    KUNIT_CASE(kmsgpipe_core_zero_write_test),
    KUNIT_CASE(kmsgpipe_core_read_past_end_test),
    {}};

static struct kunit_suite kmsgpipe_core_test_suite = {
    .name = "kmsgpipe-core",
    .test_cases = kmsgpipe_core_test_cases,
};

kunit_test_suite(kmsgpipe_core_test_suite);
