/*(Copyright)

Microsoft Copyright 2017
Confidential Information

*/

#include "DiceCore.h"
#include "DiceSha256.h"
#include <stdio.h>
#define logging 0

// Digest info
uint8_t vDigest[DICE_DIGEST_LENGTH] = { 0x00 };
uint8_t rDigest[DICE_DIGEST_LENGTH] = { 0x00 };
DICE_SHA256_CONTEXT  DiceHashCtx = { 0x00 };
DICE_DATA DiceData;

int DiceCore(DICE_DATA DiceDataInput)
{
	DiceData = DiceDataInput;

#if logging
    printf("Riot Core code:\r\n");
    for(int i = 0; i < DiceData.riotSize; i++){
        if(i == (DiceData.riotSize - 1)){
            printf("%02x\r\n", DiceData.riotCore[i]);
        }
        else{
            printf("%02x", DiceData.riotCore[i]);
        }
    }
#endif

    // Compute digest of RIoT Core
    if (_DiceMeasure(DiceData.riotCore, DiceData.riotSize, rDigest, DICE_DIGEST_LENGTH)) {
        // Enter remediation
        return -1;
    }

#if logging
    printf("rDigest:\r\n");
    for(int i = 0; i < DICE_DIGEST_LENGTH; i++){
        if(i == (DICE_DIGEST_LENGTH - 1)){
            printf("%02x\r\n", rDigest[i]);
        }
        else{
            printf("%02x", rDigest[i]);
        }
    }
#endif

    // Derive CDI based on measurement of RIoT Core and UDS.
   if (_DiceDeriveCDI(rDigest, DICE_DIGEST_LENGTH)) {
       // Enter remediation
       return -1;
   }

    // Clean up potentially sensitive data
    _BZERO(vDigest, DICE_DIGEST_LENGTH);
    _BZERO(rDigest, DICE_DIGEST_LENGTH);
    _BZERO(&DiceHashCtx, sizeof(DiceHashCtx));

    return 0;
}

static uint32_t _DiceMeasure(uint8_t *data, size_t dataSize, uint8_t *digest, size_t digestSize)
{
    // Validate parameters
    if ((!data) || (dataSize < (2 * sizeof(uint32_t))) ||
        (!digest) || (digestSize != DICE_DIGEST_LENGTH)) {
        return -1;
    }

    // Measure data area
    DiceSHA256(data, dataSize, digest);

    // Success
    return 0;
}

static uint32_t _DiceDeriveCDI(uint8_t *digest, size_t digestSize)
{
    // Validate parameter
    if (!(digest) || (digestSize != DICE_DIGEST_LENGTH)) {
        return -1;
    }

    // Don't use the UDS directly.
    DiceSHA256(DiceData.UDS->bytes, DICE_UDS_LENGTH, vDigest);

    // Derive CDI value based on UDS and RIoT Core measurement
    DiceSHA256_2(vDigest, DICE_DIGEST_LENGTH, rDigest, DICE_DIGEST_LENGTH, DiceData.CDI->bytes);

    // Success
    return 0;
}

static void _BZERO(void *p, uint32_t l)
{
	uint32_t i;
    for (i = 0; i < (l); i++) {
        ((uint8_t*)p)[i] = 0;
    }
}

