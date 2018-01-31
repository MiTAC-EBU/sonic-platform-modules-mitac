#/bin/bash

# process name/id
DAEMON_NAME=`basename $0`
DAEMON_PID="$$"

# default log file
DEF_LOG_FILE="/var/log/syslog"

DEF_SEVERITY="INFO"

#/*
#* FEATURE:
#*   log_msg
#* PURPOSE:
#*   log message
#* PARAMETERS:
#*   msg                  (IN) message
#* RETURNS:
#*
#*/
function log_msg() {
  local msg=$1

  echo -e "`date +"%b %_d %T"` `hostname` $DEF_SEVERITY  $DAEMON_NAME[$DAEMON_PID]: ${msg}" >> ${DEF_LOG_FILE}
}
