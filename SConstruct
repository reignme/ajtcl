# Copyright 2012-2013, Qualcomm Innovation Center, Inc.
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

vars = Variables()
vars.Add(EnumVariable('WS', 'Whitespace Policy Checker', 'check', allowed_values=('check', 'detail', 'fix', 'off')))
vars.Add(PathVariable('ALLJOYN_DIR', 'The path to the AllJoyn repositories',  os.environ.get('ALLJOYN_DIR'), PathVariable.PathIsDir))

env = Environment(variables = vars)

# Add/remove projects from build
env.SConscript('test/SConstruct')

if env['WS'] != 'off' and not env.GetOption('clean'):
    if not os.environ.get('ALLJOYN_DIR'):
       print "ALLJOYN_DIR not set"
       if not GetOption('help'):
           Exit()

    import sys
    bin_dir = os.environ.get('ALLJOYN_DIR') + '/build_core/tools/bin'
    sys.path.append(bin_dir)
    import whitespace

    def wsbuild(target, source, env):
        print "Evaluating whitespace compliance..."
        print "Note: enter 'scons -h' to see whitespace (WS) options"
        return whitespace.main([env['WS'],'ajuncrustify.cfg'])

    env.Command('#/ws', Dir('$DISTDIR'), wsbuild)

