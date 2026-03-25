#include "btm9011.h"

HAL_StatusTypeDef BTM9011_Init(BTM9011_HandleTypeDef *hbtm,
                                SPI_HandleTypeDef *hspi,
                                GPIO_TypeDef *cs_port, uint16_t cs_pin,
                                uint8_t num_drivers)
{
	/* Check if parsed values equal NULL */

    if (hbtm == NULL || hspi == NULL || cs_port == NULL)
        return HAL_ERROR;
    if (num_drivers == 0 || num_drivers > BTM9011_MAX_DRIVERS)
        return HAL_ERROR;

    /* Init all the BTM9011 chip data to the BTM handle hbtm */

    hbtm->hspi        = hspi;
    hbtm->cs_port     = cs_port;
    hbtm->cs_pin      = cs_pin;
    hbtm->num_drivers = num_drivers;

    /* Set the CS pin to idle */

    HAL_GPIO_WritePin(cs_port, cs_pin, GPIO_PIN_RESET); /* CS idle */
    return HAL_OK;
}

HAL_StatusTypeDef BTM9011_Send(BTM9011_HandleTypeDef *hbtm, const uint8_t *cmds)
{
	/* Check if parsed values equal NULL */

    if (hbtm == NULL || cmds == NULL)
        return HAL_ERROR;

    /*
     * Daisy-chain ordering: the first byte clocked out ends up in the
     * furthest chip (driver N). Reverse the cmds array so that cmds[0]
     * (driver 1, nearest) is latched into chip 1.
     */

    uint8_t tx[BTM9011_MAX_DRIVERS];
    for (uint8_t i = 0; i < hbtm->num_drivers; i++)
        tx[i] = cmds[hbtm->num_drivers - 1 - i];

    /* SPI transmit, before CS is pulled high and after transmit low to latch it */
    HAL_GPIO_WritePin(hbtm->cs_port, hbtm->cs_pin, GPIO_PIN_SET);   /* CS active */
    HAL_StatusTypeDef status = HAL_SPI_Transmit(hbtm->hspi, tx, hbtm->num_drivers, 100);
    HAL_Delay(1); /* Small delay */
    HAL_GPIO_WritePin(hbtm->cs_port, hbtm->cs_pin, GPIO_PIN_RESET); /* latch */

    return status;
}

uint8_t BTM9011_BuildCmd(BTM9011_Dir dir, uint8_t duty_0_31)
{
    uint8_t in1, in2;

    switch (dir)
    {
        case BTM9011_DIR_FORWARD: in1 = 1; in2 = 1; break;
        case BTM9011_DIR_REVERSE: in1 = 1; in2 = 0; break;
        case BTM9011_DIR_COAST:   in1 = 0; in2 = 1; break;
        default:                  return 0x00; /* BTM9011_DIR_BRAKE */
    }

    if (duty_0_31 > BTM9011_DUTY_MAX)
        duty_0_31 = BTM9011_DUTY_MAX;

    return (uint8_t)((in1 << 6) | (in2 << 5) | (duty_0_31 & 0x1F));
}
