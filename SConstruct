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

env = SConscript(['SConscript'])

vars.Update(env)
Help(vars.GenerateHelpText(env))

# Add/remove projects from build
env.SConscript('test/SConscript')
env.SConscript('samples/SConscript')

# Build googletests for VARIANT=debug and for Win/Linux only (not for embedded)
if env['TARG'] == 'win32' or env['TARG'] == 'linux':
    if env['VARIANT'] == 'debug':
        if env.has_key('GTEST_DIR'):
            env.SConscript('unit_test/SConscript')
        else:
            print 'GTEST_DIR is not set, skipping unittest build'
