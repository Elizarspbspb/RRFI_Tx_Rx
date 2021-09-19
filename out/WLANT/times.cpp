//
// Created by elizar on 25.06.2021.
//

#include <fstream>

long times(char *time_str, char *time_str2, char *time_str3)
{
    struct timespec ts;
    long msec;
    clock_gettime(CLOCK_REALTIME, &ts);
    if (ts.tv_nsec >= 999500000)
    {
        ts.tv_sec++;
        msec = 0;
    }
    else
    {
        msec = (ts.tv_nsec + 500000) / 1000;
    }
    struct tm* ptm = localtime(&ts.tv_sec);
    if (ptm == NULL)
    {
        perror("localtime");
        return 1;
    }
    time_str[sizeof("1900-01-01 23:59:59")];
    time_str[strftime(time_str, sizeof(time_str),"%Y-%m", ptm)];
    time_str2[strftime(time_str2, sizeof(time_str2),"%d %H", ptm)];
    time_str3[strftime(time_str3, sizeof(time_str3),"%M:%S", ptm)];
    return msec;
}
