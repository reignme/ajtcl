#ifndef _AJ_VERSION_H
#define _AJ_VERSION_H
/**
 * @file
 */
/******************************************************************************
 * Copyright 2013, Qualcomm Innovation Center, Inc.
 *
 *    All rights reserved.
 *    This file is licensed under the 3-clause BSD license in the NOTICE.txt
 *    file for this project. A copy of the 3-clause BSD license is found at:
 *
 *        http://opensource.org/licenses/BSD-3-Clause.
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the license is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the license for the specific language governing permissions and
 *    limitations under the license.
 ******************************************************************************/

#define AJ_MAJOR_VERSION   3    /**< major version */
#define AJ_MINOR_VERSION   3    /**< minor version */
#define AJ_RELEASE_VERSION 2    /**< release version */

#define AJ_VERSION ((AJ_MAJOR_VERSION) << 24) | ((AJ_MINOR_VERSION) << 16) | (AJ_RELEASE_VERSION)   /**< macro to generate the version from major, minor & release */

#endif
