# Copyright 2012-2013, Qualcomm Innovation Center, Inc.
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

env = SConscript(['SConscript'])

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
