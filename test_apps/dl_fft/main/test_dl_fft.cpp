#include "dl_fft.h"
#include "dl_rfft.h"
#include "test_fft.h"

static const char *TAG = "TEST DL AUDIO";
static int LOOP = 10;

TEST_CASE("Test dl fft", "[dl_fft]")
{
    const float *input[5] = {fft_input_128, fft_input_256, fft_input_512, fft_input_1024, fft_input_2048};
    const float *output[5] = {fft_output_128, fft_output_256, fft_output_512, fft_output_1024, fft_output_2048};
    int test_nfft[5] = {128, 256, 512, 1024, 2048};
    float target_db = 90;
    int ram_size_before = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    uint32_t start = 0, end = 0;

    for (int i = 0; i < 5; i++) {
        int nfft = test_nfft[i];
        printf("test fft(%d) float: ", nfft);
        float *x = (float *)heap_caps_aligned_alloc(16, nfft * sizeof(float) * 2, MALLOC_CAP_8BIT);
        memcpy(x, input[i], nfft * 2 * sizeof(float));

        dl_fft_f32_t *fft_handle = dl_fft_f32_init(nfft, MALLOC_CAP_8BIT);
        dl_fft_f32_run(fft_handle, x);
        TEST_ASSERT_EQUAL(true, check_fft_results(x, output[i], nfft, target_db, 1e-3));

        start = esp_timer_get_time();
        for (int k = 0; k < LOOP; k++) {
            dl_fft_f32_run(fft_handle, x);
        }
        end = esp_timer_get_time();
        printf("time:%ld us\n", (end - start) / LOOP);
        dl_fft_f32_deinit(fft_handle);
        heap_caps_free(x);
    }

    int ram_size_end = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    ESP_LOGI(TAG, "ram size before: %d, end:%d", ram_size_before, ram_size_end);
    TEST_ASSERT_EQUAL(true, ram_size_before - ram_size_end < 300);
}

TEST_CASE("Test dl rfft", "[dl_fft]")
{
    const float *input[5] = {rfft_input_128, rfft_input_256, rfft_input_512, rfft_input_1024, rfft_input_2048};
    const float *output[5] = {rfft_output_128, rfft_output_256, rfft_output_512, rfft_output_1024, rfft_output_2048};
    int test_nfft[5] = {128, 256, 512, 1024, 2048};
    float target_db = 90;
    int ram_size_before = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    uint32_t start = 0, end = 0;

    for (int i = 0; i < 5; i++) {
        int nfft = test_nfft[i];
        printf("test rfft(%d) float: ", nfft);
        float *x = (float *)heap_caps_aligned_alloc(16, nfft * sizeof(float), MALLOC_CAP_8BIT);
        float *gt = (float *)malloc(sizeof(float) * nfft);
        memcpy(x, input[i], nfft * sizeof(float));
        memcpy(gt, output[i], nfft * sizeof(float));
        gt[1] = output[i][nfft];

        dl_fft_f32_t *fft_handle = dl_rfft_f32_init(nfft, MALLOC_CAP_8BIT);
        dl_rfft_f32_run(fft_handle, x);
        TEST_ASSERT_EQUAL(true, check_fft_results(x, gt, nfft, target_db, 1e-3));

        start = esp_timer_get_time();
        for (int k = 0; k < LOOP; k++) {
            dl_rfft_f32_run(fft_handle, x);
        }
        end = esp_timer_get_time();
        printf("time:%ld us\n", (end - start) / LOOP);
        dl_rfft_f32_deinit(fft_handle);
        heap_caps_free(x);
        free(gt);
    }

    int ram_size_end = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    ESP_LOGI(TAG, "ram size before: %d, end:%d", ram_size_before, ram_size_end);
    TEST_ASSERT_EQUAL(true, ram_size_before == ram_size_end);
}

TEST_CASE("Test dl fft s16", "[dl_fft]")
{
    const int16_t *input[5] = {
        fft_input_s16_128, fft_input_s16_256, fft_input_s16_512, fft_input_s16_1024, fft_input_s16_2048};
    const float *output[5] = {fft_output_128, fft_output_256, fft_output_512, fft_output_1024, fft_output_2048};
    int test_nfft[5] = {128, 256, 512, 1024, 2048};
    float target_db = 50;
    int ram_size_before = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    uint32_t start = 0, end = 0;
    int out_exponent;

    for (int i = 0; i < 5; i++) {
        int nfft = test_nfft[i];
        printf("test fft(%d) s16: ", nfft);
        int16_t *x = (int16_t *)heap_caps_aligned_alloc(16, nfft * sizeof(int16_t) * 2, MALLOC_CAP_8BIT);
        float *y = (float *)heap_caps_aligned_alloc(16, nfft * sizeof(float) * 2, MALLOC_CAP_8BIT);
        memcpy(x, input[i], nfft * 2 * sizeof(int16_t));

        dl_fft_s16_t *fft_handle = dl_fft_s16_init(nfft, MALLOC_CAP_8BIT);
        dl_fft_s16_run(fft_handle, x, -15, &out_exponent); // -15 means x is in Q15 format

        // check snr
        dl_short_to_float(x, nfft * 2, out_exponent, y);
        TEST_ASSERT_EQUAL(true, check_fft_results(y, output[i], nfft, target_db, 3e-2));

        start = esp_timer_get_time();
        for (int k = 0; k < LOOP; k++) {
            dl_fft_s16_run(fft_handle, x, -15, &out_exponent);
        }
        end = esp_timer_get_time();
        printf("time:%ld us\n", (end - start) / LOOP);
        dl_fft_s16_deinit(fft_handle);
        heap_caps_free(x);
        heap_caps_free(y);
    }

    int ram_size_end = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    ESP_LOGI(TAG, "ram size before: %d, end:%d", ram_size_before, ram_size_end);
    TEST_ASSERT_EQUAL(true, ram_size_before == ram_size_end);
}

TEST_CASE("Test dl fft hp s16", "[dl_fft]")
{
    const int16_t *input[5] = {
        fft_input_s16_128, fft_input_s16_256, fft_input_s16_512, fft_input_s16_1024, fft_input_s16_2048};
    const float *output[5] = {fft_output_128, fft_output_256, fft_output_512, fft_output_1024, fft_output_2048};
    int test_nfft[5] = {128, 256, 512, 1024, 2048};
    float target_db = 65; // high precision int16 fft
    int ram_size_before = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    uint32_t start = 0, end = 0;
    int out_exponent;

    for (int i = 0; i < 5; i++) {
        int nfft = test_nfft[i];
        printf("test fft(%d) s16: ", nfft);
        int16_t *x = (int16_t *)heap_caps_aligned_alloc(16, nfft * sizeof(int16_t) * 2, MALLOC_CAP_8BIT);
        float *y = (float *)heap_caps_aligned_alloc(16, nfft * sizeof(float) * 2, MALLOC_CAP_8BIT);
        memcpy(x, input[i], nfft * 2 * sizeof(int16_t));

        dl_fft_s16_t *fft_handle = dl_fft_s16_init(nfft, MALLOC_CAP_8BIT);
        dl_fft_s16_hp_run(fft_handle, x, -15, &out_exponent);

        // check snr
        dl_short_to_float(x, nfft * 2, out_exponent, y);
        TEST_ASSERT_EQUAL(true, check_fft_results(y, output[i], nfft, target_db, 5e-3));

        start = esp_timer_get_time();
        for (int k = 0; k < LOOP; k++) {
            dl_fft_s16_hp_run(fft_handle, x, -15, &out_exponent);
        }
        end = esp_timer_get_time();
        printf("time:%ld us\n", (end - start) / LOOP);
        dl_fft_s16_deinit(fft_handle);
        heap_caps_free(x);
        heap_caps_free(y);
    }

    int ram_size_end = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    ESP_LOGI(TAG, "ram size before: %d, end:%d", ram_size_before, ram_size_end);
    TEST_ASSERT_EQUAL(true, ram_size_before == ram_size_end);
}

TEST_CASE("Test dl rfft s16", "[dl_fft]")
{
    const int16_t *input[5] = {
        rfft_input_s16_128, rfft_input_s16_256, rfft_input_s16_512, rfft_input_s16_1024, rfft_input_s16_2048};
    const float *output[5] = {rfft_output_128, rfft_output_256, rfft_output_512, rfft_output_1024, rfft_output_2048};
    int test_nfft[5] = {128, 256, 512, 1024, 2048};
    float target_db = 45;
    int ram_size_before = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    uint32_t start = 0, end = 0;
    int out_exponent;

    for (int i = 0; i < 5; i++) {
        int nfft = test_nfft[i];
        printf("test rfft(%d) float: ", nfft);
        int16_t *x = (int16_t *)heap_caps_aligned_alloc(16, nfft * sizeof(int16_t), MALLOC_CAP_8BIT);
        float *y = (float *)malloc(nfft * sizeof(float));
        float *gt = (float *)malloc(sizeof(float) * nfft);
        memcpy(x, input[i], nfft * sizeof(int16_t));
        memcpy(gt, output[i], nfft * sizeof(float));
        gt[1] = output[i][nfft];

        dl_fft_s16_t *fft_handle = dl_rfft_s16_init(nfft, MALLOC_CAP_8BIT);
        dl_rfft_s16_run(fft_handle, x, -15, &out_exponent);
        dl_short_to_float(x, nfft, out_exponent, y);
        TEST_ASSERT_EQUAL(true, check_fft_results(y, gt, nfft, target_db, 5e-2));

        start = esp_timer_get_time();
        for (int k = 0; k < LOOP; k++) {
            dl_rfft_s16_run(fft_handle, x, -15, &out_exponent);
        }
        end = esp_timer_get_time();
        printf("time:%ld us\n", (end - start) / LOOP);
        dl_rfft_s16_deinit(fft_handle);
        free(x);
        free(y);
        free(gt);
    }

    int ram_size_end = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    ESP_LOGI(TAG, "ram size before: %d, end:%d", ram_size_before, ram_size_end);
    TEST_ASSERT_EQUAL(true, ram_size_before == ram_size_end);
}

TEST_CASE("Test dl rfft hp s16", "[dl_fft]")
{
    const int16_t *input[5] = {
        rfft_input_s16_128, rfft_input_s16_256, rfft_input_s16_512, rfft_input_s16_1024, rfft_input_s16_2048};
    const float *output[5] = {rfft_output_128, rfft_output_256, rfft_output_512, rfft_output_1024, rfft_output_2048};
    int test_nfft[5] = {128, 256, 512, 1024, 2048};
    float target_db = 55;
    int ram_size_before = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    uint32_t start = 0, end = 0;
    int out_exponent;

    for (int i = 0; i < 5; i++) {
        int nfft = test_nfft[i];
        printf("test rfft(%d) float: ", nfft);
        int16_t *x = (int16_t *)heap_caps_aligned_alloc(16, nfft * sizeof(int16_t), MALLOC_CAP_8BIT);
        float *y = (float *)malloc(nfft * sizeof(float));
        float *gt = (float *)malloc(sizeof(float) * nfft);
        memcpy(x, input[i], nfft * sizeof(int16_t));
        memcpy(gt, output[i], nfft * sizeof(float));
        gt[1] = output[i][nfft];

        dl_fft_s16_t *fft_handle = dl_rfft_s16_init(nfft, MALLOC_CAP_8BIT);
        dl_rfft_s16_hp_run(fft_handle, x, -15, &out_exponent);
        dl_short_to_float(x, nfft, out_exponent, y);
        TEST_ASSERT_EQUAL(true, check_fft_results(y, gt, nfft, target_db, 1e-2));

        start = esp_timer_get_time();
        for (int k = 0; k < LOOP; k++) {
            dl_rfft_s16_hp_run(fft_handle, x, -15, &out_exponent);
        }
        end = esp_timer_get_time();
        printf("time:%ld us\n", (end - start) / LOOP);
        dl_rfft_s16_deinit(fft_handle);
        free(x);
        free(y);
        free(gt);
    }

    int ram_size_end = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    ESP_LOGI(TAG, "ram size before: %d, end:%d", ram_size_before, ram_size_end);
    TEST_ASSERT_EQUAL(true, ram_size_before == ram_size_end);
}

// TEST_CASE("Test sr fft hp s16", "[dl_fft]")
// {
//     const int16_t *input[5] = {
//         fft_input_s16_128, fft_input_s16_256, fft_input_s16_512, fft_input_s16_1024, fft_input_s16_2048};
//     const float *output[5] = {fft_output_128, fft_output_256, fft_output_512, fft_output_1024, fft_output_2048};
//     int test_nfft[5] = {128, 256, 512, 1024, 2048};
//     float target_db = 65; // high precision int16 fft
//     int ram_size_before = heap_caps_get_free_size(MALLOC_CAP_8BIT);
//     uint32_t start = 0, end = 0;
//     int out_exponent;

//     for (int i = 0; i < 5; i++) {
//         int nfft = test_nfft[i];
//         printf("test fft(%d) s16: ", nfft);
//         int16_t *x = (int16_t *)heap_caps_aligned_alloc(16, nfft * sizeof(int16_t) * 2, MALLOC_CAP_8BIT);
//         float *y = (float *)heap_caps_aligned_alloc(16, nfft * sizeof(float) * 2, MALLOC_CAP_8BIT);
//         memcpy(x, input[i], nfft * 2 * sizeof(int16_t));

//         fft_s16_info_t *fft_handle = fft_info_init(nfft*2);
//         int ls = test_real_fft_hp2(x, fft_handle);

//         // check snr
//         dl_short_to_float(x, nfft, -15 + ls, y);
//         TEST_ASSERT_EQUAL(true, check_fft_results(y, output[i], nfft, target_db, 5e-3));

//         start = esp_timer_get_time();
//         for (int k = 0; k < LOOP; k++) {
//             test_real_fft_hp2(x, fft_handle);
//         }
//         end = esp_timer_get_time();
//         printf("time:%ld us\n", (end - start) / LOOP);
//         heap_caps_free(x);
//         heap_caps_free(y);
//     }

//     int ram_size_end = heap_caps_get_free_size(MALLOC_CAP_8BIT);
//     ESP_LOGI(TAG, "ram size before: %d, end:%d", ram_size_before, ram_size_end);
//     TEST_ASSERT_EQUAL(true, ram_size_before == ram_size_end);
// }

// TEST_CASE("Test sr rfft hp s16", "[dl_fft]")
// {
//     const int16_t *input[5] = {
//         rfft_input_s16_128, rfft_input_s16_256, rfft_input_s16_512, rfft_input_s16_1024, rfft_input_s16_2048};
//     const float *output[5] = {rfft_output_128, rfft_output_256, rfft_output_512, rfft_output_1024, rfft_output_2048};
//     int test_nfft[5] = {128, 256, 512, 1024, 2048};
//     float target_db = 55;
//     int ram_size_before = heap_caps_get_free_size(MALLOC_CAP_8BIT);
//     uint32_t start = 0, end = 0;

//     for (int i = 0; i < 5; i++) {
//         int nfft = test_nfft[i];
//         printf("test rfft(%d) float: ", nfft);
//         int16_t *x = (int16_t *)heap_caps_aligned_alloc(16, nfft * sizeof(int16_t), MALLOC_CAP_8BIT);
//         float *y = (float *)malloc(nfft * sizeof(float));
//         float *gt = (float *)malloc(sizeof(float) * nfft);
//         memcpy(x, input[i], nfft * sizeof(int16_t));
//         memcpy(gt, output[i], nfft * sizeof(float));
//         // gt[1] = output[i][nfft];

//         fft_s16_info_t *fft_handle = fft_info_init(nfft);
//         int ls = test_real_fft_hp(x, fft_handle);
//         dl_short_to_float(x, nfft, -15 + ls, y);
//         for (int j = 0; j < 10; j++) {
//             printf("%f %f\n", y[j], gt[j]);
//         }
//         TEST_ASSERT_EQUAL(true, check_fft_results(y, gt, nfft, target_db, 1e-2));

//         start = esp_timer_get_time();
//         for (int k = 0; k < LOOP; k++) {
//             test_real_fft(x, fft_handle);
//         }
//         end = esp_timer_get_time();
//         printf("time:%ld us\n", (end - start) / LOOP);
//         // fft_info_destroy(fft_handle);
//         // free(x);
//         free(y);
//         free(gt);
//     }

//     int ram_size_end = heap_caps_get_free_size(MALLOC_CAP_8BIT);
//     ESP_LOGI(TAG, "ram size before: %d, end:%d", ram_size_before, ram_size_end);
//     TEST_ASSERT_EQUAL(true, ram_size_before == ram_size_end);
// }
