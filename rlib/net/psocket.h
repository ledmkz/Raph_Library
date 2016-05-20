/*
 *
 * Copyright (c) 2016 Project Raphine
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Author: Levelfour
 * 
 */

#ifndef __RAPH_LIB_NET_PSOCKET_H__
#define __RAPH_LIB_NET_PSOCKET_H__

#ifndef __KERNEL__

#include <stdio.h>
#include <stdint.h>
#include <buf.h>
#include <polling.h>
#include <functional.h>
#include <net/socket_interface.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

class PoolingSocket : public SocketInterface {
public:
  static const uint32_t kMaxPacketLength = 1460;
  struct Packet {
    int32_t len;
    uint8_t buf[kMaxPacketLength];
  };

  PoolingSocket() : _client(-1) {}
  virtual int32_t Open() override;
  virtual void SetReceiveCallback(int cpuid, const Function &func) override {
    _rx_buffered.SetFunction(cpuid, func);
  }

  void ReuseRxBuffer(Packet *packet) {
    kassert(_rx_reserved.Push(packet));
  }
  void ReuseTxBuffer(Packet *packet) {
    kassert(_tx_reserved.Push(packet));
  }
  bool GetTxPacket(Packet *packet) {
    if (_tx_reserved.Pop(packet)) {
      packet->len = 0;
      return true;
    } else {
      return false;
    }
  }
  bool GetRxPacket(Packet *&packet) {
    return _rx_reserved.Pop(packet);
  }
  bool TransmitPacket(Packet *packet) {
    return _tx_buffered.Push(packet);
  }
  bool ReceivePacket(Packet *&packet) {
    return _rx_buffered.Pop(packet);
  }

private:
  static const uint32_t kPoolDepth = 300;
  typedef RingBuffer<Packet *, kPoolDepth> PacketPoolRingBuffer;
  typedef FunctionalRingBuffer<Packet *, kPoolDepth> PacketPoolFunctionalRingBuffer;

  void InitPacketBuffer();
  void SetupPollingHandler();
  void Poll(void *arg);

  PacketPoolRingBuffer _tx_buffered;
  PacketPoolRingBuffer _tx_reserved;

  PacketPoolFunctionalRingBuffer _rx_buffered;
  PacketPoolFunctionalRingBuffer _rx_reserved;

  PollingFunc _polling;

  int _socket;
  int _client;
};

#endif // !__KERNEL__

#endif // __RAPH_LIB_NET_PSOCKET_H__
