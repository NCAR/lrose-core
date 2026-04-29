#!/usr/bin/env python3

import argparse
import asyncio
import signal
from typing import Set

from websockets.asyncio.client import connect


class WsToTcpBridge:
    """
    Bridge ASCII newline-delimited messages from a WebSocket source
    to one or more plain TCP clients.

    WebSocket side:
      - Connects as a client to ws://... or wss://...

    TCP side:
      - Opens a local TCP server.
      - Any connected TCP client receives each aircraft message.
    """

    def __init__(
        self,
        websocket_url: str,
        tcp_host: str,
        tcp_port: int,
        reconnect_delay_secs: float = 5.0,
    ):
        self.websocket_url = websocket_url
        self.tcp_host = tcp_host
        self.tcp_port = tcp_port
        self.reconnect_delay_secs = reconnect_delay_secs

        self.tcp_clients: Set[asyncio.StreamWriter] = set()
        self.n_messages_forwarded = 0

        self.stop_event = asyncio.Event()

    async def start(self) -> None:
        server = await asyncio.start_server(
            self._handle_tcp_client,
            self.tcp_host,
            self.tcp_port,
        )

        sockets = server.sockets or []
        for sock in sockets:
            print(f"Listening for TCP clients on {sock.getsockname()}")

        websocket_task = asyncio.create_task(self._websocket_loop())

        async with server:
            await self.stop_event.wait()

        websocket_task.cancel()

        await self._close_all_tcp_clients()

    async def stop(self) -> None:
        self.stop_event.set()

    async def _handle_tcp_client(
        self,
        reader: asyncio.StreamReader,
        writer: asyncio.StreamWriter,
    ) -> None:
        peer = writer.get_extra_info("peername")
        print(f"TCP client connected: {peer}")

        self.tcp_clients.add(writer)

        try:
            # We do not expect commands from the TCP client, but reading
            # lets us detect when the client disconnects.
            while not self.stop_event.is_set():
                data = await reader.read(1024)
                if data == b"":
                    break

        except ConnectionError:
            pass

        finally:
            print(f"TCP client disconnected: {peer}")
            self.tcp_clients.discard(writer)

            writer.close()
            try:
                await writer.wait_closed()
            except ConnectionError:
                pass

    async def _websocket_loop(self) -> None:
        while not self.stop_event.is_set():

            try:
                print(f"Connecting to WebSocket: {self.websocket_url}")

                async with connect(self.websocket_url) as websocket:
                    print("WebSocket connected")

                    async for message in websocket:
                        line = self._message_to_ascii_line(message)
                        await self._broadcast_to_tcp_clients(line)

            except asyncio.CancelledError:
                raise

            except Exception as e:
                print(f"WebSocket error/disconnect: {e}")

            if not self.stop_event.is_set():
                print(
                    f"Reconnecting in {self.reconnect_delay_secs:.1f} seconds"
                )
                await asyncio.sleep(self.reconnect_delay_secs)

    def _message_to_ascii_line(self, message) -> bytes:
        """
        Convert a WebSocket message to one ASCII newline-delimited TCP line.

        If the WebSocket message is text, encode as ASCII.
        If it is bytes, pass it through.
        """

        if isinstance(message, str):
            data = message.encode("ascii", errors="replace")
        else:
            data = bytes(message)

        if len(data) == 0 or not data.endswith(b"\n"):
            data += b"\n"

        return data

    async def _broadcast_to_tcp_clients(self, data: bytes) -> None:
        if not self.tcp_clients:
            return

        dead_clients = []

        for writer in list(self.tcp_clients):
            try:
                writer.write(data)
                await writer.drain()

            except ConnectionError:
                dead_clients.append(writer)

        for writer in dead_clients:
            self.tcp_clients.discard(writer)
            writer.close()
            try:
                await writer.wait_closed()
            except ConnectionError:
                pass

        self.n_messages_forwarded += 1

        print(
            f"Forwarded message {self.n_messages_forwarded}: "
            f"{len(data)} bytes to {len(self.tcp_clients)} TCP client(s)"
        )

    async def _close_all_tcp_clients(self) -> None:
        for writer in list(self.tcp_clients):
            writer.close()
            try:
                await writer.wait_closed()
            except ConnectionError:
                pass

        self.tcp_clients.clear()


async def main() -> None:
    parser = argparse.ArgumentParser(
        description="Bridge newline-delimited ASCII WebSocket messages to TCP."
    )

    parser.add_argument(
        "websocket_url",
        help="Input WebSocket URL, e.g. ws://example.com:8080/telemetry",
    )

    parser.add_argument(
        "tcp_port",
        type=int,
        help="Local TCP listen port, e.g. 5000",
    )

    parser.add_argument(
        "--tcp-host",
        default="0.0.0.0",
        help="Local TCP listen address. Default: 0.0.0.0",
    )

    parser.add_argument(
        "--reconnect-delay",
        type=float,
        default=5.0,
        help="Seconds to wait before reconnecting the WebSocket. Default: 5",
    )

    args = parser.parse_args()

    bridge = WsToTcpBridge(
        websocket_url=args.websocket_url,
        tcp_host=args.tcp_host,
        tcp_port=args.tcp_port,
        reconnect_delay_secs=args.reconnect_delay,
    )

    loop = asyncio.get_running_loop()

    for signame in ("SIGINT", "SIGTERM"):
        try:
            loop.add_signal_handler(
                getattr(signal, signame),
                lambda: asyncio.create_task(bridge.stop()),
            )
        except NotImplementedError:
            # Some platforms, notably Windows, do not support this API.
            pass

    await bridge.start()


if __name__ == "__main__":
    asyncio.run(main())

