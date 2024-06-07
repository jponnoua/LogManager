#include "../h/myLogManager.h"

myLogManager::myLogManager()
    : isRunning(true) {
    mq_unlink(MY_LOGMANAGER_MQ_NAME);
    mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(myMsg_msg_queue);
    attr.mq_curmsgs = 0;
    this->mq = mq_open(MY_LOGMANAGER_MQ_NAME, O_CREAT | O_RDONLY, 0644, &attr);
    if (this->mq == (mqd_t)-1) {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }
}

myLogManager::~myLogManager() {
    mq_close(this->mq);
    mq_unlink(MY_LOGMANAGER_MQ_NAME);
}

void myLogManager::run() {
    while (this->isRunning) {
        myMsg_msg_queue logMessage;
        ssize_t bytes_read = mq_receive(this->mq, reinterpret_cast<char*>(&logMessage), sizeof(logMessage), nullptr);
        if (bytes_read == sizeof(logMessage)) {
            char buffer[32];
            struct tm *timeinfo;
            uint64_t timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            time_t timestamp_s = timestamp_ms / 1000;
            uint64_t ms = timestamp_ms % 1000;
            timeinfo = localtime(&timestamp_s);
            strftime(buffer, 32, "%x %X", timeinfo);

            switch(logMessage.msg.contents.contents_log.error_output) {
                case LOG_OUT_SYSLOG:
                    syslog(logMessage.msg.contents.contents_log.error_type, "[%s.%" PRIu64 "] %s %s (%s, %s, l%d)\n", buffer, ms, loglevel2str[logMessage.msg.contents.contents_log.error_type], logMessage.msg.contents.contents_log.error_msg, logMessage.msg.contents.contents_log.file, logMessage.msg.contents.contents_log.func, logMessage.msg.contents.contents_log.line_number);
                    break;

                case LOG_OUT_STDOUT:
                    fprintf(stdout, "[%s.%" PRIu64 "] %s %s (%s, %s, l%d)\n", buffer, ms, loglevel2str[logMessage.msg.contents.contents_log.error_type], logMessage.msg.contents.contents_log.error_msg, logMessage.msg.contents.contents_log.file, logMessage.msg.contents.contents_log.func, logMessage.msg.contents.contents_log.line_number);
                    break;

                case LOG_OUT_FILE:
                {
                    FILE *pFile = fopen (logMessage.msg.contents.contents_log.error_file_output,"a+");
                    if (pFile != NULL)
                    {
                        fprintf(pFile, "[%s.%" PRIu64 "] %s %s (%s, %s, l%d)\n", buffer, ms, loglevel2str[logMessage.msg.contents.contents_log.error_type], logMessage.msg.contents.contents_log.error_msg, logMessage.msg.contents.contents_log.file, logMessage.msg.contents.contents_log.func, logMessage.msg.contents.contents_log.line_number);
                        fclose (pFile);
                    }
			        break;
                }

                default:
                    break;
            }
        } else {
            perror("mq_receive");
        }
    }
}

void myLogManager::stop() {
    this->isRunning = false;
}