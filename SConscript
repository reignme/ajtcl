# Copyright 2013, Qualcomm Innovation Center, Inc.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.

import os
import shutil

vars = Variables()

# Common build variables
vars.Add(EnumVariable('HOST', 'Host platform variant', 'win32', allowed_values=('win32', 'linux', 'arduino')))
vars.Add(EnumVariable('VARIANT', 'Build variant', 'debug', allowed_values=('debug', 'release')))
vars.Add(PathVariable('ALLJOYN_DIR', 'The path to the AllJoyn repositories',  os.environ.get('ALLJOYN_DIR'), PathVariable.PathIsDir))
vars.Add(PathVariable('OPENSSL_DIR', 'OpenSSL Base', os.environ.get('OPENSSL_BASE'), PathVariable.PathIsDir))
vars.Add(EnumVariable('MSVC_VERSION', 'MSVC compiler version - Windows', '10.0', allowed_values=('8.0', '9.0', '10.0', '11.0', '11.0Exp')))
vars.Add(EnumVariable('WS', 'Whitespace Policy Checker', 'check', allowed_values=('check', 'detail', 'fix', 'off')))

env = Environment(variables = vars, MSVC_VERSION='${MSVC_VERSION}')

env.Append(CPPDEFINES = ['AJ_MAIN'])

# Define compile/link options only for win32/linux.
# In case of target platforms, the compilation/linking does not take place
# using SCons files.
if env['HOST'] == 'win32':
    if not env.has_key('OPENSSL_DIR'):
        env.Append(OPENSSL_DIR='c:\OpenSSL-Win64')
    env['libs'] = ['wsock32', 'libeay32']
    env.Append(CFLAGS=['/I${OPENSSL_DIR}\include', '/J', '/W3'])
    env.Append(CPPDEFINES=['_CRT_SECURE_NO_WARNINGS'])
    env.Append(LINKFLAGS=['/LIBPATH:${OPENSSL_DIR}\lib'])
    if env['VARIANT'] == 'debug':
        env.Append(CFLAGS=['/MD', '/Zi', '/Od'])
        env.Append(LINKFLAGS=['/debug'])
    else:
        env.Append(CPPDEFINES = ['NDEBUG'])
        env.Append(CFLAGS=['/MD', '/Gy', '/O1', '/GF'])
        env.Append(LINKFLAGS=['/opt:ref'])
        env.Append(LFLAGS=['/NODEFAULTLIB:libcmt.lib'])
        env.Append(LINKFLAGS=['/NODEFAULTLIB:libcmt.lib'])
elif env['HOST'] == 'linux':
    env['libs'] = ['rt', 'crypto']
    env.Append(LINKFLAGS=[''])
    env.Append(CFLAGS=['-Wall',
                       '-pipe',
                       '-static',
                       '-funsigned-char',
                       '-Wpointer-sign',
                       '-Wimplicit-function-declaration',
                       '-fno-strict-aliasing'])
    if env['VARIANT'] == 'debug':
        env.Append(CFLAGS='-g')
    else:
        env.Append(CPPDEFINES=['NDEBUG'])
        env.Append(CFLAGS='-Os')
        env.Append(LINKFLAGS='-s')

# Include paths
env['includes'] = [ os.getcwd() + '/inc', os.getcwd() + '/host/${HOST}']

# Host-specific headers and sources
env['aj_host_headers'] = [Glob('host/' + env['HOST'] + '/*.h')]
env['aj_host_srcs'] = [Glob('host/' + env['HOST'] + '/*.c')]

# AllJoyn Thin Client headers and sources (host/target independent)
env['aj_headers'] = [Glob('inc/*.h')]
env['aj_srcs'] = [Glob('src/*.c')]

# Build objects for the host-specific sources and AllJoyn Thin Client sources
if env['HOST'] == 'win32' or env['HOST'] == 'linux':
    env['aj_obj'] = env.Object(env['aj_srcs'] + env['aj_host_srcs'], CPPPATH=env['includes'])

Export('env')

if env['WS'] != 'off' and not env.GetOption('clean'):
    if not env.has_key('ALLJOYN_DIR'):
       print "ALLJOYN_DIR not set"
       if not GetOption('help'):
           Exit()

    # Set the location of the uncrustify config file
    env['uncrustify_cfg'] = os.getcwd() + '/ajuncrustify.cfg'

    import sys
    bin_dir = env['ALLJOYN_DIR'] + '/build_core/tools/bin'
    sys.path.append(bin_dir)
    import whitespace

    def wsbuild(target, source, env):
        print "Evaluating whitespace compliance..."
        print "Note: enter 'scons -h' to see whitespace (WS) options"
        return whitespace.main([env['WS'],env['uncrustify_cfg']])

    env.Command('#/ws', Dir('$DISTDIR'), wsbuild)

# In case of Arduino target, package the 'SDK' suitable for development
# on Arduino IDE
if env['HOST'] == 'arduino':
    arduinoLibDir = 'build/arduino_due/libraries/AllJoyn/'

    # Arduino sketches need the corresponding platform-independent sources
    tests = [ ]
    tests.append('svclite')
    tests.append('clientlite')
    tests.append('siglite')
    tests.append('bastress2')
    tests.append('mutter')
    testInputs = [ ]
    testOutputs = [ ]

    # Install the generic .c files from the test directory into their
    # destination while changing the extension
    # Also install the .ino file for the test sketch
    for test in Flatten(tests):
        in_path = File('test/' + test + '.c')
        out_path = File('host/arduino/tests/AJ_' + test + '/' + test + '.cpp')

        env.Install(Dir(arduinoLibDir + 'tests/AJ_' + test + '/').abspath, File('host/arduino/tests/AJ_' + test + '/AJ_' + test + '.ino'))
        env.InstallAs(File(arduinoLibDir + 'tests/AJ_' + test + '/' + test + '.cpp').abspath, in_path.abspath)

    replaced_names = []
    for x in Flatten([env['aj_srcs'], env['aj_host_srcs']]):
        replaced_names.append( File(arduinoLibDir + x.name.replace('.c', '.cpp') ) )

    # change the extension
    install_renamed_files = env.InstallAs(Flatten(replaced_names), Flatten([env['aj_srcs'], env['aj_host_srcs']]))
    install_host_headers = env.Install(arduinoLibDir, env['aj_host_headers'])
    install_headers = env.Install(arduinoLibDir, env['aj_headers'])

    # install the examples into their source
    env.Install(Dir(arduinoLibDir).abspath, 'host/arduino/examples/')

    # install the 'sessions' test app
    env.Install(Dir(arduinoLibDir).abspath + '/tests', 'host/arduino/tests/AJ_sessions')
    env.Install(Dir(arduinoLibDir + 'tests/AJ_sessions').abspath, env.Glob('host/arduino/tests/AJ_sessions/*'))

    # Install basic samples
    basicsamples = [ ]
    basicsamples.append('basic_service')
    basicsamples.append('basic_client')
    basicsamples.append('signal_service')
    basicsamples.append('signalConsumer_client')

    securesamples = [ ]
    securesamples.append('SecureClient')
    securesamples.append('SecureService')

    for sample in Flatten(basicsamples):
        in_path = File('samples/basic/' + sample + '.c')
        out_path = File('host/arduino/samples/AJ_' + sample + '/' + sample + '.cpp')
        env.Install(Dir(arduinoLibDir + 'samples/AJ_' + sample + '/').abspath, File('host/arduino/samples/AJ_' + sample + '/AJ_' + sample + '.ino'))
        env.InstallAs(File(arduinoLibDir + 'samples/AJ_' + sample + '/' + sample + '.cpp').abspath, in_path.abspath)

    for sample in Flatten(securesamples):
        in_path = File('samples/secure/' + sample + '.c')
        out_path = File('host/arduino/samples/AJ_' + sample + '/' + sample + '.cpp')
        env.Install(Dir(arduinoLibDir + 'samples/AJ_' + sample + '/').abspath, File('host/arduino/samples/AJ_' + sample + '/AJ_' + sample + '.ino'))
        env.InstallAs(File(arduinoLibDir + 'samples/AJ_' + sample + '/' + sample + '.cpp').abspath, in_path.abspath)

Return('env')
