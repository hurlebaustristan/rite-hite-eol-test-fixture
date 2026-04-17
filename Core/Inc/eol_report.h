#ifndef EOL_REPORT_H
#define EOL_REPORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define EOL_REPORT_MAX_ROWS         48u
#define EOL_REPORT_TEST_NAME_LEN    32u
#define EOL_REPORT_EXPECTED_LEN     48u
#define EOL_REPORT_ACTUAL_LEN       48u

typedef enum
{
    EOL_REPORT_STATE_EMPTY = 0,
    EOL_REPORT_STATE_RUNNING,
    EOL_REPORT_STATE_READY,
    EOL_REPORT_STATE_EXPORTED
} eol_report_state_t;

typedef enum
{
    EOL_REPORT_OUTCOME_PASS = 0,
    EOL_REPORT_OUTCOME_FAIL
} eol_report_outcome_t;

typedef enum
{
    EOL_REPORT_ROW_PASS = 0,
    EOL_REPORT_ROW_FAIL
} eol_report_row_status_t;

typedef struct
{
    eol_report_row_status_t status;
    char testName[EOL_REPORT_TEST_NAME_LEN];
    char expected[EOL_REPORT_EXPECTED_LEN];
    char actual[EOL_REPORT_ACTUAL_LEN];
} eol_report_row_t;

void EOL_Report_ResetForNewRun(void);
bool EOL_Report_AddRow(eol_report_row_status_t status,
                       const char* testName,
                       const char* expected,
                       const char* actual);
void EOL_Report_FinalizePass(void);
void EOL_Report_FinalizeFail(void);

eol_report_state_t EOL_Report_GetState(void);
eol_report_outcome_t EOL_Report_GetOutcome(void);
uint32_t EOL_Report_GetRunSequence(void);
uint8_t EOL_Report_GetRowCount(void);
bool EOL_Report_CopyRow(uint8_t index, eol_report_row_t* outRow);
bool EOL_Report_MarkExported(uint32_t runSequence);

#ifdef __cplusplus
}
#endif

#endif /* EOL_REPORT_H */
