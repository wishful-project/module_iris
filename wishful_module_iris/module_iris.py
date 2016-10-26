import logging
import wishful_upis as upis
import wishful_framework as wishful_module

import socket
import subprocess

import threading
import queue
import select

__author__ = "Nicholas Kaminski, Maicon Kist, Alextian Liberato"
__copyright__ = "Copyright (c) 2016, Trinity College Dublin"
__version__ = "0.0.1"
__email__ = "kaminskn@tcd.ie, kistm@tcd.ie, liberata@tcd.ie"

# Class that directly sends commands to Iris
class IrisInterface:
    def __init__(self,
                 ip='127.0.0.1',
                 port_server=4004,
                 port_client=4005,
                 DEBUG=False):
        self.log = logging.getLogger('iris_module.interface.main')
        self.name = "iris_socket"
        self.ip = ip
        self.port_server = port_server
        self.port_client = port_client
        self.rx_buffer = 64
        self.DEBUG = DEBUG

        self._stop_event = threading.Event()
        self._select_queue = queue.Queue()
        self.tx_count = 0
        self.rx_count = 0

    def _receive_loop(self, server_sock):
        conn_list = [server_sock]
        server_sock.listen(1)
        select_ptr = select.select
        while not(self._stop_event.is_set()):
            select_thread = threading.Thread(target=self._socket_select,
                    args=(conn_list, self._select_queue, select_ptr),
                    name="select_thread")
            select_thread.setDaemon(True)
            select_thread.start()
            read_socks = self._select_queue.get(True)
            for sock in read_socks:
                if sock == server_sock:
                    # New connection
                    new_sock, addr = sock.accept()
                    conn_list.append(new_sock)
                else:
                    try:
                        data = sock.recv(self.rx_buffer)
                        self.handle(data)
                    except:
                        sock.close()
                        conn_list.remove(sock)

        for sock in conn_list:
            sock.close()

    def _socket_select(self, conn_list, output, select_ptr):
        read_socks, write_socks, error_socks = select_ptr(conn_list, [], [])
        output.put(read_socks)

    def send_command(self, command_type, engine, component, parameter,
                     value=None):
        ''' This function sends a string to forge controller(Iris radio)
        to change the parameter in  the component.
        Example:
            In radio.xml file we have usrptx component:

            <softwareradio name="Radio1">
                <controller class="forgecontroller">
                </controller>
                <engine name="phyengine1" class="phyengine">
                        <component name="usrptx1" class="usrptx">
                            <parameter name="frequency" value="2400000000"/>
                            <parameter name="rate" value="500000"/>
                            <parameter name="fixlooffset" value="2500000"/>
                            <parameter name="antenna" value="TX/RX"/>
                            <parameter name="gain" value="30"/>
                            <parameter name="numzerosamps" value="50"/>
                            <port name="input1" class="input"/>
                        </component>
                </engine>

            To change the frequency to 2,8GHz use:

            send_command(phyengine1.usrptx1.frequency=2800000000)
        '''
        
        msg = command_type + ':' + engine + '.' + component + '.' + parameter
        if command_type == 'set':
            msg += '=' + str(value)

        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        try:
            s.connect((self.ip, self.port_client))
        except:
            logging.getLogger('iris_module.main').error("Connection Failed: \
                    could not send command to Iris")
            return

        s.send(msg.encode())
        
        logging.getLogger('iris_module.main').debug("Socket sent: %s" % msg)

        response = s.recv(4).decode("utf-8")
        
        logging.getLogger('iris_module.main').debug("Socket received: %s"
                                                    % response)

        s.close()
        return response

    def handle(self, data):
        x = data[0:1]
        if x == '1':
            self.tx_count += 1
        elif x == '2':
            self.rx_count += 1

    def __enter__(self):
        return self

    def __exit__(self, *args):
        self.shutdown()

    def shutdown(self):
        self._stop_event.set()
        self._select_queue.put_nowait([])
        self._rx_thread.join()


@wishful_module.build_module
class IrisModule(wishful_module.AgentModule):
    def __init__(self):
        super(IrisModule, self).__init__()
        self.log = logging.getLogger('iris_module.main')

        self.iris = IrisInterface(port_client=1235)
        self.param_map = self.parse_map('ofdm.csv')

        print ("IrisModule up and running. Connected to ip: %s, port: %s"
               % (self.iris.ip, self.iris.port_client))

    def generic_interact(self, command_type, name, value=0):
        '''
        Function that translates the configuration from wishful to iris
        @param command_type Set/Get
        @param name Name of the parameter that we want to change (from the cvs file)
        @param value Value of the parameter. Ignored when command_type == 'get'
        '''
        try:
            engine, component, parameter = self.param_map[name]
        except KeyError:
            msg = 'No map available for %s' % name
            self.log.debug(msg)
            raise KeyError(msg)

        self.log.debug("Sending cmd to IrisInterface")
        return self.iris.send_command(command_type, engine,
                                      component, parameter, value)


    def parse_map(self, filename):
        '''
        Parse the CSV  that exposes the parameters that can be configured in the IRIS waveform
        The FIRST line of the CSV file MUST be IDENTICAL TO THE LINE BELOW:
            engine,component,parameter,name

        The following lines are as follows: Replace accordingly:
            engine1,component1,parameter1, name_exposed
            engine2,component2,parameter2, name_exposed
            ...

        Example from ofdm.csv:
            engine,component,parameter,name
            phyengine1,usrprx1,frequency,frequency
            . . . . . . . . . . . . . . . . . . . . .
            phyengine1,usrprx1,rate,rate
            phyengine1,usrprx1,gain,gain
            phyengine1,usrprx1,bandwidth,bandwidth
        '''
        FIRST_LINE = 'engine,component,parameter,name'
        lines = []
        param_map = {}

        # read all lines at once
        with open(filename) as f:
            lines = f.readlines()

        # split on ',', remove extra whitespaces
        keys = lines.pop(0)
        if keys.strip() != FIRST_LINE:
            self.log.debug("%s not formatted correctly: Header must be '%s'" %
                           (filename, FIRST_LINE))
            return

        # for each remaining line
        for line in lines:
            if len(line) > 0:
                vals = line.split(',')

                if len(vals) != len(FIRST_LINE.split(',')):
                    self.log.debug("%s not formatted correctly: missing values"
                                   % filename)
                    return

                line_dict = {}
                for idx, key in enumerate(FIRST_LINE.split(',')):
                    line_dict[key] = vals[idx].strip()

                param_map[line_dict['name']] = [line_dict[key]
                            for key in ['engine', 'component', 'parameter']]
        return param_map

    @wishful_module.bind_function(upis.radio.set_frequency)
    def set_frequency(self, frequency):
        self.log.debug("IrisModule set_frequency: %s on interface: %s"
                       % (frequency, self.interface))

        return self.generic_interact('set', 'frequency', frequency)

    @wishful_module.bind_function(upis.radio.set_rate)
    def set_rate(self, rate):
        self.log.debug("IrisModule set_frequency: %s on interface: %s"
                       % (rate, self.interface))

        return self.generic_interact('set', 'rate', rate)

    @wishful_module.bind_function(upis.radio.set_gain)
    def set_gain(self, gain):
        self.log.debug("IrisModule set_frequency: %s on interface: %s"
                       % (gain, self.interface))

        return self.generic_interact('set', 'gain', gain)
        
    @wishful_module.bind_function(upis.radio.set_bandwidth)
    def set_bandwidth(self, bandwidth):
        self.log.debug("IrisModule set_frequency: %s on interface: %s"
                       % (bandwidth, self.interface))

        return self.generic_interact('set', 'bandwidth', bandwidth)        
