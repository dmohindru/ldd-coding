#include "kmsgpipe.h"
#include "unity.h"
#include <string.h>

#define TEST_CAPACITY 4
#define TEST_DATA_SIZE 32

static kmsgpipe_buffer_t buf;
static uint8_t base_buffer[TEST_CAPACITY * TEST_DATA_SIZE];
static kmsg_record_t record_buf[TEST_CAPACITY];

void setUp(void)
{
    int ret_val = kmsgpipe_init(&buf, base_buffer, record_buf, TEST_CAPACITY, TEST_DATA_SIZE);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, ret_val, "Init should return 0");
    // assert all the buffer fields
    TEST_ASSERT_EQUAL_UINT64_MESSAGE(base_buffer, buf.base, "Failed on base buffer memory address");
    TEST_ASSERT_EQUAL_UINT64_MESSAGE(record_buf, buf.records, "Failed on record buffer memory address");
    TEST_ASSERT_EQUAL_INT_MESSAGE(TEST_DATA_SIZE, buf.data_size, "Failed on data_size field");
    TEST_ASSERT_EQUAL_INT_MESSAGE(TEST_CAPACITY, buf.capacity, "Failed on capacity field");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, buf.head, "Failed on head field");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, buf.tail, "Failed on tail field");
}

void tearDown(void)
{
}

void should_add_add_to_buffer_and_update_metadata_records(void)
{
    uint8_t first_data[] = "first data";
    uint8_t second_data[] = "second";
    uint8_t third_data[] = "third data";
    uint8_t forth_data[] = "forth";

    uid_t first_uid = 1000;
    uid_t second_uid = 1001;
    uid_t third_uid = 1002;
    uid_t forth_uid = 1003;

    gid_t first_gid = 2000;
    gid_t second_gid = 2001;
    gid_t third_gid = 2002;
    gid_t forth_gid = 2003;

    ktime_t first_ts = 10000;
    ktime_t second_ts = 10001;
    ktime_t third_ts = 10002;
    ktime_t forth_ts = 10003;

    ssize_t first_ret_val = kmsgpipe_push(&buf, first_data, strlen((char *)first_data), first_uid, first_gid, first_ts);
    ssize_t second_ret_val = kmsgpipe_push(&buf, second_data, strlen((char *)second_data), second_uid, second_gid, second_ts);
    ssize_t third_ret_val = kmsgpipe_push(&buf, third_data, strlen((char *)third_data), third_uid, third_gid, third_ts);
    ssize_t forth_ret_val = kmsgpipe_push(&buf, forth_data, strlen((char *)forth_data), forth_uid, forth_gid, forth_ts);

        // Assert return values
    TEST_ASSERT_EQUAL_INT_MESSAGE(strlen((char *)first_data), first_ret_val, "Failed on first data items push");
    TEST_ASSERT_EQUAL_INT_MESSAGE(strlen((char *)second_data), second_ret_val, "Failed on second data items push");
    TEST_ASSERT_EQUAL_INT_MESSAGE(strlen((char *)third_data), third_ret_val, "Failed on third data items push");
    TEST_ASSERT_EQUAL_INT_MESSAGE(strlen((char *)forth_data), forth_ret_val, "Failed on forth data items push");

    // Assert head and tail pointer
    TEST_ASSERT_EQUAL_INT_MESSAGE(3, buf.head, "Failed on head field after insertions");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, buf.tail, "Failed on tail field after insertions");

    // Assert metadata array
    kmsg_record_t first_meta_data = record_buf[0];
    kmsg_record_t forth_meta_data = record_buf[3];

    TEST_ASSERT_EQUAL_INT_MESSAGE(strlen((char *)first_data), first_meta_data.len, "Failed on len field for first metadata");
    TEST_ASSERT_EQUAL_INT_MESSAGE(first_uid, first_meta_data.owner_uid, "Failed on owner_uid field for first metadata");
    TEST_ASSERT_EQUAL_INT_MESSAGE(first_gid, first_meta_data.owner_gid, "Failed on owner_gid field for first metadata");
    TEST_ASSERT_EQUAL_INT_MESSAGE(first_ts, first_meta_data.timestamp, "Failed on timestamp field for first metadata");
    TEST_ASSERT_TRUE_MESSAGE(first_meta_data.valid, "Failed on bool field for first metadata");

    TEST_ASSERT_EQUAL_INT_MESSAGE(strlen((char *)forth_data), forth_meta_data.len, "Failed on len field for forth metadata");
    TEST_ASSERT_EQUAL_INT_MESSAGE(forth_uid, forth_meta_data.owner_uid, "Failed on owner_uid field for forth metadata");
    TEST_ASSERT_EQUAL_INT_MESSAGE(forth_gid, forth_meta_data.owner_gid, "Failed on owner_gid field for forth metadata");
    TEST_ASSERT_EQUAL_INT_MESSAGE(forth_ts, forth_meta_data.timestamp, "Failed on timestamp field for forth metadata");
    TEST_ASSERT_TRUE_MESSAGE(forth_meta_data.valid, "Failed on bool field for forth metadata");

    // Assert actual data in main buffer
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(first_data, buf.base, strlen((char *)first_data), "Failed on first data item in data buffer");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(second_data, buf.base + TEST_DATA_SIZE, strlen((char *)second_data), "Failed on second data item in data buffer");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(third_data, buf.base + (TEST_DATA_SIZE * 2), strlen((char *)third_data), "Failed on third data item in data buffer");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(forth_data, buf.base + (TEST_DATA_SIZE * 3), strlen((char *)forth_data), "Failed on forth data item in data buffer");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(should_add_add_to_buffer_and_update_metadata_records);

    return UNITY_END();
}