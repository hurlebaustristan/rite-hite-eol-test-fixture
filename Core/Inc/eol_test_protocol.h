/**
  ******************************************************************************
  * @file    eol_test_protocol.h
  * @brief   Shared protocol definitions for STM32 <-> ESP32 EOL test comm.
  *
  *          Message format (simple ASCII, newline-terminated):
  *            STM32 sends:  "PING\n"
 *            ESP32 replies: "PONG"
  *
  *          Error codes are unique across all test stages so the
  *          Fault_Screen can display a meaningful diagnostic.
  ******************************************************************************
  */

#ifndef EOL_TEST_PROTOCOL_H
#define EOL_TEST_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ======================================================================== */
/*  Message Strings                                                         */
/* ======================================================================== */
#define EOL_MSG_PING        "PING\n"
#define EOL_MSG_PING_LEN    5           /* includes '\n'                     */

#define EOL_MSG_PONG        "PONG"     /* ESP32 sends bare PONG, no newline */
#define EOL_MSG_PONG_LEN    4           /* 4 bytes: P-O-N-G                  */

#define EOL_MSG_READY       "READY\n"
#define EOL_MSG_READY_LEN   6           /* includes '\n'                     */

#define EOL_MSG_OK          "OK"       /* ESP32 sends bare OK, no newline   */
#define EOL_MSG_OK_LEN      2           /* 2 bytes: O-K                      */

/* ======================================================================== */
/*  Timing                                                                  */
/* ======================================================================== */
#define EOL_PING_TIMEOUT_MS     3000    /* per-attempt receive timeout (debug/manual typing) */
#define EOL_PING_RETRY_DELAY_MS 3000    /* delay between PING attempts       */
#define EOL_PING_MAX_RETRIES    3       /* total attempts before failure     */
#define EOL_MCU_RESET_PULSE_MS     1000 /* PB11 HIGH pulse width             */
#define EOL_MCU_RESET_RECOVERY_MS  1000 /* ESP32 reboot settle time          */
#define EOL_FAULT_CHECK_PULSE_MS     1000 /* PA8 HIGH pulse width             */
#define EOL_FAULT_CHECK_RECOVERY_MS  1000 /* ESP32 settle time after pulse    */
#define EOL_FAULT_DISPLAY_MS    3000    /* red-text display before goto Fault */

/* ======================================================================== */
/*  Error Codes  (unique per test stage)                                    */
/* ======================================================================== */
typedef enum
{
    EOL_ERR_NONE                  = 0x0000,

    /* --- Communication Stage (0x01xx) --- */
    EOL_ERR_COMM_PING_NO_RESPONSE = 0x0101,  /* No reply to PING after 3 attempts */
    EOL_ERR_COMM_PING_BAD_REPLY   = 0x0102,  /* Reply received but not "PONG"     */

    /* --- MCU Reset Stage (0x02xx) --- */
    EOL_ERR_MCU_RESET_NO_RECONNECT = 0x0201, /* ESP32 did not reconnect after reset */
    EOL_ERR_MCU_RESET_PING_BAD_REPLY = 0x0202, /* Reply received but not "PONG"      */
    EOL_ERR_MCU_RESET_PULSE_VERIFY_FAIL = 0x0203, /* PB11 state mismatch after write  */

    /* --- Fault Check Stage (0x03xx) --- */
    EOL_ERR_FAULT_CHECK_NO_RECONNECT   = 0x0301, /* No OK reply after FAULT pulse    */
    EOL_ERR_FAULT_CHECK_BAD_REPLY      = 0x0302, /* Reply received but not "OK"      */
    EOL_ERR_FAULT_CHECK_PULSE_VERIFY_FAIL = 0x0303, /* PA8 state mismatch after write */

    /* --- Digital Inputs Stage (0x04xx) --- */
    EOL_ERR_DI_NO_RESPONSE             = 0x0401, /* No DIxOK/DIxFAIL reply           */
    EOL_ERR_DI_FAIL_TOKEN              = 0x0402, /* ESP32 replied DIxFAIL            */
    EOL_ERR_DI_INVALID_REPLY           = 0x0403, /* Wrong or malformed reply token   */

    /* --- Analog Inputs Stage (0x05xx) --- */
    EOL_ERR_AI_NO_RESPONSE             = 0x0501, /* No analog sample reply           */
    EOL_ERR_AI_INVALID_REPLY           = 0x0502, /* Malformed or mismatched reply    */
    EOL_ERR_AI_OUT_OF_RANGE            = 0x0503, /* Measured value outside tolerance */

    /* --- Outputs Stage (0x06xx) --- */
    EOL_ERR_OUTPUT_NO_RESPONSE         = 0x0601, /* No relay command acknowledgement */
    EOL_ERR_OUTPUT_INVALID_REPLY       = 0x0602, /* Malformed or mismatched ack      */
    EOL_ERR_OUTPUT_STATE_MISMATCH      = 0x0603, /* Relay readback mismatch          */

    /* --- Buttons / LEDs Stage (0x07xx) --- */
    EOL_ERR_BUTTON_LED_NO_RESPONSE     = 0x0701, /* No acknowledgement from ESP32    */
    EOL_ERR_BUTTON_LED_INVALID_REPLY   = 0x0702, /* Malformed or mismatched ack      */
    EOL_ERR_BUTTON_LED_OPERATOR_REJECTED = 0x0703, /* Operator marked LED check bad  */
    EOL_ERR_BUTTON_LED_TIMEOUT         = 0x0704, /* Automatic button press/release detect timeout */
    EOL_ERR_BUTTON_LED_STATE_MISMATCH  = 0x0705, /* ESP32 measured opposite button state */

} eol_error_code_t;

/* ======================================================================== */
/*  Test Stage Identifiers                                                  */
/* ======================================================================== */
typedef enum
{
    EOL_STAGE_IDLE = 0,
    EOL_STAGE_COMM,             /* PING / PONG handshake          */
    EOL_STAGE_MCU_RESET,        /* Reset ESP32, verify reconnect  */
    EOL_STAGE_FAULT_CHECK,      /* Assert / de-assert fault line  */
    EOL_STAGE_DONE,             /* All Comm_Reset tests passed    */
} eol_stage_t;

/* ======================================================================== */
/*  Progress Events (non-terminal stage updates for UI)                     */
/* ======================================================================== */
typedef enum
{
    EOL_PROGRESS_EVENT_NONE = 0,
    EOL_PROGRESS_EVT_MCU_RESET_BUTTON_PULSE_DONE = 1,
    EOL_PROGRESS_EVT_FAULT_CHECK_BUTTON_PULSE_DONE = 2,
} eol_progress_event_t;

/* ======================================================================== */
/*  Test Result Passed Back to the UI                                       */
/* ======================================================================== */
typedef struct
{
    eol_stage_t      stage;         /* Which stage just completed / failed */
    eol_error_code_t errorCode;     /* EOL_ERR_NONE if the stage passed   */
} eol_test_result_t;

#ifdef __cplusplus
}
#endif

#endif /* EOL_TEST_PROTOCOL_H */
