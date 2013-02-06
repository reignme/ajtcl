/**
 * @file accelerometer.c
 */
/******************************************************************************
 * Copyright 2013, Qualcomm Innovation Center, Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 ******************************************************************************/

#include <stdio.h>

#include "alljoyn.h"
#include "aj_debug.h"

#include "efm32_cmu.h"
#include "efm32_gpio.h"
#include "efm32_i2c.h"

/*
 * We ultimately want to talk to the 3-axis accelerometer and magnetometer
 * chip on the RTX4100 Wireless Sensor Application Board (WSAB).  This chip is
 * an LSM303DLHC "ultra compact high performance e-compass 3D accelerometer and
 * 3D magnetometer module" fom STMicroelectronics (www.st.com).
 *
 * The LSM303DLHC is connected to the Energy Micro Tiny Gecko microcontroller
 * on the WSAB over the Inter Integrated Circuit (IIC or I2C) serial bus.
 * Energy Micro provides a low-library library for the Tiny Gecko family of
 * processors that we use.  Since we only really want to initialize the
 * serial bus and use it to read and write LSM303DLHC registers, we wrap the
 * low level I2C library from Energy Micro in a few helpers to take care of
 * the initialization and provide the ability to read and write registers.
 *
 * THe LSM303DLHC data sheet from STMicroelectronics provides the register
 * descriptions of the module.
 *
 * From the LOSM303DLHC datasheet, we note that the default slave address for
 * the accelerometer is 0x32 and the slave address of the magnetometer is 0x3c.
 * The least significant bit is the read/write bit with write=0 and read=1.
 */
#define ACCELEROMETER_I2C_SLAVE_ADDRESS  0x32
#define MAGNETOMETER_I2C_SLAVE_ADDRESS   0x3c

#define BIT(x) (1 << x)

/*
 * All we have to do figure out what to do to run the accelerometer/magnetometer
 * is to look in teh LSM303DLHC datasheet in the Register Description section
 * to figure out the register addresses and bit descriptions; and have a peek
 * at one of the ST Application Notes for guidance about what bits need to be
 * set for a minumum case.
 */
#define CTRL_REG1_A                 0x20

#define CTRL_REG1_A_XEN             BIT(0)    /* X-axis enable */
#define CTRL_REG1_A_YEN             BIT(1)    /* Y-axis enable */
#define CTRL_REG1_A_ZEN             BIT(2)    /* Z-axis enable */
#define CTRL_REG1_A_LPEN            BIT(3)    /* Low power mode enable */
#define CTRL_REG1_A_ODR_POWER_DOWN  0         /* Ouput data rate power down */
#define CTRL_REG1_A_ODR_1_HZ        0x10      /* Ouput data rate 1 Hertz */
#define CTRL_REG1_A_ODR_10_HZ       0x20      /* Ouput data rate 10 Hertz */
#define CTRL_REG1_A_ODR_25_HZ       0x30      /* Ouput data rate 25 Hertz */
#define CTRL_REG1_A_ODR_50_HZ       0x40      /* Ouput data rate 50 Hertz */
#define CTRL_REG1_A_ODR_100_HZ      0x50      /* Ouput data rate 100 Hertz */
#define CTRL_REG1_A_ODR_200_HZ      0x60      /* Ouput data rate 200 Hertz */
#define CTRL_REG1_A_ODR_400_HZ      0x70      /* Ouput data rate 400 Hertz */
#define CTRL_REG1_A_ODR_1620_HZ     0x80      /* Ouput data rate 1620 Hertz */
#define CTRL_REG1_A_ODR_5376_HZ     0x90      /* Ouput data rate 5376 Hertz */

#define CTRL_REG4_A                 0x23

#define CTRL_REG4_A_SIM             BIT(0)    /* Serial interface mode */

#define CTRL_REG4_A_HR              BIT(3)    /* High Resolution mode */
#define CTRL_REG4_A_FS_2G           0x00      /* Full Scale is +- 2G */
#define CTRL_REG4_A_FS_4G           0x10      /* Full Scale is +- 4G */
#define CTRL_REG4_A_FS_8G           0x20      /* Full Scale is +- 8G */
#define CTRL_REG4_A_FS_16G          0x30      /* Full Scale is +- 16G */
#define CTRL_REG4_A_BLE             BIT(6)    /* Big Little Endian big mode */
#define CTRL_REG4_A_BDU             BIT(7)    /* Block Data Update (see dox) */

#define STATUS_REG_A 0x27

#define STATUS_REG_XDA               BIT(0)   /* New X-axis data available */
#define STATUS_REG_YDA               BIT(1)   /* New Y-axis data available */
#define STATUS_REG_ZDA               BIT(2)   /* New Z-axis data available */
#define STATUS_REG_ZYXDA             BIT(3)   /* New X-, Y- or Z-axis data available */
#define STATUS_REG_XOR               BIT(4)   /* New X-axis data overwrote previous */
#define STATUS_REG_YOR               BIT(5)   /* New Y-axis data overwrote previous */
#define STATUS_REG_ZOR               BIT(6)   /* New Z-axis data overwrote previous */
#define STATUS_REG_ZYXOR 128         BIT(7)   /* New X-, Y- or Z-axis data overwrote */

#define OUT_X_L_A                    0x28     /* Accelerometer LSB of Ax */
#define OUT_X_H_A                    0x29     /* Accelerometer MSB of Ax */
#define OUT_Y_L_A                    0x2a     /* Accelerometer LSB of Ay */
#define OUT_Y_H_A                    0x2b     /* Accelerometer MSB of Ay */
#define OUT_Z_L_A                    0x2c     /* Accelerometer LSB of Az */
#define OUT_Z_H_A                    0x2d     /* Accelerometer MSB of Az */

/*
 * We don't want to do "nuthin' fancy" here so all we really need to do is to
 * be able to set control registers and read status registers on the LSM303DLHC
 * using the I2C driver privitives from the EFM32 library.  Since this is all
 * we need, we provide an initializatino helper to set up the pins on the Gecko
 * microcontroller and call into the I2C Driver to initialize it.  We also
 * cook up ReadRegister and WriteRegister helper functions to let us write and
 * read the registers on the accelerometer/magnetometer.  Since this is just a
 * simple demo, we won't worry about error reporting at all.
 */
const I2C_Init_TypeDef I2C_INIT = I2C_INIT_DEFAULT;

void InitializeI2C(void)
{
    /*
     * If you are familiar with embeded systems and the way ARM processors
     * work, these comments will be blatantly obvious.  If you are not so
     * lucky, hopefully this will give you a handle to figure out what all
     * of the seemingly impenetrable gobbledygook is.
     *
     * In order to conserve power, most subsystems in the processor are
     * not fed a clock signal by default.  So before we can use the I2C serial
     * port, we have to ask the Clock Management Unit to start giving it clock
     * signals.
     */
    CMU_ClockEnable(cmuClock_I2C0, true);

    /*
     * GPIO is arranged in a bank of six GPIO registers.  These registers are
     * called GPIO A through GPIO F.  Only 56 of the 96 outputs are wired to
     * pins on the Gecko EFM32G230 Microcontroller.  The naming convention is
     * that bit 0 of GPIO Port A is called PA0.
     *
     * If you look at the pinout of the microcontroller, you will find that
     * PA0 is pin 1 of the chip.  If you then go to the RTX documentation for
     * the RTX4100 WSAB, you will find that they use the naming convention
     * from the microcontroller pins.  For example, pin 2 of the SMT pads
     * connector on the RTX4100 is called PA0, is a type I/O, connects to
     * processor pin 1 and has the function(s) PA0/TIM0_CC0/I2C0_SDA.  Pin 3 of
     * the SMT pads conenctor is called PA1, is a type I/O, connects to
     * processor pin 2 and has the function(s) PA1/TIM0_CC1/I2C0_SCL/CMU_CLK1.
     *
     * Various microcontroller functionality can be multiplexed onto its
     * various pins.  The microcontroller data sheet lists the alternate
     * functionality available for each pin and the name of the function
     * in table 4.1.  This provides the source of the labels given above.
     * There is also an alternate functionality overview in table 4.2 that
     * provides some information for deciphering the gibberish above.  For
     * example, pin PA0 has alternate functionality listed as "I2C0 Serial
     * Data input / output" which is the expansion of I2C_SDA.  PA0 has
     * another alternate function listed as "Timer 0 Capture Compare input /
     * output channel 0" which is the translation of TIM0_CC0.
     *
     * Of interest to us are PA0 and PA1 which will have the functions I2C0_SDA
     * and I2C0_SCL.  We don't want the contents of the GPIO register for port
     * A to interfere with the alternate function, so we need to turn them off.
     *
     * GPIO registers have a large number of pin modes that can be set.  On the
     * output side, this configures the way that the pin is driven by the
     * electronics of the chip.  What we want to do is to set the output to
     * be open-drain.  This means that the output pin will only pull the output
     * down if the pin is programmed to logical zero.  A logical one will
     * essentially disconnect the pin and let some other internal or external
     * device control the pin.
     *
     * So, this is a long winded explanation of why bits one and zero of GPIO
     * port are set to one in a Wired AND mode.  We are turning off those bits
     * bits in the GPIO register to let the I2C serial port take them over when
     * we initialize it below.
     */
    GPIO_PinModeSet(gpioPortA, 1, gpioModeWiredAnd, 1);
    GPIO_PinModeSet(gpioPortA, 0, gpioModeWiredAnd, 1);

    /*
     * We have disabled the GPIO pins so they are not going to interfere with
     * the I2C controller, but we have got to enable the I2C pins to be able
     * to wiggle the PA0 and PA1 pins.
     *
     * Each module has an I/O Routing Register associated with it.  This
     * register is one of the memory mapped registers used to control the
     * function.  In the case of the I2C interface, the routing register is
     * entered in the typedef called I2C_TypeDef.  The definition for I2C0
     * is a pointer to one of these memory mapped types.  Thus when we refer
     * to I2C0->ROUTE below we are setting the I/O Routing register for the
     * Inter Integrated Circuit serial bus.
     *
     * I2C_ROUTE_SDAPEN is the SDA Pin Enable bit which allows the I2C function
     * to drive the I2C0_SDA function.  I2C_ROUTE_SCLPEN is the SCL Pin Enable
     * bit which causes the I2C0_SCL function to be driven.  As mentioned
     * above there is an alternate functionality overview in table 4.2 of the
     * microcontroller datasheet. We saw above that the pin connected to the
     * serial data pin of the LSM303DLHC accelerometer was the microcontroller
     * pin named PA0/TIM0_CC0/I2C0_SDA.  This tells us that we need to connect
     * the internal I2C0 functionality to the PA0 pin of the processor.  If
     * we look in table 4.2 for the functionality I2C0_SDA we find that it
     * can be connected to three locations.  Location 0 corresponds to PA0.
     * If we look in table 4.2 for the functionality I2C0_SCL we find that it
     * can be connected to three locations.  Location 0 corresponds to PA1.
     * Therefore if we set location 0 in the I/O routing register, we will
     * route the I2C functionality out pins PA0 and PA1, and if we set the
     * two enable bits, we have made our I/O subsystem connection from teh
     * microcontroller to the accelerometer.  Since the location is zero,
     * all we need to do is to set the enable bits in the routing register.
     */
    I2C0->ROUTE = I2C_ROUTE_SDAPEN | I2C_ROUTE_SCLPEN;

    /*
     * Call the Energy Micro Library function to actually initialize the I2C
     * serial port we have so carefully conected up.
     */
    I2C_Init(I2C0, &I2C_INIT);
}

/*
 * @brief Read a register from one of the I2C devices we're concerned with.
 *
 * @param slaveAddress The I2C address of the slave that contains the function
 *     we want t access, for example, ACCELEROMETER_I2C_SLAVE_ADDRESS.
 * @param registerAddress The address of the register within the function we
 *     want to access, for example CTRL_REG1_A for control register one of the
 *     accelerometer (A).
 * @param pData The address into which we want to read the contents of the
 *     addressed byte register.
 */
void ReadRegister(
    unsigned short slaveAddress,
    unsigned char registerAddress,
    unsigned char*pData)
{
    I2C_TransferSeq_TypeDef sequence;
    I2C_TransferReturn_TypeDef rc;

    /*
     * Reading from a slave device is a little counter-intuitive.  The goal is
     * to read a byte, but before reading data from the slave device, you have
     * got to tell it what internal address (register) you want to read.
     * So a read of the slave is done with a WRITE_READ sequence.
     *
     * The I2C_TransferSeqTypeDef describes the kind of sequence that the I2C
     * driver will go through when you kick off the transfer.  The EFM library
     * define for the desired sequence is I2C_FLAG_WRITE_READ which indicates
     * the appropriate sequence.
     *
     *     S - Start
     *     Sr - Repeated start
     *     ADDR(W) - address with W/R bit cleared
     *     ADDR(R) - address with W/R bit set
     *     DATA - Data written from/read into buffer
     *     P - Stop
     *
     * Since the sequence is a write of the register address followed by a read
     * of the addressed register, there are two buffer descriptions needed in
     * the provided sequence.
     *
     * The first descripttor, buf[0], contains a pointer to the data for the
     * write (the register address) and a data length which is always one.  The
     * register address is the offset of the register within the function block,
     * for example CTRL_REG1_A (see the accelerometer datasheet).
     *
     * The second descroptor, buf[1], contains a pointer to where the data will
     * be read into and a length of the data buffer, which is always one.
     *
     * The addr field of the sequence (slave address) is the address of
     * the function block we are trying to get to, for example ACC_I2C_ADDRESS
     * for the accelerometer.  This is the base address (write address) of the
     * function and not the read address.  Bumping the address is handled by
     * the driver.
     */
    sequence.flags = I2C_FLAG_WRITE_READ;
    sequence.buf[0].data = &registerAddress;
    sequence.buf[0].len  = 1;
    sequence.buf[1].data = pData;
    sequence.buf[1].len  = 1;
    sequence.addr = slaveAddress;

    /*
     * For each transfer, we have to tell the I2C Controller to start a
     * transfer on the I2C0 device (described by the sequence above) and
     * then poll for completion.
     */
    rc = I2C_TransferInit(I2C0, &sequence);

    while (rc == i2cTransferInProgress) {
        rc = I2C_Transfer(I2C0);
    }

    if (rc != i2cTransferDone) {
        printf("ReadRegister(): Error %d\n", rc);
    }
}

/*
 * @brief Write a register in one of the I2C devices we're concerned with.
 *
 * @param slaveAddress The I2C address of the slave that contains the function
 *     we want t access, for example, ACCELEROMETER_I2C_SLAVE_ADDRESS.
 * @param registerAddress The address of the register within the function we
 *     want to access, for example CTRL_REG1_A for control register one of the
 *     accelerometer (A).
 * @param data The data we want to write to the addressed register.
 */
void WriteRegister(
    unsigned short slaveAddress,
    unsigned char registerAddress,
    unsigned char data)
{
    I2C_TransferSeq_TypeDef sequence;
    I2C_TransferReturn_TypeDef rc;

    /*
     * Writing to a slave device is a little counter-intuitive.  The goal is
     * to write a byte, but before writing data to the slave device, you have
     * got to tell it what internal address (register) you want to read.
     * So a write to the slave is done with a WRITE_WRITE sequence.
     *
     * The I2C_TransferSeqTypeDef describes the kind of sequence that the I2C
     * driver will go through when you kick off the transfer.  The EFM library
     * define for the desired sequence is I2C_FLAG_WRITE_WRITE which indicates
     * the appropriate sequence.
     *
     *     S - Start
     *     ADDR(W) - address with W/R bit cleared
     *     DATA - Data written
     *     P - Stop
     *
     * Since the sequence is a write of the register address followed by a write
     * of the addressed register, there are two buffer descriptions needed in
     * the provided sequence.
     *
     * The first descripttor, buf[0], contains a pointer to the data for the
     * write (the register address) and a data length which is always one.  The
     * register address is the offset of the register within the function block,
     * for example CTRL_REG1_A (see the accelerometer datasheet).
     *
     * The second descroptor, buf[1], contains a pointer to where the data will
     * be read from and a length of the data buffer, which is always one.
     *
     * The addr field of the sequence (slave address) is the address of
     * the function block we are trying to get to, for example ACC_I2C_ADDRESS
     * for the accelerometer.  This is the base address (write address) of the
     * function.
     */
    sequence.flags = I2C_FLAG_WRITE_WRITE;
    sequence.buf[0].data = &registerAddress;
    sequence.buf[0].len  = 1;
    sequence.buf[1].data = &data;
    sequence.buf[1].len  = 1;
    sequence.addr = slaveAddress;

    /*
     * For each transfer, we have to tell the I2C Controller to start a
     * transfer on the I2C0 device (described by the sequence above) and
     * then poll for completion.
     */
    rc = I2C_TransferInit(I2C0, &sequence);

    while (rc == i2cTransferInProgress) {
        rc = I2C_Transfer(I2C0);
    }

    if (rc != i2cTransferDone) {
        printf("WriteRegister(): Error %d\n", rc);
    }
}

/*
 * The AllJoyn part of the equation is to use an interface definition shared
 * between us (the service) who generates an accelerometer and magnetic field
 * sensor raw value signal and the AllJoyn client that will receive the signal
 * and do whatever it likes with the information.  There is enough information
 * there to do a tilt compensated electronic compass, so we call the service
 * the ecompass sample service.
 */
static const char SERVICE_NAME[] = "org.alljoyn.bus.samples.ecompass";
static const uint16_t SERVICE_PORT = 42;

/*
 * The ecompass interface has just one signal in it to send a bunch of signed
 * sixteen bit integers corresponding to Ax, Ay, Az, Mx, My, Mz which are the
 * components of the acceleration and magnetic field vectors.  Since we are a
 * lowly embedded system, we aren't going to try and give any more meaning to
 * the measurements, such as perhaps coming up with a tilt-compensated
 * electronic compass, since that would require all kinds of trig functions
 * and matrix math that would be better done in a signal handler on a grown-
 * up computer.
 */
static const char* ecompassInterface[] = {
    "org.alljoyn.bus.samples.ecompass",
    "!NewRawAccelerationAndMagneticFieldValues >n >n >n >n >n >n",
    NULL
};

static const AJ_InterfaceDescription myInterfaces[] = {
    ecompassInterface,
    NULL
};

static const AJ_Object AppObjects[] = {
    { "/org/alljoyn/bus/samples/ecompass", myInterfaces },
    { NULL }
};

#define APP_RAW_ECOMPASS_SIGNAL AJ_APP_MESSAGE_ID(0, 0, 0)

/*
 * @brief Let the application have some cycles to do some work.  Called
 * periodically from the task.
 *
 */
static void AppDoWork(AJ_BusAttachment* bus)
{
    static int initialized = FALSE;
    unsigned char data, dataL, dataH;
    static short Ax, Ay, Az, Mx, My, Mz;
    bool accelerationChanged = false, magneticFieldChanged = false;

    if (initialized == FALSE) {
        InitializeI2C();
        initialized = TRUE;

        /*
         * Enable the X, Y and Z axes on the accelerometer and tell it to
         * provide updates at a 25 Hz rate.  We pick 25 hertz because we
         * want the updates to happen at about the same frame rate as would
         * happen in a video.
         *
         * XXX How much bandwidth is this actually going to take up on our
         * wireless network?
         *
         * This is going to start our accelerometer making acceleration
         * measurements.  Again, we're not going to do anything fancy,
         * we're just going to poll for new data and ship it out below.
         */
        data = CTRL_REG1_A_XEN |
               CTRL_REG1_A_YEN |
               CTRL_REG1_A_ZEN |
               CTRL_REG1_A_ODR_25_HZ;

        printf("Write 0x%x to CTRL_REG1_A\n", data);
        WriteRegister(ACCELEROMETER_I2C_SLAVE_ADDRESS, CTRL_REG1_A, data);
        data = 0;
        ReadRegister(ACCELEROMETER_I2C_SLAVE_ADDRESS, CTRL_REG1_A, &data);
        printf("Read back 0x%x from CTRL_REG1_A\n", data);

        /*
         * The LSM303DLH application note for a tilt compensated electronic
         * compass suggests to select big endian mode and then read the
         * accelerometer output registers with the low byte register providing
         * the low byte of the acceleration axis value.  Not what one would
         * expect.  In fact, it seems that the chip works as you would expect.
         * If you set big endian mode, the most significant byte of the
         * acceleration comes out the least sinificant byte register.  So,
         * we use little endian mode which is accomplished by not setting
         * CTRL_REG4_A_BLE.  There is nothing mentioned in the application
         * notes, but there is also a high resolution mode.  If you do not set
         * that bit, you don't get what seems to be really usable information.
         * There's not a good description of why that I can find.
         */
        data = CTRL_REG4_A_HR;
        printf("Write 0x%x to CTRL_REG4_A\n", data);
        WriteRegister(ACCELEROMETER_I2C_SLAVE_ADDRESS, CTRL_REG4_A, data);
        data = 0;
        ReadRegister(ACCELEROMETER_I2C_SLAVE_ADDRESS, CTRL_REG4_A, &data);
        printf("Read back 0x%x from CTRL_REG4_A\n", data);
    }

    /*
     * We have asked the accelerometer to update periodically basedn on the
     * ODR (Output Data Rate) setting in CTRL_REG1_A.  It should be running
     * on its own, providing us new data every period.  We look at the
     * Data Available bit in STATUS_REG_A to tell us if there's new data.
     * The bit ZYXDA is the logical OR of all of the X, Y and Z axis data
     * available bits.  When we see that bit asserted, we go and read all
     * of the accelerometer registers to construct the X, Y and Z components
     * of an acceleration vector.
     *
     * Keeping in sync with all of the pictures in the HOWTO used to set up
     * the WSAB and the WSAB Dock, the battery side of the board is "up" and
     * corresponds to the +Z-axis.  The end of the WSAB that has the the
     * daughter board protruding from it (it's a little pointy) is the
     * "front" end and corresponds to the +X-axis.  The side of the WSAB with
     * the battery switch is the "back" and corresponds to the -X-axis.  The
     * "right" side of the WSAB is the side that does not have the switches
     * and corresponds to the +Y-axis.  The switch side is therefore the
     * -Y-axis.
     *
     * The bottom line is that when the WSAB is sitting on a flat surface with
     * the batteries up, the +Z-axis is pointing up so you get a -Az
     * value since gravity points down.  When you tip the front of the
     * WSAB up, you read a -Ax value since the +X-axis is then pointing up and
     * gravity pulls in the opposite direction.  When you tip the right side of
     * the WSAB up you read a -Ay value since the +Y-axis points up and gravity
     * pulls in the opposite direction.
     *
     * We set the full scale selection bits in CTRL_REG4_A to be +- 2g, so one
     * gravity should correspond to half of the capacity of a signed 16-bit
     * integer, or plus or minus 16384.
     */
    ReadRegister(ACCELEROMETER_I2C_SLAVE_ADDRESS, STATUS_REG_A, &data);
    if (data & STATUS_REG_ZYXDA) {

        if (data & STATUS_REG_XDA) {
            ReadRegister(ACCELEROMETER_I2C_SLAVE_ADDRESS, OUT_X_L_A, &dataL);
            ReadRegister(ACCELEROMETER_I2C_SLAVE_ADDRESS, OUT_X_H_A, &dataH);
            Ax = (short)dataH << 8 | (short)dataL;
        }

        if (data & STATUS_REG_YDA) {
            ReadRegister(ACCELEROMETER_I2C_SLAVE_ADDRESS, OUT_Y_L_A, &dataL);
            ReadRegister(ACCELEROMETER_I2C_SLAVE_ADDRESS, OUT_Y_H_A, &dataH);
            Ay = (short)dataH << 8 | (short)dataL;
        }

        if (data & STATUS_REG_ZDA) {
            ReadRegister(ACCELEROMETER_I2C_SLAVE_ADDRESS, OUT_Z_L_A, &dataL);
            ReadRegister(ACCELEROMETER_I2C_SLAVE_ADDRESS, OUT_Z_H_A, &dataH);
            Az = (short)dataH << 8 | (short)dataL;
        }

        printf("******** Ax = %d, Ay = %d, Az = %d\n", Ax, Ay, Az);
        accelerationChanged = true;
    }

    Mx = My = Mz = 0;

    if (accelerationChanged || magneticFieldChanged) {
        /*
         * Send an AllJoyn signal with the new raw Ax, Ay, Az, Mx, My, Mz values.
         */
        AJ_Message msg;
        AJ_MarshalSignal(bus, &msg, APP_RAW_ECOMPASS_SIGNAL, NULL, 0, 0);
        AJ_MarshalArgs(&msg, "nnnnnn", Ax, Ay, Az, Mx, My, Mz);
        AJ_DeliverMsg(&msg);
    }
}

static const char PWD[] = "ABCDEFGH";

static uint32_t PasswordFunc(uint8_t* buffer, uint32_t bufLen)
{
    memcpy(buffer, PWD, sizeof(PWD));
    return sizeof(PWD) - 1;
}

#define CONNECT_TIMEOUT    (1000 * 60)
#define UNMARSHAL_TIMEOUT  (10)

int AJ_Main(void)
{
    AJ_Status status = AJ_OK;
    AJ_BusAttachment bus;
    uint8_t connected = FALSE;

    /*
     * One time initialization before calling any other AllJoyn APIs
     */
    AJ_Initialize();

    AJ_PrintXML(AppObjects);
    AJ_RegisterObjects(AppObjects, NULL);

    while (TRUE) {
        AJ_Message msg;

        if (!connected) {
            status = AJ_Connect(&bus, "org.alljoyn.router", CONNECT_TIMEOUT);
            if (status != AJ_OK) {
                printf("AllJoyn failed to connect sleeping for 15 seconds\n");
                AJ_Sleep(15 * 1000);
                continue;
            }
            printf("AllJoyn connected\n");
            /*
             * Kick things off by binding a session port
             */
            status = AJ_BusBindSessionPort(&bus, SERVICE_PORT, NULL);
            if (status != AJ_OK) {
                printf("Failed to send bind session port message\n");
                break;
            }
            connected = TRUE;
        }

        status = AJ_UnmarshalMsg(&bus, &msg, UNMARSHAL_TIMEOUT);
        if (status != AJ_OK) {
            if (status == AJ_ERR_TIMEOUT) {
                AppDoWork(&bus);
            }
            continue;
        }

        switch (msg.msgId) {
        case AJ_REPLY_ID(AJ_METHOD_BIND_SESSION_PORT):
            /*
             * TODO check the reply args to tell if the request succeeded
             */
            status = AJ_BusRequestName(&bus, SERVICE_NAME, AJ_NAME_REQ_DO_NOT_QUEUE);
            break;

        case AJ_REPLY_ID(AJ_METHOD_REQUEST_NAME):
            /*
             * TODO check the reply args to tell if the request succeeded
             */
            status = AJ_BusAdvertiseName(&bus, SERVICE_NAME, AJ_TRANSPORT_ANY, AJ_BUS_START_ADVERTISING);
            break;

        case AJ_REPLY_ID(AJ_METHOD_ADVERTISE_NAME):
            /*
             * TODO check the reply args to tell if the request succeeded
             */
            break;

        case AJ_METHOD_ACCEPT_SESSION:
            status = AJ_BusReplyAcceptSession(&msg, TRUE);
            break;

        case AJ_SIGNAL_SESSION_LOST:
            /*
             * Force a disconnect
             */
            status = AJ_ERR_READ;
            break;

        default:
            /*
             * Pass to the built-in handlers
             */
            status = AJ_BusHandleBusMessage(&msg);
            break;
        }
        /*
         * Messages must be closed to free resources
         */
        AJ_CloseMsg(&msg);

        if (status == AJ_ERR_READ) {
            printf("AllJoyn disconnect\n");
            AJ_Disconnect(&bus);
            connected = FALSE;
            /*
             * Sleep a little while before trying to reconnect
             */
            AJ_Sleep(10 * 1000);
        }
    }

    printf("accelerometer EXIT %d\n", status);

    return status;
}

#ifdef AJ_MAIN
int main()
{
    return AJ_Main();
}
#endif
