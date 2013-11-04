#include <servicesync/moat.h>
#include "app_debug.h"

sse_char* 
SSEErrorCode_ToChar(sse_int in_code) {
  switch (in_code) {
    CASE_LABEL_CHAR(SSE_E_OK);
    CASE_LABEL_CHAR(SSE_E_GENERIC);
    CASE_LABEL_CHAR(SSE_E_NOMEM);
    CASE_LABEL_CHAR(SSE_E_INVAL);
    CASE_LABEL_CHAR(SSE_E_ALREADY);
    CASE_LABEL_CHAR(SSE_E_AGAIN);
    CASE_LABEL_CHAR(SSE_E_INPROGRESS);
    CASE_LABEL_CHAR(SSE_E_PERM);
    CASE_LABEL_CHAR(SSE_E_ACCES);
    CASE_LABEL_CHAR(SSE_E_TIMEDOUT);
    CASE_LABEL_CHAR(SSE_E_INTR);
    CASE_LABEL_DEFAULT(SSE_E_UNKNOWN);
  }
}

