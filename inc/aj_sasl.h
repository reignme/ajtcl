#ifndef _AJ_SASL_H
#define _AJ_SASL_H
/**
 * @file aj_sasl.h
 * @defgroup aj_sasl Simple Authentication and Security Layer Support
 * @{
 */
/******************************************************************************
 * Copyright 2012-2013, Qualcomm Innovation Center, Inc.
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

#include "aj_target.h"
#include "aj_bufio.h"
#include "aj_bus.h"
#include "aj_guid.h"

/**
 * Enumeration for authentication result
 */
typedef enum {
    AJ_AUTH_STATUS_SUCCESS,  /**< Indicates an authentication exchange completed succesfully */
    AJ_AUTH_STATUS_CONTINUE, /**< Indicates an authentication exchange is continuing */
    AJ_AUTH_STATUS_RETRY,    /**< Indicates an authentication failed but should be retried */
    AJ_AUTH_STATUS_FAILURE,  /**< Indicates an authentication failed fatally */
    AJ_AUTH_STATUS_ERROR     /**< Indicates an authentication challenge or response was badly formed */
} AJ_AuthResult;

/**
 * Challenge or response function for an authentication mechanism.
 *
 * @param inStr   The NUL terminated challenge or response string being input to the handler. This
 *                should be NULL on the first challenge or response.
 * @param outStr  A buffer to receive the challenger or response to send out
 * @param outLen  The length of the outStr buffer
 *
 * @return
 *         - AJ_AUTH_STATUS_SUCCESS if the authentication has completed succesfully
 *         - AJ_AUTH_STATUS_CONTINUE if the authentication is continuing
 *         - AJ_AUTH_STATUS_RETRY if the authentication failed (e.g. due to an incorrect password) but should be retried
 *         - AJ_AUTH_STATUS_FAILED if the authentication failed and should not be retried
 *         - AJ_AUTH_STATUS_ERROR if the data passed in was invalid or unexpected
 */
typedef AJ_AuthResult (*AJ_AuthAdvanceFunc)(const char* inStr, char* outStr, uint32_t outLen);

/**
 * Initializes an authentication mechanism for the specified role
 *
 * @param role     Indicates if the authentication mechanism is being initialized as a
 *                 reponder or challenger.
 * @param pwdFunc  Callback function for requesting a password.
 */
typedef AJ_Status (*AJ_AuthInitFunc)(uint8_t role, AJ_AuthPwdFunc pwdFunc);

/**
 * Finalizes the authentication mechanism storing credentials for the authenticated peer in the
 * keystore.
 *
 * @param peerGuid The guid to associate the credentials with or NULL if the
 *                 authentication was unsuccessful.
 *
 * @return
 *         - AJ_OK if the finalization completed sucesfully.
 *         - Other error status codes
 */
typedef AJ_Status (*AJ_AuthFinalFunc)(const AJ_GUID* peerGuid);

/**
 * Struct defining the interface to an authentication mechanism
 */
typedef struct _AJ_AuthMechanism {
    AJ_AuthInitFunc Init;               /**< Initialize an authentication mechnism */
    AJ_AuthAdvanceFunc Challenge;       /**< Challenge of response function for an auth mechanism */
    AJ_AuthAdvanceFunc Response;        /**< Challenge of response function for an auth mechanism */
    AJ_AuthFinalFunc Final;             /**< Finalizes the auth mechanism storing credentials */
    const char* name;                   /**< Name of authentication mechanism */
} AJ_AuthMechanism;


/**
 * Enumeration for SASL status
 */
typedef enum {
    AJ_SASL_IDLE,              /**< Idle state */
    AJ_SASL_SEND_AUTH_REQ,     /**< Initial responder state */
    AJ_SASL_WAIT_FOR_AUTH,     /**< Initial challenger stat */
    AJ_SASL_WAIT_FOR_BEGIN,    /**< Wait for a begin */
    AJ_SASL_WAIT_FOR_DATA,     /**< Wait for a data */
    AJ_SASL_WAIT_FOR_OK,       /**< Wait for a OK */
    AJ_SASL_WAIT_FOR_REJECT,   /**< Wait for a reject */
    AJ_SASL_WAIT_EXT_RESPONSE, /**< Wait for a response */
    AJ_SASL_AUTHENTICATED,     /**< Authentication was successful - conversation it over */
    AJ_SASL_FAILED             /**< Authentication failed - conversation it over */
} AJ_SASL_State;

#define AJ_AUTH_CHALLENGER  0  /**< Challenger role */
#define AJ_AUTH_RESPONDER   1  /**< Responder initiates the SASL conversation */

/**
 * The context structure for the SASL conversion
 */
typedef struct _AJ_SASL_Context {

    uint8_t role;                            /**< Indicates if the role is Challenger or Responder */
    uint8_t authCount;                       /**< Sanity check to terminate state machine */
    AJ_SASL_State state;                     /**< Current state of the SASL conversation */
    AJ_AuthPwdFunc pwdFunc;                  /**< Password function if applicable */
    const AJ_AuthMechanism* const* mechList; /**< NULL terminated array of authentication mechanismse */
    const AJ_AuthMechanism* mechanism;       /**< The authentication mechansim current in use */
    uint8_t nextMech;                        /**< Index of the next authentication mechanism to be used */

} AJ_SASL_Context;

/**
 * Initializes a context structure for starting a SASL conversation.
 *
 * @param context    A SASL context structure
 * @param mechList   NULL terminated array of authentication mechanisms
 * @param role       Defines if the context is being initialized for the responder or challenger
 *                   side of an authentication conversation.
 * @param pwdFunc    Callback function for requesting a password, or NULL if not applicable.
 *
 * @return           Return AJ_Status
 */
AJ_Status AJ_SASL_InitContext(AJ_SASL_Context* context, const AJ_AuthMechanism* const* mechList, uint8_t role, AJ_AuthPwdFunc pwdFunc);

/**
 * Advances the SASL state machine.
 *
 * @param context The context structure for the SASL conversion.
 * @param inStr   Input authentication string
 * @param outStr  Buffer for output authentication string
 * @param outLen  The length of the output buffer
 *
 * @return        Return AJ_Status
 */
AJ_Status AJ_SASL_Advance(AJ_SASL_Context* context, char* inStr, char* outStr, uint32_t outLen);

/**
 * @}
 */
#endif
