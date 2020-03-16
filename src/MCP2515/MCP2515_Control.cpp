/**
 * @brief   Arduino library for controlling the MCP2515 in order to receive/transmit CAN frames.
 * @author  Alexander Entinger, MSc / LXRobotics GmbH
 * @license LGPL 3.0
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include "MCP2515_Control.h"

#include <Arduino.h>

#undef min
#undef max
#include <algorithm>

/**************************************************************************************
 * NAMESPACE
 **************************************************************************************/

namespace MCP2515
{

/**************************************************************************************
 * CTOR/DTOR
 **************************************************************************************/

MCP2515_Control::MCP2515_Control(MCP2515_Io & io)
: _io{io}
{

}

/**************************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 **************************************************************************************/

void MCP2515_Control::transmit(TxB const txb, uint32_t const id, uint8_t const * data, uint8_t const len)
{
  RxTxBuffer tx_buffer;

  bool const is_ext = (id & CAN_EFF_BITMASK) == CAN_EFF_BITMASK;
  bool const is_rtr = (id & CAN_RTR_BITMASK) == CAN_RTR_BITMASK;

  /* Load address registers */
  /*  ID[28:27] = EID[17:16]
   *  ID[26:19] = EID[15: 8]
   *  ID[18:11] = EID[ 7: 0]
   *  ID[10: 3] = SID[10: 3]
   *  ID[ 3: 0] = SID[ 3: 0]
   */
  tx_buffer.reg.sidl = static_cast<uint8_t>((id & 0x00000007) << 5);
  tx_buffer.reg.sidh = static_cast<uint8_t>((id & 0x000007F8) >> 3);
  if(is_ext)
  {
    tx_buffer.reg.sidl |= static_cast<uint8_t>((id & 0x18000000) >> 27);
    tx_buffer.reg.sidl |= bm(TXBnSIDL::EXIDE);
    tx_buffer.reg.eid0  = static_cast<uint8_t>((id & 0x0007F800) >> 11);
    tx_buffer.reg.eid8  = static_cast<uint8_t>((id & 0x07F80000) >> 19);
  }
  else
  {
    tx_buffer.reg.eid0  = 0;
    tx_buffer.reg.eid8  = 0;
  }

  /* Load data length register */
  tx_buffer.reg.dlc = is_rtr ? (len | (1 << static_cast<uint8_t>(TXBnDLC::RTR))) : len;

  /* Load data buffer */
  memcpy(tx_buffer.reg.data, data, std::min<uint8_t>(len, 8));

  /* Write to transmit buffer */
  _io.loadTxBuffer(txb, tx_buffer.buf);

  /* Request transmission */
  _io.requestTx(txb);
}

void MCP2515_Control::receive(RxB const rxb, uint32_t & id, uint8_t * data, uint8_t & len)
{
  RxTxBuffer rx_buffer;

  /* Read content of receive buffer */
  _io.readRxBuffer(rxb, rx_buffer.buf);

  /* Assemble ID from registers */
  id = (static_cast<uint32_t>(rx_buffer.reg.sidh) << 3) + (static_cast<uint32_t>(rx_buffer.reg.sidl) >> 5);

  if(rx_buffer.reg.sidl & bm(RXBnSIDL::IDE) == bm(RXBnSIDL::IDE))
  {
    id = (id << 2) + (rx_buffer.reg.sidl & 0x03);
    id = (id << 8) + rx_buffer.reg.eid8;
    id = (id << 8) + rx_buffer.reg.eid0;
    id |= CAN_EFF_BITMASK;
  }

  Register const ctrl_reg_addr = (rxb == RxB::RxB0) ? Register::RXB0CTRL : Register::RXB1CTRL;
  uint8_t const ctrl_reg_val = _io.readRegister(ctrl_reg_addr);
  if(ctrl_reg_val & bm(RXB0CTRL::RXRTR) == bm(RXB0CTRL::RXRTR))
  {
    id |= CAN_RTR_BITMASK;
  }

  /* Read amount of bytes received */
  len = rx_buffer.reg.dlc & 0x0F;

  /* Call registered callback with received data */
  memcpy(data, rx_buffer.reg.data, std::min<uint8_t>(len, 8));
}


/**************************************************************************************
 * NAMESPACE
 **************************************************************************************/

} /* MCP2515 */
