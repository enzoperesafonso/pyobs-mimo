import asyncio
from typing import Union, List, Tuple
from aioserial import AioSerial

class MicroMONET:
    def __init__(self, device: str = "/dev/ttyS0", baudrate: int = 9600):
        self._aioserial = AioSerial(device, baudrate=baudrate)


