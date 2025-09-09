#ifndef READLINE_H
#define READLINE_H

#include <cstddef>

#ifdef __cplusplus
#include "../builtins/history.h"
extern "C" {
#endif

typedef struct readlineCompletions {
  size_t len;
  char **cvec;
} readlineCompletions;

typedef void(readlineCompletionCallback)(const char *, readlineCompletions *);
typedef char*(readlineHintsCallback)(const char *, int *color, int *bold);
typedef void(readlineFreeHintsCallback)(void *);

void readlineSetCompletionCallback(readlineCompletionCallback *);
void readlineSetHintsCallback(readlineHintsCallback *);
void readlineSetFreeHintsCallback(readlineFreeHintsCallback *);
void readlineAddCompletion(readlineCompletions *, const char *);

#ifdef __cplusplus
void readlineSetHistoryInstance(olsh::Builtins::History* history);
#endif

char *readline(const char *prompt);
void readlineFree(void *ptr);
int readlineHistoryAdd(const char *line);
int readlineHistorySetMaxLen(int len);
int readlineHistorySave(const char *filename);
int readlineHistoryLoad(const char *filename);
void readlineHistoryReset(void);
void readlineClearScreen(void);
void readlineSetMultiLine(int ml);
void readlinePrintKeyCodes(void);

#ifdef __cplusplus
}
#endif

#endif // READLINE_H
