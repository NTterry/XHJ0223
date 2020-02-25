

#include "u_log.h"
#include <stdlib.h>
#include <string.h>
#include "stdarg.h"
#include "stdint.h"
#include "stdio.h"

#define MAX_LOG_MSG_LEN 255

extern uint32_t HAL_GetTick(void);

static const char *_get_filename(const char *p)
{
#ifdef WIN32
    char ch = '\\';
#else
    char ch = '/';
#endif
    const char *q = strrchr(p,ch);
    if(q == NULL)
    {
        q = p;
    }
    else
    {
        q++;
    }
    return q;
}


int HAL_Snprintf(_IN_ char *str, const int len, const char *fmt, ...)
{
    va_list args;
    int rc;

    va_start(args, fmt);
    rc = vsnprintf(str, len, fmt, args);
    va_end(args);

    return rc;
}

void HAL_Printf(_IN_ const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    fflush(stdout);
}


void IOT_Log_Gen(const char *file, const char *func, const int line, const char *fmt, ...)
{
    /* format log content */
	const char *file_name = _get_filename(file);

	char 	sg_text_buf[MAX_LOG_MSG_LEN + 1];
	char	*tmp_buf = sg_text_buf;
	char    *o = tmp_buf;
	uint32_t cur_tim = HAL_GetTick();
    memset(tmp_buf, 0, sizeof(sg_text_buf));

    o += HAL_Snprintf(o, sizeof(sg_text_buf), "(%d.%.3d)%s|%s(%d): ", cur_tim/1000,cur_tim%1000, file_name, func, line);

    va_list     ap;
    va_start(ap, fmt);
    o += vsnprintf(o, MAX_LOG_MSG_LEN - 2 - strlen(tmp_buf), fmt, ap);
    va_end(ap);

    strcat(tmp_buf, "\r\n");

#ifdef LOG_UPLOAD
    /* append to upload buffer */
    if (level <= g_log_upload_level) {
        append_to_upload_buffer(tmp_buf, strlen(tmp_buf));
    }
#endif

    HAL_Printf("%s", tmp_buf);

    return;
}
