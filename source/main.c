/*
*************************************************************
Projekt do predmetu IMP
autor:	Kristian Liščinský (xlisci01)
dátum:	2.1.2018
*************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>
#include <locale.h>
#include <math.h>
#include <stdint.h>
#include <inttypes.h>
#include "board.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_clock.h"
#include "fsl_crc.h"
#include "fsl_debug_console.h"
#include "fsl_uart.h"



static const uint16_t table_for_16bCRC[256] = {0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 0x8108, 0x9129, 0xa14a, 0xb16b,
0xc18c, 0xd1ad, 0xe1ce, 0xf1ef, 0x1231, 0x0210,0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6, 0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c,
0xf3ff, 0xe3de, 0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485, 0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4, 0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc, 0x48c4, 0x58e5,
0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823, 0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b, 0x5af5, 0x4ad4, 0x7ab7, 0x6a96,
0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a, 0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03,
0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49, 0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78, 0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f, 0x1080, 0x00a1,
0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067, 0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e, 0x02b1, 0x1290, 0x22f3, 0x32d2,
0x4235, 0x5214, 0x6277, 0x7256, 0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d, 0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447,
0x5424, 0x4405, 0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c, 0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab, 0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3, 0xcb7d, 0xdb5c,
0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a, 0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92, 0xfd2e, 0xed0f, 0xdd6c, 0xcd4d,
0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1, 0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba,
0x8fd9, 0x9ff8, 0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

static const uint32_t table_for_32bCRC[256] = {0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3, 0x0EDB8832,
0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB,
0xF4D4B551, 0x83D385C7, 0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4,
0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87,
0x58684C11, 0xC1611DAB, 0xB6662D3D, 0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433, 0x7807C9A2, 0x0F00F934,
0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01, 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1,
0xF50FC457, 0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65, 0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C,
0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F, 0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81,
0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84, 0x0D6D6A3E,
0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1, 0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1,
0xA6BC5767, 0x3FB506DD, 0x48B2364B, 0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79, 0xCB61B38C, 0xBC66831A,
0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B,
0x5BDEAE1D, 0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777, 0x88085AE6,
0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45, 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7,
0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9, 0xBDBDF21C, 0xCABAC28A, 0x53B39330,
0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

/*
*************************************************************
*************************************************************
*************************************************************
************      KOMUNIKÁCIA POMOCOU UART      *************
*************************************************************
*************************************************************
*************************************************************
*/
void Start_Uart(void) {
	uart_config_t config;
	UART_GetDefaultConfig(&config);
	config.baudRate_Bps = 115200U;
	config.enableTx = true;
	config.parityMode = kUART_ParityDisabled;
	UART_Init(UART5, &config, kCLOCK_BusClk);
}

/*
*************************************************************
*************************************************************
*************************************************************
**********      VÝPOČET POMOCOU MODULU Z ČIPU      **********
*************************************************************
*************************************************************
*************************************************************
Algoritmy boli použité z manuálovej stránky KDS SDK stránky:
https://mcuxpresso.nxp.com/apidoc/2.0/group__crc__driver.html
*/


uint16_t crc16_from_module(char *information, size_t length){
	crc_config_t config;
	CRC_Type *base = CRC0;
    config.polynomial = 0x1021U;
    config.seed = 0x0U;
    config.reflectIn = false;
    config.reflectOut = false;
    config.complementChecksum = false;
    config.crcBits = kCrcBits16;
    config.crcResult = kCrcFinalChecksum;
    
    //Enables and configures the CRC peripheral module.
    CRC_Init(base, &config);
    CRC_WriteData(base, (uint8_t *) information, length);

    /*
    Reads CRC data register (intermediate or final checksum).
    The configured type of transpose and complement are applied.
	Returns intermediate or final 16-bit checksum
	*/
    return CRC_Get16bitResult(base);
}

uint32_t crc32_from_module(char *information, size_t length){
	crc_config_t config;
	CRC_Type *base = CRC0;
    config.polynomial = 0x04C11DB7U;
    config.seed = 0xFFFFFFFFU;
    config.reflectIn = true;
    config.reflectOut = true;
    config.complementChecksum = true;
    config.crcBits = kCrcBits32;
    config.crcResult = kCrcFinalChecksum;
    
    //Enables and configures the CRC peripheral module.
    CRC_Init(base, &config);
    CRC_WriteData(base, (uint8_t *) information, length);
    
    /*
    Reads CRC data register (intermediate or final checksum).
    The configured type of transpose and complement are applied.
	Returns intermediate or final 32-bit checksum
	*/
    return CRC_Get32bitResult(base);
}

/*
*************************************************************
*************************************************************
*************************************************************
**********   VÝPOČET POMOCOU POLYNÓMU Z TABUĽKY    **********
*************************************************************
*************************************************************
*************************************************************
Algoritmus pre crc32 bol použitý zo stránky:
https://en.wikipedia.org/wiki/Cyclic_redundancy_check
*/

uint16_t crc16_from_table(char *information, size_t length){
    //Initialize crc-16 to starting value
    uint16_t crc16 = 0x0U;
    //Index of crc16 table
    uint16_t nLookupIndex = 0;
    uint16_t i = 0;
    do{
    	//((crc shr 8) xor byte) and 0xFF
        nLookupIndex = ((crc16 >> 8) ^ *information) & 0xFF;
        //next byte
        information = information + 1;
        //crc16 = (crc_table[index] xor (crc shl 8))
        crc16 = table_for_16bCRC[nLookupIndex] ^ (crc16 << 8);
        i++;
    }while((i < length)); //for each byte

    return (uint16_t)crc16;
}

uint32_t crc32_from_table(char *information, size_t length){
    //Initialize crc-32 to starting value
    uint32_t crc32 = 0xFFFFFFFFU;
    //Index of crc32 table
    uint32_t nLookupIndex = 0;
    uint32_t i = 0;

    do{
        //(crc xor byte) and 0xFF
        nLookupIndex = (crc32 ^ *information) & 0xFF;
        //next byte
        information = information + 1;
        //crc32 = (crc32 shr 8) xor CRCTable[index]
        crc32 = (crc32 >> 8) ^ table_for_32bCRC[nLookupIndex];
        i++;
    }while((i < length));  //for each byte

    //Finalize the CRC-32 value by inverting all the bits
    //crc32 ← crc32 xor 0xFFFFFFFF
    crc32 = crc32 ^ 0xFFFFFFFF;

    return crc32;
}

/*
*************************************************************
*************************************************************
*************************************************************
**********   VÝPOČET PODĽA POLYNÓMU (bez tabuľky)  **********
*************************************************************
*************************************************************
*************************************************************
Algoritmus pre crc32 bol použitý zo stránky:
https://stackoverflow.com/questions/21001659/crc32-algorithm
-implementation-in-c-without-a-look-up-table-and-with-a-public-li
*/

uint16_t crc16(char *information, size_t length){
	uint16_t crc16 = 0x0U;
	uint16_t i = 0;
	while(i < length){
		crc16 = crc16 ^ ((information[i]) << 8);
		i++;
		for(uint16_t j = 0; j< 8; ++j){
			if((crc16 & 0x8000) == 0){
				crc16 = crc16 << 1;
			}
			else{
				crc16 = ((crc16 << 1) ^ 0x1021U);
			}
		}
	}
	return crc16;	
}

uint32_t crc32(char *information){
   int i, j;
   unsigned int byte, crc, mask;

   i = 0;
   crc = 0xFFFFFFFF;
   while (information[i] != 0) {
      byte = information[i];            // Get next byte.
      crc = crc ^ byte;
      for (j = 7; j >= 0; j--) {    // Do eight times, because 8 bits
         mask = -(crc & 1);
         crc = (crc >> 1) ^ (0xEDB88320 & mask);
      }
      i = i + 1;
   }
   return ~crc;
}
/*
	Printf function to write output
*/
void my_printf(char *data){
	UART_WriteBlocking(UART5, (uint8_t *) data, strlen(data));
}

void predved_sa(void){
	char* test_string = "Glory glory Manchester United";
	uint16_t crc16_module = crc16_from_module(test_string, strlen(test_string));
	uint32_t crc32_module = crc32_from_module(test_string, strlen(test_string));
	
	uint16_t crc16_table = crc16_from_table(test_string, strlen(test_string));
	uint32_t crc32_table = crc32_from_table(test_string, strlen(test_string));

	uint16_t crc16_simple = crc16(test_string, strlen(test_string));
	uint32_t crc32_simple = crc32(test_string);

	char string_crc16_module[16];
	char string_crc32_module[32];

	char string_crc32_table[32];
	char string_crc16_table[16];

	char string_crc32_simple[32];
	char string_crc16_simple[16];


	sprintf(string_crc16_module,"0x%"PRIx16"", crc16_module);
	sprintf(string_crc32_module,"0x%"PRIx32"", crc32_module);

	sprintf(string_crc32_table,"0x%"PRIx32"", crc32_table);
	sprintf(string_crc16_table,"0x%"PRIx16"", crc16_table);

	sprintf(string_crc16_simple,"0x%"PRIx16"", crc16_simple);
	sprintf(string_crc32_simple,"0x%"PRIx32"", crc32_simple);

	my_printf("\r\nZadané data:\t");
	my_printf(test_string);
	my_printf("\r\nOčakávané hodnoty CRC:\r\nCRC-16:\t0x8cba\r\nCRC-32:\t0xf7903f1f");

	my_printf("\r\nHODNOTA ZÍSKANÁ POMOCOU HW MODULU\n\r16 bitová varianta: ");
	my_printf(string_crc16_module);
	my_printf("\r\n32 bitová varianta: ");
	my_printf(string_crc32_module);
	
	my_printf("\r\nHODNOTA ZÍSKANÁ POMOCOU POLYNÓMU Z TABUĽKY\r\n16 bitová varianta: ");
	my_printf(string_crc16_table);
	my_printf("\r\n32 bitová varianta: ");
	my_printf(string_crc32_table);

	my_printf("\r\nHODNOTA ZÍSKANÁ POMOCOU POLYNÓMU (bez tabuľky)\r\n16 bitová varianta: ");
	my_printf(string_crc16_simple);
	my_printf("\r\n32 bitová varianta: ");
	my_printf(string_crc32_simple);
}

void zanesena1chyba(void){
	char* test_string = "Hlory glory Manchester United";
	uint16_t crc16_module = crc16_from_module(test_string, strlen(test_string));
	uint32_t crc32_module = crc32_from_module(test_string, strlen(test_string));
	
	uint16_t crc16_table = crc16_from_table(test_string, strlen(test_string));
	uint32_t crc32_table = crc32_from_table(test_string, strlen(test_string));

	uint16_t crc16_simple = crc16(test_string, strlen(test_string));
	uint32_t crc32_simple = crc32(test_string);

	char string_crc16_module[16];
	char string_crc32_module[32];

	char string_crc32_table[32];
	char string_crc16_table[16];

	char string_crc32_simple[32];
	char string_crc16_simple[16];


	sprintf(string_crc16_module,"0x%"PRIx16"", crc16_module);
	sprintf(string_crc32_module,"0x%"PRIx32"", crc32_module);

	sprintf(string_crc32_table,"0x%"PRIx32"", crc32_table);
	sprintf(string_crc16_table,"0x%"PRIx16"", crc16_table);

	sprintf(string_crc16_simple,"0x%"PRIx16"", crc16_simple);
	sprintf(string_crc32_simple,"0x%"PRIx32"", crc32_simple);

	my_printf("\r\n\nZAVEDENIE 1 CHYBY DO PôVODNÝCH DÁT\n\rZadané dáta:\t");
	my_printf(test_string);
	my_printf("\r\nOčakávané hodnoty CRC\r\nCRC-16:\t0x899d\r\nCRC-32:\t0xab2af300");
	my_printf("\r\nHODNOTA ZÍSKANÁ POMOCOU HW MODULU\n\r16 bitová varianta: ");
	my_printf(string_crc16_module);
	my_printf("\r\n32 bitová varianta: ");
	my_printf(string_crc32_module);
	
	my_printf("\r\nHODNOTA ZÍSKANÁ POMOCOU POLYNÓMU Z TABUĽKY\r\n16 bitová varianta: ");
	my_printf(string_crc16_table);
	my_printf("\r\n32 bitová varianta: ");
	my_printf(string_crc32_table);

	my_printf("\r\nHODNOTA ZÍSKANÁ POMOCOU POLYNÓMU (bez tabuľky)\r\n16 bitová varianta: ");
	my_printf(string_crc16_simple);
	my_printf("\r\n32 bitová varianta: ");
	my_printf(string_crc32_simple);
}

void zanesenieviacchyb(void){
	char* test_string = "Geoty wlory Marvhesher Anited";
	uint16_t crc16_module = crc16_from_module(test_string, strlen(test_string));
	uint32_t crc32_module = crc32_from_module(test_string, strlen(test_string));
	
	uint16_t crc16_table = crc16_from_table(test_string, strlen(test_string));
	uint32_t crc32_table = crc32_from_table(test_string, strlen(test_string));

	uint16_t crc16_simple = crc16(test_string, strlen(test_string));
	uint32_t crc32_simple = crc32(test_string);

	char string_crc16_module[16];
	char string_crc32_module[32];

	char string_crc32_table[32];
	char string_crc16_table[16];

	char string_crc32_simple[32];
	char string_crc16_simple[16];


	sprintf(string_crc16_module,"0x%"PRIx16"", crc16_module);
	sprintf(string_crc32_module,"0x%"PRIx32"", crc32_module);

	sprintf(string_crc32_table,"0x%"PRIx32"", crc32_table);
	sprintf(string_crc16_table,"0x%"PRIx16"", crc16_table);

	sprintf(string_crc16_simple,"0x%"PRIx16"", crc16_simple);
	sprintf(string_crc32_simple,"0x%"PRIx32"", crc32_simple);

	my_printf("\r\n\nZAVEDENIE VIAC CHÝB DO PôVODNÝCH DÁT:\r\nZadané data:\t");
	my_printf(test_string);
	my_printf("\r\nOčakávané hodnoty CRC\n\rCRC-16:\t0xa87f\r\nCRC-32:\t0x83753456");
	my_printf("\r\nHODNOTA ZÍSKANÁ POMOCOU HW MODULU\n\r16 bitová varianta: ");
	my_printf(string_crc16_module);
	my_printf("\r\n32 bitová varianta: ");
	my_printf(string_crc32_module);
	
	my_printf("\r\nHODNOTA ZÍSKANÁ POMOCOU POLYNÓMU Z TABUĽKY\r\n16 bitová varianta: ");
	my_printf(string_crc16_table);
	my_printf("\r\n32 bitová varianta: ");
	my_printf(string_crc32_table);

	my_printf("\r\nHODNOTA ZÍSKANÁ POMOCOU POLYNÓMU (bez tabuľky)\r\n16 bitová varianta: ");
	my_printf(string_crc16_simple);
	my_printf("\r\n32 bitová varianta: ");
	my_printf(string_crc32_simple);
}

int main(void) {
	BOARD_InitPins();
	BOARD_BootClockRUN();
	BOARD_InitDebugConsole();
	
	Start_Uart();
	predved_sa();
	zanesena1chyba();
	zanesenieviacchyb();	
}