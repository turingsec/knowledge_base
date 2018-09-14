import sys
import os
import zipfile
import compileall,imp, gc, traceback,io
import shutil
sys.path.insert(0, './project')
import start
import additional_import

sys_modules = [
    (x,sys.modules[x]) for x in sys.modules.keys()
]

all_dependencies=set(
    [
        x.split('.')[0] for x,m in sys_modules \
            if not '(built-in)' in str(m) and x != '__main__'
    ]
)

debug = False
debug_with_encrypt = True
encrypt_py_file = True

def nop(*args):
    pass

if debug:
    dprint = print
else:
    dprint = nop

all_dependencies = sorted(list(set(all_dependencies)))
if sys.platform == 'win32':
    all_dependencies.append('win32comext')

dprint(all_dependencies)

import pyencrypt as encrypt_py
try:
    shutil.rmtree("pylib")
except:
    pass
os.mkdir("pylib")

zf = zipfile.ZipFile('.\\library_source.zip', mode='w', compression=zipfile.ZIP_DEFLATED)
try:
    content = set()

    for dep in all_dependencies:
        mdep = __import__(dep)
        dprint("DEPENDENCY: ", dep, mdep)
        if hasattr(mdep, '__path__') and getattr(mdep, '__path__'):
            dprint('adding package %s / %s'%(dep, mdep.__path__))
            path, root = os.path.split(list(mdep.__path__)[0])
            for root, dirs, files in os.walk(list(mdep.__path__)[0]):
                for f in list(set([x.rsplit('.',1)[0] for x in files])):
                    found=False
                    for ext in ('.dll', '.pyd', '.so', '.py'):
                        if ( ext == '.py' or ext == '.pyc' ) and found:
                            continue

                        pypath = os.path.join(root,f+ext)
                        if os.path.exists(pypath):
                            if ext == '.py':
                                try:
                                    compileall.compile_file(os.path.relpath(pypath))
                                except ValueError:
                                    compileall.compile_file(pypath)
                                except:
                                    continue
                            zipname = '/'.join([root[len(path)+1:], f.split('.', 1)[0] + ext])
                            zipname = zipname.replace('\\', '/')
                            found=True

                            if zipname.startswith('network/transports/') and \
                              not zipname.startswith('network/transports/__init__.py'):
                                continue

                            # Remove various testcases if any
                            if any([ '/'+x+'/' in zipname for x in [
                                'tests', 'test', 'SelfTest', 'SelfTests', 'examples',
                                'experimental'
                                ]
                            ]):
                                continue

                            if zipname in content:
                                continue

                            dprint('adding file : {}'.format(zipname))
                            content.add(zipname)
                            
                            if encrypt_py_file and ext == '.py':
                                encData = encrypt_py.encrypt(os.path.join(root,f+ext))
                                zf.writestr(zipname, encData)
                            else:
                                dprint(os.path.join(root,f+ext))
                                if ext == '.dll' and "PyQt" in root:
                                    print("igore",os.path.join(root,f+ext))
                                    continue
                                if ext in ('.dll', '.pyd'):
                                    name = f
                                    if '.cp36-win32' in f:
                                        name = f[:f.index('.cp36-win32')]
                                    if dep in ('pywintypes', 'pythoncom'):
                                        name = dep + '36'
                                    shutil.copy(os.path.join(root,f+ext),os.path.join("pylib",name+ext))
                                    zf.writestr(zipname, '')
                                else:
                                    zf.write(os.path.join(root,f+ext), zipname)

        else:
            found_patch = None
            ext = ''
            try:
                _, ext = os.path.splitext(mdep.__file__)
            except:
                continue
            filepath = mdep.__file__
            if dep+ext in content:
                continue
            if ext in ('.pyc','.pyo'):
                ext = '.py'
                filepath = mdep.__file__[:-1]
                dprint('adding %s -> %s'%(filepath, dep+ext))
                input('warning:not a py file\nenter to continue...')
            elif ext in ('.pyd','.dll'):
                dprint('add %s -> %s '%(filepath, dep+ext))
                name = dep
                if '.cp36-win32' in dep:
                    name = dep[:dep.index('.cp36-win32')]
                if dep in ('pywintypes', 'pythoncom'):
                    name = dep + '36'
                zf.writestr(dep+ext, '')
                shutil.copy(filepath,os.path.join("pylib", name+ext))
                continue
            else:
                dprint("adding py", mdep.__file__)
            
            if encrypt_py_file and ext == '.py':
                encData = encrypt_py.encrypt(filepath)
                zf.writestr(dep+ext, encData)
            else:
                zf.write(filepath, dep+ext)

finally:
    zf.close()

from Crypto.Cipher import AES
from Crypto import Random

class AESCipher(object):

    def __init__(self): 
        self.bs = 32
        self.key = bytes.fromhex('1794XXXX19985XXXXee89f9bdffac8dcb3676e2555db9469384493d14708aed5')

    def encrypt(self, raw):
        raw = self._pad(raw)
        iv = Random.new().read(AES.block_size)
        cipher = AES.new(self.key, AES.MODE_CBC, iv)
        return iv + cipher.encrypt(raw)

    def decrypt(self, enc):
        iv = enc[:AES.block_size]
        cipher = AES.new(self.key, AES.MODE_CBC, iv)
        return self._unpad(cipher.decrypt(enc[AES.block_size:]))

    def _pad(self, s):
        print((self.bs - len(s) % self.bs))
        return s + (self.bs - len(s) % self.bs) * chr(self.bs - len(s) % self.bs).encode('ascii')

    @staticmethod
    def _unpad(s):
        return s[:-ord(s[len(s)-1:])]

def encrypt():
    aes = AESCipher()
    with open('.\\library_source.zip','rb') as f:
        data = f.read()

    print(len(data))
    import codecs
    print(codecs.encode(data[:32],'hex'))
    encData = aes.encrypt(data)
    print(len(encData))
    with open('.\\library.zip','wb') as f:
        print("encrypt done")
        f.write(encData)


if debug_with_encrypt:
    encrypt()
print("Done")