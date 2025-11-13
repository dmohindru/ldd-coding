#include "kmsgpipe.h"
#include "unity.h"

#define TEST_CAPACITY 4
#define TEST_DATA_SIZE 32

static kmsgpipe_buffer_t buf;
static uint8_t base_buffer[TEST_CAPACITY * TEST_DATA_SIZE];
static kmsg_record_t record_buf[TEST_CAPACITY];

void setUp(void)
{
    int ret_val = kmsgpipe_init(&buf, base_buffer, record_buf, TEST_CAPACITY, TEST_DATA_SIZE);
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, ret_val, "Init should return 0");
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

    kmsgpipe_push(&buf, first_data, sizeof(first_data), first_uid, first_gid, 1000);

    TEST_ASSERT_EQUAL_INT_MESSAGE(1, 0, "I will be failing");
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(should_add_add_to_buffer_and_update_metadata_records);

    return UNITY_END();
}