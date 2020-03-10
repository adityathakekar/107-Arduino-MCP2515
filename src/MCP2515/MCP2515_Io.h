/**
 * @brief   Arduino library for controlling the MCP2515 in order to receive/transmit CAN frames.
 * @author  Alexander Entinger, MSc / LXRobotics GmbH
 * @license LGPL 3.0
 */

#ifndef MCP2515_MCP2515_IO_H_
#define MCP2515_MCP2515_IO_H_

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <stdint.h>

#include <Arduino.h>
#include <SPI.h>

#include "MCP2515_Const.h"

/**************************************************************************************
 * NAMESPACE
 **************************************************************************************/

namespace MCP2515
{

/**************************************************************************************
 * TYPEDEF
 **************************************************************************************/

enum class TxB : uint8_t
{
  TxB0 = 0,
  TxB1 = 1,
  TxB2 = 2
};

enum class RxB : uint8_t
{
  RxB0 = 0,
  RxB1 = 1
};

union RxTxBuffer
{
  struct
  {
    uint8_t sidh;
    uint8_t sidl;
    uint8_t eid8;
    uint8_t eid0;
    uint8_t dlc;
    uint8_t data[8];
  } reg;
  uint8_t buf[5+8];
};

/**************************************************************************************
 * CLASS DECLARATION
 **************************************************************************************/

class MCP2515_Io
{

public:

  MCP2515_Io(int const cs_pin);


  void    begin();

  uint8_t readRegister  (Register const reg);
  void    writeRegister (Register const reg, uint8_t const data);
  void    modifyRegister(Register const reg, uint8_t const mask, uint8_t const data);

  static uint8_t constexpr TX_BUF_SIZE = 5 + 8;
  static uint8_t constexpr RX_BUF_SIZE = TX_BUF_SIZE;

  void    loadTxBuffer  (TxB const txb, uint8_t const * tx_buf_data); /* tx_buf_data = {SIDH, SIDL, EID8, EID0, DLC, DATA[0-8 Byte] } */
  void    requestTx     (TxB const txb);
  void    readRxBuffer  (RxB const rxb, uint8_t * rx_buf_data);       /* rx_buf_data = {SIDH, SIDL, EID8, EID0, DLC, DATA[0-8 Byte] } */

  void    reset();
  uint8_t status();

private:

  int const _cs_pin;

  inline void init_cs () { pinMode(_cs_pin, OUTPUT); deselect(); }
  inline void init_spi() { SPI.begin(); }

  inline void select  () { digitalWrite(_cs_pin, LOW);  }
  inline void deselect() { digitalWrite(_cs_pin, HIGH); }

};

static_assert(sizeof(RxTxBuffer) == MCP2515_Io::TX_BUF_SIZE, "Union RxTxBuffer exceeds expected size of MCP2515_Io::TX/RX_BUF_SIZE bytes");

/**************************************************************************************
 * FREE FUNCTION DECLARATION
 **************************************************************************************/

void setBit(MCP2515_Io & io, Register const reg, uint8_t const bit_pos);
void clrBit(MCP2515_Io & io, Register const reg, uint8_t const bit_pos);

/**************************************************************************************
 * NAMESPACE
 **************************************************************************************/

} /* MCP2515 */

#endif /* MCP2515_MCP2515_IO_H_ */