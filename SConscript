# Copyright 2013, Qualcomm Innovation Center, Inc.
#
#    All rights reserved.
#    This file is licensed under the 3-clause BSD license in the NOTICE.txt
#    file for this project. A copy of the 3-clause BSD license is found at:
#
#        http://opensource.org/licenses/BSD-3-Clause.
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the license is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the license for the specific language governing permissions and
#    limitations under the license.

import os
import shutil
import platform

if platform.system() == 'Linux':
    default_target = 'linux'
    default_msvc_version = None
elif platform.system() == 'Windows':
    default_target = 'win32'
    default_msvc_version = '10.0'

vars = Variables()

# Common build variables
vars.Add(EnumVariable('TARG', 'Target platform variant', default_target, allowed_values=('win32', 'linux', 'arduino')))
vars.Add(EnumVariable('VARIANT', 'Build variant', 'debug', allowed_values=('debug', 'release')))
vars.Add(PathVariable('ALLJOYN_DIR', 'The path to the AllJoyn repositories', os.environ.get('ALLJOYN_DIR'), PathVariable.PathIsDir))
vars.Add(PathVariable('GTEST_DIR', 'The path to googletest sources', os.environ.get('GTEST_DIR'), PathVariable.PathIsDir))
vars.Add(EnumVariable('WS', 'Whitespace Policy Checker', 'check', allowed_values=('check', 'detail', 'fix', 'off')))

if default_msvc_version:
    vars.Add(EnumVariable('MSVC_VERSION', 'MSVC compiler version - Windows', default_msvc_version, allowed_values=('8.0', '9.0', '10.0', '11.0', '11.0Exp')))

if ARGUMENTS.get('TARG', default_target) == 'win32':
    msvc_version = ARGUMENTS.get('MSVC_VERSION')
    env = Environment(variables = vars, MSVC_VERSION=msvc_version, TARGET_ARCH='x86')
else:
    env = Environment(variables = vars)
Help(vars.GenerateHelpText(env))


# Define compile/link options only for win32/linux.
# In case of target platforms, the compilation/linking does not take place
# using SCons files.
if env['TARG'] == 'win32':
    env['libs'] = ['wsock32', 'advapi32']
    env.Append(CFLAGS=['/J', '/W3'])
    env.Append(CPPDEFINES=['_CRT_SECURE_NO_WARNINGS'])
    if env['VARIANT'] == 'debug':
        env.Append(CFLAGS=['/MD', '/Zi', '/Od'])
        env.Append(LINKFLAGS=['/debug'])
    else:
        env.Append(CPPDEFINES = ['NDEBUG'])
        env.Append(CFLAGS=['/MD', '/Gy', '/O1', '/GF'])
        env.Append(LINKFLAGS=['/opt:ref'])
        env.Append(LFLAGS=['/NODEFAULTLIB:libcmt.lib'])
        env.Append(LINKFLAGS=['/NODEFAULTLIB:libcmt.lib'])
elif env['TARG'] == 'linux':
    if os.environ.has_key('CROSS_PREFIX'):
        env.Replace(CC = os.environ['CROSS_PREFIX'] + 'gcc')
        env.Replace(CXX = os.environ['CROSS_PREFIX'] + 'g++')
        env.Replace(LINK = os.environ['CROSS_PREFIX'] + 'gcc')
        env['ENV']['STAGING_DIR'] = os.environ.get('STAGING_DIR', '')

    if os.environ.has_key('CROSS_PATH'):
        env['ENV']['PATH'] = ':'.join([ os.environ['CROSS_PATH'], env['ENV']['PATH'] ] )

    env['libs'] = ['rt', 'crypto', 'pthread']
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
env['includes'] = [ os.getcwd() + '/inc', os.getcwd() + '/target/${TARG}']

# Target-specific headers and sources
env['aj_targ_headers'] = [Glob('target/' + env['TARG'] + '/*.h')]
env['aj_targ_srcs'] = [Glob('target/' + env['TARG'] + '/*.c')]

# AllJoyn Thin Client headers and sources (target independent)
env['aj_headers'] = [Glob('inc/*.h')]
env['aj_srcs'] = [Glob('src/*.c')]
env['aj_sw_crypto'] = [Glob('crypto/*.c')]
env['aj_malloc'] = [Glob('malloc/*.c')]

# Set-up the environment for Win/Linux
if env['TARG'] in [ 'win32', 'linux' ]:
    # To compile, sources need access to include files
    env.Append(CPPPATH = [env['includes']])

    # Win/Linux programs need libs to link
    env.Append(LIBS = [env['libs']])

    # Win/Linux programs need their own 'main' function
    env.Append(CPPDEFINES = ['AJ_MAIN'])

    # Produce shared libraries for these platforms
    srcs = env['aj_srcs'] + env['aj_targ_srcs']
    if env['TARG'] == 'win32':
        srcs += env['aj_sw_crypto'] + env['aj_malloc']

    env.SharedLibrary('ajtcl', srcs)

# Build objects for the target-specific sources and AllJoyn Thin Client sources
if env['TARG'] == 'win32':
    env['aj_obj'] = env.Object(env['aj_srcs'] + env['aj_targ_srcs'] + env['aj_sw_crypto'] + env['aj_malloc'])
else:
    if env['TARG'] == 'linux':
        env['aj_obj'] = env.Object(env['aj_srcs'] + env['aj_targ_srcs'])

Export('env')

if env['WS'] != 'off' and not env.GetOption('clean') and not env.GetOption('help'):
    if not env.has_key('ALLJOYN_DIR'):
       print "ALLJOYN_DIR not set (it is needed when WS option is enabled)"
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
if env['TARG'] == 'arduino':
    arduinoLibDir = 'build/arduino_due/libraries/AllJoyn/'

    # Arduino sketches need the corresponding platform-independent sources
    tests = [ ]
    tests.append('svclite')
    tests.append('clientlite')
    tests.append('siglite')
    tests.append('bastress2')
    tests.append('mutter')
    tests.append('sessions')
    tests.append('aestest')
    testInputs = [ ]
    testOutputs = [ ]

    # Install the generic .c files from the test directory into their
    # destination while changing the extension
    # Also install the .ino file for the test sketch
    for test in Flatten(tests):
        in_path = File('test/' + test + '.c')
        out_path = File('target/arduino/tests/AJ_' + test + '/' + test + '.cpp')

        env.Install(Dir(arduinoLibDir + 'tests/AJ_' + test + '/').abspath, File('target/arduino/tests/AJ_' + test + '/AJ_' + test + '.ino'))
        env.InstallAs(File(arduinoLibDir + 'tests/AJ_' + test + '/' + test + '.cpp').abspath, in_path.abspath)

    replaced_names = []
    for x in Flatten([env['aj_srcs'], env['aj_targ_srcs'], env['aj_sw_crypto']]):
        replaced_names.append( File(arduinoLibDir + x.name.replace('.c', '.cpp') ) )

    # change the extension
    install_renamed_files = env.InstallAs(Flatten(replaced_names), Flatten([env['aj_srcs'], env['aj_targ_srcs'], env['aj_sw_crypto']]))
    install_host_headers = env.Install(arduinoLibDir, env['aj_targ_headers'])
    install_headers = env.Install(arduinoLibDir, env['aj_headers'])

    # install the examples into their source
    env.Install(Dir(arduinoLibDir).abspath, 'target/arduino/examples/')

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
        out_path = File('target/arduino/samples/AJ_' + sample + '/' + sample + '.cpp')
        env.Install(Dir(arduinoLibDir + 'samples/AJ_' + sample + '/').abspath, File('target/arduino/samples/AJ_' + sample + '/AJ_' + sample + '.ino'))
        env.InstallAs(File(arduinoLibDir + 'samples/AJ_' + sample + '/' + sample + '.cpp').abspath, in_path.abspath)

    for sample in Flatten(securesamples):
        in_path = File('samples/secure/' + sample + '.c')
        out_path = File('target/arduino/samples/AJ_' + sample + '/' + sample + '.cpp')
        env.Install(Dir(arduinoLibDir + 'samples/AJ_' + sample + '/').abspath, File('target/arduino/samples/AJ_' + sample + '/AJ_' + sample + '.ino'))
        env.InstallAs(File(arduinoLibDir + 'samples/AJ_' + sample + '/' + sample + '.cpp').abspath, in_path.abspath)

Return('env')
