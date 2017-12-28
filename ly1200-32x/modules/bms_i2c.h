/*
 **********************************************************************
 *
 * @filename  bms_i2c.c
 *
 * @purpose   Driver to instantiate BMS i2c devices
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
#ifndef _BMS_I2C_H_
#define _BMS_I2C_H_

const char *bms_i2c_adapter_names[] = {
    "SMBus I801 adapter",
    "SMBus iSMT adapter",
};

enum i2c_adapter_type {
    I2C_ADAPTER_I801 = 0,
    I2C_ADAPTER_ISMT,
};

enum bms_module_switch_bus {
    I2C_STAGE1_MUX_CHAN0 = 0,
    I2C_STAGE1_MUX_CHAN1,
    I2C_STAGE1_MUX_CHAN2,
    I2C_STAGE1_MUX_CHAN3,
    I2C_STAGE1_MUX_CHAN4,
    I2C_STAGE1_MUX_CHAN5,
    I2C_STAGE1_MUX_CHAN6,
    I2C_STAGE1_MUX_CHAN7,
};

#endif /* _BMS_I2C_H_ */

