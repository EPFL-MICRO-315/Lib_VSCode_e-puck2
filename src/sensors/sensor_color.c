#include "sensor_color.h"
#include <camera/po8030.h>
#include <main.h>
#include <spi_comm.h>
#include <ch.h>
#include <usbcfg.h>


static semaphore_t color_sem;
static BSEMAPHORE_DECL(image_ready_sem, TRUE);
uint8_t r = 0, g = 0, b = 0;

/*
 * Returns the line's width extracted from the image buffer given
 * Returns 0 if line not found
 */
uint8_t averageBuffer (uint8_t *buffer);

uint8_t sensor_measure_color(void) {
    uint8_t c;
    chSemWait(&color_sem);
    if((r > b) && (r > g))
        c = RED;
    else if((b > r) && (b > g))
        c = BLUE;
    else
        c = NONE;
    chSemSignal(&color_sem);
    return c;
}

uint8_t averageBuffer(uint8_t *buffer) {
    uint32_t mean = 0;
    for (uint16_t i = 0; i < IMAGE_BUFFER_SIZE / 5; i++)
        if (buffer[i])
            mean += buffer[i];
    mean /= (IMAGE_BUFFER_SIZE / 5);
    return mean;
}

static THD_WORKING_AREA(waCaptureImage, 256);
static THD_FUNCTION(CaptureImage, arg) {
    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    // Takes pixels 0 to IMAGE_BUFFER_SIZE of the line 10 + 11 (minimum 2 lines because reasons)
	po8030_advanced_config(FORMAT_RGB565, 0, 10, IMAGE_BUFFER_SIZE, 2, SUBSAMPLING_X1, SUBSAMPLING_X1);
    //desactivate white balance to keep values non adaptative to the image
    po8030_set_awb(0);
    dcmi_enable_double_buffering();
    dcmi_set_capture_mode(CAPTURE_ONE_SHOT);
    dcmi_prepare();

    while(1) {
        // starts a capture
        dcmi_capture_start();
        // waits for the capture to be done
        wait_image_ready();
        // signals an image has been captured
        chBSemSignal(&image_ready_sem);
    }
}

static THD_WORKING_AREA(waProcessImage, 1024);
static THD_FUNCTION(ProcessImage, arg) {
    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    uint8_t *img_buff_ptr;
    uint8_t image_red[IMAGE_BUFFER_SIZE / 5] = {0};
    uint8_t image_blue[IMAGE_BUFFER_SIZE / 5] = {0};
    uint8_t image_green[IMAGE_BUFFER_SIZE / 5] = {0};
    for(;;) {
        // waits until an image has been captured
        chBSemWait(&image_ready_sem);
        // gets the pointer to the array filled with the last image in RGB565
        img_buff_ptr = dcmi_get_last_image_ptr();
        for (uint8_t i; i < IMAGE_BUFFER_SIZE / 5; i += 1) {
            image_red[i] = 0;
            image_blue[i] = 0;
            image_green[i] = 0;
        }

        uint16_t j = 0;
        // Extracts only the center pixels
        for (uint16_t i = (IMAGE_BUFFER_SIZE * 0.8); i < (IMAGE_BUFFER_SIZE * 1.2); i += 2) {
            // extracts first 5bits of the first byte
            // extracts lasts 5 bits of the second byte
            image_red[j / 2] = (uint16_t)img_buff_ptr[i] & 0xF8;
            image_green[j / 2] = (int)(img_buff_ptr[i] & 0x07 << 5) | (img_buff_ptr[i + 1] & 0xE0) >> 3;
            image_blue[j / 2] = (uint16_t)(img_buff_ptr[i + 1] & 0x1F) << 3;
            j += 2;
        }
        chSemWait(&color_sem);
        r = averageBuffer(image_red);
        b = averageBuffer(image_blue);
        g = averageBuffer(image_green);
        chSemSignal(&color_sem);
        chThdSleepMilliseconds(100);
    }
}

void sensor_color_init(void) {
    chSemObjectInit(&color_sem, 1);
    dcmi_start();
    po8030_start();
    chThdCreateStatic(waProcessImage, sizeof(waProcessImage), NORMALPRIO, ProcessImage, NULL);
    chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO, CaptureImage, NULL);
}