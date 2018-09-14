#encoding = utf-8
import locale
import sys, gc, os
import _memimporter
from types import ModuleType
from importlib.abc import MetaPathFinder,Loader
from importlib.machinery import ModuleSpec
import http

modules = {}
__debug = False
__debug_level = 0
__debug_with_encrypt = True

current_exe_path = ''

def dprint(msg, level = 1):
    global __debug
    global __debug_level
    if __debug and level <= __debug_level:
        print(msg)

def dinput(lable,msg):
    global __debug
    global __debug_level
    if __debug and __debug_level:
        print(msg)
        input(lable)

def exception():
    import traceback
    exc_info = sys.exc_info()
    dprint(traceback.format_exception(*exc_info))
    del exc_info

class MySourceFileLoader(Loader):
    def __init__(self, contents, is_pkg, path, extension):
        self.contents = contents
        self.is_pkg = is_pkg
        self.path = path
        self.extension = extension

    def create_module(self, spec):
        dprint('[Loader Info] source file loader creating module {}'.format(spec.name))

        m = ModuleType(spec.name)
        m.__file__ = '{}'.format(self.path)
        if self.is_pkg:
            m.__path__ = [m.__file__.rsplit('/', 1)[0]]
            m.__package__ = spec.name
        else:
            m.__package__ = spec.name.rsplit('.', 1)[0]

        return m

    def exec_module(self, module):
        dprint('[Loader Info] source file loader executing module {}'.format(module.__name__))

        try:
            if self.extension == 'py':
                code = compile(self.contents, module.__file__, 'exec')
                exec(code, module.__dict__)
            else:
                exec (marshal.loads(self.contents[8:]), mod.__dict__)
            dprint("[Loader Info] loaded module:{}".format(module.__name__))
        except Exception as e:
            dprint('@!@[Loader Error] Failed to exec {}: {}'.format(module.__name__, e))
            dinput('>>>',module.__name__)
            raise e

class MyExtensionLoader(Loader):
    def __init__(self, extension):
        self.extension = extension

    def create_module(self, spec):
        dprint('[Loader Info] extension loader creating module {}'.format(spec.name))

        try:
            init_func_name = "PyInit_" + spec.name.rsplit('.', 1)[-1]
            path = os.path.join(current_exe_path, 'pylib')
            if spec.name in ( 'pywintypes', 'pythoncom' ):
                path = os.path.join(path, spec.name + "36." + self.extension)
            else:
                path = os.path.join(path, spec.name.rsplit('.', 1)[-1] + 
                    '.' + self.extension)
            m = _memimporter.import_module(init_func_name, spec.name, path, spec)
            dprint("[Loader Info] loaded {} done.\n\n".format(init_func_name))

        except Exception as e:
            dprint('@!@[Loader Error] extension loader create {} failed: {}'.format(spec.name, e))
            return None

        else:
            return m

    def exec_module(self, module):
        pass

class MyPackageFinder(MetaPathFinder):
    def __init__(self, path = None):
        dprint("[Finder Info] init finder")

    def find_spec(self, fullname, path, target = None):
        global modules

        dprint("[Finder Info] finding module, fullname: {}, path: {}".format(fullname, path), 0)

        try:
            filepath, content = get_module_files(fullname)

            if filepath is None:
                if fullname.startswith('win32com.'):
                    return self.find_spec("win32comext."+fullname.split('.',1)[1],None)

                filepath = get_module_namespace(fullname)
                if filepath is None:
                    dprint("[Finder Error] can't find module:{}".format(fullname))
                    dinput('>', fullname)
                    return None
                else:
                    spec = ModuleSpec(fullname, None)
                    spec.submodule_search_locations = [filepath]
                    dinput('>>', filepath)
                    return spec

            extension = filepath.rsplit(".",1)[1].strip().lower()
            is_pkg = filepath.endswith('/__init__.py')

            dprint('[Finder Info] loading module fullname: {}, filepath: {}, extension: {}, is_pkg: {}'.format(fullname, filepath, extension, is_pkg))

            if extension in ("py", 'pyc', 'pyo'):
                dprint('[Finder Info] file len:{:d}'.format(len(content)))
                return ModuleSpec(fullname, MySourceFileLoader(content, is_pkg, filepath, extension))
            elif extension in ("pyd", "so", "dll"):
                return ModuleSpec(fullname, MyExtensionLoader(extension))

        except Exception as e:
            dprint('@!@[Finder Error] Failed to load {}: {}'.format(fullname, e))
            raise e

        finally:
            # If a module loaded, delete it from modules to save memory
            # Don't delete network.conf module
            if filepath and filepath in modules and not filepath.startswith('network/conf'):
                dprint('[Finder Info] {} remove {} from bundle / count = {}'.format(fullname, filepath, len(modules)))
                del modules[filepath]

def get_module_files(filename):
    global modules

    path = filename.replace('.', '/')
    files = [
        module for module in modules.keys() \
        if module.rsplit(".", 1)[0] == path or path + '/__init__.py' == module
    ]

    if len(files) == 0:
        return None, None
    elif len(files) == 1:
        return files[0], modules[files[0]]
    else:
        # criterias have priority, be careful
        criterias = [
            lambda f: any([
                f.endswith('/__init__'+ext) for ext in [
                    '.py', '.pyo', '.pyc', 
                ]
            ]),
            lambda f: any ([
                f.endswith(ext) for ext in [
                    '.pyo', '.pyc'
                ]
            ]),
            lambda f: any ([
                f.endswith(ext) for ext in [
                    '.py', '.pyd', '.so', '.dll'
                ]
            ]),
            ]

        for criteria in criterias:
            for pyfile in files:
                if criteria(pyfile):
                    return pyfile, modules[pyfile]

        return None, None

def get_module_namespace(fullname):
    global modules

    path = fullname.replace('.', '/')

    for mod in modules.keys():
        if mod.startswith(path):
            return path

    return None

def decrypt_library(library_data):
    if not __debug_with_encrypt:
        return library_data

    key = bytes.fromhex('1794XXXX19985XXXXee89f9bdffac8dcb3676e2555db9469384493d14708aed5')
    iv = library_data[:16]
    decrypt_data = _memimporter.aes_cbc_decrypt(library_data[16:], key, iv)
    decrypt_data = decrypt_data[:-ord(decrypt_data[len(decrypt_data)-1:])]
    print(len(decrypt_data))
    return decrypt_data

def init_encrypted_modules(current_exe_path):
    global modules
    import zipfile, io

    with open(os.path.join(current_exe_path, "library.zip"), "rb") as f:
        library_data = f.read()

    library_data = decrypt_library(library_data)
    library_io = io.BytesIO(library_data)

    # compression can be ZIP_DEFLATED, ZIP_BZIP2 or ZIP_LZMA
    with zipfile.ZipFile(library_io, compression = zipfile.ZIP_DEFLATED) as zip:
        modules = dict([
                    (z.filename, zip.open(z.filename).read()) for z in zip.infolist()
                    ])

def initial():
    gc.set_threshold(128)

    sys.meta_path.pop()
    sys.meta_path.append(MyPackageFinder())
    sys.path = []
    sys.path_hooks = []
    sys.path_importer_cache.clear()


if __name__ == '__main__':
    current_exe_path = os.path.abspath(os.path.join(os.path.dirname(os.__file__),'..'))
    sys.frozen = 1
    dprint("executable:{}".format(sys.executable))

    try:
        init_encrypted_modules(current_exe_path)
        initial()
        import init
        init.initAll(test = False)
    except Exception as e:
        print(e)
        _memimporter.messagebox("文件已损坏!".encode(locale.getdefaultlocale()[1]))
    pass # nothing ,just remark as end.