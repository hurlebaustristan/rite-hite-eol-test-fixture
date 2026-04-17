#include "eol_report.h"

#include <string.h>

typedef struct
{
    eol_report_state_t   state;
    eol_report_outcome_t outcome;
    uint32_t             runSequence;
    uint8_t              rowCount;
    eol_report_row_t     rows[EOL_REPORT_MAX_ROWS];
} eol_report_ctx_t;

static eol_report_ctx_t ctx;

static void sanitizeCopy(char* dst, size_t dstLen, const char* src)
{
    size_t outIndex = 0u;

    if ((dst == 0) || (dstLen == 0u))
    {
        return;
    }

    if (src == 0)
    {
        dst[0] = '\0';
        return;
    }

    while ((src[0] != '\0') && (outIndex < (dstLen - 1u)))
    {
        char ch = *src++;

        if ((ch == '|') || (ch == '\r') || (ch == '\n') || (ch == '\t'))
        {
            ch = ' ';
        }

        dst[outIndex++] = ch;
    }

    dst[outIndex] = '\0';
}

void EOL_Report_ResetForNewRun(void)
{
    memset(ctx.rows, 0, sizeof(ctx.rows));
    ctx.rowCount = 0u;
    ctx.state = EOL_REPORT_STATE_RUNNING;
    ctx.outcome = EOL_REPORT_OUTCOME_PASS;
}

bool EOL_Report_AddRow(eol_report_row_status_t status,
                       const char* testName,
                       const char* expected,
                       const char* actual)
{
    eol_report_row_t* row = 0;

    if ((ctx.state != EOL_REPORT_STATE_RUNNING) || (ctx.rowCount >= EOL_REPORT_MAX_ROWS))
    {
        return false;
    }

    row = &ctx.rows[ctx.rowCount];
    row->status = status;
    sanitizeCopy(row->testName, sizeof(row->testName), testName);
    sanitizeCopy(row->expected, sizeof(row->expected), expected);
    sanitizeCopy(row->actual, sizeof(row->actual), actual);
    ctx.rowCount++;
    return true;
}

void EOL_Report_FinalizePass(void)
{
    if (ctx.state == EOL_REPORT_STATE_RUNNING)
    {
        ctx.outcome = EOL_REPORT_OUTCOME_PASS;
        ctx.runSequence++;
        ctx.state = EOL_REPORT_STATE_READY;
    }
}

void EOL_Report_FinalizeFail(void)
{
    if (ctx.state == EOL_REPORT_STATE_RUNNING)
    {
        ctx.outcome = EOL_REPORT_OUTCOME_FAIL;
        ctx.runSequence++;
        ctx.state = EOL_REPORT_STATE_READY;
    }
}

eol_report_state_t EOL_Report_GetState(void)
{
    return ctx.state;
}

eol_report_outcome_t EOL_Report_GetOutcome(void)
{
    return ctx.outcome;
}

uint32_t EOL_Report_GetRunSequence(void)
{
    return ctx.runSequence;
}

uint8_t EOL_Report_GetRowCount(void)
{
    return ctx.rowCount;
}

bool EOL_Report_CopyRow(uint8_t index, eol_report_row_t* outRow)
{
    if ((outRow == 0) || (index >= ctx.rowCount))
    {
        return false;
    }

    *outRow = ctx.rows[index];
    return true;
}

bool EOL_Report_MarkExported(uint32_t runSequence)
{
    if ((ctx.state != EOL_REPORT_STATE_READY) || (ctx.runSequence != runSequence))
    {
        return false;
    }

    ctx.state = EOL_REPORT_STATE_EXPORTED;
    return true;
}
