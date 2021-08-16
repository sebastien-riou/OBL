"""Library to communicate with OBL"""
import io
import logging
import re
import pysatl

__version__ = '0.0.0'
__title__ = 'obl'
__description__ = 'Library to communicate with OBL'
__long_description__ = """
OBL is a minimalist boot loader. It is "open" as it gives access to everything
on the device.
"""
__uri__ = 'https://github.com/sebastien-riou/OBL'
__doc__ = __description__ + ' <' + __uri__ + '>'
__author__ = 'Sebastien Riou'
# For all support requests, please open a new issue on GitHub
__email__ = 'matic@nimp.co.uk'
__license__ = 'Apache 2.0'
__copyright__ = ''

class Obl(object):
    """ OBL main class

    Generic OBL host implementation. It interface to actual hardware via a
    "communication driver" which shall implement few functions.
    See :class:`SocketComDriver` for example.

    Args:
        is_master (bool): Set to ``True`` to be master, ``False`` to be slave
        com_driver (object): A SATL communication driver
        skip_init (bool): If ``True`` the initialization phase is skipped

    """

    @property
    def DATA_SIZE_LIMIT(self):
        """int: max data field length"""
        return 1<<16

    @property
    def LENLEN(self):
        """int: length in bytes of the length fields"""
        return 3

    @property
    def com(self):
        """object: Communication hardware driver"""
        return self._com

    @property
    def spy_frame_tx(self):
        """functions called to spy on each tx frame"""
        return self._spy_frame_tx

    @spy_frame_tx.setter
    def spy_frame_tx(self, value):
        self._spy_frame_tx = value

    @property
    def spy_frame_rx(self):
        """functions called to spy on each rx frame"""
        return self._spy_frame_rx

    @spy_frame_rx.setter
    def spy_frame_rx(self, value):
        self._spy_frame_rx = value

    def __init__(self,com_driver):
        self._com = com_driver
        self._spy_frame_tx = None
        self._spy_frame_rx = None
        self.frame_tx = ""
        self.frame_rx = ""
        self._receive(2) #consume initial prompt

    @staticmethod
    def ba(data):
        return pysatl.Utils.ba(data)

    def _send(self,str):
        self.frame_tx+=str
        self._com.tx(str.encode("utf-8"))

    def _receive(self,length):
        out=self._com.rx(length).decode("utf-8")
        self.frame_rx+=out
        return out

    def _send_cmd(self,cmd):
        self.frame_rx = ""
        self._send("%s"%cmd)

    def _send_addr(self,addr):
        self._send(" %x"%addr)

    def _send_len(self,length):
        self._send(" %x"%length)

    def _send_data(self,data):
        for b in data:
            self._send(" %02x"%b)

    def _send_cmd_commit(self):
        self._send("\n")
        if self._spy_frame_tx is not None:
            self._spy_frame_tx()

    def _receive_status(self):
        """consume status + trailing character"""
        self.frame_rx = ""
        str=""
        c=""
        while c not in [" ", "\n"]:
            str+=c
            c=self._receive(1)
            print("received '%s'"%c)
        return str

    def _receive_and_check_status(self):
        status = self._receive_status()
        if("ok"!=status):
            raise Exception("Unexpected status: '%s'"%status)

    def _receive_data(self,length):
        """consume data and trailing character"""
        str = self._receive(length*3)
        out=self.ba(str)
        assert(len(out)==length)
        return out

    def _receive_all(self):
        """consume fully the response"""
        resp=""
        c=""
        while(c != "\n"):
            resp += c
            c = self._receive(1)
        return resp

    def _receive_done(self):
        self._receive(1) #consume prompt
        if self._spy_frame_rx is not None:
            self._spy_frame_rx()

    def info(self):
        self._send_cmd("info")
        self._send_cmd_commit()
        self._receive_and_check_status()
        outstr = self._receive_all()
        self._receive_done()
        #regex = re.compile(r'''([\S]+=(?!\S+=)\S+)+''', re.VERBOSE)
        regex = re.compile(r'''([\S]+)=(.*)''', re.VERBOSE)
        out=dict()
        for pair in outstr.split(','):
            m = regex.findall(pair)
            print(m)
            assert(len(m)==1)
            print(m[0])
            out[m[0][0]] = m[0][1]
        #pairs = dict([match.split('=', 1) for match in matches])
        return out

    def read(self,*,size=1,address=0,access_width=8,loop_size=256):
        assert(access_width in [8,16,32,64])
        assert(self.DATA_SIZE_LIMIT >= loop_size)
        assert(0==(loop_size%(access_width//8)))
        loops = size // loop_size
        out=bytearray()
        def do_read_cmd(size):
            self._send_cmd("rd%02x"%access_width)
            self._send_addr(address)
            self._send_len(size)
            self._send_cmd_commit()
            self._receive_and_check_status()
            out = self._receive_data(size)
            self._receive_done()
            return out
        for i in range(0,loops):
            out+=do_read_cmd(loop_size)
            address+=loop_size
            size-=loop_size
        if size>0:
            out+=do_read_cmd(size)
        return out

    def write(self,*,data,address=0,access_width=8,loop_size=256):
        if data is not bytes:
            data=self.ba(data)
        def do_write_cmd(data):
            self._send_cmd("wr%02x"%access_width)
            self._send_addr(address)
            self._send_data(data)
            self._send_cmd_commit()
            self._receive_and_check_status()
            self._receive_done()
        assert(access_width in [8,16,32,64])
        assert(loop_size<=self.DATA_SIZE_LIMIT)
        assert(0==(loop_size%(access_width//8)))
        size = len(data)
        loops = size // loop_size
        for i in range(0,loops):
            do_write_cmd(data[i*loop_size:(i+1)*loop_size])
            address+=loop_size
            size-=loop_size
        if size>0:
            do_write_cmd(data[-size:])


    # def fill(self,val,size=1,address=0,access_width=8,loop_size=256):
    #     offset=self._set_base(address)
    #     valbytes=self.ba(val)
    #     vallen=len(valbytes)
    #     n=size // vallen
    #     dat = valbytes * n
    #     self.sbl_write(self.ser,dat[:size],offset,access_width,loop_size,verbose=self.verbose)
    #
    # def load_ihex(self,ihex,offset=0,access_width=8,loop_size=256):
    #     all_sections = ihex.segments()
    #     print("input hex file sections:")
    #     for sec in all_sections:
    #         print("0x%08X 0x%08X"%(sec[0],sec[1]-1))
    #     for sec in all_sections:
    #         dat=bytearray()
    #         for i in range(sec[0],sec[1]):
    #             dat.append(ihex[i])
    #         self.write(dat,address=sec[0]+offset)
    #
    # def verify_ihex(self,ihex,offset=0,access_width=8,loop_size=252):
    #     all_sections = ihex.segments()
    #     print("input hex file sections:")
    #     for sec in all_sections:
    #         print("0x%08X 0x%08X"%(sec[0],sec[1]-1))
    #     for sec in all_sections:
    #         dat=bytearray()
    #         for i in range(sec[0],sec[1]):
    #             dat.append(ihex[i])
    #         device_dat=self.read(len(dat),address=sec[0]+offset)
    #         assert(dat==device_dat)
    #
    # def exec(self,address,data=None,rxsize=0,waitack=True,waitstatus=True):
    #     offset=self._set_base(address)
    #     return self.sbl_exec(self.ser,offset,data,rxsize,waitack=waitack,waitstatus=waitstatus)
    #
    # def exit(self):
    #     return self.sbl_exit(self.ser)

    @staticmethod
    def newWithSocket(*,host='localhost',port=5000):
        import socket
        import fileinput
        import sys
        serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        serversocket.settimeout(None)
        serversocket.bind((host, port))
        print("Wait connection")
        serversocket.listen(1) # become a server socket, maximum 1 connections
        connection, address = serversocket.accept()
        assert(serversocket.gettimeout() == None)
        link = connection
        server_com = SocketComDriver(link)
        print("Server connected")
        server  = Obl(com_driver=server_com)
        print("Server init done")
        return server

class SocketComDriver(object):
    """Parameterized model for a communication peripheral and low level rx/tx functions

    Args:
        sock (socket): `socket` object used for communication
        bufferlen (int): Number of bytes that can be received in a row at max rate
        granularity (int): Smallest number of bytes that can be transported over the link
        ack (bool): if ``False``, :func:`tx_ack` and :func:`rx_ack` do nothing

    """
    def __init__(self,sock):
        self._sock = sock
        adapter=self.SocketAsStream(sock)
        self._impl = StreamComDriver(adapter)

    class SocketAsStream(object):
        def __init__(self,sock):
            self._sock=sock

        def write(self,data):
            self._sock.send(data)

        def read(self,length):
            return self._sock.recv(length)

    @property
    def sock(self):
        """socket: `socket` object used for communication"""
        return self._sock


    def tx(self,data):
        """Transmit data

        Args:
            data (bytes): bytes to transmit
        """
        self._impl.tx(data)

    def rx(self,length):
        """Receive data

        Args:
            length (int): length to receive

        Returns:
            bytes: received data
        """
        return self._impl.rx(length)


class StreamComDriver(object):
    """Parameterized model for a communication peripheral and low level rx/tx functions"""
    def __init__(self,stream):
        self._stream = stream

    @property
    def stream(self):
        """stream: `stream` object used for communication"""
        return self._stream

    def tx(self,data):
        """Transmit data

        Args:
            data (bytes): bytes to transmit
        """
        #print("tx ",data,flush=True)
        self._stream.write(data)

    def rx(self,length):
        """Receive data

        Args:
            length (int): length to receive

        Returns:
            bytes: received data
        """
        #print("rx %d "%length,end="",flush=True)
        data = bytearray()
        remaining = length
        #print("length=",length,flush=True)
        #print("remaining=",remaining,flush=True)
        while(remaining):
            #print("remaining=",remaining)
            #print("receive: ",end="")
            dat = self._stream.read(remaining)
            #print("received: ",Utils.hexstr(dat))
            if 0==len(dat):
                print(self._stream)
                #print(self._stream.timeout)
                raise Exception("Connection broken")
            data += dat
            remaining -= len(dat)
        return data

if __name__ == "__main__":
    import socket
    import fileinput
    import sys
    port = 5000
    if len(sys.argv)>1:
        port = int(sys.argv[1])
    serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    serversocket.settimeout(None)
    serversocket.bind(('localhost', port))
    print("Wait connection")
    serversocket.listen(1) # become a server socket, maximum 1 connections


    connection, address = serversocket.accept()
    assert(serversocket.gettimeout() == None)

    link = connection
    server_com = SocketComDriver(link)

    print("Server connected")
    server  = Obl(com_driver=server_com)
    print("Server init done")

    #for line in fileinput.input():
    #    line =  line.rstrip()
    #    if(line=='exit'):
    #        break
    prefer_IPython=True
    use_code=1
    if prefer_IPython:
        try:
            from IPython import embed
            embed()
            use_code=0
        except Exception as e:
            pass
    if use_code:
        import code
        code.interact("interactive (code.interact)", local={**globals(), **locals()})
