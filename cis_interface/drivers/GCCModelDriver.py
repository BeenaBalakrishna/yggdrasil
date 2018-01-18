import os
from cis_interface import platform
from cis_interface.communication import _default_comm
from cis_interface.tools import (
    _zmq_installed, _ipc_installed, locate_path, popen_nobuffer, print_encoded)
from cis_interface.drivers.ModelDriver import ModelDriver


_top_dir = os.path.normpath(os.path.join(os.path.dirname(__file__), '../'))
_incl_interface = os.path.join(_top_dir, 'interface')
_incl_io = os.path.join(_top_dir, 'io')
_incl_seri = os.path.join(_top_dir, 'serialize')
_incl_comm = os.path.join(_top_dir, 'communication')
_regex_win32_lib = os.path.join(_top_dir, 'regex_win32.lib')


_linker_flags = []
_compile_flags = []
if platform._is_win:
    _regex_win32 = os.path.split(_regex_win32_lib)
    _compile_flags += ["-D_CRT_SECURE_NO_WARNINGS", "-I" + _regex_win32[0]]
    _linker_flags += [_regex_win32[1], '/LIBPATH:"%s"' % _regex_win32[0]]
if _zmq_installed:
    if platform._is_win:
        _zmq_dirs = dict()
        for l in ["zmq", "czmq"]:
            for ext in ["h", "lib"]:
                f = "%s.%s" % (l, ext)
                p = locate_path(f)
                if not p:
                    raise Exception("Could not locate %s." % f)
                _zmq_dirs[f] = os.path.dirname(p)
                if ext == "h":
                    _compile_flags.append("-I%s" % _zmq_dirs[f])
                elif ext == "lib":
                    _linker_flags += [f, '/LIBPATH:"%s"' % _zmq_dirs[f]]
    else:
        _linker_flags += ["-lczmq", "-lzmq"]
    _compile_flags += ["-DZMQINSTALLED"]
if _ipc_installed:
    _compile_flags += ["-DIPCINSTALLED"]
for x in [_incl_interface, _incl_io, _incl_comm, _incl_seri]:
    _compile_flags += ["-I" + x]
if _default_comm == 'IPCComm':
    _compile_flags += ["-DIPCDEF"]


def build_regex_win32():
    r"""Build the regex_win32 library."""
    _regex_win32_dir = os.path.dirname(_regex_win32_lib)
    _regex_win32_cpp = os.path.join(_regex_win32_dir, 'regex_win32.cpp')
    _regex_win32_obj = os.path.join(_regex_win32_dir, 'regex_win32.obj')
    # Compile object
    cmd = ['cl', '/c', '/Zi', '/EHsc',
           '/I', '%s' % _regex_win32_dir, _regex_win32_cpp]
    # '/out:%s' % _regex_win32_obj,
    comp_process = popen_nobuffer(cmd, cwd=_regex_win32_dir)
    output, err = comp_process.communicate()
    exit_code = comp_process.returncode
    if exit_code != 0:  # pragma: debug
        print(' '.join(cmd))
        print_encoded(output, end="")
        raise RuntimeError("Could not create regex_win32.obj")
    assert(os.path.isfile(_regex_win32_obj))
    # Create library
    cmd = ['lib', '/out:%s' % _regex_win32_lib, _regex_win32_obj]
    comp_process = popen_nobuffer(cmd, cwd=_regex_win32_dir)
    output, err = comp_process.communicate()
    exit_code = comp_process.returncode
    if exit_code != 0:  # pragma: debug
        print(' '.join(cmd))
        print_encoded(output, end="")
        raise RuntimeError("Could not build regex_win32.lib")
    assert(os.path.isfile(_regex_win32_lib))


if platform._is_win and (not os.path.isfile(_regex_win32_lib)):
    build_regex_win32()


class GCCModelDriver(ModelDriver):
    r"""Class for running gcc compiled drivers.

    Args:
        name (str): Driver name.
        args (str or list): Argument(s) for running the model on the command
            line. If the first element ends with '.c', the driver attempts to
            compile the code with the necessary interface include directories.
            Additional arguments that start with '-I' are included in the
            compile command. Others are assumed to be runtime arguments.
        cc (str, optional): C/C++ Compiler that should be used. Defaults to
            gcc for '.c' files, and g++ for '.cpp' or '.cc' files on Linux or
            OSX. Defaults to cl on Windows.
        **kwargs: Additional keyword arguments are passed to parent class.

    Attributes (in additon to parent class's):
        compiled (bool): True if the compilation was succesful. False otherwise.
        cfile (str): Source file.
        cc (str): C/C++ Compiler that should be used.
        flags (list): List of compiler flags.
        efile (str): Compiled executable file.

    Raises:
        RuntimeError: If the compilation fails.

    """

    def __init__(self, name, args, cc=None, **kwargs):
        super(GCCModelDriver, self).__init__(name, args, **kwargs)
        self.debug()
        self.cc = cc
        # Prepare arguments to compile the file
        self.parse_arguments(args)
        compile_args = [self.cc]
        if not platform._is_win:
            compile_args += ["-o", self.efile]
        compile_args += self.src + self.ccflags
        if platform._is_win:
            compile_args += ['/link', '/out:%s' % self.efile]
        compile_args += self.ldflags
        if os.path.isfile(self.efile):
            os.remove(self.efile)
        if platform._is_win:
            self.args = [os.path.splitext(self.efile)[0]]
        else:
            self.args = [os.path.join(".", self.efile)]
        self.args += self.run_args
        self.compiled = True
        self.debug("Compiling")
        comp_process = popen_nobuffer(compile_args)
        output, err = comp_process.communicate()
        exit_code = comp_process.returncode
        if exit_code != 0:  # pragma: debug
            self.compiled = False
            self.error('%s', ' '.join(compile_args))
            self.print_encoded(output, end="")
            raise RuntimeError("Compilation failed with code %d." % exit_code)
        assert(os.path.isfile(self.efile))
        # self.print_encoded(output, end="")
        self.compiled = True
        self.debug('Compiled executable with %s', self.cc)

    def parse_arguments(self, args):
        r"""Sort arguments based on their syntax. Arguments ending with '.c' or
        '.cpp' are considered source and the first one will be compiled to an
        executable. Arguments starting with '-L' or '-l' are treated as linker
        flags. Arguments starting with '-' are treated as compiler flags. Any
        arguments that do not fall into one of the categories will be treated
        as a command line argument for the executable.

        Args:
            args (list): List of arguments provided.

        Raises:
            RuntimeError: If there is not a valid source file in the argument
                list.

        """
        if isinstance(args, str):
            args = [args]
        self.src = []
        self.ldflags = _linker_flags
        if platform._is_win:
            self.ccflags = ['/W4']
        else:
            self.ccflags = ['-g', '-Wall']
        self.ccflags += _compile_flags
        self.ccflags.append('-DCIS_DEBUG=%d' % self.logger.getEffectiveLevel())
        self.run_args = []
        self.efile = None
        is_object = False
        is_link = False
        for a in args:
            if os.path.splitext(a)[1] in ['.c', '.cpp', '.cc']:
                self.src.append(a)
            elif a.startswith('-l') or a.startswith('-L') or is_link:
                if a.startswith('/out:'):
                    self.efile = a[5:]
                elif a.startswith('-l') or a.startswith('-L') and platform._is_win:
                    a1 = '/LIBPATH:"%s"' % a[2:]
                    if a1 not in self.ldflags:
                        self.ldflags.append(a1)
                elif a not in self.ldflags:
                    self.ldflags.append(a)
            elif a == '-o':
                # Next argument should be the name of the executable
                is_object = True
            elif a == '/link':
                # Following arguments should be linker options
                is_link = True
            elif a.startswith('-') or (platform._is_win and a.startswith('/')):
                if a not in self.ccflags:
                    self.ccflags.append(a)
            else:
                if is_object:
                    # Previous argument was -o flag
                    self.efile = a
                    is_object = False
                else:
                    self.run_args.append(a)
        # Check source file
        if len(self.src) == 0:
            raise RuntimeError("Could not locate a source file in the " +
                               "provided arguments.")
        # Change format for path (windows compat of examples)
        if platform._is_win:
            for i in range(len(self.src)):
                self.src[i] = os.path.join(*(self.src[i].split('/')))
        # Get pimary file
        self.cfile = self.src[0]
        src_base, src_ext = os.path.splitext(self.cfile)
        # Select compiler
        if self.cc is None:
            if platform._is_win:
                self.cc = 'cl'
            else:
                if src_ext in '.c':
                    self.cc = 'gcc'
                else:
                    self.cc = 'g++'
        # Create/fix executable
        if self.efile is None:
            if platform._is_win:
                osuffix = '_%s.exe' % src_ext[1:]
            else:
                osuffix = '_%s.out' % src_ext[1:]
            self.efile = src_base + osuffix
        if not os.path.isabs(self.efile):
            self.efile = os.path.normpath(os.path.join(self.workingDir, self.efile))
        # Get flag specifying standard library
        if '++' in self.cc and (not platform._is_win):
            std_flag = None
            for a in self.ccflags:
                if a.startswith('-std='):
                    std_flag = a
                    break
            if std_flag is None:
                self.ccflags.append('-std=c++11')
        
    def run(self):
        r"""Run the compiled executable if it exists."""
        if self.compiled:
            super(GCCModelDriver, self).run()
        else:  # pragma: debug
            self.error("Error compiling.")

    def cleanup(self):
        r"""Remove compile executable."""
        if os.path.isfile(self.efile):
            os.remove(self.efile)
        super(GCCModelDriver, self).cleanup()
