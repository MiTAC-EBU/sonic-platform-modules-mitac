/*
 **********************************************************************
 *
 * @filename  bms_fb_i2c.c
 *
 * @purpose   Driver to instantiate BMS fan board i2c devices
 *
 * @create    2017/1/5
 *
 * @author    eddy weng <eddy.weng@mic.com.tw>
 *
 * @history   2017/1/5: init version
 *
 **********************************************************************
 */
/*
 * $Copyright: Copyright 2017 MiTAC Co., Ltd.
 * This program is the proprietary software of MiTAC Corporation
 * and/or its licensors, and may only be used, duplicated, modified
 * or distributed pursuant to the terms and conditions of a separate,
 * written license agreement executed between you and MiTAC
 * (an "Authorized License").  Except as set forth in an Authorized
 * License, MiTAC grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and
 * MiTAC expressly reserves all rights in and to the Software
 * and all intellectual property rights therein.  IF YOU HAVE
 * NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
 * IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY MITAC AND DISCONTINUE
 * ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     THIS PROGRAM, INCLUDING ITS STRUCTURE, SEQUENCE AND ORGANIZATION,
 * CONSTITUTES THE VALUABLE TRADE SECRETS OF MITAC, AND YOU SHALL USE
 * ALL REASONABLE EFFORTS TO PROTECT THE CONFIDENTIALITY THEREOF,
 * AND TO USE THIS INFORMATION ONLY IN CONNECTION WITH YOUR USE OF
 * MITAC PRODUCTS.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
 * PROVIDED "AS IS" AND WITH ALL FAULTS AND MITAC MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
 * OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  MITAC SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * MITAC OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
 * INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
 * ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
 * TO USE THE SOFTWARE EVEN IF MITAC HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
 * THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
 * ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * 4.     REDISTRIBUTIONS IN BINARY FORM MUST REPRODUCE BELOW COPYRIGHT NOTICS
 */
/* fan board i2c topology
 *                             +------------+
 *                             |fan control |
 *                           +-|  MAX31790  |
 *                           | |  (3:0x20)  |
 *                           | +------------+
 *                           |
 *                           | +------------+
 *                           | |fan control |
 * (switch board connecter)--+-|  MAX31790  |
 *   (I2C Switch chan1)      | |  (3:0x23)  |
 *                           | +------------+
 *                           |
 *                           +--------------(FAN Modules * 6)
 *
 *                             +------------+
 *                             |Ther. Sensor|
 * (switch board connecter)----|   TMP75    |
 *    (I2C Switch chan3)       |  (5:0x4D)  |
 *                             +------------+
 */


#include <linux/i2c.h>
#include <linux/module.h>
#include "bms_i2c.h"

#define BMS_FB_I2C_CLIENT_NUM 3


static struct i2c_client *bms_fb_clients[BMS_FB_I2C_CLIENT_NUM] = {NULL};
static int bms_fb_client_index = 0;

static int __init __find_i2c_adap(struct device *dev, void *data)
{
    const char *name = data;
    static const char *prefix = "i2c-";
    struct i2c_adapter *adapter;

    if (strncmp(dev_name(dev), prefix, strlen(prefix)) != 0)
    {
        return 0;
    }
    adapter = to_i2c_adapter(dev);

    return (strncmp(adapter->name, name, strlen(name)) == 0);
}

static int __init find_i2c_adapter_num(enum i2c_adapter_type type)
{
    struct device *dev = NULL;
    struct i2c_adapter *adapter;
    const char *name = bms_i2c_adapter_names[type];

    /* find the adapter by name */
    dev = bus_find_device(&i2c_bus_type, NULL, (void *)name,
                  __find_i2c_adap);
    if (!dev) {
        pr_err("%s: i2c adapter %s not found on system.\n",
               __func__, name);
        return -ENODEV;
    }
    adapter = to_i2c_adapter(dev);

    return adapter->nr;
}

static int __init find_i2c_mux_adapter_num(int parent_num, int num)
{
    struct device *dev = NULL;
    struct i2c_adapter *adapter;
    char name[48];

    snprintf(name, sizeof(name), "i2c-%d-mux (chan_id %d)",
         parent_num, num);
    /* find the adapter by name */
    dev = bus_find_device(&i2c_bus_type, NULL, (void *)name,
                  __find_i2c_adap);
    if (!dev) {
        pr_err("%s: i2c adapter %s not found on system.\n",
               __func__, name);
        return -ENODEV;
    }
    adapter = to_i2c_adapter(dev);

    return adapter->nr;
}

static __init struct i2c_client *bms_fb_setup_tmp75(
        struct i2c_adapter *adap, int addr)
{
    struct i2c_board_info info_spd = {
        I2C_BOARD_INFO("tmp75", addr),
    };

    return i2c_new_device(adap, &info_spd);
}

static __init struct i2c_client *bms_fb_setup_max31790(
        struct i2c_adapter *adap, int addr)
{
    struct i2c_board_info info_spd = {
        I2C_BOARD_INFO("max31790", addr),
    };

    return i2c_new_device(adap, &info_spd);
}

static int __init bms_fb_setup_devices(void)
{
    struct i2c_adapter *adap;
    int adap_num;
    int parent_num;

    parent_num = find_i2c_adapter_num(I2C_ADAPTER_ISMT);
    if (parent_num < 0)
        return parent_num;

    /* Mux chan1 steup */
    adap_num = find_i2c_mux_adapter_num(parent_num, I2C_STAGE1_MUX_CHAN1);
    if (adap_num >= 0){
        adap = i2c_get_adapter(adap_num);
        if(adap) {
            bms_fb_clients[bms_fb_client_index++] = bms_fb_setup_max31790(adap, 0x20);
            bms_fb_clients[bms_fb_client_index++] = bms_fb_setup_max31790(adap, 0x23);
        }else{
            pr_err("%s failed to get i2c adap %d.\n", __func__, adap_num);
        }
    }else{
        pr_err("%s failed to find i2c mux adap number %d.\n", __func__, I2C_STAGE1_MUX_CHAN1);
    }

    adap_num = find_i2c_mux_adapter_num(parent_num, I2C_STAGE1_MUX_CHAN2);
    if (adap_num >= 0){
        adap = i2c_get_adapter(adap_num);
        if(adap) {
            bms_fb_clients[bms_fb_client_index++] = bms_fb_setup_tmp75(adap, 0x4d);
        }else{
            pr_err("%s failed to get i2c adap %d.\n", __func__, adap_num);
        }
    }else{
        pr_err("%s failed to find i2c mux adap number %d.\n", __func__, I2C_STAGE1_MUX_CHAN2);
    }

    return 0;
}

static int __init bms_fb_i2c_init(void)
{
    bms_fb_setup_devices();

    return 0;
}

static void __exit bms_fb_i2c_exit(void){
    int i;

    for (i=(bms_fb_client_index-1); i>=0; i--) {
        if (bms_fb_clients[i]) {
            i2c_unregister_device(bms_fb_clients[i]);
            bms_fb_clients[i] = NULL;
        }
    }

    bms_fb_client_index = 0;

}

module_init(bms_fb_i2c_init);
module_exit(bms_fb_i2c_exit);


MODULE_DESCRIPTION("BMS Fan Board I2c driver");
MODULE_AUTHOR("eddy weng <eddy.weng@mic.com.tw>");
MODULE_LICENSE("GPL");

