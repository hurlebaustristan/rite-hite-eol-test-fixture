#include "export_uart.h"
#include "auto_test.h"
#include "button_led_test.h"
#include "eol_report.h"
#include "uart_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXPORT_UART_CMD_BUF_LEN   48u
#define EXPORT_UART_TX_TIMEOUT_MS 200u
#define EXPORT_UART_ROW_BUF_LEN   192u

typedef struct
{
    char    commandBuf[EXPORT_UART_CMD_BUF_LEN];
    uint8_t commandLen;
    bool    lineOverflow;
} export_uart_ctx_t;

static export_uart_ctx_t ctx;

static const char* outcomeName(eol_report_outcome_t outcome)
{
    return (outcome == EOL_REPORT_OUTCOME_FAIL) ? "FAIL" : "PASS";
}

static const char* rowStatusName(eol_report_row_status_t status)
{
    return (status == EOL_REPORT_ROW_FAIL) ? "FAIL" : "PASS";
}

static bool txLine(const char* line)
{
    return (USART1_Transmit((const uint8_t*)line,
                            (uint16_t)strlen(line),
                            EXPORT_UART_TX_TIMEOUT_MS) == HAL_OK);
}

static void sendStatus(void)
{
    char line[64];
    const eol_report_state_t state = EOL_Report_GetState();
    const char* phaseToken = AutoTest_GetPhaseToken();

    switch (state)
    {
    case EOL_REPORT_STATE_EMPTY:
        (void)txLine("STATUS|EMPTY\n");
        break;

    case EOL_REPORT_STATE_RUNNING:
        if (AutoTest_IsAutoMode() && ButtonLedTest_IsWaitingForVisualDecision())
        {
            if ((phaseToken != 0) && (phaseToken[0] != '\0'))
            {
                (void)snprintf(line, sizeof(line), "STATUS|WAITING_VISUAL|%s\n", phaseToken);
                (void)txLine(line);
            }
            else
            {
                (void)txLine("STATUS|WAITING_VISUAL\n");
            }
        }
        else
        {
            if (AutoTest_IsAutoMode() && (phaseToken != 0) && (phaseToken[0] != '\0'))
            {
                (void)snprintf(line, sizeof(line), "STATUS|RUNNING|%s\n", phaseToken);
                (void)txLine(line);
            }
            else
            {
                (void)txLine("STATUS|RUNNING\n");
            }
        }
        break;

    case EOL_REPORT_STATE_READY:
    case EOL_REPORT_STATE_EXPORTED:
        (void)snprintf(line,
                       sizeof(line),
                       "STATUS|%s|%s|%lu|%u\n",
                       (state == EOL_REPORT_STATE_READY) ? "READY" : "EXPORTED",
                       outcomeName(EOL_Report_GetOutcome()),
                       (unsigned long)EOL_Report_GetRunSequence(),
                       (unsigned)EOL_Report_GetRowCount());
        (void)txLine(line);
        break;

    default:
        (void)txLine("STATUS|EMPTY\n");
        break;
    }
}

static void sendExportRun(uint32_t requestedSequence)
{
    char line[EXPORT_UART_ROW_BUF_LEN];
    eol_report_row_t row;
    const eol_report_state_t state = EOL_Report_GetState();
    const uint32_t currentSequence = EOL_Report_GetRunSequence();
    const uint8_t rowCount = EOL_Report_GetRowCount();
    uint8_t index = 0u;
    bool txOk = true;

    if ((state == EOL_REPORT_STATE_EMPTY) || (state == EOL_REPORT_STATE_RUNNING))
    {
        (void)txLine("EXPORT_ERROR|NOT_READY\n");
        return;
    }

    if (requestedSequence != currentSequence)
    {
        (void)txLine("EXPORT_ERROR|BAD_SEQ\n");
        return;
    }

    if (state == EOL_REPORT_STATE_EXPORTED)
    {
        (void)txLine("EXPORT_ERROR|ALREADY_EXPORTED\n");
        return;
    }

    (void)snprintf(line,
                   sizeof(line),
                   "EXPORT_BEGIN|%lu|%s|%u\n",
                   (unsigned long)currentSequence,
                   outcomeName(EOL_Report_GetOutcome()),
                   (unsigned)rowCount);
    txOk = txLine(line);

    for (index = 0u; txOk && (index < rowCount); index++)
    {
        if (!EOL_Report_CopyRow(index, &row))
        {
            txOk = false;
            break;
        }

        (void)snprintf(line,
                       sizeof(line),
                       "ROW|%u|%s|%s|%s|%s\n",
                       (unsigned)index,
                       rowStatusName(row.status),
                       row.testName,
                       row.expected,
                       row.actual);
        txOk = txLine(line);
    }

    if (txOk)
    {
        (void)snprintf(line, sizeof(line), "EXPORT_END|%lu\n", (unsigned long)currentSequence);
        txOk = txLine(line);
    }

    if (txOk)
    {
        (void)EOL_Report_MarkExported(currentSequence);
    }
}

static void processCommandLine(void)
{
    const char exportRunPrefix[] = "EXPORT_RUN|";
    char* endPtr = 0;
    unsigned long requestedSequence = 0ul;

    if (strcmp(ctx.commandBuf, "EXPORT_STATUS") == 0)
    {
        sendStatus();
        return;
    }

    if (strncmp(ctx.commandBuf, exportRunPrefix, sizeof(exportRunPrefix) - 1u) == 0)
    {
        requestedSequence = strtoul(&ctx.commandBuf[sizeof(exportRunPrefix) - 1u], &endPtr, 10);
        if ((endPtr == 0) || (*endPtr != '\0'))
        {
            (void)txLine("EXPORT_ERROR|BAD_SEQ\n");
            return;
        }

        sendExportRun((uint32_t)requestedSequence);
        return;
    }

    if (strcmp(ctx.commandBuf, "START_AUTO") == 0)
    {
        if (AutoTest_IsAutoMode())
        {
            (void)txLine("AUTO_ERROR|ALREADY_RUNNING\n");
        }
        else
        {
            AutoTest_RequestStart();
            (void)txLine("AUTO_STARTED\n");
        }
        return;
    }
}

void ExportUart_Init(void)
{
    memset(&ctx, 0, sizeof(ctx));
    USART1_Config_Init();
}

void ExportUart_Tick(void)
{
    uint8_t byteIn = 0u;

    while (USART1_ReadByte(&byteIn))
    {
        if (byteIn == '\r')
        {
            continue;
        }

        if (byteIn == '\n')
        {
            if (!ctx.lineOverflow && (ctx.commandLen > 0u))
            {
                ctx.commandBuf[ctx.commandLen] = '\0';
                processCommandLine();
            }

            ctx.commandLen = 0u;
            ctx.lineOverflow = false;
            memset(ctx.commandBuf, 0, sizeof(ctx.commandBuf));
            continue;
        }

        if (ctx.lineOverflow)
        {
            continue;
        }

        if ((byteIn < 32u) || (byteIn > 126u))
        {
            ctx.lineOverflow = true;
            continue;
        }

        if (ctx.commandLen >= (EXPORT_UART_CMD_BUF_LEN - 1u))
        {
            ctx.lineOverflow = true;
            continue;
        }

        ctx.commandBuf[ctx.commandLen++] = (char)byteIn;
    }
}
