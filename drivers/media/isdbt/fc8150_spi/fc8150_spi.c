/*****************************************************************************
 Copyright(c) 2012 FCI Inc. All Rights Reserved

 File name : fc8150_spi.c

 Description : fc8150 host interface

*******************************************************************************/
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/slab.h>

#include "fci_types.h"
#include "fc8150_regs.h"
#include "fci_oal.h"

#define SPI_BMODE           0x00
#define SPI_WMODE           0x10
#define SPI_LMODE           0x20
#define SPI_READ            0x40
#define SPI_WRITE           0x00
#define SPI_AINC            0x80
#define CHIPID              (0 << 3)

#define DRIVER_NAME "fc8150_spi"

struct spi_device *fc8150_spi;

static u8 tx_data[10];
static u8 rdata_buf[8192];
static u8 wdata_buf[8192];

static DEFINE_MUTEX(lock);



static int __devinit fc8150_spi_probe(struct spi_device *spi)
{
	s32 ret;


	printk( "fc8150_spi_probe\n");

	spi->max_speed_hz =  19200000;	//9600000; //16000000;	//24000000;
	spi->bits_per_word = 8;
	spi->mode =  SPI_MODE_0;

	ret = spi_setup(spi);

	if (ret < 0)
	{
		printk("fc8150_spi_probe ret =%d \n",ret);
		return ret;
	}

	if(spi == NULL)
	printk("fc8150_spi_probe spi == NULL 2\n");
	
	fc8150_spi = spi;
	if(fc8150_spi == NULL)
	printk("fc8150_spi_probe spi == NULL 3\n");
	
	return ret;
}

static int fc8150_spi_remove(struct spi_device *spi)
{

	return 0;
}


static const struct of_device_id tmm_spi_match_table[] = {
	{   .compatible = "isdbt_spi_comp",
	},
	{}
};

static struct spi_driver fc8150_spi_driver = {
	.driver = {
		.name		= DRIVER_NAME,
		.owner		= THIS_MODULE,
		.of_match_table = tmm_spi_match_table,
	},
	.probe		= fc8150_spi_probe,
	.remove		= __devexit_p(fc8150_spi_remove),
};

static int fc8150_spi_write_then_read(struct spi_device *spi
	, u8 *txbuf, u16 tx_length, u8 *rxbuf, u16 rx_length)
{
	int res = 0;

	struct spi_message	message;
	struct spi_transfer	x;

	spi_message_init(&message);
	memset(&x, 0, sizeof x);

	spi_message_add_tail(&x, &message);

	memcpy(&wdata_buf[0], txbuf, tx_length);

	x.tx_buf = &wdata_buf[0];
	x.rx_buf = &rdata_buf[0];
	x.len = tx_length + rx_length;
	x.cs_change = 0;
	x.bits_per_word = 8;

	res = spi_sync(spi, &message);

	memcpy(rxbuf, x.rx_buf + tx_length, rx_length);

	return res;
}

static int spi_bulkread(HANDLE hDevice, u16 addr
	, u8 command, u8 *data, u16 length)
{
	int res;

	tx_data[0] = addr & 0xff;
	tx_data[1] = (addr >> 8) & 0xff;
	tx_data[2] = (command & 0xf0) | CHIPID | ((length >> 16) & 0x07);
	tx_data[3] = (length >> 8) & 0xff;
	tx_data[4] = length & 0xff;

	res = fc8150_spi_write_then_read(fc8150_spi, &tx_data[0]
		, 5, data, length);

	if (res) {
		printk( "fc8150_spi_bulkread fail : %d\n", res);
		return BBM_NOK;
	}

	return BBM_OK;
}

static int spi_bulkwrite(HANDLE hDevice, u16 addr
	, u8 command, u8 *data, u16 length)
{
	int i;
	int res;

	tx_data[0] = addr & 0xff;
	tx_data[1] = (addr >> 8) & 0xff;
	tx_data[2] = (command & 0xf0) | CHIPID | ((length >> 16) & 0x07);
	tx_data[3] = (length >> 8) & 0xff;
	tx_data[4] = length & 0xff;

	for (i = 0; i < length; i++)
		tx_data[5+i] = data[i];

	res = fc8150_spi_write_then_read(fc8150_spi
		, &tx_data[0], length+5, NULL, 0);

	if (res) {
		printk( "fc8150_spi_bulkwrite fail : %d\n", res);
		return BBM_NOK;
	}

	return BBM_OK;
}

static int spi_dataread(HANDLE hDevice, u16 addr
	, u8 command, u8 *data, u32 length)
{
	int res;

	tx_data[0] = addr & 0xff;
	tx_data[1] = (addr >> 8) & 0xff;
	tx_data[2] = (command & 0xf0) | CHIPID | ((length >> 16) & 0x07);
	tx_data[3] = (length >> 8) & 0xff;
	tx_data[4] = length & 0xff;

	res = fc8150_spi_write_then_read(fc8150_spi
		, &tx_data[0], 5, data, length);

	if (res) {
		printk( "fc8150_spi_dataread fail : %d\n", res);
		return BBM_NOK;
	}

	return BBM_OK;
}

int fc8150_spi_init(HANDLE hDevice, u16 param1, u16 param2)
{
	int res = 0;

	printk( "fc8150_spi_init : %d\n", res);

	res = spi_register_driver(&fc8150_spi_driver);

	if (res) {
		printk( "fc8150_spi register fail : %d\n", res);
		return BBM_NOK;
	}

	return res;
}

int fc8150_spi_byteread(HANDLE hDevice, u16 addr, u8 *data)
{
	int res;
	u8 command = SPI_READ;

	mutex_lock(&lock);
	res = spi_bulkread(hDevice, addr, command, data, 1);
	mutex_unlock(&lock);

	return res;
}

int fc8150_spi_wordread(HANDLE hDevice, u16 addr, u16 *data)
{
	int res;
	u8 command = SPI_READ | SPI_AINC;

	mutex_lock(&lock);
	res = spi_bulkread(hDevice, addr, command, (u8 *)data, 2);
	mutex_unlock(&lock);
	return res;
}

int fc8150_spi_longread(HANDLE hDevice, u16 addr, u32 *data)
{
	int res;
	u8 command = SPI_READ | SPI_AINC;

	mutex_lock(&lock);
	res = spi_bulkread(hDevice, addr, command, (u8 *)data, 4);
	mutex_unlock(&lock);

	return res;
}

int fc8150_spi_bulkread(HANDLE hDevice, u16 addr, u8 *data, u16 length)
{
	int res;
	u8 command = SPI_READ | SPI_AINC;

	mutex_lock(&lock);
	res = spi_bulkread(hDevice, addr, command, data, length);
	mutex_unlock(&lock);

	return res;
}

int fc8150_spi_bytewrite(HANDLE hDevice, u16 addr, u8 data)
{
	int res;
	u8 command = SPI_WRITE;

	mutex_lock(&lock);
	res = spi_bulkwrite(hDevice, addr, command, (u8 *)&data, 1);
	mutex_unlock(&lock);

	return res;
}

int fc8150_spi_wordwrite(HANDLE hDevice, u16 addr, u16 data)
{
	int res;
	u8 command = SPI_WRITE | SPI_AINC;

	mutex_lock(&lock);
	res = spi_bulkwrite(hDevice, addr, command, (u8 *)&data, 2);
	mutex_unlock(&lock);

	return res;
}

int fc8150_spi_longwrite(HANDLE hDevice, u16 addr, u32 data)
{
	int res;
	u8 command = SPI_WRITE | SPI_AINC;

	mutex_lock(&lock);
	res = spi_bulkwrite(hDevice, addr, command, (u8 *)&data, 4);
	mutex_unlock(&lock);

	return res;
}

int fc8150_spi_bulkwrite(HANDLE hDevice, u16 addr, u8 *data, u16 length)
{
	int res;
	u8 command = SPI_WRITE | SPI_AINC;

	mutex_lock(&lock);
	res = spi_bulkwrite(hDevice, addr, command, data, length);
	mutex_unlock(&lock);

	return res;
}

int fc8150_spi_dataread(HANDLE hDevice, u16 addr, u8 *data, u32 length)
{
	int res;
	u8 command = SPI_READ;

	mutex_lock(&lock);
	res = spi_dataread(hDevice, addr, command, data, length);
	mutex_unlock(&lock);

	return res;
}

int fc8150_spi_deinit(HANDLE hDevice)
{
	return BBM_OK;
}

