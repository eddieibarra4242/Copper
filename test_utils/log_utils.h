#pragma once

void reset_log_checks(void);

void expect_log(const char* message);
void expect_error(const char* error_message);
void expect_critical_error(const char *error_message);
