/*
 **********************************************************************
 *
 * @filename  bms_cb_i2c.c
 *
 * @purpose   Driver to instantiate BMS cpu board i2c devices
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
/* cpu board i2c topology
 *
 *                        +--------------+
 *                        |   DDR4  #1   |
 *                      +-|  SPD EEPROM  |
 *                      | |   (0:0x50)   |
 *                      | +--------------+
 *                      | +--------------+
 *                      | |   DDR4  #2   |
 *                      +-|  SPD EEPROM  |
 *                      | |   (0:0x52)   |
 *  +----------------+  | +--------------+
 *  |                |  | +--------------+
 *  |                |  | |Thermal Sensor|
 *  |                |  +-|    TMP75     |
 *  |       +------+ |  | |   (0:0x4E)   |
 *  |       | SMB0 |-|--+ +--------------+
 *  |       +------+ |    +--------------+
 *  |                |    |  ID_EEPROM   |
 *  |                |  +-|    M24C02    |
 *  |                |  | |   (1:0x56)   |
 *  |                |  | +--------------+
 *  |       +------+ |  | +--------------+
 *  |       | SMB1 |-|--+-| System CPLD  |
 *  |       +------+ |  | |    LCMXO2    |
 *  +----------------+  | |   (1:0x31)   |
 *                      | +--------------+
 *                      |
 *                      +------(switch board connecter)
 *
 */

#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/delay.h>
#include "bms_i2c.h"

#define BMS_CB_I2C_CLIENT_NUM 5
#define BMS_CB_ADAPTER_BASE 0

static struct i2c_client *bms_cb_clients[BMS_CB_I2C_CLIENT_NUM] = {NULL};
static int bms_cb_client_index = 0;

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


static __init struct i2c_client *bms_cb_setup_spd(
        struct i2c_adapter *adap, int addr)
{
    struct i2c_board_info info_spd = {
        I2C_BOARD_INFO("spd", addr),
    };

    return i2c_new_device(adap, &info_spd);
}


static __init struct i2c_client *bms_cb_setup_eeprom_24c02(
        struct i2c_adapter *adap, int addr)
{
    struct i2c_board_info info_spd = {
        I2C_BOARD_INFO("24c02", addr),
    };

    return i2c_new_device(adap, &info_spd);
}

static __init struct i2c_client *bms_cb_setup_tmp75(
        struct i2c_adapter *adap, int addr)
{
    struct i2c_board_info info_spd = {
        I2C_BOARD_INFO("tmp75", addr),
    };

    return i2c_new_device(adap, &info_spd);
}

static __init struct i2c_client *bms_cb_setup_system_cpld(struct i2c_adapter *adap)
{
    struct i2c_board_info info = {
        I2C_BOARD_INFO("system_cpld", 0x31),
    };

    return i2c_new_device(adap, &info);
}

static int __init bms_cb_setup_devices_i801(void)
{
    struct i2c_adapter *adap;
    int adap_num = find_i2c_adapter_num(I2C_ADAPTER_I801);

    if (adap_num < 0)
        return adap_num;

    adap = i2c_get_adapter(adap_num);
    if (!adap) {
        pr_err("%s failed to get i2c adap %d.\n", __func__, adap_num);
        goto exit;
    }

    bms_cb_clients[bms_cb_client_index++] = bms_cb_setup_spd(adap, 0x50);
    bms_cb_clients[bms_cb_client_index++] = bms_cb_setup_spd(adap, 0x52);
    bms_cb_clients[bms_cb_client_index++] = bms_cb_setup_tmp75(adap, 0x4e);

exit:
    return 0;
}

static int __init bms_cb_setup_devices_ismt(void)
{
    struct i2c_adapter *adap;
    int adap_num = find_i2c_adapter_num(I2C_ADAPTER_ISMT);

    if (adap_num < 0)
        return adap_num;

    adap = i2c_get_adapter(adap_num);
    if (!adap) {
        pr_err("%s failed to get i2c adap %d.\n", __func__, adap_num);
        return 0;
    }

    bms_cb_clients[bms_cb_client_index++] = bms_cb_setup_system_cpld(adap);
    bms_cb_clients[bms_cb_client_index++] = bms_cb_setup_eeprom_24c02(adap, 0x56);
    return 0;
}

static int __init bms_cb_i2c_init(void)
{
    /* Initial bms_cb_slients array. */
    memset(bms_cb_clients, 0x0, BMS_CB_I2C_CLIENT_NUM);

    bms_cb_setup_devices_i801();
    mdelay(200);
    bms_cb_setup_devices_ismt();

    return 0;
}


static void __exit bms_cb_i2c_exit(void){
    int i;

    for (i=(bms_cb_client_index-1); i>=0; i--) {
        if (bms_cb_clients[i]) {
            i2c_unregister_device(bms_cb_clients[i]);
            bms_cb_clients[i] = NULL;
        }
    }

    bms_cb_client_index = 0;

}


module_init(bms_cb_i2c_init);
module_exit(bms_cb_i2c_exit);


MODULE_DESCRIPTION("BMS Cpu Board I2c driver");
MODULE_AUTHOR("eddy weng <eddy.weng@mic.com.tw>");
MODULE_LICENSE("GPL");

