/* test_main.c
 *
 * Copyright (C) 2006-2022 wolfSSL Inc.
 *
 * This file is part of wolfSSL.
 *
 * wolfSSL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * wolfSSL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA
 */

#define MCUEXPRESSO_FRDM_K82F_SDK

#ifdef MCUEXPRESSO_FRDM_K82F_SDK
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MK82F25615.h"
#include "fsl_debug_console.h"
#include "fsl_rtc.h"
#include "fsl_trng.h"
#endif /* MCUEXPRESSO_FRDM_K82F_SDK */

#ifdef HAVE_CONFIG_H
    #include <config.h>
#endif

#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/wc_port.h>
#include <wolfcrypt/test/test.h>

typedef struct func_args {
    int    argc;
    char** argv;
    int    return_code;
} func_args;

static func_args args = { 0 } ;

void main(void)
{
    int test_num = 0;

#ifdef MCUEXPRESSO_FRDM_K82F_SDK

    rtc_config_t    rtc_config;
    trng_config_t   trng_config;

    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
	#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
    /* Init FSL debug console. */
    BOARD_InitDebugConsole();
	#endif

    /* Initialize and start Real-Time Clock (RTC) */
    RTC_GetDefaultConfig(&rtc_config);
    RTC_Init(RTC, &rtc_config);
    RTC_StartTimer(RTC);

    /* Initialize and start True Random Number Generator (TRNG) */
    TRNG_GetDefaultConfig(&trng_config);
    TRNG_Init(TRNG0, &trng_config);

#endif /* MCUEXPRESSO_FRDM_K82F_SDK */

    wolfCrypt_Init(); /* required for ksdk_port_init */
    do
    {
        /* Used for testing, must have a delay so no data is missed while serial is initializing */
        #ifdef WOLFSSL_FRDM_K64_JENKINS
            /* run twice */
            if(test_num == 2){
                printf("\n&&&&&&&&&&&&& done &&&&&&&&&&&&&&&");
                delay_us(1000000);
                break;
            }
            delay_us(1000000); /* 1 second */
        #endif

        printf("\nCrypt Test %d:\n", test_num);
        wolfcrypt_test(&args);
        printf("Crypt Test %d: Return code %d\n", test_num, args.return_code);

        test_num++;
    } while(args.return_code == 0);

    /* Print this again for redundancy */
    #ifdef WOLFSSL_FRDM_K64_JENKINS
        printf("\n&&&&&&&&&&&&&& done &&&&&&&&&&&&&\n");
        delay_us(1000000);
    #endif

    wolfCrypt_Cleanup();
}

#ifdef MCUEXPRESSO_FRDM_K82F_SDK

/* Return total elapsed seconds */
double current_time(int reset)
{
	double seconds;

	(void)reset; /* Not supported */

	rtc_datetime_t datetime;
	RTC_GetDatetime(RTC, &datetime);

	/* Perform simplified time-since-Epoch calculation (no accounting
	   for leap years or variable number of days in a month) */
	seconds = ((datetime.year - 1970) * 31557600)
			  + ((datetime.month - 1) * 2629800)
              + ((datetime.day - 1) * 86400)
			  + (datetime.hour * 3600)
			  + (datetime.minute * 60)
			  + datetime.second;

	return seconds;
}

/* Return seconds since Epoch (timer ignored) */
unsigned long custom_time(unsigned long* timer)
{
	return current_time(0);
}

unsigned int hw_rand(void)
{
    unsigned int data;

    TRNG_GetRandomData(TRNG0, &data, sizeof(data));

    return data;
}

#endif /* MCUEXPRESSO_FRDM_K82F_SDK */

unsigned int custom_rand_generate(void)
{
    return hw_rand();
}

int custom_rand_generate_block(unsigned char* output, unsigned int sz)
{
    uint32_t i = 0;

    while (i < sz)
    {
        /* If not aligned or there is odd/remainder */
        if( (i + sizeof(CUSTOM_RAND_TYPE)) > sz ||
            ((uint32_t)&output[i] % sizeof(CUSTOM_RAND_TYPE)) != 0
        ) {
            /* Single byte at a time */
            output[i++] = (unsigned char)custom_rand_generate();
        }
        else {
            /* Use native 8, 16, 32 or 64 copy instruction */
            *((CUSTOM_RAND_TYPE*)&output[i]) = custom_rand_generate();
            i += sizeof(CUSTOM_RAND_TYPE);
        }
    }

    return 0;
}
