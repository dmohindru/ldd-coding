#include "kmsgpipe.h"
#include "unity.h"
#include <string.h>

#define TEST_CAPACITY 4
#define TEST_DATA_SIZE 32

static uint8_t first_data[] = "first data";
static uint8_t second_data[] = "second";
static uint8_t third_data[] = "third data";
static uint8_t forth_data[] = "forth";

static uid_t first_uid = 1000;
static uid_t second_uid = 1001;
static uid_t third_uid = 1002;
static uid_t forth_uid = 1003;

static gid_t first_gid = 2000;
static gid_t second_gid = 2001;
static gid_t third_gid = 2002;
static gid_t forth_gid = 2003;

static ktime_t first_ts = 10000;
static ktime_t second_ts = 10001;
static ktime_t third_ts = 10002;
static ktime_t forth_ts = 10003;

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

void should_add_to_buffer_and_update_metadata_records(void)
{

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

void should_return_no_space_error_when_buffer_is_full(void)
{
}

void should_return_message_size_error_when_data_item_larger_than_data_size() {}

void should_pop_items_in_order_they_were_pushed(void)
{
}

void should_return_no_data_error_when_poping_from_empty_buffer(void)
{
}

void should_return_unauthorized_access_error_when_poping_no_allowed_item(void)
{
}

void should_remove_expired_data_items_from_buffer(void)
{
}

void should_get_correct_data_item_count_from_buffer(void)
{
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(should_add_to_buffer_and_update_metadata_records);
    RUN_TEST(should_return_no_space_error_when_buffer_is_full);
    RUN_TEST(should_return_message_size_error_when_data_item_larger_than_data_size);
    RUN_TEST(should_pop_items_in_order_they_were_pushed);
    RUN_TEST(should_return_no_data_error_when_poping_from_empty_buffer);
    RUN_TEST(should_return_unauthorized_access_error_when_poping_no_allowed_item);
    RUN_TEST(should_remove_expired_data_items_from_buffer);
    RUN_TEST(should_get_correct_data_item_count_from_buffer);

    return UNITY_END();
}