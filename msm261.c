#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include "msm261.h"

#define MSM261_LOG_PREFIX "MSM261: "

static bool msm261_debug = false;
module_param(msm261_debug, bool, 0644);
MODULE_PARM_DESC(msm261_debug, "Enable debug output for MSM261");


/* Hardware initialization */
int msm261_hw_init(struct msm261_priv *msm261)
{
    int ret, i;
    unsigned long flags;

    if (msm261_debug)
        dev_info(msm261->dev, "MSM261: Hardware initialization starting\n");

    /* Request GPIOs with proper labels */
    ret = devm_gpio_request(msm261->dev, msm261->bck_gpio, "MSM261_BCK");
    if (ret < 0) {
        dev_err(msm261->dev, "Failed to request BCK GPIO: %d\n", ret);
        return ret;
    }

    ret = devm_gpio_request(msm261->dev, msm261->ws_gpio, "MSM261_WS");
    if (ret < 0) {
        dev_err(msm261->dev, "Failed to request WS GPIO: %d\n", ret);
        return ret;
    }

    /* Configure clock GPIOs */
    gpio_direction_output(msm261->bck_gpio, 0);
    gpio_direction_output(msm261->ws_gpio, 0);

    /* Configure data GPIOs */
    for (i = 0; i < NUM_DATA_LINES; i++) {
        char label[32];
        snprintf(label, sizeof(label), "MSM261_DATA%d", i);

        ret = devm_gpio_request(msm261->dev, msm261->data_gpio[i], label);
        if (ret < 0) {
            dev_err(msm261->dev, "Failed to request DATA%d GPIO: %d\n", i, ret);
            return ret;
        }
        gpio_direction_input(msm261->data_gpio[i]);
    }

    /* Configure for normal mode operation */
    local_irq_save(flags);

    /* Set initial clock state */
    gpio_set_value(msm261->bck_gpio, 0);
    gpio_set_value(msm261->ws_gpio, 0);

    /* Delay for power-up sequence (per datasheet) */
    udelay(100);

    local_irq_restore(flags);

    if (msm261_debug)
        dev_info(msm261->dev, "MSM261: Hardware initialization completed\n");
    return 0;
}

/* I2S configuration */
int msm261_set_i2s_config(struct msm261_priv *msm261, unsigned int bclk, unsigned int rate)
{
    unsigned long flags;

    if (msm261_debug)
        dev_info(msm261->dev, "MSM261: Configuring I2S: BCLK=%uHz, Rate=%uHz\n", bclk, rate);

    /* Validate clock frequency */
    if (bclk < MSM261_NORMAL_MODE_MIN_CLK || bclk > MSM261_NORMAL_MODE_MAX_CLK) {
        dev_err(msm261->dev, "Invalid BCLK frequency %uHz (valid: %u-%uHz)\n",
                bclk, MSM261_NORMAL_MODE_MIN_CLK, MSM261_NORMAL_MODE_MAX_CLK);
        return -EINVAL;
    }

    local_irq_save(flags);

    /* Configure I2S timing */
    gpio_set_value(msm261->bck_gpio, 0);
    gpio_set_value(msm261->ws_gpio, 0);
    udelay(1);  /* Setup time */

    local_irq_restore(flags);

    if (msm261_debug)
        dev_info(msm261->dev, "MSM261: I2S configuration completed\n");
    return 0;
}

static const struct snd_kcontrol_new msm261_controls[] = {
    SOC_SINGLE("Mic Array Gain", 0, 0, 100, 0),
};

/* DAPM widgets */
static const struct snd_soc_dapm_widget msm261_dapm_widgets[] = {
    SND_SOC_DAPM_MIC("MIC1", NULL),
    SND_SOC_DAPM_MIC("MIC2", NULL),
    SND_SOC_DAPM_MIC("MIC3", NULL),
    SND_SOC_DAPM_MIC("MIC4", NULL),
    SND_SOC_DAPM_MIC("MIC5", NULL),
    SND_SOC_DAPM_MIC("MIC6", NULL),
    SND_SOC_DAPM_MIC("MIC7", NULL),
};

/* DAPM routes */
static const struct snd_soc_dapm_route msm261_dapm_routes[] = {
    {"Capture", NULL, "MIC1"},
    {"Capture", NULL, "MIC2"},
    {"Capture", NULL, "MIC3"},
    {"Capture", NULL, "MIC4"},
    {"Capture", NULL, "MIC5"},
    {"Capture", NULL, "MIC6"},
    {"Capture", NULL, "MIC7"},
};

static int msm261_component_probe(struct snd_soc_component *component)
{
    struct msm261_priv *msm261;
    int ret;

    dev_info(component->dev, "MSM261: Platform probe starting with msm261_debug=%d\n", msm261_debug);

    if (msm261_debug)
        dev_info(component->dev, MSM261_LOG_PREFIX "Component probe starting\n");

    msm261 = devm_kzalloc(component->dev, sizeof(*msm261), GFP_KERNEL);
    if (!msm261)
        return -ENOMEM;

    msm261->component = component;
    snd_soc_component_set_drvdata(component, msm261);

    /* Add controls */
    ret = snd_soc_add_component_controls(component, msm261_controls,
                                       ARRAY_SIZE(msm261_controls));
    if (ret < 0) {
        dev_err(component->dev, MSM261_LOG_PREFIX "Failed to add controls\n");
        return ret;
    }

    if (msm261_debug)
        dev_info(component->dev, MSM261_LOG_PREFIX "Component probe completed\n");

    return 0;
}

static const struct snd_soc_component_driver soc_component_dev_msm261 = {
    .probe = msm261_component_probe,
    .dapm_widgets = msm261_dapm_widgets,
    .num_dapm_widgets = ARRAY_SIZE(msm261_dapm_widgets),
    .dapm_routes = msm261_dapm_routes,
    .num_dapm_routes = ARRAY_SIZE(msm261_dapm_routes),
    .idle_bias_on = 1,
    .use_pmdown_time = 1,
    .endianness = 1,
    .legacy_dai_naming = 0,
};

static struct snd_pcm_hardware msm261_pcm_hw = {
    .info = (SNDRV_PCM_INFO_MMAP |
             SNDRV_PCM_INFO_INTERLEAVED |
             SNDRV_PCM_INFO_BLOCK_TRANSFER),
    .formats = SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S32_LE,
    .rates = SNDRV_PCM_RATE_8000_48000,
    .rate_min = 8000,
    .rate_max = 48000,
    .channels_min = 1,
    .channels_max = NUM_MICS,
    .buffer_bytes_max = 32768,
    .period_bytes_min = 4096,
    .period_bytes_max = 8192,
    .periods_min = 2,
    .periods_max = 8,
};

/* PCM ops */
static int msm261_pcm_open(struct snd_pcm_substream *substream)
{
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct snd_soc_component *component;
    struct msm261_priv *msm261;
    int ret;

    component = snd_soc_rtd_to_codec(rtd, 0)->component;
    msm261 = snd_soc_component_get_drvdata(component);

    substream->runtime->hw = msm261_pcm_hw;
    msm261->streaming = false;

    ret = snd_pcm_hw_constraint_integer(substream->runtime,
                                      SNDRV_PCM_HW_PARAM_PERIODS);
    if (ret < 0)
        return ret;

    dev_info(msm261->dev, "MSM261: PCM opened\n");
    return msm261_hw_init(msm261);
}

static int msm261_pcm_close(struct snd_pcm_substream *substream)
{
    struct snd_soc_pcm_runtime *rtd = substream->private_data;
    struct snd_soc_component *component;
    struct msm261_priv *msm261;

    component = snd_soc_rtd_to_codec(rtd, 0)->component;
    msm261 = snd_soc_component_get_drvdata(component);

    msm261->streaming = false;
    dev_info(msm261->dev, "MSM261: PCM closed\n");
    return 0;
}

/* DAI ops */
static int msm261_dai_hw_params(struct snd_pcm_substream *substream,
                               struct snd_pcm_hw_params *params,
                               struct snd_soc_dai *dai)
{
    struct msm261_priv *msm261 = snd_soc_dai_get_drvdata(dai);
    unsigned int rate = params_rate(params);
    unsigned int channels = params_channels(params);
    unsigned int bclk = rate * channels * 32; // 32-bit per channel

    return msm261_set_i2s_config(msm261, bclk, rate);
}

static int msm261_dai_trigger(struct snd_pcm_substream *substream,
                             int cmd, struct snd_soc_dai *dai)
{
    struct snd_soc_component *component = dai->component;
    struct msm261_priv *msm261 = snd_soc_component_get_drvdata(component);
    int ret = 0;

    switch (cmd) {
    case SNDRV_PCM_TRIGGER_START:
    case SNDRV_PCM_TRIGGER_RESUME:
        msm261->streaming = true;
        dev_info(msm261->dev, "MSM261: Streaming started\n");
        break;
    case SNDRV_PCM_TRIGGER_STOP:
    case SNDRV_PCM_TRIGGER_SUSPEND:
        msm261->streaming = false;
        dev_info(msm261->dev, "MSM261: Streaming stopped\n");
        break;
    default:
        ret = -EINVAL;
    }

    return ret;
}

static const struct snd_soc_dai_ops msm261_dai_ops = {
    .hw_params = msm261_dai_hw_params,
    .trigger = msm261_dai_trigger,
};

static struct snd_soc_dai_driver msm261_dai = {
    .name = "msm261-pcm",
    .capture = {
        .stream_name = "Capture",
        .channels_min = 1,
        .channels_max = NUM_MICS,
        .rates = SNDRV_PCM_RATE_8000_48000,
        .formats = SNDRV_PCM_FMTBIT_S16_LE |
                  SNDRV_PCM_FMTBIT_S24_LE |
                  SNDRV_PCM_FMTBIT_S32_LE,
    },
    .ops = &msm261_dai_ops,
};

static const struct snd_pcm_ops msm261_pcm_ops = {
    .open = msm261_pcm_open,
    .close = msm261_pcm_close,
};

static int msm261_platform_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct device_node *np = dev->of_node;
    struct msm261_priv *msm261;
    const char *compatible;
    int ret, i;

    dev_info(dev, "MSM261: Platform probe starting\n");

    if (!np) {
        dev_err(dev, "MSM261: No device tree node found\n");
        return -EINVAL;
    }

    msm261 = devm_kzalloc(dev, sizeof(*msm261), GFP_KERNEL);
    if (!msm261) {
        dev_err(dev, "MSM261: Failed to allocate private data\n");
        return -ENOMEM;
    }

    compatible = of_get_property(np, "compatible", NULL);
    if (compatible)
        dev_info(dev, "MSM261: Compatible: %s\n", compatible);

    // Get GPIOs from device tree
    msm261->bck_gpio = of_get_named_gpio(np, "bck-gpios", 0);
    msm261->ws_gpio = of_get_named_gpio(np, "ws-gpios", 0);

    // Print device tree information
    dev_info(dev, "MSM261: Node name: %s\n", np->full_name);

    for (i = 0; i < NUM_DATA_LINES; i++) {
        msm261->data_gpio[i] = of_get_named_gpio(np, "data-gpios", i);
        if (!gpio_is_valid(msm261->data_gpio[i])) {
            dev_err(dev, "Invalid data GPIO %d\n", i);
            return -EINVAL;
        }
    }

    // Store private data
    msm261->dev = dev;
    platform_set_drvdata(pdev, msm261);

    // Register component and DAI
    ret = devm_snd_soc_register_component(dev, &soc_component_dev_msm261,
                                        &msm261_dai, 1);
    if (ret < 0) {
        dev_err(dev, "MSM261: Failed to register component: %d\n", ret);
        return ret;
    }

    dev_info(dev, "MSM261: BCK GPIO: %d\n", msm261->bck_gpio);
    dev_info(dev, "MSM261: WS GPIO: %d\n", msm261->ws_gpio);
    for (i = 0; i < NUM_DATA_LINES; i++) {
        dev_info(dev, "MSM261: DATA%d GPIO: %d\n", i, msm261->data_gpio[i]);
    }

    dev_info(dev, "MSM261: Platform probe completed\n");

    return 0;
}

static const struct of_device_id msm261_of_match[] = {
    { .compatible = "msm,msm261s4030h0" },
    { }
};
MODULE_DEVICE_TABLE(of, msm261_of_match);

static struct platform_driver msm261_platform_driver = {
    .probe = msm261_platform_probe,
    .driver = {
        .name = DRIVER_NAME,
        .owner = THIS_MODULE,
        .of_match_table = msm261_of_match,
    },
};

static int __init msm261_init(void)
{
    pr_info("MSM261: Initializing driver\n");
    return platform_driver_register(&msm261_platform_driver);
}

static void __exit msm261_exit(void)
{
    pr_info("MSM261: Cleaning up driver\n");
    platform_driver_unregister(&msm261_platform_driver);
}

module_init(msm261_init);
module_exit(msm261_exit);

MODULE_DESCRIPTION("MSM261S4030H0 7-mic array ALSA SoC driver");
MODULE_AUTHOR("Your Name");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRIVER_VERSION);
