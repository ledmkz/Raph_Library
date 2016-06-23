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

#ifndef __KERNEL__

#include <net/psocket.h>
#include <mem/uvirtmem.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int32_t PoolingSocket::Open() {
  if((_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("");
    return _socket;
  } else {
    // turn on non-blocking mode
    int flag = fcntl(_socket, F_GETFL);
    fcntl(_socket, F_SETFL, flag | O_NONBLOCK);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(_port);
    bind(_socket, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr));

    FD_ZERO(&_fds);

    _timeout.tv_sec = 0;
    _timeout.tv_usec = 0;

    InitPacketBuffer();
    SetupPollingHandler();
    return 0;
  }
}

void PoolingSocket::InitPacketBuffer() {
  while(!_tx_reserved.IsFull()) {
    Packet *packet = reinterpret_cast<Packet *>(virtmem_ctrl->Alloc(sizeof(Packet)));
    kassert(_tx_reserved.Push(packet));
  }

  while(!_rx_reserved.IsFull()) {
    Packet *packet = reinterpret_cast<Packet *>(virtmem_ctrl->Alloc(sizeof(Packet)));
    kassert(_rx_reserved.Push(packet));
  }
}

void PoolingSocket::Poll(void *arg) {
  if (_client == -1) {
    listen(_socket, 1);
    if ((_client = accept(_socket, nullptr, nullptr)) > 0) {
      FD_SET(_client, &_fds);
    }
  }

  if (_client != -1) {
    Packet *packet;

    fd_set tmp_fds;
    memcpy(&tmp_fds, &_fds, sizeof(_fds));

    // receive packet (non-blocking)
    if (select(_client+1, &tmp_fds, 0, 0, &_timeout) >= 0) {
      if (FD_ISSET(_client, &tmp_fds)) {
        if (GetRxPacket(packet)) {
          int32_t rval = read(_client, packet->buf, kMaxPacketLength);
          if (rval > 0) {
            packet->len = rval;
            _rx_buffered.Push(packet);
          } else {
            if (rval == 0) {
              // socket may be closed by foreign host
              close(_client);
              _client = -1;
            }

            ReuseRxBuffer(packet);
          }
        }
      }
    }
  }
  
  if (_client != -1) {
    Packet *packet;

    // transmit packet (if non-sent packet remains in buffer)
    if (_tx_buffered.Pop(packet)) {
      int32_t rval = write(_client, packet->buf, packet->len);
      if (rval == packet->len) {
        ReuseTxBuffer(packet);
      }
    }
  }
}

void PoolingSocket::SetupPollingHandler() {
  ClassFunction<PoolingSocket> polling_func;
  polling_func.Init(this, &PoolingSocket::Poll, nullptr);
  _polling.Init(polling_func);
  _polling.Register(0);
}

#endif // !__KERNEL__
