#ifndef LOGGING_H
#define LOGGING_H

void log_init(void);

void log_info(const char* info);

void log_warn(const char* warn);

void log_err(const char* err);

#endif /* LOGGING_H */
