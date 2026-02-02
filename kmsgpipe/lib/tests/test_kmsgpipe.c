#include "kmsgpipe.h"
#include "unity.h"
#include <string.h>

#define TEST_CAPACITY 4
#define TEST_DATA_SIZE 32

static uint8_t first_data[] = "first data";
static uint8_t second_data[] = "second";
static uint8_t third_data[] = "3rd data";
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
static ktime_t second_ts = 10010;
static ktime_t third_ts = 10020;
static ktime_t forth_ts = 10030;

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

    for (ssize_t i = 0; i < TEST_CAPACITY; i++)
    {
        TEST_ASSERT_FALSE_MESSAGE(record_buf[i].valid, "Failed on record_buf false status");
    }
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
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, buf.head, "Failed on head field after all insertions");
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
    kmsgpipe_push(&buf, first_data, strlen((char *)first_data), first_uid, first_gid, first_ts);
    kmsgpipe_push(&buf, second_data, strlen((char *)second_data), second_uid, second_gid, second_ts);
    kmsgpipe_push(&buf, third_data, strlen((char *)third_data), third_uid, third_gid, third_ts);
    kmsgpipe_push(&buf, forth_data, strlen((char *)forth_data), forth_uid, forth_gid, forth_ts);

    ssize_t ret_val = kmsgpipe_push(&buf, first_data, strlen((char *)first_data), first_uid, first_gid, first_ts);

    TEST_ASSERT_EQUAL_INT_MESSAGE(-ENOSPC, ret_val, "Failed on return value when buffer is full");
}

void should_return_message_size_error_when_data_item_larger_than_data_size()
{
    uint8_t long_data[] = "Quick brown fox jumped over a lazy doy";
    ssize_t ret_val = kmsgpipe_push(&buf, long_data, strlen((char *)long_data), first_uid, first_gid, first_ts);

    TEST_ASSERT_EQUAL_INT_MESSAGE(-EMSGSIZE, ret_val, "Failed on return value when len of data is larger than DATA_SIZE");
}

void should_pop_items_in_order_they_were_pushed(void)
{
    kmsgpipe_push(&buf, first_data, strlen((char *)first_data), first_uid, first_gid, first_ts);
    kmsgpipe_push(&buf, second_data, strlen((char *)second_data), second_uid, second_gid, second_ts);
    kmsgpipe_push(&buf, third_data, strlen((char *)third_data), third_uid, third_gid, third_ts);
    kmsgpipe_push(&buf, forth_data, strlen((char *)forth_data), forth_uid, forth_gid, forth_ts);

    uint8_t out_data_buf1[TEST_DATA_SIZE];
    uint8_t out_data_buf2[TEST_DATA_SIZE];
    uint8_t out_data_buf3[TEST_DATA_SIZE];
    uint8_t out_data_buf4[TEST_DATA_SIZE];

    ssize_t first_ret_val = kmsgpipe_pop(&buf, out_data_buf1, first_uid, first_gid);
    ssize_t second_ret_val = kmsgpipe_pop(&buf, out_data_buf2, second_uid, second_gid);
    ssize_t third_ret_val = kmsgpipe_pop(&buf, out_data_buf3, third_uid, third_gid);
    ssize_t forth_ret_val = kmsgpipe_pop(&buf, out_data_buf4, forth_uid, forth_gid);

    TEST_ASSERT_EQUAL_INT_MESSAGE(strlen((char *)first_data), first_ret_val, "Failed on poping first data item");
    TEST_ASSERT_EQUAL_INT_MESSAGE(strlen((char *)second_data), second_ret_val, "Failed on poping second data item");
    TEST_ASSERT_EQUAL_INT_MESSAGE(strlen((char *)third_data), third_ret_val, "Failed on poping third data item");
    TEST_ASSERT_EQUAL_INT_MESSAGE(strlen((char *)forth_data), forth_ret_val, "Failed on poping forth data item");

    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(first_data, out_data_buf1, strlen((char *)first_data), "Failed on poping and comparing out buffer first data");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(second_data, out_data_buf2, strlen((char *)second_data), "Failed on poping and comparing out buffer second data");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(third_data, out_data_buf3, strlen((char *)third_data), "Failed on poping and comparing out buffer third data");
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(forth_data, out_data_buf4, strlen((char *)forth_data), "Failed on poping and comparing out buffer forth data");

    // assert position of head/tail
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, buf.head, "Failed on head field after all insertions");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, buf.tail, "Failed on tail field after insertions");
}

void should_return_no_data_error_when_poping_from_empty_buffer(void)
{
    uint8_t out_buf[TEST_DATA_SIZE];
    ssize_t ret_val = kmsgpipe_pop(&buf, out_buf, 0, 0);
    TEST_ASSERT_EQUAL_INT_MESSAGE(-ENODATA, ret_val, "Failed on return value when buffer is empty");
}

void should_return_unauthorized_access_error_when_poping_not_allowed_item(void)
{
    kmsgpipe_push(&buf, first_data, strlen((char *)first_data), first_uid, first_gid, first_ts);

    uint8_t out_buf[TEST_DATA_SIZE];
    ssize_t ret_val = kmsgpipe_pop(&buf, out_buf, second_uid, second_gid);
    TEST_ASSERT_EQUAL_INT_MESSAGE(-EACCES, ret_val, "Failed on unauthorized check for a pop operation");
}

void should_authorized_and_return_data_when_poping_process_is_root(void)
{
    kmsgpipe_push(&buf, first_data, strlen((char *)first_data), first_uid, first_gid, first_ts);

    uint8_t out_buf[TEST_DATA_SIZE];
    uid_t root_user_uid = 0;
    ssize_t ret_val = kmsgpipe_pop(&buf, out_buf, root_user_uid, second_gid);
    TEST_ASSERT_EQUAL_INT_MESSAGE(strlen((char *)first_data), ret_val, "Failed on authorized check for a pop operation when uid is of root process");
}

void should_authorized_and_return_data_when_poping_process_uid_different_gid_same(void)
{
    kmsgpipe_push(&buf, first_data, strlen((char *)first_data), first_uid, first_gid, first_ts);

    uint8_t out_buf[TEST_DATA_SIZE];
    ssize_t ret_val = kmsgpipe_pop(&buf, out_buf, second_uid, first_gid);
    TEST_ASSERT_EQUAL_INT_MESSAGE(strlen((char *)first_data), ret_val, "Failed on authorized check for a pop operation when uid is different but gid is same");
}

void should_wrap_around_head_pointer_and_add_data_item_to_buffer(void)
{
    kmsgpipe_push(&buf, first_data, strlen((char *)first_data), first_uid, first_gid, first_ts);
    kmsgpipe_push(&buf, second_data, strlen((char *)second_data), second_uid, second_gid, second_ts);
    kmsgpipe_push(&buf, third_data, strlen((char *)third_data), third_uid, third_gid, third_ts);
    kmsgpipe_push(&buf, forth_data, strlen((char *)forth_data), forth_uid, forth_gid, forth_ts);

    uint8_t out_data_buf[TEST_DATA_SIZE];
    kmsgpipe_pop(&buf, out_data_buf, first_uid, first_gid);
    kmsgpipe_pop(&buf, out_data_buf, second_uid, second_gid);
    kmsgpipe_pop(&buf, out_data_buf, third_uid, third_gid);

    ssize_t ret_val = kmsgpipe_push(&buf, first_data, strlen((char *)first_data), first_uid, first_gid, first_ts);
    TEST_ASSERT_EQUAL_INT_MESSAGE(strlen((char *)first_data), ret_val, "Failed on wrapped around head pointer push operation");

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, buf.head, "Failed on head pointer value on wrapped around push operation check");
    TEST_ASSERT_EQUAL_INT_MESSAGE(3, buf.tail, "Failed on tail pointer value on wrapped around push operation check");
}

void should_remove_expired_data_items_from_buffer(void)
{
    kmsgpipe_push(&buf, first_data, strlen((char *)first_data), first_uid, first_gid, first_ts);
    kmsgpipe_push(&buf, second_data, strlen((char *)second_data), second_uid, second_gid, second_ts);
    kmsgpipe_push(&buf, third_data, strlen((char *)third_data), third_uid, third_gid, third_ts);
    kmsgpipe_push(&buf, forth_data, strlen((char *)forth_data), forth_uid, forth_gid, forth_ts);

    ssize_t delete_count = kmsgpipe_cleanup_expired(&buf, second_ts + 5);
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, delete_count, "Failed on deleted message count on removing expired messages from buf");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, buf.head, "Failed on head field when removing expired messages");
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, buf.tail, "Failed on tail field when removing expired messages");
}

void should_not_remove_expired_data_from_empty_buffer(void)
{
    ssize_t delete_count = kmsgpipe_cleanup_expired(&buf, second_ts + 5);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, delete_count, "Failed on deleted message count on removing expired messages from buf");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, buf.head, "Failed on head field when removing expired messages");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, buf.tail, "Failed on tail field when removing expired messages");
}

void should_get_correct_data_item_count_from_buffer(void)
{
    kmsgpipe_push(&buf, first_data, strlen((char *)first_data), first_uid, first_gid, first_ts);
    kmsgpipe_push(&buf, second_data, strlen((char *)second_data), second_uid, second_gid, second_ts);
    kmsgpipe_push(&buf, third_data, strlen((char *)third_data), third_uid, third_gid, third_ts);
    kmsgpipe_push(&buf, forth_data, strlen((char *)forth_data), forth_uid, forth_gid, forth_ts);

    ssize_t count = kmsgpipe_get_message_count(&buf);
    TEST_ASSERT_EQUAL_INT_MESSAGE(4, count, "Failed on message count from buffer");
}

void should_get_correct_data_item_count_from_buffer_when_head_is_wrapped_around(void)
{
    kmsgpipe_push(&buf, first_data, strlen((char *)first_data), first_uid, first_gid, first_ts);
    kmsgpipe_push(&buf, second_data, strlen((char *)second_data), second_uid, second_gid, second_ts);
    kmsgpipe_push(&buf, third_data, strlen((char *)third_data), third_uid, third_gid, third_ts);
    kmsgpipe_push(&buf, forth_data, strlen((char *)forth_data), forth_uid, forth_gid, forth_ts);
    uint8_t out_buf[TEST_DATA_SIZE];

    kmsgpipe_pop(&buf, out_buf, first_uid, first_gid);
    kmsgpipe_pop(&buf, out_buf, second_uid, second_gid);
    kmsgpipe_pop(&buf, out_buf, third_uid, third_gid);

    kmsgpipe_push(&buf, first_data, strlen((char *)first_data), first_uid, first_gid, first_ts);

    ssize_t count = kmsgpipe_get_message_count(&buf);
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, count, "Failed on message count from buffer when head is wrapped around");
}

void should_clear_all_messages_from_buffer(void)
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
    RUN_TEST(should_return_unauthorized_access_error_when_poping_not_allowed_item);
    RUN_TEST(should_authorized_and_return_data_when_poping_process_is_root);
    RUN_TEST(should_authorized_and_return_data_when_poping_process_uid_different_gid_same);
    RUN_TEST(should_wrap_around_head_pointer_and_add_data_item_to_buffer);
    RUN_TEST(should_remove_expired_data_items_from_buffer);
    RUN_TEST(should_not_remove_expired_data_from_empty_buffer);
    RUN_TEST(should_get_correct_data_item_count_from_buffer);
    RUN_TEST(should_get_correct_data_item_count_from_buffer_when_head_is_wrapped_around);
    RUN_TEST(should_clear_all_messages_from_buffer);

    return UNITY_END();
}