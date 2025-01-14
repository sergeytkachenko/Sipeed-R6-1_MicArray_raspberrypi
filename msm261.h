#ifndef MSM261_H
#define MSM261_H

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/initval.h>
#include <sound/tlv.h>
#include <linux/gpio.h>
#include <linux/regmap.h>

#define DRIVER_NAME     "msm261"
#define DRIVER_VERSION  "1.0"
#define NUM_MICS        7
#define NUM_DATA_LINES  4

/* Clock frequency definitions */
#define MSM261_NORMAL_MODE_MIN_CLK   1000000  /* 1.0 MHz */
#define MSM261_NORMAL_MODE_MAX_CLK   4000000  /* 4.0 MHz */

struct msm261_priv {
    struct device *dev;
    struct snd_soc_component *component;
    struct regmap *regmap;
    int bck_gpio;
    int ws_gpio;
    int data_gpio[NUM_DATA_LINES];
    bool streaming;
};

int msm261_hw_init(struct msm261_priv *msm261);
int msm261_set_i2s_config(struct msm261_priv *msm261, unsigned int bclk, unsigned int rate);

#endif /* MSM261_H */